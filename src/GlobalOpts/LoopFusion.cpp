#include "llvm/Transforms/Utils/LoopFusion.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Instructions.h"
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/DependenceAnalysis.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>


// #define DEBUG

using namespace llvm;


/** Returns true if the loops are adjacent, i.e. the exit block of the first loop is the preheader 
 * of the second one (or the guard block if the loop is guarded). Otherwise, it returns false.
 * 
 * @param l1 loop 1
 * @param l2 loop 2
 * @return bool
*/
bool areAdjacent (Loop *l1, Loop *l2)
{
    // check for all the exit blocks of l1
    SmallVector<BasicBlock*, 4> exit_blocks;

    l1->getUniqueNonLatchExitBlocks(exit_blocks);

    for (BasicBlock *BB : exit_blocks)
    {
        if (l2->isGuarded() && BB != dyn_cast<BasicBlock>(l2->getLoopGuardBranch()))
        {
            outs() << "Second Loop is guarded, exit block of first loop is not equal to entry block of second loop";
            return false;
        }

        if (BB != l2->getLoopPreheader() || BB->size() > 1)
        {
            outs() << "exit block of first loop is not equal to entry block of second loop or there are instructions between the loops";
            return false;
        }
    }
    return true;
}


/** @brief Returns true if the loops have the same number of iterations.
 * Otherwise, it returns false. The number of iterations is computed based on the number of backedges taken.
 * 
 * @param l1 loop 1
 * @param l2 loop 2
 * @param SE scalar evolution
 * @return bool
*/
bool haveSameIterationsNumber (Loop *l1, Loop *l2, ScalarEvolution *SE)
{
    auto getTripCount = [SE] (Loop *l) -> const SCEV * {
        const SCEV *trip_count = SE->getBackedgeTakenCount(l);

        if (isa<SCEVCouldNotCompute>(trip_count))
        {
            outs() << "Trip count of loop " << l->getName() << " could not be computed.";
            return nullptr;
        }
        #ifdef DEBUG
            outs() << "Trip count: " << *trip_count << "\n";
        #endif
        return trip_count;
    };

    return getTripCount(l1) == getTripCount(l2);
}


/** @brief Get the entry block of a loop, if it is guarded it returns the guard block.
 * 
 * @param l loop
 * @return BasicBlock *
 */
BasicBlock *getEntryBlock (Loop *l)
{
    if (l->isGuarded())
        return l->getLoopGuardBranch()->getParent();
    return l->getLoopPreheader();
}


/** @brief Returns true if the loops are control flow equivalent.
 * I.e. when l1 executes, also l2 executes and when l2 executes also l1 executes.
 * Otherwise, it returns false.
 * 
 * @param l1 loop 1
 * @param l2 loop 2
 * @param DT domiator tree
 * @param PDT post dominator tree
 * @return bool
*/
bool areFlowEquivalent (Loop *l1, Loop *l2, DominatorTree *DT, PostDominatorTree *PDT)
{
    BasicBlock *B1 = getEntryBlock(l1);
    BasicBlock *B2 = getEntryBlock(l2);
    
    return (DT->dominates(B1, B2) && PDT->dominates(B2, B1));
}


