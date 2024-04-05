//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define DEBUG

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "unordered_set"

using namespace llvm;

/**
* Map associating binary operations with their opposite operation
* Signed operations are excluded
*/
const std::unordered_map<Instruction::BinaryOps, Instruction::BinaryOps> oppositeOp =
{
  {Instruction::Add, Instruction::Sub},
  {Instruction::Sub, Instruction::Add},
  {Instruction::Mul, Instruction::UDiv},
  {Instruction::UDiv, Instruction::Mul},
  {Instruction::Shl, Instruction::LShr},
  {Instruction::LShr, Instruction::Shl}
};

/**
 * Get a representation of a single variable binary operation in terms of a couple generic value - integer constant
 * (e.g. X + 1).
 * For commutative operations, the positions of the operands in the original instruction are swapped, instead,
 * in case of subtractions and divisions, a valid representaton is returned only in case the value is at the first place.
 * In case of subtractions and divisions where the only constant is the first operand, the second attribute 
 * in the returned pair will be nullptr.
 * 
 * The value represents the generic content of a SSA registry and is a Value*. The Value type is necessary in order to include
 * also the Argument objects (representig function's arguments), and Constant objects (in case there are two of them).
 * The function assumes that inst is a binary operation.
 * 
 * @param inst the binary instruction
 * @return a pair of a Value object and a constant; if a constant is not present or it is present in the wrong position
 * in case of subtraction and division, it returns a nullptr in its place, and the value returned is the first operand
 * 
*/
std::pair<Value*, ConstantInt*>* getValAndConst (Instruction &inst)
{
  unsigned int opcode = inst.getOpcode();
  Value *val1 = inst.getOperand(0);
  Value *val2 = inst.getOperand(1);
  ConstantInt *CI = dyn_cast<ConstantInt>(val1);
  if ((opcode == BinaryOperator::Add || opcode == BinaryOperator::Mul) && CI)
  {
    return new std::pair<Value*, ConstantInt*> (val2, CI);
  }
  else
  {
    /**
    * if val2 is not castable to ConstantInt the second entity of the pair is nullptr
    * in case val1 and val2 are both constants and the operation is not Add or Mul, the value is actually the first constant
    * and the constant is the second one
    */
    return new std::pair<Value*, ConstantInt*> (val1, dyn_cast<ConstantInt>(val2));
  }
  return nullptr;
}

/**
 * Get the number of integer constants in the binary instruction.
 * 
 * @param inst the binary instruction
 * @return the number of integer constants
*/
size_t getNConstants (Instruction &inst)
{
  ConstantInt *C1 = dyn_cast<ConstantInt>(inst.getOperand(0));
  ConstantInt *C2 = dyn_cast<ConstantInt>(inst.getOperand(1));
  size_t counter = 0;
  if (C1)
    counter++;
  if (C2)
    counter++;
  return counter;
}

/** @brief Apply constant folding optimization on a binary instruction and susbtitute the instruction uses, if possible.
 * 
 * Shifts are not folder.
 * 
 * @param inst the binary instruction
 * @return true if optimized, false otherwise
*/
bool ConstantFolding (Instruction &inst)
{
  #ifdef  DEBUG
  llvm::outs() << "Entered in function ConstantFolding\n";
  #endif

  ConstantInt *C1 = dyn_cast<ConstantInt>(inst.getOperand(0));
  ConstantInt *C2 = dyn_cast<ConstantInt>(inst.getOperand(1));

  if (!C1 || !C2)
    return false;

  #ifdef  DEBUG
  llvm::outs() << "C1 :" << C1 << ",\tC2 :" <<C2 << "\n";
  #endif

  APInt fact1 = C1->getValue();
  APInt fact2 = C2->getValue();

  switch (inst.getOpcode())
  {
    case BinaryOperator::Add:
      if (C1->isZero() || C2->isZero())
        return false;
      fact1 += fact2;
      break;
    
    case BinaryOperator::Sub:
      fact1 -= fact2;
      break;

    case BinaryOperator::Mul:
      fact1 *= fact2;
      break;

    case BinaryOperator::SDiv:
      fact1 = fact1.udiv(fact2);
      break;

    case BinaryOperator::UDiv:
      fact1 = fact1.udiv(fact2);
      break;

    default:
      return false;
  }

  ConstantInt *result = ConstantInt::get(C1->getType(), fact1.getZExtValue());
  ConstantInt *zero = ConstantInt::get(C1->getType(), 0);
  // a dummy add instruction with second operand 0 is added, which will be optmized in subsequent steps
  Instruction *addi = BinaryOperator::Create(Instruction::Add, result, zero);

  #ifdef  DEBUG
  llvm::outs() << "Folding done. Result = " << result << "\nSubstituing operation\n";
  #endif

  addi->insertAfter(&inst);
  inst.replaceAllUsesWith(addi);

  return true;
}

