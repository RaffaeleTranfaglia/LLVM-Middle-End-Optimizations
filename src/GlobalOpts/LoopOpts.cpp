#include "llvm/Transforms/Utils/LoopOpts.h"
#include "llvm/IR/Dominators.h"

#define DEBUG

using namespace llvm;

const std::string invariant_tag = "invariant";
const std::string use_dominator = "use_dominator";
const std::string exits_dominator = "exits_dominator";
const std::string dead_tag = "dead";

/** @brief Remove all the previously applied metadatas.
 * 
 * @param inst instruction
*/
void clearMetadata (Instruction *inst) 
{
    SmallVector<std::string> tags = {invariant_tag, use_dominator, exits_dominator, dead_tag};
    for (auto type: tags)
        inst->setMetadata(type, NULL);
}

/** @brief Apply a specified metadata to an instruction.
 * 
 * @param inst instruction
 * @param tag tag to apply
*/
void applyMetadata (Instruction *inst, const std::string tag)
{
    LLVMContext &C = inst->getContext();
    MDNode *N = MDNode::get(C, MDString::get(C, ""));
    inst->setMetadata(tag, N);
}

/** @brief Check if instruction is already marked as Invariant.
 * 
 * @param inst checked instruction
*/
bool isAlreadyLoopInvariant (Instruction *inst)
{
    return inst->getMetadata(invariant_tag);
}

/** @brief Check if the Value is LoopInvariant.
 * A value is defined loop invariant if verifies at least one of these conditions:
 *  - It is a parameter of the function
 *  - The relative Instruction is already marked as LoopInvariant
 *  - Loop doesn not contain the relative instruction
 * Instructions which have phi instructions as operands are not invariant because a phi instruction 
 * value change basing on the execution flow by definition.
 * 
 * @param v value
 * @param L loop
*/ 
bool isLoopInvariant (Value *v, Loop* L)
{
    //NULL when v is an argument of the function
    Instruction *v_inst = dyn_cast<Instruction>(v); 
    
    if (!v_inst || isAlreadyLoopInvariant(v_inst) || !L->contains(v_inst))
        return true;
    
    #ifdef DEBUG
        outs() << "[isLoopInvariant]\tAnalyzing Value: " << *v_inst << "\n";
    #endif

    Constant *c_inst = dyn_cast<Constant>(v);

    if (c_inst){
        
        #ifdef DEBUG
            outs() << "[isLoopInvariant]\t\tValue resolved to constant: " << *c_inst <<"\n";
        #endif
        
        return true;
    }
    
    return false;
}

/** @brief Mark with a metadata an Instruction if it is LoopInvariant. 
 * 
 * @param inst instruction
 * @param L loop
*/
void markIfLoopInvariant (Instruction *inst, Loop* L)
{
    Value *val1 = inst->getOperand(0);
    Value *val2 = inst->getOperand(1);

    #ifdef DEBUG
        outs() << "[markIfLoopInvariant]\t\tAnalyzing operands: " << *val1 << ", " << *val2 << "\n";
    #endif
    
    if (!isLoopInvariant(val1, L) || !isLoopInvariant(val2, L))
        return;

    applyMetadata(inst, invariant_tag);

    #ifdef DEBUG
        outs() << "[markIfLoopInvariant]\tLoop invariant instruction detected: " << *inst << "\n";
    #endif

    return;
}

/** @brief Mark with a metadata all the blocks in the loop which dominate the exits. 
 * 
 * @param L Loop
 * @param DT dominator tree
*/
void markExitsDominatorBlocks (Loop &L, DominatorTree *DT)
{
    SmallVector<BasicBlock*> exiting_blocks;

    for (auto BI = L.block_begin(); BI != L.block_end(); ++BI)
    {
        BasicBlock *BB = *BI;
        if (L.isLoopExiting(BB))
            exiting_blocks.push_back(BB);
    }

    for (auto BI = L.block_begin(); BI != L.block_end(); ++BI)
    {
        BasicBlock *BB = *BI;
        #ifdef DEBUG
            outs() << "[markExitsDominatorBlocks]\tAnalyzing block: " << *BI << "\n";
        #endif


        bool is_dominator = true;
        for (auto EB: exiting_blocks)
        {
            if (!DT->dominates(BB, EB))
            {
                is_dominator = false;
                break;
            }
        }

        if (is_dominator){
            #ifdef DEBUG
                outs() << "[markExitsDominatorBlocks]\t\tThis Block is dominator" << "\n";
            #endif

            applyMetadata(BB->getTerminator(), exits_dominator);
        }
    }
    return;
}

/** @brief Recursively get Uses for a given Instruction.
 * 
 * @param inst instruction
*/
std::vector<Use*> getUses (Instruction *inst)
{
    std::vector<Use*> uses_to_check;
    for (Value::use_iterator iter = inst->use_begin(); iter != inst->use_end(); ++iter)
    {
        Use *use_of_inst = &(*iter);
        Instruction *user_inst = dyn_cast<Instruction>(iter->getUser());
        
        #ifdef DEBUG
            outs() << "[getUses]\tFound User: " << *(user_inst) << " of " << *inst << "\n";
        #endif
        
        /*
            Given that a PHINode instruction stores different expressions connected to a variable; 
            in order to obtain the uses of the original Instruction it is needed to obtain the uses of the PHI 
            Instruction (this operation can be repeated multiple times).
        */  
        if (isa<PHINode>(user_inst))
        {
            std::vector<Use*> res = getUses(user_inst);
            uses_to_check.insert(uses_to_check.end(), res.begin(), res.end());
        }
        else
            uses_to_check.push_back(use_of_inst);
    }
    outs()<<"\n";
    return uses_to_check;
}

