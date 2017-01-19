//===- X86Relocator.cpp -------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86Relocator.h"
#include "X86RelocationFunctions.h"

#include <mcld/LinkerConfig.h>
#include <mcld/IRBuilder.h>
#include <mcld/Support/MsgHandling.h>
#include <mcld/LD/LDSymbol.h>
#include <mcld/LD/ELFFileFormat.h>
#include <mcld/LD/ELFSegmentFactory.h>
#include <mcld/LD/ELFSegment.h>
#include <mcld/Object/ObjectBuilder.h>

#include <llvm/ADT/Twine.h>
#include <llvm/Support/DataTypes.h>
#include <llvm/Support/ELF.h>

using namespace mcld;

//===--------------------------------------------------------------------===//
// X86_32 Relocation helper function
//===--------------------------------------------------------------------===//
/// helper_DynRel - Get an relocation entry in .rel.dyn
static
Relocation& helper_DynRel_init(ResolveInfo* pSym,
                               Fragment& pFrag,
                               uint64_t pOffset,
                               X86Relocator::Type pType,
                               X86_32Relocator& pParent)
{
  X86_32GNULDBackend& ld_backend = pParent.getTarget();
  Relocation& rel_entry = *ld_backend.getRelDyn().create();
  rel_entry.setType(pType);
  rel_entry.targetRef().assign(pFrag, pOffset);
  if (pType == llvm::ELF::R_386_RELATIVE || NULL == pSym)
    rel_entry.setSymInfo(NULL);
  else
    rel_entry.setSymInfo(pSym);

  return rel_entry;
}

/// helper_use_relative_reloc - Check if symbol ceuse relocation
/// R_386_RELATIVE
static bool
helper_use_relative_reloc(const ResolveInfo& pSym,
                          const X86_32Relocator& pFactory)

{
  // if symbol is dynamic or undefine or preemptible
  if (pSym.isDyn() ||
      pSym.isUndef() ||
      pFactory.getTarget().isSymbolPreemptible(pSym))
    return false;
  return true;
}

static
X86_32GOTEntry& helper_GOT_init(Relocation& pReloc,
                                bool pHasRel,
					                      X86_32Relocator& pParent)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  X86_32GNULDBackend& ld_backend = pParent.getTarget();
  assert(NULL == pParent.getSymGOTMap().lookUp(*rsym));

  X86_32GOTEntry* got_entry = ld_backend.getGOT().create();
  pParent.getSymGOTMap().record(*rsym, *got_entry);

  if (!pHasRel) {
    // No corresponding dynamic relocation, initialize to the symbol value.
    got_entry->setValue(X86Relocator::SymVal);
  }
  else {
    // Initialize got_entry content and the corresponding dynamic relocation.
    if (helper_use_relative_reloc(*rsym, pParent)) {
      helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_386_RELATIVE,
                                                                       pParent);
      got_entry->setValue(X86Relocator::SymVal);
    }
    else {
      helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_386_GLOB_DAT,
                                                                       pParent);
      got_entry->setValue(0x0);
    }
  }
  return *got_entry;
}

static
X86Relocator::Address helper_GOT_ORG(X86_32Relocator& pParent)
{
  return pParent.getTarget().getGOTPLT().addr();
}

static
X86Relocator::Address helper_get_GOT_address(Relocation& pReloc,
                                             X86_32Relocator& pParent)
{
  X86_32GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  assert(NULL != got_entry);
  return pParent.getTarget().getGOT().addr() + got_entry->getOffset();
}

static
PLTEntryBase& helper_PLT_init(Relocation& pReloc, X86_32Relocator& pParent)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  X86_32GNULDBackend& ld_backend = pParent.getTarget();
  assert(NULL == pParent.getSymPLTMap().lookUp(*rsym));

  PLTEntryBase* plt_entry = ld_backend.getPLT().create();
  pParent.getSymPLTMap().record(*rsym, *plt_entry);

  // initialize plt and the corresponding gotplt and dyn rel entry.
  assert(NULL == pParent.getSymGOTPLTMap().lookUp(*rsym) &&
         "PLT entry not exist, but GOTPLT entry exist!");
  X86_32GOTEntry* gotplt_entry = ld_backend.getGOTPLT().create();
  pParent.getSymGOTPLTMap().record(*rsym, *gotplt_entry);

  // init the corresponding rel entry in .rel.plt
  Relocation& rel_entry = *ld_backend.getRelPLT().create();
  rel_entry.setType(llvm::ELF::R_386_JUMP_SLOT);
  rel_entry.targetRef().assign(*gotplt_entry);
  rel_entry.setSymInfo(rsym);
  return *plt_entry;
}

static
X86Relocator::Address helper_get_PLT_address(ResolveInfo& pSym, X86_32Relocator& pParent)
{
  PLTEntryBase* plt_entry = pParent.getSymPLTMap().lookUp(pSym);
  assert(NULL != plt_entry);
  return pParent.getTarget().getPLT().addr() + plt_entry->getOffset();
}

//===--------------------------------------------------------------------===//
// X86_32 Relocation Functions and Tables
//===--------------------------------------------------------------------===//
DECL_X86_32_APPLY_RELOC_FUNCS

/// the prototype of applying function
typedef Relocator::Result (*X86_32ApplyFunctionType)(Relocation& pReloc,
						     X86_32Relocator& pParent);

// the table entry of applying functions
struct X86_32ApplyFunctionTriple
{
  X86_32ApplyFunctionType func;
  unsigned int type;
  const char* name;
  unsigned int size;
};

// declare the table of applying functions
static const X86_32ApplyFunctionTriple X86_32ApplyFunctions[] = {
  DECL_X86_32_APPLY_RELOC_FUNC_PTRS
};

//===--------------------------------------------------------------------===//
// X86Relocator
//===--------------------------------------------------------------------===//
X86Relocator::X86Relocator(const LinkerConfig& pConfig)
  : Relocator(pConfig) {
}

X86Relocator::~X86Relocator()
{
}

void X86Relocator::scanRelocation(Relocation& pReloc,
                                  IRBuilder& pLinker,
                                  Module& pModule,
                                  LDSection& pSection,
                                  Input& pInput)
{
  if (LinkerConfig::Object == config().codeGenType())
    return;
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  assert(NULL != rsym &&
         "ResolveInfo of relocation not set while scanRelocation");

  assert(NULL != pSection.getLink());
  if (0 == (pSection.getLink()->flag() & llvm::ELF::SHF_ALLOC))
    return;

  // Scan relocation type to determine if the GOT/PLT/Dynamic Relocation
  // entries should be created.
  if (rsym->isLocal()) // rsym is local
    scanLocalReloc(pReloc, pLinker, pModule, pSection);
  else // rsym is external
    scanGlobalReloc(pReloc, pLinker, pModule, pSection);

  // check if we should issue undefined reference for the relocation target
  // symbol
  if (rsym->isUndef() && !rsym->isDyn() && !rsym->isWeak() && !rsym->isNull())
    issueUndefRef(pReloc, pSection, pInput);
}

void X86Relocator::addCopyReloc(ResolveInfo& pSym, X86GNULDBackend& pTarget)
{
  Relocation& rel_entry = *pTarget.getRelDyn().create();
  rel_entry.setType(pTarget.getCopyRelType());
  assert(pSym.outSymbol()->hasFragRef());
  rel_entry.targetRef().assign(*pSym.outSymbol()->fragRef());
  rel_entry.setSymInfo(&pSym);
}