/**
 * Check if the distance between the memory accesses of two instructions is negative
 * 
 * The function can only handle simple subscript in the form [c1 + a*i] and [c2 + a*i],
 * where i is an induction variable, c1 and c2 are loop invariant,
 * and a is a constant stride
 * @param inst1 first instruction to analyze
 * @param inst2 second instruction to analyze
 * @param loop1 Loop that contains first instruction
 * @param loop2 Loop that contains second instruction
 * @param SE the scalar evolution
*/
bool isDistanceNegative (Instruction *inst1, Instruction *inst2, Loop *loop1, Loop *loop2, ScalarEvolution &SE)
{   

    // This lambda returns a polynomial recurrence on the trip count, a pointer to an object of type SCEVAddRecExpr,
    // the reason is that this class offers more utilities than a regular SCEV
    auto getSCEVAddRec = [&SE](Instruction *instruction_to_analyze, 
        Loop *loop_of_the_instruction) -> const SCEVAddRecExpr* 
    {
        // get GEP instruction
        Value *instruction_arguments = getLoadStorePointerOperand(instruction_to_analyze);        
        const SCEV *SCEV_from_instruction = SE.getSCEVAtScope(instruction_arguments, loop_of_the_instruction);   

        #ifdef DEBUG
            outs() << "SCEV: " << *SCEV_from_instruction << " with type " << SCEV_from_instruction->getSCEVType() << "\n";
        #endif

        // only convert "compatible" types of SCEV
        if ((SCEV_from_instruction->getSCEVType() != SCEVTypes::scAddRecExpr
        && SCEV_from_instruction->getSCEVType() != SCEVTypes::scAddExpr))
          return nullptr;

        SmallPtrSet<const SCEVPredicate *, 4> preds;

        // create polinomial chain of recurrences
        const SCEVAddRecExpr *polynomial_recurrence = SE.convertSCEVToAddRecWithPredicates(
            SCEV_from_instruction, loop_of_the_instruction, preds);       
    
        #ifdef DEBUG
            if (polynomial_recurrence)
                outs() << "Polynomial recurrence " << *polynomial_recurrence << "\n";
        #endif
        
        return polynomial_recurrence;

    };

    const SCEVAddRecExpr *inst1_add_rec = getSCEVAddRec(inst1, loop1); 
    const SCEVAddRecExpr *inst2_add_rec = getSCEVAddRec(inst2, loop2);
    
    if (!(inst1_add_rec && inst2_add_rec)){
        outs() << "Can't find a polynomial recurrence for inst!\n";
        return true;
    }

    // Recover the base address of the two arrays, since they need to be the same
    if (SE.getPointerBase(inst1_add_rec) != SE.getPointerBase(inst2_add_rec)) {
        outs() << "can't analyze SCEV with different pointer base\n";
        // in this case there are no negative distance dependences between the instructions
        return false;
    }

    const SCEV* start_first_inst = inst1_add_rec->getStart();
    const SCEV* start_second_inst = inst2_add_rec->getStart();
    const SCEV* stride_first_inst = inst1_add_rec->getStepRecurrence(SE);
    const SCEV* stride_second_inst = inst2_add_rec->getStepRecurrence(SE);

    #ifdef DEBUG
        outs() << "First instruction start: " << *start_first_inst << "\n";
        outs() << "Second instruction start: " << *start_second_inst << "\n";
        outs() << "First instruction step recurrence: " << *stride_first_inst << "\n";
        outs() << "Second instruction step recurrence: " << *stride_second_inst << "\n";
    #endif

    // the two evolutions shall have the same non-null stride
    if (!SE.isKnownNonZero(stride_first_inst) || stride_first_inst != stride_second_inst){
        outs() << "Cannot compute distance\n";
        return true;
    }

    // delta represents the distance, in number of memory cells, between the starting addresses which are used to access memory
    // in instruction 1 and 2
    const SCEV *inst_delta = SE.getMinusSCEV(start_first_inst, start_second_inst);
    const SCEV *dependence_dist = nullptr;
    
    // check whether the distance can be computed
    const SCEVConstant *const_delta = dyn_cast<SCEVConstant>(inst_delta);
    const SCEVConstant *const_stride = dyn_cast<SCEVConstant>(stride_first_inst);
    if (const_delta && const_stride) 
    {
        // The dependence distance between the two instructions is computed from the delta and from the stride sign.
        //
        // Given two accesses to array A in two different positions i and i',
        // the formula to calculate the distance between the indexes is the following:
        // d = i' - i = (c1 - c2) / stride
        // since here there is only interest in the sign of the distance, the division by the stride has been skipped:
        // only the sign of the stride has been taken into consideration thanks to a flag.
      
        #ifdef DEBUG
            outs() << "SCEVs: stride = " << *stride_first_inst << ", delta = " << *inst_delta << "\n";
            outs() << "Consts: stride = " << *const_stride << ", delta = " << *const_delta << "\n";
        #endif

        APInt int_stride = const_stride->getAPInt();
        APInt int_delta = const_delta->getAPInt();
        unsigned n_bits = int_stride.getBitWidth();
        APInt int_zero = APInt(n_bits, 0);

        #ifdef DEBUG
            outs() << "Int stride: " << int_stride << ", int delta: " << int_delta << "\n";
        #endif

        // in case of stride 0, no distance can be calculted
        // constant access to an array position is considered as a dependency incompatible with loop fusion
        if (int_stride == 0)
            return true;
        // if the delta is not a multiplier of the stride, then there are no dependencies
        if ((int_delta != 0 && int_delta.abs().urem(int_stride.abs()) != 0))
            return false;

        // if stride < 0, reverse the delta to obtain the distance
        bool reverse_delta = false;
        if (int_stride.slt(int_zero))
            reverse_delta = true;

        dependence_dist = reverse_delta ? SE.getNegativeSCEV(inst_delta) : inst_delta;

        #ifdef DEBUG
            outs() << "Dependence distance: " << *dependence_dist << "\n";
        #endif
    }
    else
    {
        outs() << "Cannot compute distance\n";
        return true;
    }

    bool is_dist_LT0 = SE.isKnownPredicate(ICmpInst::ICMP_SLT, dependence_dist, SE.getZero(stride_first_inst->getType()));
    
    #ifdef DEBUG
        outs() << "Predicate 'dependence dist < 0': " << (is_dist_LT0 ? "True" : "False") << "\n";
    #endif

    return is_dist_LT0;
}


