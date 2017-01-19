//===- MipsTargetMachine.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MIPS_TARGET_MACHINE_H
#define MIPS_TARGET_MACHINE_H

#include <mcld/CodeGen/TargetMachine.h>

namespace mcld {

class MipsBaseTargetMachine : public MCLDTargetMachine
{
public:
  MipsBaseTargetMachine(llvm::TargetMachine &pTM,
                        const llvm::Target &pLLVMTarget,
                        const mcld::Target &pMCLDTarget,
                        const std::string &pTriple);
};

} // namespace of mcld

#endif