/// defineSymbolforCopyReloc
/// For a symbol needing copy relocation, define a copy symbol in the BSS
/// section and all other reference to this symbol should refer to this
/// copy.
/// @note This is executed at `scan relocation' stage.
LDSymbol& X86Relocator::defineSymbolforCopyReloc(IRBuilder& pBuilder,
                                                 const ResolveInfo& pSym,
                                                 X86GNULDBackend& pTarget)
{
  // get or create corresponding BSS LDSection
  LDSection* bss_sect_hdr = NULL;
  ELFFileFormat* file_format = pTarget.getOutputFormat();
  if (ResolveInfo::ThreadLocal == pSym.type())
    bss_sect_hdr = &file_format->getTBSS();
  else
    bss_sect_hdr = &file_format->getBSS();

  // get or create corresponding BSS SectionData
  assert(NULL != bss_sect_hdr);
  SectionData* bss_section = NULL;
  if (bss_sect_hdr->hasSectionData())
    bss_section = bss_sect_hdr->getSectionData();
  else
    bss_section = IRBuilder::CreateSectionData(*bss_sect_hdr);

  // Determine the alignment by the symbol value
  // FIXME: here we use the largest alignment
  uint32_t addralign = config().targets().bitclass() / 8;

  // allocate space in BSS for the copy symbol
  Fragment* frag = new FillFragment(0x0, 1, pSym.size());
  uint64_t size = ObjectBuilder::AppendFragment(*frag,
                                                *bss_section,
                                                addralign);
  bss_sect_hdr->setSize(bss_sect_hdr->size() + size);

  // change symbol binding to Global if it's a weak symbol
  ResolveInfo::Binding binding = (ResolveInfo::Binding)pSym.binding();
  if (binding == ResolveInfo::Weak)
    binding = ResolveInfo::Global;

  // Define the copy symbol in the bss section and resolve it
  LDSymbol* cpy_sym = pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
                      pSym.name(),
                      (ResolveInfo::Type)pSym.type(),
                      ResolveInfo::Define,
                      binding,
                      pSym.size(),  // size
                      0x0,          // value
                      FragmentRef::Create(*frag, 0x0),
                      (ResolveInfo::Visibility)pSym.other());

  // output all other alias symbols if any
  Module &pModule = pBuilder.getModule();
  Module::AliasList* alias_list = pModule.getAliasList(pSym);
  if (NULL!=alias_list) {
    Module::alias_iterator it, it_e=alias_list->end();
    for (it=alias_list->begin(); it!=it_e; ++it) {
      const ResolveInfo* alias = *it;
      if (alias!=&pSym && alias->isDyn()) {
        pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Resolve>(
                           alias->name(),
                           (ResolveInfo::Type)alias->type(),
                           ResolveInfo::Define,
                           binding,
                           alias->size(),  // size
                           0x0,          // value
                           FragmentRef::Create(*frag, 0x0),
                           (ResolveInfo::Visibility)alias->other());
      }
    }
  }

  return *cpy_sym;
}

//===--------------------------------------------------------------------===//
// X86_32Relocator
//===--------------------------------------------------------------------===//
X86_32Relocator::X86_32Relocator(X86_32GNULDBackend& pParent,
                                 const LinkerConfig& pConfig)
  : X86Relocator(pConfig), m_Target(pParent) {
}

Relocator::Result
X86_32Relocator::applyRelocation(Relocation& pRelocation)
{
  Relocation::Type type = pRelocation.type();

  if (type >= sizeof (X86_32ApplyFunctions) / sizeof (X86_32ApplyFunctions[0]) ) {
    return Unknown;
  }

  // apply the relocation
  return X86_32ApplyFunctions[type].func(pRelocation, *this);
}

const char* X86_32Relocator::getName(Relocation::Type pType) const
{
  return X86_32ApplyFunctions[pType].name;
}

Relocator::Size X86_32Relocator::getSize(Relocation::Type pType) const
{
  return X86_32ApplyFunctions[pType].size;;
}