/**
 * Checks if two loops contain any negative distance dependencies
 * 
 * @param loop1 the first loop
 * @param loop2 the second loop
 * @param SE the scalar evolution
 * @param DI the dependency info
 * @param LI the loop info
 * @return true if there are negative distance dependencies, false otherwise
 */
bool areDistanceIndependent (Loop *loop1, Loop *loop2, ScalarEvolution &SE, DependenceInfo &DI, LoopInfo &LI)
{
    // get all the loads and stores
    std::vector<Instruction*> loads_first_loop, stores_first_loop, loads_second_loop, stores_second_loop;

    // lambda to collect loads and stores in vectors 
    auto collectLoadStores = [] (std::vector<Instruction*> *loads, std::vector<Instruction*> *stores, Loop *l) {
        for (auto BI = l->block_begin(); BI != l->block_end(); ++BI) {
            
            BasicBlock *BB = *BI;

            for (auto i = BB->begin(); i != BB->end(); i++) {
                Instruction *inst = dyn_cast<Instruction>(i);

                if (inst){
                    if (isa<StoreInst>(inst))
                        stores->push_back(inst);
                    if (isa<LoadInst>(inst))
                        loads->push_back(inst);
                }
                else
                    continue;

            }}
    };
    
    collectLoadStores(&loads_first_loop, &stores_first_loop, loop1);
    collectLoadStores(&loads_second_loop, &stores_second_loop, loop2);

    #ifdef DEBUG        
        outs() << "\n Loads first loop dump \n";
        for(auto i : loads_first_loop)   
            outs() << *i << "\n";

        outs() << "\n Loads second loop dump \n";
        for(auto i : loads_second_loop)   
            outs() << *i << "\n";
        
        outs() << "\n Stores first loop dump \n";    
        for(auto i : stores_first_loop)  
            outs() << *i << "\n";
        
        outs() << "\n Stores second loop dump \n";  
        for(auto i : stores_second_loop)  
            outs() << *i << "\n";
    #endif
    
    for (auto store: stores_first_loop)
    {        
        for (auto load: loads_second_loop)
        {
            auto instruction_dependence = DI.depends(store, load, true);

            #ifdef DEBUG
                outs() << "Checking " << *load << " " << *store << " dep? " << (instruction_dependence ? "True" : "False") << "\n";
            #endif

            if (!instruction_dependence)
                continue;

            // check that load and store inst are not part of a nested loop
            if(LI.getLoopFor(load->getParent()) != loop2 || LI.getLoopFor(store->getParent()) != loop1)
            {
                outs() << "One of the instructions is in a nested loop, can't perform fusion\n";
                return false;
            }

            // If isDistanceNegative, then there is a negative distance dependency, so return false
            if (isDistanceNegative(store, load, loop1, loop2, SE))
                return false;
        }
    }

    for (auto store: stores_second_loop)
    {        
        for (auto load: loads_first_loop)
        {
            auto instruction_dependence = DI.depends(store, load, true);

            #ifdef DEBUG
                outs() << "Checking " << *load << " " << *store << " dep? " << (instruction_dependence ? "True" : "False") << "\n";
            #endif

            if (!instruction_dependence) 
                continue;

            // check that load and store inst are not part of a nested loop
            if(LI.getLoopFor(load->getParent()) != loop1 || LI.getLoopFor(store->getParent()) != loop2)
            {
                outs() << "One of the instructions is in a nested loop, can't perform fusion\n";
                return false;
            }

            // If isDistanceNegative, then there is a negative distance dependency, so return false
            if (isDistanceNegative(load, store, loop1, loop2, SE))
                return false;
        }
    }

    return true;
}


