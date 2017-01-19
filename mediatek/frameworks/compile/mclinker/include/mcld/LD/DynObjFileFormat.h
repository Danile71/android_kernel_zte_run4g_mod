//===- DynObjFileFormat.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_DYNOBJFILEFORMAT_H
#define MCLD_LD_DYNOBJFILEFORMAT_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

namespace mcld
{

/** \class DynObjFormat
 *  \brief DynObjFormat describes the file format for dynamic objects.
 */
class DynObjFormat : public LDFileFormat
{

};

} // namespace of mcld

#endif