void X86_32Relocator::scanLocalReloc(Relocation& pReloc,
                                     IRBuilder& pBuilder,
                                     Module& pModule,
                                     LDSection& pSection)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();

  switch(pReloc.type()){

    case llvm::ELF::R_386_32:
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rel.dyn
      if (config().isCodeIndep()) {
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
        // set up the dyn rel directly
        helper_DynRel_init(rsym,
                           *pReloc.targetRef().frag(),
                           pReloc.targetRef().offset(),
                           llvm::ELF::R_386_RELATIVE,
                           *this);
      }
      return;

    case llvm::ELF::R_386_16:
    case llvm::ELF::R_386_8:
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rel.dyn
      if (config().isCodeIndep()) {
        // set up the dyn rel directly
        helper_DynRel_init(rsym, *pReloc.targetRef().frag(),
                            pReloc.targetRef().offset(), pReloc.type(), *this);
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      return;

    case llvm::ELF::R_386_PLT32:
      return;

    case llvm::ELF::R_386_GOTOFF:
    case llvm::ELF::R_386_GOTPC:
      // FIXME: A GOT section is needed
      return;

    case llvm::ELF::R_386_GOT32:
      // Symbol needs GOT entry, reserve entry in .got
      // return if we already create GOT for this symbol
      if (rsym->reserved() & ReserveGOT)
        return;

      // FIXME: check STT_GNU_IFUNC symbol

      // If building shared object or the symbol is undefined, a dynamic
      // relocation is needed to relocate this GOT entry. Reserve an
      // entry in .rel.dyn
      if (LinkerConfig::DynObj ==
                   config().codeGenType() || rsym->isUndef() || rsym->isDyn()) {
        helper_GOT_init(pReloc, true, *this);
        // set GOT bit
        rsym->setReserved(rsym->reserved() | ReserveGOT);
        return;
      }

      // elsewhere if the GOT is used in statically linked binaries,
      // the GOT entry is enough and no relocation is needed.
      helper_GOT_init(pReloc, false, *this);
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;

    case llvm::ELF::R_386_PC32:
    case llvm::ELF::R_386_PC16:
    case llvm::ELF::R_386_PC8:
      return;

    case llvm::ELF::R_386_TLS_GD: {
      // FIXME: no linker optimization for TLS relocation
      if (rsym->reserved() & ReserveGOT)
        return;

      // define the section symbol for .tdata or .tbss
      // the target symbol of the created dynamic relocation should be the
      // section symbol of the section which this symbol defined. so we
      // need to define that section symbol here
      ELFFileFormat* file_format = getTarget().getOutputFormat();
      const LDSection* sym_sect =
               &rsym->outSymbol()->fragRef()->frag()->getParent()->getSection();
      LDSymbol* sect_sym = NULL;
      if (&file_format->getTData() == sym_sect) {
        if (!getTarget().hasTDATASymbol()) {
          sect_sym = pModule.getSectionSymbolSet().get(*sym_sect);
          getTarget().setTDATASymbol(*sect_sym);
        }
      }
      else if (&file_format->getTBSS() == sym_sect || rsym->isCommon()) {
        if (!getTarget().hasTBSSSymbol()) {
          sect_sym = pModule.getSectionSymbolSet().get(*sym_sect);
          getTarget().setTBSSSymbol(*sect_sym);
        }
      }
      else
        error(diag::invalid_tls) << rsym->name() << sym_sect->name();

      // set up a pair of got entries and a dyn rel
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      X86_32GOTEntry* got_entry1 = getTarget().getGOT().create();
      X86_32GOTEntry* got_entry2 = getTarget().getGOT().create();
      getSymGOTMap().record(*rsym, *got_entry1, *got_entry2);
      // set up value of got entries, the value of got_entry2 should be the
      // symbol value, which has to be set during apply relocation
      got_entry1->setValue(0x0);

      // setup dyn rel for got_entry1
      Relocation& rel_entry1 = helper_DynRel_init(rsym, *got_entry1, 0x0,
                                        llvm::ELF::R_386_TLS_DTPMOD32, *this);
      // for local tls symbol, add rel entry against the section symbol this
      // symbol belong to (.tdata or .tbss)
      rel_entry1.setSymInfo(sect_sym->resolveInfo());
      return;
    }
    case llvm::ELF::R_386_TLS_LDM:
      getTLSModuleID();
      return;

    case llvm::ELF::R_386_TLS_LDO_32:
      return;

    case llvm::ELF::R_386_TLS_IE: {
      getTarget().setHasStaticTLS();

      // if building shared object, a RELATIVE dynamic relocation is needed
      if (LinkerConfig::DynObj == config().codeGenType()) {
         helper_DynRel_init(rsym, *pReloc.targetRef().frag(),
                                              pReloc.targetRef().offset(),
                                              llvm::ELF::R_386_RELATIVE, *this);
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      else {
        // for local sym, we can convert ie to le if not building shared object
        convertTLSIEtoLE(pReloc, pSection);
        return;
      }

      if (rsym->reserved() & ReserveGOT)
        return;

      // set up the got and the corresponding rel entry
      X86_32GOTEntry* got_entry = getTarget().getGOT().create();
      getSymGOTMap().record(*rsym, *got_entry);
      got_entry->setValue(0x0);
      helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_386_TLS_TPOFF,
                                                                         *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      // add symbol to dyn sym table
      getTarget().getRelDyn().addSymbolToDynSym(*rsym->outSymbol());
      return;
    }

    case llvm::ELF::R_386_TLS_GOTIE: {
      getTarget().setHasStaticTLS();
      if (rsym->reserved() & ReserveGOT)
        return;
      // set up the got and the corresponding dyn rel
      X86_32GOTEntry* got_entry = getTarget().getGOT().create();
      getSymGOTMap().record(*rsym, *got_entry);
      got_entry->setValue(0x0);
      helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_386_TLS_TPOFF,
                                                                        *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      getTarget().getRelDyn().addSymbolToDynSym(*rsym->outSymbol());
      return;
    }

    case llvm::ELF::R_386_TLS_LE:
    case llvm::ELF::R_386_TLS_LE_32:
      getTarget().setHasStaticTLS();
      // if buildint shared object, a dynamic relocation is needed
      if (LinkerConfig::DynObj == config().codeGenType()) {
        helper_DynRel_init(rsym,
                           *pReloc.targetRef().frag(),
                           pReloc.targetRef().offset(),
                           llvm::ELF::R_386_TLS_TPOFF,
                           *this);
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
        // the target symbol of the dynamic relocation is rsym, so we need to
        // emit it into .dynsym
        assert(NULL != rsym->outSymbol());
        getTarget().getRelDyn().addSymbolToDynSym(*rsym->outSymbol());
      }
      return;

    default:
      fatal(diag::unsupported_relocation) << (int)pReloc.type()
                                          << "mclinker@googlegroups.com";
      break;
  } // end switch
}

void X86_32Relocator::scanGlobalReloc(Relocation& pReloc,
                                      IRBuilder& pBuilder,
                                      Module& pModule,
                                      LDSection& pSection)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();

  switch(pReloc.type()) {
    case llvm::ELF::R_386_32:
    case llvm::ELF::R_386_16:
    case llvm::ELF::R_386_8:
      // Absolute relocation type, symbol may needs PLT entry or
      // dynamic relocation entry
      if (getTarget().symbolNeedsPLT(*rsym)) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)){
          // Symbol needs PLT entry, we need a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rel.plt.
          helper_PLT_init(pReloc, *this);
          // set PLT bit
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }

      if (getTarget().symbolNeedsDynRel(*rsym, (rsym->reserved() & ReservePLT),
                                                                        true)) {
        // symbol needs dynamic relocation entry, set up the dynrel entry
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym, getTarget());
          addCopyReloc(*cpy_sym.resolveInfo(), getTarget());
        }
        else {
          // set Rel bit and the dyn rel
          rsym->setReserved(rsym->reserved() | ReserveRel);
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
          if (llvm::ELF::R_386_32 == pReloc.type() &&
              helper_use_relative_reloc(*rsym, *this))
            helper_DynRel_init(rsym,
                               *pReloc.targetRef().frag(),
                               pReloc.targetRef().offset(),
                               llvm::ELF::R_386_RELATIVE,
                               *this);
          else
            helper_DynRel_init(rsym,
                               *pReloc.targetRef().frag(),
                               pReloc.targetRef().offset(),
                               pReloc.type(),
                               *this);
        }
      }
      return;

    case llvm::ELF::R_386_GOTOFF:
    case llvm::ELF::R_386_GOTPC: {
      // FIXME: A GOT section is needed
      return;
    }

    case llvm::ELF::R_386_PLT32:
      // A PLT entry is needed when building shared library

      // return if we already create plt for this symbol
      if (rsym->reserved() & ReservePLT)
        return;

      // if the symbol's value can be decided at link time, then no need plt
      if (getTarget().symbolFinalValueIsKnown(*rsym))
        return;

      // if symbol is defined in the ouput file and it's not
      // preemptible, no need plt
      if (rsym->isDefine() && !rsym->isDyn() &&
          !getTarget().isSymbolPreemptible(*rsym))
        return;

      // Symbol needs PLT entry, we need a PLT entry
      // and the corresponding GOT and dynamic relocation entry
      // in .got and .rel.plt
      helper_PLT_init(pReloc, *this);
      // set PLT bit
      rsym->setReserved(rsym->reserved() | ReservePLT);
      return;

    case llvm::ELF::R_386_GOT32:
      // Symbol needs GOT entry, reserve entry in .got
      // return if we already create GOT for this symbol
      if (rsym->reserved() & ReserveGOT)
        return;
      // If building shared object or the symbol is undefined, a dynamic
      // relocation is needed to relocate this GOT entry. Reserve an
      // entry in .rel.dyn
      if (LinkerConfig::DynObj ==
                   config().codeGenType() || rsym->isUndef() || rsym->isDyn())
        helper_GOT_init(pReloc, true, *this);
      else
        helper_GOT_init(pReloc, false, *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;

    case llvm::ELF::R_386_PC32:
    case llvm::ELF::R_386_PC16:
    case llvm::ELF::R_386_PC8:

      if (getTarget().symbolNeedsPLT(*rsym) &&
          LinkerConfig::DynObj != config().codeGenType()) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)){
          // Symbol needs PLT entry, we need a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rel.plt.
          // set PLT bit
          helper_PLT_init(pReloc, *this);
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }

      if (getTarget().symbolNeedsDynRel(*rsym, (rsym->reserved() & ReservePLT),
                                                                       false)) {
        // symbol needs dynamic relocation entry, setup an entry in .rel.dyn
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym,
                                                                   getTarget());
          addCopyReloc(*cpy_sym.resolveInfo(), getTarget());
        }
        else {
          // set Rel bit and the dyn rel
          rsym->setReserved(rsym->reserved() | ReserveRel);
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
          if (llvm::ELF::R_386_32 == pReloc.type() &&
              helper_use_relative_reloc(*rsym, *this))
            helper_DynRel_init(rsym,
                               *pReloc.targetRef().frag(),
                               pReloc.targetRef().offset(),
                               llvm::ELF::R_386_RELATIVE,
                               *this);
          else
            helper_DynRel_init(rsym,
                               *pReloc.targetRef().frag(),
                               pReloc.targetRef().offset(),
                               pReloc.type(),
                               *this);
        }
      }
      return;

    case llvm::ELF::R_386_TLS_GD: {
      // FIXME: no linker optimization for TLS relocation
      if (rsym->reserved() & ReserveGOT)
        return;

      // set up a pair of got entries and a pair of dyn rel
      X86_32GOTEntry* got_entry1 = getTarget().getGOT().create();
      X86_32GOTEntry* got_entry2 = getTarget().getGOT().create();
      getSymGOTMap().record(*rsym, *got_entry1, *got_entry2);
      got_entry1->setValue(0x0);
      got_entry2->setValue(0x0);
      // setup dyn rel for got entries against rsym
      helper_DynRel_init(rsym, *got_entry1, 0x0,
                                        llvm::ELF::R_386_TLS_DTPMOD32, *this);
      helper_DynRel_init(rsym, *got_entry2, 0x0,
                                        llvm::ELF::R_386_TLS_DTPOFF32, *this);

      // add the rsym to dynamic symbol table
      getTarget().getRelDyn().addSymbolToDynSym(*rsym->outSymbol());
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;
    }

    case llvm::ELF::R_386_TLS_LDM:
      getTLSModuleID();
      return;

    case llvm::ELF::R_386_TLS_LDO_32:
      return;

    case llvm::ELF::R_386_TLS_IE: {
      getTarget().setHasStaticTLS();
      // if buildint shared object, a RELATIVE dynamic relocation is needed
      if (LinkerConfig::DynObj == config().codeGenType()) {
        helper_DynRel_init(rsym, *pReloc.targetRef().frag(),
                                               pReloc.targetRef().offset(),
                                               llvm::ELF::R_386_RELATIVE, *this);
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      } else {
        // for global sym, we can convert ie to le if its final value is known
        if (getTarget().symbolFinalValueIsKnown(*rsym)) {
          convertTLSIEtoLE(pReloc, pSection);
          return;
        }
      }
      if (rsym->reserved() & ReserveGOT)
        return;
      // set up the got and the corresponding rel entry
      X86_32GOTEntry* got_entry = getTarget().getGOT().create();
      getSymGOTMap().record(*rsym, *got_entry);
      got_entry->setValue(0x0);
      helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_386_TLS_TPOFF,
                                                                         *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;
    }

    case llvm::ELF::R_386_TLS_GOTIE: {
      getTarget().setHasStaticTLS();
      if (rsym->reserved() & ReserveGOT)
        return;
      // set up the got and the corresponding dyn rel
      X86_32GOTEntry* got_entry = getTarget().getGOT().create();
      getSymGOTMap().record(*rsym, *got_entry);
      got_entry->setValue(0x0);
      helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_386_TLS_TPOFF,
                                                                        *this);
      getTarget().getRelDyn().addSymbolToDynSym(*rsym->outSymbol());
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;
    }

    case llvm::ELF::R_386_TLS_LE:
    case llvm::ELF::R_386_TLS_LE_32:
      getTarget().setHasStaticTLS();
      // if buildint shared object, a dynamic relocation is needed
      if (LinkerConfig::DynObj == config().codeGenType()) {
        helper_DynRel_init(rsym,
                           *pReloc.targetRef().frag(),
                           pReloc.targetRef().offset(),
                           llvm::ELF::R_386_TLS_TPOFF,
                           *this);
        getTarget().getRelDyn().addSymbolToDynSym(*rsym->outSymbol());
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      return;

    default: {
      fatal(diag::unsupported_relocation) << (int)pReloc.type()
                                          << "mclinker@googlegroups.com";
      break;
    }
  } // end switch
}