/** @brief Mark the give instruction with a metadata if it dominates their uses.
 * 
 * @param inst instruction
 * @param DT dominator tree
 * @param L loop
*/
void markIfUseDominator (Instruction *inst, DominatorTree *DT, Loop *L)
{
    std::vector<Use*> uses = getUses(inst);
    Value *inst_val = dyn_cast<Value>(inst);

    for (Use *use : uses)
    {
        #ifdef DEBUG
        outs() << "[markIfUseDominator]\t"<< *inst_val << " is "<< ((DT->dominates(inst_val, *use)) ? "" : "not") << " a dominator of " << *(use->getUser()) <<"\n";
        #endif

        if (L->contains(dyn_cast<Instruction>(use->getUser())) && !DT->dominates(inst_val, *use))
            return;
    }
    
    applyMetadata(inst, use_dominator);

    #ifdef DEBUG
        outs() << "[markIfUseDominator]\tInstruction "<<*inst<<" marked as use dominator\n";
    #endif

    return;
}

/** @brief Mark the give instruction as dead if it is considered such outside the loop.
 * An instruction is dead in a fixed point p if it is never used from that point onwards.
 * 
 * @param inst instruction
 * @param L loop
*/
void markIfDeadInstruction (Instruction *inst, Loop *L)
{
    std::vector<Use*> uses = getUses(inst);
    bool isDead = true;
    for (Use *use : uses)
    {
        User *user = use->getUser();
        Instruction *user_inst = dyn_cast<Instruction>(user);
        if (!L->contains(user_inst))
        {
            isDead = false;
            break;
        }
    }

    if (isDead)
        applyMetadata(inst, dead_tag);
    return;
}

/** @brief Move the marked instructions outside the loop.
 * Execute a DFS on the dominator tree in order to move in the preheader the instructions which are marked
 * as invariant, use dominators, and exits dominators or deads. 
 * The dfs is exploited to maintain the relative order of the moved instructions.
 * 
 * @param node_DT dominator tree node
 * @param preheader preheader of the loop
*/
bool codeMotion (DomTreeNode *node_DT, BasicBlock *preheader)
{
    bool code_changed = false;
    SmallVector<Instruction*> to_be_moved;
    if (!node_DT)
        return false;
    BasicBlock *node = node_DT->getBlock();

    #ifdef DEBUG
        outs() << "[codeMotion]\tBB : " << *node << "\n";
    #endif
    
    for (BasicBlock::iterator inst = node->begin(); inst != node->end(); inst++)
    {
        #ifdef DEBUG
            outs() << "[codeMotion]\t" << *inst << "\n";
        #endif
        // if at least one of the three main conditions is false, then the instruction must not be moved in preheader block
        bool not_move = ((!inst->getMetadata(dead_tag) && !node->getTerminator()->getMetadata(exits_dominator)) 
            || !inst->getMetadata(use_dominator) || !inst->getMetadata(invariant_tag));
        clearMetadata(&(*inst));
        #ifdef DEBUG
            if (not_move)
                outs() << "[codeMotion]\t\tThe instruction is moved\n";
        #endif
        if (not_move)
            continue;
        to_be_moved.push_back(&(*inst));
    }

    Instruction *last_preheader_inst = &(*(preheader->getTerminator()));

    code_changed = (to_be_moved.size() >= 1);

    for (auto inst: to_be_moved)
    {
        // move inst in preheader
        #ifdef DEBUG
            outs() << "[codeMotion]\t" << "To be deleted inst " << *inst << "\n";
            outs() << "[codeMotion]\t" << "Trying to insert before inst " << *last_preheader_inst << "\n";
        #endif
        inst->removeFromParent();
        inst->insertBefore(last_preheader_inst);
        #ifdef DEBUG
            outs() << "[codeMotion]\t" << "Newly inserted inst " << *inst << "\n";
        #endif
    }

    for (DomTreeNode *child : node_DT->children())
    {
        code_changed = codeMotion(child, preheader) || code_changed;
    }
    return code_changed;
}


PreservedAnalyses LoopOpts::run (Loop &L, LoopAnalysisManager &LAM, 
                                    LoopStandardAnalysisResults &LAR, LPMUpdater &LU)
{
    DominatorTree *DT = &LAR.DT;
    
    #ifdef DEBUG
        outs() << "[run]\tPre-header: " << *(L.getLoopPreheader()) << "\n";
        outs() << "[run]\tHeader: " << *(L.getHeader()) << "\n";
    #endif
    for (auto BI = L.block_begin(); BI != L.block_end(); ++BI)
    {
        BasicBlock *BB = *BI;
        #ifdef DEBUG
            outs() << "[run]\tBasic block: " << *BB << "\n";
        #endif
        for (auto i = BB->begin(); i != BB->end(); i++)
        {
            Instruction *inst = dyn_cast<Instruction>(i);
            if (!inst->isBinaryOp())
                continue;
            #ifdef DEBUG
                outs() << "[run]\tInstruction: " << *inst << "\n";
            #endif
            
            markIfLoopInvariant(inst, &L);
            markIfUseDominator(inst, DT, &L);
            markIfDeadInstruction(inst, &L);
        }
    }

    markExitsDominatorBlocks(L, DT);

    if (codeMotion(DT->getRootNode(), L.getLoopPreheader()))
        return PreservedAnalyses::none();

    #ifdef DEBUG
        outs()<<"[run]\tNothing changed!"<<"\n";
    #endif    
    return PreservedAnalyses::all();
}