/** @brief Fuses the given loops.
 * The body of the second loop, after beeing unlinked, is connected after the body of the first loop.
 * 
 * @param l1 loop 1
 * @param l2 loop 2
*/
void fuseLoop (Loop *l1, Loop *l2)
{
    BasicBlock *l2_entry_block = l2->isGuarded() ? l2->getLoopGuardBranch()->getParent() : l2->getLoopPreheader(); 

    SmallVector<BasicBlock *> exits_blocks;
    
    /*
    Replace the uses of the induction variable of the second loop with 
    the induction variable of the first loop.
    */
    PHINode *index1 = l1->getCanonicalInductionVariable();
    PHINode *index2 = l2->getCanonicalInductionVariable();
    if (!index1 || !index2)
    {
        outs() << "Induction variables are not canonical\n";
        return;
    }
    index2->replaceAllUsesWith(index1);

    /*
    Data structure to get reference to the basic blocks that will undergo relocation.
    */
    struct LoopStructure
    {
        BasicBlock *header, *latch, *body_head, *body_tail;

        LoopStructure (Loop *l)
        {
            this->header = l->getHeader();
            this->latch = l->getLoopLatch();
            this->body_head = getBodyHead(l, header);
            this->body_tail = latch->getUniquePredecessor();
        }

        BasicBlock *getBodyHead (Loop *l, BasicBlock *header)
        {
            for (auto sit = succ_begin(header); sit != succ_end(header); sit++)
            {
                BasicBlock *successor = dyn_cast<BasicBlock>(*sit);
                if (l->contains(successor))
                    return successor;
            }
            return nullptr;
        }
    };
    
    LoopStructure *first_loop = new LoopStructure(l1);
    LoopStructure *second_loop = new LoopStructure(l2);

    l2->getExitBlocks(exits_blocks);
    for (BasicBlock *BB : exits_blocks)
    {
        for (pred_iterator pit = pred_begin(BB); pit != pred_end(BB); pit++)
        {
            BasicBlock *predecessor = dyn_cast<BasicBlock>(*pit);
            if (predecessor == l2->getHeader())
            {
                l1->getHeader()->getTerminator()->replaceUsesOfWith(l2_entry_block, BB);
            }
        }
    }

    BranchInst *new_branch = BranchInst::Create(second_loop->latch);
    ReplaceInstWithInst(second_loop->header->getTerminator(), new_branch);

    first_loop->body_tail->getTerminator()->replaceUsesOfWith(first_loop->latch, second_loop->body_head);
    second_loop->body_tail->getTerminator()->replaceUsesOfWith(second_loop->latch, first_loop->latch);

    delete first_loop; delete second_loop;

    outs() << "Fusion done\n";
    return;
}


PreservedAnalyses LoopFusion::run (Function &F,FunctionAnalysisManager &AM)
{   
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    SmallVector<Loop *, 4> loops_forest = LI.getLoopsInPreorder();

    if (loops_forest.size() <= 1)
        return PreservedAnalyses::all();

    std::unordered_map<unsigned, Loop*> last_loop_at_level = {{loops_forest[0]->getLoopDepth(), loops_forest[0]}};

    bool fusion_happened = false;

    for (size_t i = 1; i < loops_forest.size(); i++)
    {
        unsigned loop_depth = loops_forest[i]->getLoopDepth();
        Loop *l1 = last_loop_at_level[loop_depth];
        Loop *l2 = loops_forest[i];

        // check whether l1 exists (i.e. there is a loop at the current loop level that has been visited before) and check for the same parent
        if (l1 && l1->getParentLoop() == l2->getParentLoop())
        {
            /*
            Expoliting the logical short-circuit, as soon as one of the functions returns false, 
            the others remaining checks are not executed and the if statement condition becomes false.
            */ 
            if (areAdjacent(l1, l2) && 
                haveSameIterationsNumber(l1, l2, &SE) && 
                areFlowEquivalent(l1, l2, &DT, &PDT) && 
                areDistanceIndependent(l1, l2, SE, DI, LI))
            {
                outs() << "Starting fusion ...\n";
                fuseLoop(l1, l2);
                fusion_happened = true;
                break;
            }
        }
        last_loop_at_level[loop_depth] = loops_forest[i];
    }

    return fusion_happened ? PreservedAnalyses::none() : PreservedAnalyses::all();
}