// Create a GOT entry for the TLS module index
X86_32GOTEntry& X86_32Relocator::getTLSModuleID()
{
  static X86_32GOTEntry* got_entry = NULL;
  if (NULL != got_entry)
    return *got_entry;

  // Allocate 2 got entries and 1 dynamic reloc for R_386_TLS_LDM
  got_entry = getTarget().getGOT().create();
  getTarget().getGOT().create()->setValue(0x0);

  helper_DynRel_init(NULL, *got_entry, 0x0, llvm::ELF::R_386_TLS_DTPMOD32,
                                                                          *this);
  return *got_entry;
}

/// convert R_386_TLS_IE to R_386_TLS_LE
void X86_32Relocator::convertTLSIEtoLE(Relocation& pReloc,
                                       LDSection& pSection)
{
  assert(pReloc.type() == llvm::ELF::R_386_TLS_IE);
  assert(NULL != pReloc.targetRef().frag());

  // 1. create the fragment references and new relocs
  uint64_t off = pReloc.targetRef().offset();
  if (off >= 4)
    off -= 4;
  else
    off = 0;

  FragmentRef* fragref = FragmentRef::Create(*pReloc.targetRef().frag(), off);
  Relocation* reloc = Relocation::Create(X86_32Relocator::R_386_TLS_OPT,
                                         *fragref,
                                         0x0);
  // FIXME: should we create a special symbol for the tls opt instead?
  reloc->setSymInfo(pReloc.symInfo());

  // 2. modify the opcodes to the appropriate ones
  uint8_t* op =  (reinterpret_cast<uint8_t*>(&reloc->target()));
  off = pReloc.targetRef().offset() - reloc->targetRef().offset() - 1;
  if (op[off] == 0xa1) {
    op[off] = 0xb8;
  } else {
    switch (op[off - 1]) {
      case 0x8b:
        assert((op[off] & 0xc7) == 0x05);
        op[off - 1] = 0xc7;
        op[off]     = 0xc0 | ((op[off] >> 3) & 7);
        break;
      case 0x03:
        assert((op[off] & 0xc7) == 0x05);
        op[off - 1] = 0x81;
        op[off]     = 0xc0 | ((op[off] >> 3) & 7);
        break;
      default:
        assert(0);
        break;
    }
  }

  // 3. insert the new relocs "BEFORE" the original reloc.
  pSection.getRelocData()->getRelocationList().insert(
    RelocData::iterator(pReloc), reloc);

  // 4. change the type of the original reloc
  pReloc.setType(llvm::ELF::R_386_TLS_LE);
}

//================================================//
// X86_32 Each relocation function implementation //
//================================================//

// R_386_NONE
X86Relocator::Result none(Relocation& pReloc, X86_32Relocator& pParent)
{
  return X86Relocator::OK;
}

