//===- X86TargetMachine.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86TargetMachine.h"

#include "X86.h"
#include <mcld/Support/TargetRegistry.h>

extern "C" void MCLDInitializeX86LDTarget() {
  // Register createTargetMachine function pointer to mcld::Target
  mcld::RegisterTargetMachine<mcld::X86TargetMachine> X(mcld::TheX86_32Target);
  mcld::RegisterTargetMachine<mcld::X86TargetMachine> Y(mcld::TheX86_64Target);
}

using namespace mcld;

//===----------------------------------------------------------------------===//
// X86TargetMachine
//===----------------------------------------------------------------------===//
X86TargetMachine::X86TargetMachine(llvm::TargetMachine& pPM,
                                   const llvm::Target& pLLVMTarget,
                                   const mcld::Target& pMCLDTarget,
                                   const std::string& pTriple)
  : MCLDTargetMachine(pPM, pLLVMTarget, pMCLDTarget, pTriple) {
}

