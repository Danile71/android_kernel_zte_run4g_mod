//===- HexagonTargetMachine.h ---------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_HEXAGON_TARGET_MACHINE_H
#define MCLD_HEXAGON_TARGET_MACHINE_H
#include "Hexagon.h"
#include <mcld/CodeGen/TargetMachine.h>

namespace mcld {

class HexagonTargetMachine : public MCLDTargetMachine
{
public:
  HexagonTargetMachine(llvm::TargetMachine &pTM,
                       const llvm::Target &pLLVMTarget,
                       const mcld::Target &pMCLDTarget,
                       const std::string &pTriple);
};

} // namespace of mcld

#endif