// R_386_32: S + A
// R_386_16
// R_386_8
X86Relocator::Result abs(Relocation& pReloc, X86_32Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();
  bool has_dyn_rel = pParent.getTarget().symbolNeedsDynRel(
                                  *rsym,
                                  (rsym->reserved() & X86Relocator::ReservePLT),
                                  true);


  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this relocation
  // but perform static relocation. (e.g., applying .debug section)
  if (0x0 == (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    pReloc.target() = S + A;
    return X86Relocator::OK;
  }

  // An external symbol may need PLT and dynamic relocation
  if (!rsym->isLocal()) {
    if (rsym->reserved() & X86Relocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
    }
    // If we generate a dynamic relocation (except R_386_RELATIVE)
    // for a place, we should not perform static relocation on it
    // in order to keep the addend store in the place correct.
    if (has_dyn_rel)
      if (llvm::ELF::R_386_32 != pReloc.type() ||
          (!helper_use_relative_reloc(*rsym, pParent)))
        return X86Relocator::OK;
  }

  // perform static relocation
  pReloc.target() = S + A;
  return X86Relocator::OK;
}

// R_386_PC32: S + A - P
// R_386_PC16
// R_386_PC8
X86Relocator::Result rel(Relocation& pReloc, X86_32Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();
  Relocator::DWord P = pReloc.place();
  bool has_dyn_rel = pParent.getTarget().symbolNeedsDynRel(
                                  *rsym,
                                  (rsym->reserved() & X86Relocator::ReservePLT),
                                  true);

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this relocation
  // but perform static relocation. (e.g., applying .debug section)
  if (0x0 == (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    pReloc.target() = S + A - P;
    return X86Relocator::OK;
  }

  // An external symbol may need PLT and dynamic relocation
  if (!rsym->isLocal()) {
    if (rsym->reserved() & X86Relocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
      pReloc.target() = S + A - P;
    }
    if (has_dyn_rel)
      if (!helper_use_relative_reloc(*rsym, pParent))
        return X86Relocator::OK;
  }

   // perform static relocation
  pReloc.target() = S + A - P;
  return X86Relocator::OK;
}

// R_386_GOTOFF: S + A - GOT_ORG
X86Relocator::Result gotoff32(Relocation& pReloc, X86_32Relocator& pParent)
{
  Relocator::DWord      A = pReloc.target() + pReloc.addend();
  X86Relocator::Address GOT_ORG = helper_GOT_ORG(pParent);
  X86Relocator::Address S = pReloc.symValue();

  pReloc.target() = S + A - GOT_ORG;
  return X86Relocator::OK;
}

// R_386_GOTPC: GOT_ORG + A - P
X86Relocator::Result gotpc32(Relocation& pReloc, X86_32Relocator& pParent)
{
  Relocator::DWord      A       = pReloc.target() + pReloc.addend();
  X86Relocator::Address GOT_ORG = helper_GOT_ORG(pParent);
  // Apply relocation.
  pReloc.target() = GOT_ORG + A - pReloc.place();
  return X86Relocator::OK;
}

// R_386_GOT32: GOT(S) + A - GOT_ORG
X86Relocator::Result got32(Relocation& pReloc, X86_32Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  if (!(rsym->reserved() & (X86Relocator::ReserveGOT)))
    return X86Relocator::BadReloc;

  // set up got entry value if the got has no dyn rel or
  // the dyn rel is RELATIVE
  X86_32GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  assert(NULL != got_entry);
  if (got_entry->getValue() == X86Relocator::SymVal)
    got_entry->setValue(pReloc.symValue());

  X86Relocator::Address GOT_S   = helper_get_GOT_address(pReloc, pParent);
  Relocator::DWord      A       = pReloc.target() + pReloc.addend();
  X86Relocator::Address GOT_ORG = helper_GOT_ORG(pParent);
  // Apply relocation.
  pReloc.target() = GOT_S + A - GOT_ORG;
  return X86Relocator::OK;
}

// R_386_PLT32: PLT(S) + A - P
X86Relocator::Result plt32(Relocation& pReloc, X86_32Relocator& pParent)
{
  // PLT_S depends on if there is a PLT entry.
  X86Relocator::Address PLT_S;
  if ((pReloc.symInfo()->reserved() & X86Relocator::ReservePLT))
    PLT_S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
  else
    PLT_S = pReloc.symValue();
  Relocator::DWord      A = pReloc.target() + pReloc.addend();
  X86Relocator::Address P = pReloc.place();
  pReloc.target() = PLT_S + A - P;
  return X86Relocator::OK;
}

// R_386_TLS_GD:
X86Relocator::Result tls_gd(Relocation& pReloc, X86_32Relocator& pParent)
{
  // global-dynamic
  ResolveInfo* rsym = pReloc.symInfo();
  // must reserve two pairs of got and dynamic relocation
  if (!(rsym->reserved() & X86Relocator::ReserveGOT))
    return X86Relocator::BadReloc;

  ELFFileFormat* file_format = pParent.getTarget().getOutputFormat();
  // setup corresponding got and dynamic relocatio entries:
  // get first got entry, if there is already a got entry for rsym, then apply
  // this relocation to the got entry directly. If not, setup the corresponding
  // got and dyn relocation entries
  X86_32GOTEntry* got_entry1 = pParent.getSymGOTMap().lookUpFirstEntry(*rsym);

  // set the got_entry2 value to symbol value
  if (rsym->isLocal())
     pParent.getSymGOTMap().lookUpSecondEntry(*rsym)->setValue(pReloc.symValue());

  // perform relocation to the first got entry
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  // GOT_OFF - the offset between the got_entry1 and _GLOBAL_OFFSET_TABLE (the
  // .got.plt section)
  X86Relocator::Address GOT_OFF =
     file_format->getGOT().addr() +
     got_entry1->getOffset() -
     file_format->getGOTPLT().addr();
  pReloc.target() = GOT_OFF + A;
  return X86Relocator::OK;
}

// R_386_TLS_LDM
X86Relocator::Result tls_ldm(Relocation& pReloc, X86_32Relocator& pParent)
{
  // FIXME: no linker optimization for TLS relocation
  const X86_32GOTEntry& got_entry = pParent.getTLSModuleID();

  // All GOT offsets are relative to the end of the GOT.
  X86Relocator::SWord GOT_S = got_entry.getOffset() -
                                      (pParent.getTarget().getGOTPLT().addr() -
                                       pParent.getTarget().getGOT().addr());
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  pReloc.target() = GOT_S + A;

  return X86Relocator::OK;
}

// R_386_TLS_LDO_32
X86Relocator::Result tls_ldo_32(Relocation& pReloc, X86_32Relocator& pParent)
{
  // FIXME: no linker optimization for TLS relocation
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  X86Relocator::Address S = pReloc.symValue();
  pReloc.target() = S + A;
  return X86Relocator::OK;
}

// R_X86_TLS_IE
X86Relocator::Result tls_ie(Relocation& pReloc, X86_32Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  if (!(rsym->reserved() & X86Relocator::ReserveGOT)) {
     return X86Relocator::BadReloc;
  }

  // set up the got and dynamic relocation entries if not exist
  X86_32GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*rsym);
  assert(NULL != got_entry);
  // perform relocation to the absolute address of got_entry
  X86Relocator::Address GOT_S =
                 pParent.getTarget().getGOT().addr() + got_entry->getOffset();

  Relocator::DWord A = pReloc.target() + pReloc.addend();
  pReloc.target() = GOT_S + A;

  return X86Relocator::OK;
}

// R_386_TLS_GOTIE
X86Relocator::Result tls_gotie(Relocation& pReloc, X86_32Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  if (!(rsym->reserved() & X86Relocator::ReserveGOT)) {
     return X86Relocator::BadReloc;
  }

  // set up the got and dynamic relocation entries if not exist
  X86_32GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*rsym);
  assert(NULL != got_entry);

  // All GOT offsets are relative to the end of the GOT.
  X86Relocator::SWord GOT_S = got_entry->getOffset() -
    (pParent.getTarget().getGOTPLT().addr() - pParent.getTarget().getGOT().addr());
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  pReloc.target() = GOT_S + A;

  return X86Relocator::OK;
}