/** @brief Apply constant algebraic identity optmization on a binary instruction and susbtitute the instruction uses,
 * if possible.
 * 
 * 
 * @param inst the binary instruction
 * @param VC the value and constant representation of the same binary instruction
 * The function assumes at least a constant is present and in the right position, hence the pair VC,
 * which is derived from a getValAndConst call, always has the second element non-null
 * @return true if optimized, false otherwise
*/
bool AlgebraicIdentity (Instruction &inst, std::pair<Value*, ConstantInt*> *VC)
{
  #ifdef  DEBUG
  llvm::outs() << "Entered in function AlgebraicIdentity\n";
  #endif

  // VC derives from getVarAndConst, which can return a ConstantInt in first position

  bool ToReplace = false;

  // Switch case has been adopted for modularity reasons
  switch (inst.getOpcode())
  {
    case BinaryOperator::Add:
    case BinaryOperator::Sub:
    case BinaryOperator::Shl:
    case BinaryOperator::LShr:
      if (VC->second->getValue().isZero())
        ToReplace = true;
      break;
    case BinaryOperator::Mul:
    case BinaryOperator::UDiv:
    case BinaryOperator::SDiv:
      if (VC->second->getValue().isOne())
        ToReplace = true;
      break;
  }

  // The Value type is necessary in order to include also the Argument objects (representig function's arguments).
  if(ToReplace)
    inst.replaceAllUsesWith(VC->first);
  return ToReplace;
}

/** @brief Apply strength reduction optmization on a binary instruction and susbtitute the instruction uses,
 * if possible.
 * In case of multiplication and in case of divisions where the constant is a power of two,
 * a shift is inserted, followed by eventual needed multiplications (to be optimized in the following stages) and subtractions.
 * 
 * @param inst the binary instruction
 * @param VC the value and constant representation of the same binary instruction
 * @return true if optimized, false otherwise
*/
bool StrengthReduction (Instruction &inst, std::pair<Value*, ConstantInt*> *VC)
{
  #ifdef DEBUG
  llvm::outs() << "Entered in function StrengthReduction\n";
  #endif

  // Negative constants are not handled
  unsigned int shiftVal = VC->second->getValue().ceilLogBase2();
  ConstantInt *shift = ConstantInt::get(VC->second->getType(), shiftVal);

  // Last instruction to be inserted
  Instruction* lastinst = nullptr;
  switch (inst.getOpcode())
  {
    case BinaryOperator::Mul:
    {
      Instruction *shli = BinaryOperator::Create(Instruction::Shl, VC->first, shift);
      shli->insertAfter(&inst);
      unsigned int restVal = (1 << shiftVal) - VC->second->getValue().getZExtValue();
      ConstantInt *rest = ConstantInt::get(VC->second->getType(), restVal);

      if (restVal == 0)
      {
        lastinst = shli;
      }
      else if (restVal == 1)
      {
        lastinst = BinaryOperator::Create(BinaryOperator::Sub, shli, VC->first);
        lastinst->insertAfter(shli);
      }
      // if rest is > 1 an intermediate multiplication is needed
      else if (restVal > 1)
      {
        Instruction *muli = BinaryOperator::Create(BinaryOperator::Mul, VC->first, rest);
        muli->insertAfter(shli);
        lastinst = BinaryOperator::Create(BinaryOperator::Sub, shli, muli);
        lastinst->insertAfter(muli);
      }
      break;
    }

    case BinaryOperator::UDiv:
    {
      if (VC->second->getValue().isPowerOf2())
      {
        lastinst = BinaryOperator::Create(Instruction::LShr, VC->first, shift);
        lastinst->insertAfter(&inst);
      }
      break;
    }
  }

  #ifdef DEBUG
  llvm::outs() << "LastInst: " << lastinst << "\n";
  #endif

  if (lastinst)
    inst.replaceAllUsesWith(lastinst);
  // if lastinst is nullptr (e.g. strength reduction has not been adopted), it returns false, otherwise true
  return lastinst;
}

