#ifndef LLVM_NATIVE_CONFIG_H

/*===-- llvm/config/llvm-native-config.h --------------------------*- C -*-===*/
/*                                                                            */
/*                     The LLVM Compiler Infrastructure                       */
/*                                                                            */
/* This file is distributed under the University of Illinois Open Source      */
/* License. See LICENSE.TXT for details.                                      */
/*                                                                            */
/*===----------------------------------------------------------------------===*/

#if defined(__i386__) || defined(__x86_64__)

/* Host triple LLVM will be executed on */
#define LLVM_HOST_TRIPLE "i686-unknown-linux"

/* LLVM architecture name for the native architecture, if available */
#define LLVM_NATIVE_ARCH X86

/* LLVM name for the native AsmParser init function, if available */
#define LLVM_NATIVE_ASMPARSER LLVMInitializeX86AsmParser

/* LLVM name for the native AsmPrinter init function, if available */
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeX86AsmPrinter

/* LLVM name for the native Disassembler init function, if available */
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeX86Disassembler

/* LLVM name for the native Target init function, if available */
#define LLVM_NATIVE_TARGET LLVMInitializeX86Target

/* LLVM name for the native TargetInfo init function, if available */
#define LLVM_NATIVE_TARGETINFO LLVMInitializeX86TargetInfo

/* LLVM name for the native target MC init function, if available */
#define LLVM_NATIVE_TARGETMC LLVMInitializeX86TargetMC


#elif defined(__arm__)

/* Host triple LLVM will be executed on */
#define LLVM_HOST_TRIPLE "armv7-none-linux-gnueabi"

/* LLVM architecture name for the native architecture, if available */
#define LLVM_NATIVE_ARCH ARM

/* LLVM name for the native AsmParser init function, if available */
#define LLVM_NATIVE_ASMPARSER LLVMInitializeARMAsmParser

/* LLVM name for the native AsmPrinter init function, if available */
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeARMAsmPrinter

/* LLVM name for the native Disassembler init function, if available */
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeARMDisassembler

/* LLVM name for the native Target init function, if available */
#define LLVM_NATIVE_TARGET LLVMInitializeARMTarget

/* LLVM name for the native TargetInfo init function, if available */
#define LLVM_NATIVE_TARGETINFO LLVMInitializeARMTargetInfo

/* LLVM name for the native target MC init function, if available */
#define LLVM_NATIVE_TARGETMC LLVMInitializeARMTargetMC


#elif defined(__mips__)

/* Host triple LLVM will be executed on */
#define LLVM_HOST_TRIPLE "mipsel-none-linux-gnueabi"

/* LLVM architecture name for the native architecture, if available */
#define LLVM_NATIVE_ARCH Mips

/* LLVM name for the native AsmParser init function, if available */
#define LLVM_NATIVE_ASMPARSER LLVMInitializeMipsAsmParser

/* LLVM name for the native AsmPrinter init function, if available */
#define LLVM_NATIVE_ASMPRINTER LLVMInitializeMipsAsmPrinter

/* LLVM name for the native Disassembler init function, if available */
#define LLVM_NATIVE_DISASSEMBLER LLVMInitializeMipsDisassembler

/* LLVM name for the native Target init function, if available */
#define LLVM_NATIVE_TARGET LLVMInitializeMipsTarget

/* LLVM name for the native TargetInfo init function, if available */
#define LLVM_NATIVE_TARGETINFO LLVMInitializeMipsTargetInfo

/* LLVM name for the native target MC init function, if available */
#define LLVM_NATIVE_TARGETMC LLVMInitializeMipsTargetMC


#else

#error "Unknown native architecture"

#endif



#if defined(_WIN32) || defined(_WIN64)

/* Define if this is Unixish platform */
/* #undef LLVM_ON_UNIX */

/* Define if this is Win32ish platform */
#define LLVM_ON_WIN32 1

/* Define to 1 if you have the <windows.h> header file. */
#define HAVE_WINDOWS_H 1

/* Define to 1 if you have the `psapi' library (-lpsapi). */
#define HAVE_LIBPSAPI 1

/* Define to 1 if you have the `imagehlp' library (-limagehlp). */
#define HAVE_LIBIMAGEHLP 1

/* Type of 1st arg on ELM Callback */
#define WIN32_ELMCB_PCSTR PSTR


#else /* Linux, Mac OS X, ... Unixish platform */

/* Define if this is Unixish platform */
#define LLVM_ON_UNIX 1

/* Define if this is Win32ish platform */
/* #undef LLVM_ON_WIN32 */

/* Type of 1st arg on ELM Callback */
/* #undef WIN32_ELMCB_PCSTR */

#endif

#endif // LLVM_NATIVE_CONFIG_H