// R_X86_TLS_LE
X86Relocator::Result tls_le(Relocation& pReloc, X86_32Relocator& pParent)
{
  if (pReloc.symInfo()->reserved() & X86Relocator::ReserveRel)
    return X86Relocator::OK;

  // perform static relocation
  // get TLS segment
  ELFSegmentFactory::const_iterator tls_seg =
    pParent.getTarget().elfSegmentTable().find(llvm::ELF::PT_TLS,
                                               llvm::ELF::PF_R,
                                               0x0);
  assert(tls_seg != pParent.getTarget().elfSegmentTable().end());
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  X86Relocator::Address S = pReloc.symValue();
  pReloc.target() = S + A - (*tls_seg)->memsz();
  return X86Relocator::OK;
}

X86Relocator::Result unsupport(Relocation& pReloc, X86_32Relocator& pParent)
{
  return X86Relocator::Unsupport;
}

//===--------------------------------------------------------------------===//
// X86_64 Relocation helper function
//===--------------------------------------------------------------------===//
/// helper_DynRel - Get an relocation entry in .rela.dyn
static
Relocation& helper_DynRel_init(ResolveInfo* pSym,
                               Fragment& pFrag,
                               uint64_t pOffset,
                               X86Relocator::Type pType,
                               X86_64Relocator& pParent)
{
  X86_64GNULDBackend& ld_backend = pParent.getTarget();
  Relocation& rel_entry = *ld_backend.getRelDyn().create();
  rel_entry.setType(pType);
  rel_entry.targetRef().assign(pFrag, pOffset);
  if (pType == llvm::ELF::R_X86_64_RELATIVE || NULL == pSym)
    rel_entry.setSymInfo(NULL);
  else
    rel_entry.setSymInfo(pSym);

  return rel_entry;
}


/// helper_use_relative_reloc - Check if symbol can use relocation
/// R_X86_64_RELATIVE
static bool
helper_use_relative_reloc(const ResolveInfo& pSym,
                          const X86_64Relocator& pFactory)

{
  // if symbol is dynamic or undefine or preemptible
  if (pSym.isDyn() ||
      pSym.isUndef() ||
      pFactory.getTarget().isSymbolPreemptible(pSym))
    return false;
  return true;
}

static
X86_64GOTEntry& helper_GOT_init(Relocation& pReloc,
                                bool pHasRel,
                                X86_64Relocator& pParent)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  X86_64GNULDBackend& ld_backend = pParent.getTarget();
  assert(NULL == pParent.getSymGOTMap().lookUp(*rsym));

  X86_64GOTEntry* got_entry = ld_backend.getGOT().create();
  pParent.getSymGOTMap().record(*rsym, *got_entry);

  // If we first get this GOT entry, we should initialize it.
  if (!pHasRel) {
    // No corresponding dynamic relocation, initialize to the symbol value.
    got_entry->setValue(X86Relocator::SymVal);
  }
  else {
    // Initialize got_entry content and the corresponding dynamic relocation.
    if (helper_use_relative_reloc(*rsym, pParent)) {
       Relocation& rel_entry = helper_DynRel_init(rsym, *got_entry, 0x0,
                                         llvm::ELF::R_X86_64_RELATIVE, pParent);
       rel_entry.setAddend(X86Relocator::SymVal);
       pParent.getRelRelMap().record(pReloc, rel_entry);
    }
    else {
       helper_DynRel_init(rsym, *got_entry, 0x0, llvm::ELF::R_X86_64_GLOB_DAT,
                                                                       pParent);
    }
    got_entry->setValue(0);
  }
  return *got_entry;
}

static
X86Relocator::Address helper_GOT_ORG(X86_64Relocator& pParent)
{
  return pParent.getTarget().getGOT().addr();
}

static
X86Relocator::Address helper_get_GOT_address(Relocation& pReloc,
                                             X86_64Relocator& pParent)
{
  X86_64GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  assert(NULL != got_entry);
  return got_entry->getOffset();
}

static
X86Relocator::Address helper_get_PLT_address(ResolveInfo& pSym,
                                             X86_64Relocator& pParent)
{
  PLTEntryBase* plt_entry = pParent.getSymPLTMap().lookUp(pSym);
  assert(NULL != plt_entry);
  return pParent.getTarget().getPLT().addr() + plt_entry->getOffset();
}

static
PLTEntryBase& helper_PLT_init(Relocation& pReloc, X86_64Relocator& pParent)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();
  X86_64GNULDBackend& ld_backend = pParent.getTarget();
  assert(NULL == pParent.getSymPLTMap().lookUp(*rsym));

  PLTEntryBase* plt_entry = ld_backend.getPLT().create();
  pParent.getSymPLTMap().record(*rsym, *plt_entry);

  // initialize plt and the corresponding gotplt and dyn rel entry.
  assert(NULL == pParent.getSymGOTPLTMap().lookUp(*rsym) &&
         "PLT entry not exist, but DynRel entry exist!");
  X86_64GOTEntry* gotplt_entry = ld_backend.getGOTPLT().create();
  pParent.getSymGOTPLTMap().record(*rsym, *gotplt_entry);

  // init the corresponding rel entry in .rel.plt
  Relocation& rel_entry = *ld_backend.getRelPLT().create();
  rel_entry.setType(llvm::ELF::R_X86_64_JUMP_SLOT);
  rel_entry.targetRef().assign(*gotplt_entry);
  rel_entry.setSymInfo(rsym);
  return *plt_entry;
}

//===--------------------------------------------------------------------===//
// X86_64 Relocation Functions and Tables
//===--------------------------------------------------------------------===//
DECL_X86_64_APPLY_RELOC_FUNCS

/// the prototype of applying function
typedef Relocator::Result (*X86_64ApplyFunctionType)(Relocation& pReloc,
						     X86_64Relocator& pParent);

// the table entry of applying functions
struct X86_64ApplyFunctionTriple
{
  X86_64ApplyFunctionType func;
  unsigned int type;
  const char* name;
  unsigned int size;
};

// declare the table of applying functions
static const X86_64ApplyFunctionTriple X86_64ApplyFunctions[] = {
  DECL_X86_64_APPLY_RELOC_FUNC_PTRS
};

//===--------------------------------------------------------------------===//
// X86_64Relocator
//===--------------------------------------------------------------------===//
X86_64Relocator::X86_64Relocator(X86_64GNULDBackend& pParent,
                                 const LinkerConfig& pConfig)
  : X86Relocator(pConfig), m_Target(pParent) {
}

Relocator::Result
X86_64Relocator::applyRelocation(Relocation& pRelocation)
{
  Relocation::Type type = pRelocation.type();

  if (type >= sizeof (X86_64ApplyFunctions) / sizeof (X86_64ApplyFunctions[0]) ) {
    return Unknown;
  }

  // apply the relocation
  return X86_64ApplyFunctions[type].func(pRelocation, *this);
}