/** @brief Apply multi-instruction optimization starting from a base operation if possible.
 * The base operation must be in the form "<val/const> <op> <const/val>", the algorithm checks if the relative
 * instruction uses are in the form <same_const/base_val> <opposite_op> <base_val/same_const>,
 * in this case the instruction that uses the base operations has its uses substituted with the value used by the
 * base operation.
 * E.g. y = x + 2; z = y -2 => y = x + 2; z = x
 * where z = x is obtained not introducing an "assignment" in the IR, but substituting the uses of z with x
 * 
 * @param inst the binary instruction
 * @param VC the value and constant representation of the same binary instruction
 * @return Reference to the operand of the examined operation which is not constant.
*/
bool MultiInstructionOpt (Instruction &inst, std::pair<Value*, ConstantInt*> *VC)
{
  #ifdef  DEBUG
  llvm::outs() << "Entered in function MultiInstructionOpt\n";
  #endif

  for (auto &use : inst.uses())
  {
    Instruction *User = dyn_cast<Instruction>(use.getUser());
    if (!User || !User->isBinaryOp())
      continue;

    std::pair<Value*, ConstantInt*> *VCUser = getValAndConst(*User);

    #ifdef DEBUG
    outs() << "I'm inside getMultiInstruction, just before the if\n";
    #endif

    if (!VCUser->second 
      || (VCUser->second->getValue() != VC->second->getValue()) 
      || (oppositeOp.at(static_cast<BinaryOperator::BinaryOps>(inst.getOpcode())) != User->getOpcode()))
      continue;
    
    #ifdef DEBUG
    outs() << "I'm inside getMultiInstruction, just before the return\n";
    #endif

    User->replaceAllUsesWith(VC->first);
    return true;
  }

  return false;
}

bool runOnBasicBlock(BasicBlock &B) 
{
  // map that tracks "dead code", namely instructions that have no more uses
  std::unordered_set<Instruction*> DeadCode;
  // flag tracking if in one single cycle on all the instructions of the block at least one optmization is done
  bool Transformed = false;
  // flag tracking is at least one optmization has been done in the whole exectution of the current function
  bool TransformedGlobal = false;

  do
  {
    Transformed = false;
    // cycle all the instruction of the basic block
    for (auto &inst : B) 
    {
      // check if the current operation is binary
      if (!inst.isBinaryOp())
        continue;

      #ifdef  DEBUG
      outs() << "Instruction: " << inst << "\n";
      #endif
      
      // if the binary instruction has no constants no optmization is possible
      size_t nConstants = getNConstants(inst);
      if (nConstants == 0)
        continue;

      // get a Value - Constant representation of the operation
      std::pair<Value*, ConstantInt*> *VC = getValAndConst(inst);
      /* the following check is needed to skip ensure there is a constant
      * in the "right" position.
      * e.g. %10 = 3 - %5
      */
      if (!VC->second)
        continue;

      /* check if the instruction is used and try all the optimizations in the following order:
      *  - algebraic identity
      *  - constant folding (only when constants are 2)
      *  - multi instruction
      *  - strength reduction
      *  Algebraic identity must be tried before constant folding to avoid folding instructions with identities, which
      *  are useless; strength reduction must be tried last, since optmizing multiplications and divisions with multi
      *  instruction allows to eliminate useless multiplication/divisions, which would be otherwise optmized with shifts
      */
      bool TransformedLocal = (!inst.getNumUses())
        || AlgebraicIdentity(inst, VC)
        || (nConstants == 2 && ConstantFolding(inst))
        || MultiInstructionOpt(inst, VC)
        || StrengthReduction(inst, VC);

      // if, after optmizations, an instruction has no uses, it's dead code
      if (!inst.getNumUses())
      {
        #ifdef  DEBUG
        outs() << "Instruction: " << inst << " is in DeadCode\n";
        #endif
        DeadCode.insert(&inst);
      }

      Transformed = Transformed || TransformedLocal;
    }

    // dead code elimination
    if (DeadCode.size() > 0)
    {
      for (auto &inst : DeadCode)
        inst->eraseFromParent();
      DeadCode.clear();
    }

    TransformedGlobal = TransformedGlobal || Transformed;
  }
  while (Transformed);

  return TransformedGlobal;
}

bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}

PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();

  return PreservedAnalyses::all();
}