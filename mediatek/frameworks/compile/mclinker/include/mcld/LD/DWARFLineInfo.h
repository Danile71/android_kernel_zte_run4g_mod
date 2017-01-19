//===- DWARFLineInfo.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_DWARFLINEINFO_H
#define MCLD_LD_DWARFLINEINFO_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif
#include <mcld/LD/DiagnosticLineInfo.h>

namespace mcld
{

/** \class DWARFLineInfo
 *  \brief DWARFLineInfo provides the conversion from address to line of code
 *  by DWARF format.
 */
class DWARFLineInfo : public DiagnosticLineInfo
{

};

} // namespace of mcld

#endif