const char* X86_64Relocator::getName(Relocation::Type pType) const
{
  return X86_64ApplyFunctions[pType].name;
}

Relocator::Size X86_64Relocator::getSize(Relocation::Type pType) const
{
  return X86_64ApplyFunctions[pType].size;
}

void X86_64Relocator::scanLocalReloc(Relocation& pReloc,
                                     IRBuilder& pBuilder,
                                     Module& pModule,
                                     LDSection& pSection)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();

  switch(pReloc.type()){
    case llvm::ELF::R_X86_64_64:
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rela.dyn
      if (config().isCodeIndep()) {
        Relocation& reloc = helper_DynRel_init(rsym,
                                               *pReloc.targetRef().frag(),
                                               pReloc.targetRef().offset(),
                                               llvm::ELF::R_X86_64_RELATIVE,
                                               *this);
        getRelRelMap().record(pReloc, reloc);
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      return;

    case llvm::ELF::R_X86_64_32:
    case llvm::ELF::R_X86_64_16:
    case llvm::ELF::R_X86_64_8:
    case llvm::ELF::R_X86_64_32S:
      // If buiding PIC object (shared library or PIC executable),
      // a dynamic relocations with RELATIVE type to this location is needed.
      // Reserve an entry in .rela.dyn
      if (config().isCodeIndep()) {
        Relocation& reloc = helper_DynRel_init(rsym,
                                               *pReloc.targetRef().frag(),
                                               pReloc.targetRef().offset(),
                                               pReloc.type(),
                                               *this);
        getRelRelMap().record(pReloc, reloc);
        // set Rel bit
        rsym->setReserved(rsym->reserved() | ReserveRel);
        getTarget().checkAndSetHasTextRel(*pSection.getLink());
      }
      return;

    case llvm::ELF::R_X86_64_PC32:
    case llvm::ELF::R_X86_64_PC16:
    case llvm::ELF::R_X86_64_PC8:
      return;

    case llvm::ELF::R_X86_64_GOTPCREL:
      // Symbol needs GOT entry, reserve entry in .got
      // return if we already create GOT for this symbol
      if (rsym->reserved() & ReserveGOT)
        return;

      // If building shared object or the symbol is undefined, a dynamic
      // relocation is needed to relocate this GOT entry. Reserve an
      // entry in .rela.dyn
      if (LinkerConfig::DynObj ==
                   config().codeGenType() || rsym->isUndef() || rsym->isDyn())
        helper_GOT_init(pReloc, true, *this);
      else
        helper_GOT_init(pReloc, false, *this);
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;

    default:
      fatal(diag::unsupported_relocation) << (int)pReloc.type()
                                          << "mclinker@googlegroups.com";
      break;
  } // end switch
}

void X86_64Relocator::scanGlobalReloc(Relocation& pReloc,
                                      IRBuilder& pBuilder,
                                      Module& pModule,
                                      LDSection& pSection)
{
  // rsym - The relocation target symbol
  ResolveInfo* rsym = pReloc.symInfo();

  switch(pReloc.type()) {
    case llvm::ELF::R_X86_64_64:
    case llvm::ELF::R_X86_64_32:
    case llvm::ELF::R_X86_64_16:
    case llvm::ELF::R_X86_64_8:
    case llvm::ELF::R_X86_64_32S:
      // Absolute relocation type, symbol may needs PLT entry or
      // dynamic relocation entry
      if (getTarget().symbolNeedsPLT(*rsym)) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)){
          // Symbol needs PLT entry, we need to reserve a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rela.plt.
          helper_PLT_init(pReloc, *this);
          // set PLT bit
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }

      if (getTarget().symbolNeedsDynRel(*rsym, (rsym->reserved() & ReservePLT),
                                                                        true)) {
        // symbol needs dynamic relocation entry, set up the dynrel entry
        if (getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
          LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym, getTarget());
          addCopyReloc(*cpy_sym.resolveInfo(), getTarget());
        }
        else {
          // set Rel bit and the dyn rel
          rsym->setReserved(rsym->reserved() | ReserveRel);
          getTarget().checkAndSetHasTextRel(*pSection.getLink());
          if (llvm::ELF::R_386_32 == pReloc.type() &&
              helper_use_relative_reloc(*rsym, *this)) {
            Relocation& reloc = helper_DynRel_init(rsym,
                                                   *pReloc.targetRef().frag(),
                                                   pReloc.targetRef().offset(),
                                                   llvm::ELF::R_X86_64_RELATIVE,
                                                   *this);
            getRelRelMap().record(pReloc, reloc);
          }
          else {
            Relocation& reloc = helper_DynRel_init(rsym,
                                                   *pReloc.targetRef().frag(),
                                                   pReloc.targetRef().offset(),
                                                   pReloc.type(),
                                                   *this);
            getRelRelMap().record(pReloc, reloc);
          }
	        getTarget().checkAndSetHasTextRel(*pSection.getLink());
        }
      }
      return;

    case llvm::ELF::R_X86_64_GOTPCREL:
      // Symbol needs GOT entry, reserve entry in .got
      // return if we already create GOT for this symbol
      if (rsym->reserved() & ReserveGOT)
        return;

      // If building shared object or the symbol is undefined, a dynamic
      // relocation is needed to relocate this GOT entry. Reserve an
      // entry in .rela.dyn
      if (LinkerConfig::DynObj ==
                   config().codeGenType() || rsym->isUndef() || rsym->isDyn())
        helper_GOT_init(pReloc, true, *this);
      else
        helper_GOT_init(pReloc, false, *this);
      // set GOT bit
      rsym->setReserved(rsym->reserved() | ReserveGOT);
      return;

    case llvm::ELF::R_X86_64_PLT32:
      // A PLT entry is needed when building shared library

      // return if we already create plt for this symbol
      if (rsym->reserved() & ReservePLT)
        return;

      // if the symbol's value can be decided at link time, then no need plt
      if (getTarget().symbolFinalValueIsKnown(*rsym))
        return;

      // if symbol is defined in the ouput file and it's not
      // preemptible, no need plt
      if (rsym->isDefine() && !rsym->isDyn() &&
         !getTarget().isSymbolPreemptible(*rsym)) {
        return;
      }

      // Symbol needs PLT entry, we need a PLT entry
      // and the corresponding GOT and dynamic relocation entry
      // in .got and .rel.plt.
      helper_PLT_init(pReloc, *this);
      // set PLT bit
      rsym->setReserved(rsym->reserved() | ReservePLT);
      return;

    case llvm::ELF::R_X86_64_PC32:
    case llvm::ELF::R_X86_64_PC16:
    case llvm::ELF::R_X86_64_PC8:
      if (getTarget().symbolNeedsPLT(*rsym) &&
          LinkerConfig::DynObj != config().codeGenType()) {
        // create plt for this symbol if it does not have one
        if (!(rsym->reserved() & ReservePLT)){
          // Symbol needs PLT entry, we need a PLT entry
          // and the corresponding GOT and dynamic relocation entry
          // in .got and .rel.plt.
          helper_PLT_init(pReloc, *this);
          // set PLT bit
          rsym->setReserved(rsym->reserved() | ReservePLT);
        }
      }

      // Only PC relative relocation against dynamic symbol needs a
      // dynamic relocation.  Only dynamic copy relocation is allowed
      // and PC relative relocation will be resolved to the local copy.
      // All other dynamic relocations may lead to run-time relocation
      // overflow.
      if (getTarget().isDynamicSymbol(*rsym) &&
	        getTarget().symbolNeedsDynRel(*rsym,
                                        (rsym->reserved() & ReservePLT),
                                        false) &&
	        getTarget().symbolNeedsCopyReloc(pReloc, *rsym)) {
   	    LDSymbol& cpy_sym = defineSymbolforCopyReloc(pBuilder, *rsym, getTarget());
	      addCopyReloc(*cpy_sym.resolveInfo(), getTarget());
      }
      return;

    default:
      fatal(diag::unsupported_relocation) << (int)pReloc.type()
                                          << "mclinker@googlegroups.com";
      break;
  } // end switch
}

