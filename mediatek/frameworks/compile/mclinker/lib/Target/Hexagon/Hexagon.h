//===- Hexagon.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_HEXAGON_H
#define MCLD_TARGET_HEXAGON_H
#include <string>

namespace llvm {
class Target;
} // namespace of llvm

namespace mcld {

class Target;
class TargetLDBackend;

extern mcld::Target TheHexagonTarget;

TargetLDBackend*
createHexagonLDBackend(const llvm::Target&, const std::string&);

} // namespace of mcld

#endif