// ===
//
// ===
// R_X86_64_NONE
X86Relocator::Result none(Relocation& pReloc, X86_64Relocator& pParent)
{
  return X86Relocator::OK;
}

// R_X86_64_64: S + A
// R_X86_64_32:
// R_X86_64_16:
// R_X86_64_8
X86Relocator::Result abs(Relocation& pReloc, X86_64Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();
  Relocation* dyn_rel = pParent.getRelRelMap().lookUp(pReloc);
  bool has_dyn_rel = (NULL != dyn_rel);

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this relocation
  // but perform static relocation. (e.g., applying .debug section)
  if (0x0 == (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    pReloc.target() = S + A;
    return X86Relocator::OK;
  }

  // A local symbol may need RELA Type dynamic relocation
  if (rsym->isLocal() && has_dyn_rel) {
    dyn_rel->setAddend(S + A);
    return X86Relocator::OK;
  }

  // An external symbol may need PLT and dynamic relocation
  if (!rsym->isLocal()) {
    if (rsym->reserved() & X86Relocator::ReservePLT) {
      S = helper_get_PLT_address(*rsym, pParent);
    }
    // If we generate a dynamic relocation (except R_X86_64_RELATIVE)
    // for a place, we should not perform static relocation on it
    // in order to keep the addend store in the place correct.
    if (has_dyn_rel) {
      if (llvm::ELF::R_X86_64_64 == pReloc.type() &&
          helper_use_relative_reloc(*rsym, pParent)) {
        dyn_rel->setAddend(S + A);
      }
      else {
        dyn_rel->setAddend(A);
        return X86Relocator::OK;
      }
    }
  }

  // perform static relocation
  pReloc.target() = S + A;
  return X86Relocator::OK;
}

// R_X86_64_32S: S + A
X86Relocator::Result signed32(Relocation& pReloc, X86_64Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();

  // There should be no dynamic relocations for R_X86_64_32S.
  if (NULL != pParent.getRelRelMap().lookUp(pReloc))
    return X86Relocator::BadReloc;

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this relocation
  // but perform static relocation. (e.g., applying .debug section)
  // An external symbol may need PLT and dynamic relocation
  if (0x0 != (llvm::ELF::SHF_ALLOC & target_sect.flag()) &&
      !rsym->isLocal() && rsym->reserved() & X86Relocator::ReservePLT)
    S = helper_get_PLT_address(*rsym, pParent);

#if notyet
  // Check 32-bit signed overflow.
  Relocator::SWord V = S + A;
  if (V > INT64_C(0x7fffffff) || V < INT64_C(-0x80000000))
    return X86Relocator::Overflow;
#endif

  // perform static relocation
  pReloc.target() = S + A;
  return X86Relocator::OK;
}

// R_X86_64_GOTPCREL: GOT(S) + GOT_ORG + A - P
X86Relocator::Result gotpcrel(Relocation& pReloc, X86_64Relocator& pParent)
{
  if (!(pReloc.symInfo()->reserved() & X86Relocator::ReserveGOT)) {
    return X86Relocator::BadReloc;
  }

  // set symbol value of the got entry if needed
  X86_64GOTEntry* got_entry = pParent.getSymGOTMap().lookUp(*pReloc.symInfo());
  if (X86Relocator::SymVal == got_entry->getValue())
    got_entry->setValue(pReloc.symValue());

  // setup relocation addend if needed
  Relocation* dyn_rel = pParent.getRelRelMap().lookUp(pReloc);
  if ((NULL != dyn_rel) && (X86Relocator::SymVal == dyn_rel->addend())) {
    dyn_rel->setAddend(pReloc.symValue());
  }

  X86Relocator::Address GOT_S   = helper_get_GOT_address(pReloc, pParent);
  Relocator::DWord      A       = pReloc.target() + pReloc.addend();
  X86Relocator::Address GOT_ORG = helper_GOT_ORG(pParent);
  // Apply relocation.
  pReloc.target() = GOT_S + GOT_ORG + A - pReloc.place();
  return X86Relocator::OK;
}

// R_X86_64_PLT32: PLT(S) + A - P
X86Relocator::Result plt32(Relocation& pReloc, X86_64Relocator& pParent)
{
  // PLT_S depends on if there is a PLT entry.
  X86Relocator::Address PLT_S;
  if ((pReloc.symInfo()->reserved() & X86Relocator::ReservePLT))
    PLT_S = helper_get_PLT_address(*pReloc.symInfo(), pParent);
  else
    PLT_S = pReloc.symValue();
  Relocator::DWord      A = pReloc.target() + pReloc.addend();
  X86Relocator::Address P = pReloc.place();
  pReloc.target() = PLT_S + A - P;
  return X86Relocator::OK;
}

// R_X86_64_PC32: S + A - P
// R_X86_64_PC16
// R_X86_64_PC8
X86Relocator::Result rel(Relocation& pReloc, X86_64Relocator& pParent)
{
  ResolveInfo* rsym = pReloc.symInfo();
  Relocator::DWord A = pReloc.target() + pReloc.addend();
  Relocator::DWord S = pReloc.symValue();
  Relocator::DWord P = pReloc.place();

  LDSection& target_sect = pReloc.targetRef().frag()->getParent()->getSection();
  // If the flag of target section is not ALLOC, we will not scan this relocation
  // but perform static relocation. (e.g., applying .debug section)
  if (0x0 == (llvm::ELF::SHF_ALLOC & target_sect.flag())) {
    pReloc.target() = S + A - P;
    return X86Relocator::OK;
  }

  // setup relocation addend if needed
  Relocation* dyn_rel = pParent.getRelRelMap().lookUp(pReloc);
  if ((NULL != dyn_rel) && (X86Relocator::SymVal == dyn_rel->addend())) {
    dyn_rel->setAddend(S);
  }

  // An external symbol may need PLT and dynamic relocation
  if (!rsym->isLocal()) {
    if (rsym->reserved() & X86Relocator::ReservePLT) {
       S = helper_get_PLT_address(*rsym, pParent);
    }
    if (pParent.getTarget().symbolNeedsDynRel(
                              *rsym,
                              (rsym->reserved() & X86Relocator::ReservePLT),
                              false)) {
      return X86Relocator::Overflow;
    }
  }

   // perform static relocation
  pReloc.target() = S + A - P;
  return X86Relocator::OK;
}

X86Relocator::Result unsupport(Relocation& pReloc, X86_64Relocator& pParent)
{
  return X86Relocator::Unsupport;
}
