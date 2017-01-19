//==- AArch64SchedA53.td - Cortex-A53 Scheduling Definitions -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the itinerary class data for the ARM Cortex A53 processors.
//
//===----------------------------------------------------------------------===//

// The first important issue is latency calculation.
// The formula is simple in getOperandLatency(): UseCycle = DefCycle - UseCycle + 1;
// This is suitable on no-forwading cases. For example, If Load produces on cycle 3,
// and add needs on cycle 1, UseCycle will be 3-1 + 1 = 3, that means we have three
// latency between two instructions (UseCycle default be 1 if no stall). This is why
// it + 1 in the formula.
//
// However, CA53 has forwarding for almost all cases, so, we should record the output
// cycle minus 1 to let it know we can use the result directly. I.e., UseCycle should
// be 2-1 + 1 = 2 (except for LATE_F5 cases).
//
// Not that the description doesn't accept multiple Bypass, i.e., [XXBypass | YYBypass],
// So we should model most forwarding rules by output cycle - 1, and only uses Bypass
// on special cases, e.g., MAC dedicated forwarding.


// The other important issue is pipeline stage model.
// CA53 integer has EARLY_ISS, LATE_ISS, EX1, EX2, EARLY_WR, LATE_WR
//      fp-neon                           F1,  F2,                F3, F4, F5, LATE_F5
//
// If we model [ISS, EX1, EX2, WR] to [0, 1, 2, 3], then it is hard to describe some
// forwarding, for example, EARLY_WR can forward to EARLY_ISS, and LATE_ISS, EX1, EX2,
// EARLY_WR, LATE_WR too, but LATE_WR can only for LATE_ISS, EX1, EX2, LATE_WR. It is
// complicated to split 1 stage to 2 early-late representation for each forwarding.
//
// We model [EARLY_ISS, LATE_ISS, EX1, EX2, WR] to [0, 1, 2, 3, 4], E.g, instruction "ldr"
// generates at EARLY_WR and the next "lsl" needs at LATE_ISS, and we can model "ldr" operand
// to 4-1, and "lsl" operand to 1, then the UseCycle will be (4-1)-1 + 1 = 3, that is
// what we want.
//
//       0    1      2      3      4      5      6
//       |    |      |      |      |      |      |
//       |    |      |      |      |      |      |
//      .''''''.''''''.''''''.''''''.''''''.''''''.
//       '''''' '''''' '''''' '''''' '''''' ''''''
//         ISS    EX1    EX2    WR
//                F1     F2     F3     F4     F5    LATE_F5
//
// The only special case is like "ldr" to "ldr", UseCycle will be (4-1)-0 + 1 = 4,
// but it should be 3, we can solve it by a Bypass.

def A53_Pipe0     : FuncUnit;
def A53_Pipe1     : FuncUnit;
def A53_PipeMac   : FuncUnit;
def A53_PipeDiv   : FuncUnit;
def A53_PipeStr   : FuncUnit;
def A53_PipeBr    : FuncUnit;
def A53_NPipeMul  : FuncUnit;
def A53_NPipeAlu  : FuncUnit;

def A53_EarlyBypass : Bypass;

def CortexA53Itineraries : ProcessorItineraries<
  /*list<FuncUnit>=*/
  [A53_Pipe0, A53_Pipe1, A53_PipeMac, A53_PipeDiv, A53_PipeStr, A53_PipeBr, A53_NPipeMul, A53_NPipeAlu],
  /*list<Bypass>=*/
  [A53_EarlyBypass],
  /*list<InstrItinData>=*/
  [
    /* General Class */
    InstrItinData<IIC_Br, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>]>,

    InstrItinData<IIC_Preload, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>],
                               [0], [A53_EarlyBypass]>,

    /* Integer Class */
    InstrItinData<IIC_iALUi,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iALUr,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 2]>,
    InstrItinData<IIC_iALUsi,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1]>,
    InstrItinData<IIC_iALUsir, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1]>,
    InstrItinData<IIC_iALUx,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>]>,

    InstrItinData<IIC_iBITi,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iBITr,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 2]>,
    InstrItinData<IIC_iBITsi, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1]>,
    InstrItinData<IIC_iBITsr, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1, 1]>,

    InstrItinData<IIC_iCMOVi,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [1]>,
    InstrItinData<IIC_iCMOVix2, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [3, 1]>,
    InstrItinData<IIC_iCMOVr,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iCMOVsi,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1]>,
    InstrItinData<IIC_iCMOVsr,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1, 1]>,

    InstrItinData<IIC_iCMPi, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2]>,
    InstrItinData<IIC_iCMPr, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iCMPsi,[InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1]>,
    InstrItinData<IIC_iCMPsr,[InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1, 1]>,

    InstrItinData<IIC_iDIV, [InstrStage<1, [A53_Pipe0]>, InstrStage<1, [A53_PipeDiv]>], [/*4-1+7*/10, 1, 1]>,

    InstrItinData<IIC_iEXTr,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iEXTAr, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1]>,
    InstrItinData<IIC_iEXTAsr,[InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1, 1]>,

    InstrItinData<IIC_iLoad_i,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0], [A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_iu  ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 2, 0], [A53_EarlyBypass, NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_bh_i,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0], [NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_bh_iu, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 2, 0], [NoBypass, NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_bh_r,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_bh_ru, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 2, 0, 0], [NoBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_bh_si, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_d_i,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 3, 0], [A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_d_r,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 3, 0, 0], [A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_d_ru,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 2, 0, 0], [A53_EarlyBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_r,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_ru,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 2, 0, 0], [A53_EarlyBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_si  ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iLoad_siu,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<2, [A53_PipeStr]>], [4, 3, 0, 0], [A53_EarlyBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,

    InstrItinData<IIC_iLoadiALU, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
                                  InstrStage<1, [A53_PipeStr]>,
                                  InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [5, 1], [NoBypass, A53_EarlyBypass]>,


    InstrItinData<IIC_iLoad_m,   [InstrStage<2, [A53_Pipe0], 0>, InstrStage<2, [A53_Pipe1]>, InstrStage<2, [A53_PipeStr]>],
                  [0, 0, 0, 0, 3], [A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass], -1>, // dynamic uops
    InstrItinData<IIC_iLoad_mu,  [InstrStage<3, [A53_Pipe0], 0>, InstrStage<3, [A53_Pipe1]>, InstrStage<3, [A53_PipeStr]>],
                  [2, 0, 0, 0, 3], [NoBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass], -1>, // dynamic uops
    InstrItinData<IIC_iLoad_mBr, [InstrStage<3, [A53_Pipe0], 0>, InstrStage<3, [A53_Pipe1]>, InstrStage<3, [A53_PipeStr]>, InstrStage<1, [A53_Pipe0, A53_Pipe1]>],
                  [2, 0, 0, 0, 3], [NoBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass, A53_EarlyBypass], -1>, // dynamic uops

    InstrItinData<IIC_iMAC16, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeMac]>], [3, 1, 1, 1]>,
    InstrItinData<IIC_iMAC32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeMac]>], [3, 1, 1, 1]>,
    InstrItinData<IIC_iMAC64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeMac]>], [4, 4, 1, 1, 1, 1]>,

    InstrItinData<IIC_iMOVi,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [1]>,
    InstrItinData<IIC_iMOVix2,[InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2]>,
    InstrItinData<IIC_iMOVix2addpc,[InstrStage<1, [A53_Pipe0, A53_Pipe1]> ,
                                    InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
                                    InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [3]>,

    InstrItinData<IIC_iMOVix2ld, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>],
                                 [5]>,

    InstrItinData<IIC_iMOVr,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iMOVsi, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1]>,
    InstrItinData<IIC_iMOVsr, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2, 1]>,

    InstrItinData<IIC_iMUL16, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeMac]>], [3, 1, 1]>,
    InstrItinData<IIC_iMUL32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeMac]>], [3, 1, 1]>,
    InstrItinData<IIC_iMUL64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeMac]>], [3, 3, 1, 1]>,

    InstrItinData<IIC_iMVNi , [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [1]>,
    InstrItinData<IIC_iMVNr , [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iMVNsi, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1]>,
    InstrItinData<IIC_iMVNsr, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1, 1]>,

    InstrItinData<IIC_iPop,    [InstrStage<1, [A53_Pipe0]>, InstrStage<3, [A53_PipeStr]>],
                               [1, 1, 3], [], -1>, // dynamic uops
    InstrItinData<IIC_iPop_Br, [InstrStage<1, [A53_Pipe0]>, InstrStage<3, [A53_PipeStr]>, InstrStage<1, [A53_Pipe0, A53_Pipe1]>],
                               [1, 1, 3], [], -1>, // dynamic uops

    InstrItinData<IIC_iStore_i,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0], [NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_iu,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [2, 3, 0], [NoBypass, NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_bh_i,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0], [NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_bh_iu, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [2, 3, 0], [NoBypass, NoBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_bh_r,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_bh_ru, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [2, 3, 0, 0], [NoBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_bh_si, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_d_r,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_d_ru,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [2, 3, 0, 0], [NoBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_r,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_ru,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [2, 3, 0, 0], [NoBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_si,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [3, 0, 0], [NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,
    InstrItinData<IIC_iStore_siu,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>], [2, 3, 0, 0], [NoBypass, NoBypass, A53_EarlyBypass, A53_EarlyBypass]>,

    InstrItinData<IIC_iStore_m , [InstrStage<2, [A53_Pipe0], 0>, InstrStage<2, [A53_Pipe1]>,
                                  InstrStage<2, [A53_PipeStr]>],
                  [], [], -1>, // dynamic uops
    InstrItinData<IIC_iStore_mu, [InstrStage<2, [A53_Pipe0], 0>, InstrStage<2, [A53_Pipe1]>,
                                  InstrStage<2, [A53_PipeStr]>],
                  [2], [], -1>, // dynamic uops

    InstrItinData<IIC_iTSTi, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2]>,
    InstrItinData<IIC_iTSTr, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iTSTsi,[InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1]>,
    InstrItinData<IIC_iTSTsr,[InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1, 1]>,

    InstrItinData<IIC_iUNAr,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 2]>,
    InstrItinData<IIC_iUNAsi, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>], [2, 1]>,

    /* Floating Class */
    InstrItinData<IIC_fpALU32, [InstrStage<1, [A53_Pipe0, A53_Pipe1], 0>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2, 2]>,
    InstrItinData<IIC_fpALU64, [InstrStage<1, [A53_Pipe0, A53_Pipe1], 0>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2, 2]>,

    InstrItinData<IIC_fpCMP32, [InstrStage<1, [A53_Pipe0, A53_Pipe1], 0>, InstrStage<1, [A53_NPipeAlu]>],
                               [2, 2]>,
    InstrItinData<IIC_fpCMP64, [InstrStage<1, [A53_Pipe0, A53_Pipe1], 0>, InstrStage<1, [A53_NPipeAlu]>],
                               [2, 2]>,

    InstrItinData<IIC_fpCVTDI, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTDS, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTHS, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTID, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTIS, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTSD, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTSH, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpCVTSI, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,

    InstrItinData<IIC_fpDIV32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeMul]>],
                               [/*5-1+15*/19, 2, 2]>,
    InstrItinData<IIC_fpDIV64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeMul]>],
                               [/*5-1+30*/34, 2, 2]>,

    InstrItinData<IIC_fpFMAC32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu], 0>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+7*/11, 3, 2, 2]>,
    InstrItinData<IIC_fpFMAC64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu], 0>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+7*/11, 3, 2, 2]>,

    InstrItinData<IIC_fpLoad32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>],
                                [4, 1]>,
    InstrItinData<IIC_fpLoad64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>],
                                [4, 1]>,

    InstrItinData<IIC_fpLoad_m, [InstrStage<1, [A53_Pipe0], 0>, InstrStage<1, [A53_Pipe1]>, InstrStage<4, [A53_PipeStr]>],
                                [1, 1, 1, 2], [], -1>,  // dynamic uops
    InstrItinData<IIC_fpLoad_mu,[InstrStage<1, [A53_Pipe0], 0>, InstrStage<1, [A53_Pipe1]>, InstrStage<4, [A53_PipeStr]>],
                                [2, 1, 1, 1, 2], [], -1>,  // dynamic uops

    InstrItinData<IIC_fpMAC32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu], 0>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+7*/11, 3, 2, 2]>,
    InstrItinData<IIC_fpMAC64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu], 0>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+7*/11, 3, 2, 2]>,

    InstrItinData<IIC_fpMOVDI, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                  [3, 3, 3]>,
    InstrItinData<IIC_fpMOVID, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                  [3, 3, 3]>,
    InstrItinData<IIC_fpMOVIS, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                  [3, 3]>,
    InstrItinData<IIC_fpMOVSI, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                  [3, 3]>,

    InstrItinData<IIC_fpMUL32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+3*/7, 3, 2, 2]>,
    InstrItinData<IIC_fpMUL64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+3*/7, 3, 2, 2]>,

    InstrItinData<IIC_fpSQRT32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+14*/18, 3, 2, 2]>,
    InstrItinData<IIC_fpSQRT64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeMul]>],
                                [/*5-1+29*/33, 3, 2, 2]>,

    InstrItinData<IIC_fpSTAT, [InstrStage<1, [A53_Pipe0], 0>, InstrStage<1, [A53_Pipe1]>]>,

    InstrItinData<IIC_fpStore32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>],
                                 [1, 1]>,
    InstrItinData<IIC_fpStore64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_PipeStr]>],
                                 [1, 1]>,

    InstrItinData<IIC_fpStore_m, [InstrStage<1, [A53_Pipe0], 0>, InstrStage<1, [A53_Pipe1]>, InstrStage<4, [A53_PipeStr]>],
                                 [1, 1, 1, 1], [], -1>, // dynamic uops
    InstrItinData<IIC_fpStore_mu,[InstrStage<1, [A53_Pipe0], 0>, InstrStage<1, [A53_Pipe1]>, InstrStage<4, [A53_PipeStr]>],
                                 [2, 1, 1, 1, 1], [], -1>, // dynamic uops

    InstrItinData<IIC_fpUNA32, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>,
    InstrItinData<IIC_fpUNA64, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>, InstrStage<1, [A53_NPipeAlu]>],
                               [5, 2]>

    /* NEON Class */ // TODO
    //// Issue through integer pipeline, and execute in NEON unit.
    ////
    //// VLD1
    //InstrItinData<IIC_VLD1,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1]>,
    //// VLD1x2
    //InstrItinData<IIC_VLD1x2,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 1]>,
    ////
    //// VLD1x3
    //InstrItinData<IIC_VLD1x3,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 1]>,
    ////
    //// VLD1x4
    //InstrItinData<IIC_VLD1x4,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 3, 1]>,
    ////
    //// VLD1u
    //InstrItinData<IIC_VLD1u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 1]>,
    ////
    //// VLD1x2u
    //InstrItinData<IIC_VLD1x2u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 2, 1]>,
    ////
    //// VLD1x3u
    //InstrItinData<IIC_VLD1x3u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 2, 1]>,
    ////
    //// VLD1x4u
    //InstrItinData<IIC_VLD1x4u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 3, 2, 1]>,
    ////
    //// VLD1ln
    //InstrItinData<IIC_VLD1ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [3, 1, 1, 1]>,
    ////
    //// VLD1lnu
    //InstrItinData<IIC_VLD1lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [3, 2, 1, 1, 1, 1]>,
    ////
    //// VLD1dup
    //InstrItinData<IIC_VLD1dup,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1]>,
    ////
    //// VLD1dupu
    //InstrItinData<IIC_VLD1dupu, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 1, 1]>,
    ////
    //// VLD2
    //InstrItinData<IIC_VLD2,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 1]>,
    ////
    //// VLD2x2
    //InstrItinData<IIC_VLD2x2,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 3, 1]>,
    ////
    //// VLD2ln
    //InstrItinData<IIC_VLD2ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [3, 3, 1, 1, 1, 1]>,
    ////
    //// VLD2u
    //InstrItinData<IIC_VLD2u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 2, 1, 1, 1]>,
    ////
    //// VLD2x2u
    //InstrItinData<IIC_VLD2x2u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 3, 2, 1]>,
    ////
    //// VLD2lnu
    //InstrItinData<IIC_VLD2lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [3, 3, 2, 1, 1, 1, 1, 1]>,
    ////
    //// VLD2dup
    //InstrItinData<IIC_VLD2dup,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 1]>,
    ////
    //// VLD2dupu
    //InstrItinData<IIC_VLD2dupu, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 2, 2, 1, 1]>,
    ////
    //// VLD3
    //InstrItinData<IIC_VLD3,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [3, 3, 4, 1]>,
    ////
    //// VLD3ln
    //InstrItinData<IIC_VLD3ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<5, [A53_PipeStr]>],
    //                            [4, 4, 5, 1, 1, 1, 1, 2]>,
    ////
    //// VLD3u
    //InstrItinData<IIC_VLD3u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [3, 3, 4, 2, 1]>,
    ////
    //// VLD3lnu
    //InstrItinData<IIC_VLD3lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<5, [A53_PipeStr]>],
    //                            [4, 4, 5, 2, 1, 1, 1, 1, 1, 2]>,
    ////
    //// VLD3dup
    //InstrItinData<IIC_VLD3dup,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 1]>,
    ////
    //// VLD3dupu
    //InstrItinData<IIC_VLD3dupu, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 2, 1, 1]>,
    ////
    //// VLD4
    //InstrItinData<IIC_VLD4,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [3, 3, 4, 4, 1]>,
    ////
    //// VLD4ln
    //InstrItinData<IIC_VLD4ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<5, [A53_PipeStr]>],
    //                            [4, 4, 5, 5, 1, 1, 1, 1, 2, 2]>,
    ////
    //// VLD4u
    //InstrItinData<IIC_VLD4u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [3, 3, 4, 4, 2, 1]>,
    ////
    //// VLD4lnu
    //InstrItinData<IIC_VLD4lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<5, [A53_PipeStr]>],
    //                            [4, 4, 5, 5, 2, 1, 1, 1, 1, 1, 2, 2]>,
    ////
    //// VLD4dup
    //InstrItinData<IIC_VLD4dup,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 3, 1]>,
    ////
    //// VLD4dupu
    //InstrItinData<IIC_VLD4dupu, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 2, 3, 3, 2, 1, 1]>,
    ////
    //// VST1
    //InstrItinData<IIC_VST1,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [1, 1, 1]>,
    ////
    //// VST1x2
    //InstrItinData<IIC_VST1x2,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [1, 1, 1, 1]>,
    ////
    //// VST1x3
    //InstrItinData<IIC_VST1x3,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2]>,
    ////
    //// VST1x4
    //InstrItinData<IIC_VST1x4,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2, 2]>,
    ////
    //// VST1u
    //InstrItinData<IIC_VST1u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1]>,
    ////
    //// VST1x2u
    //InstrItinData<IIC_VST1x2u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1]>,
    ////
    //// VST1x3u
    //InstrItinData<IIC_VST1x3u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2]>,
    ////
    //// VST1x4u
    //InstrItinData<IIC_VST1x4u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2, 2]>,
    ////
    //// VST1ln
    //InstrItinData<IIC_VST1ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [1, 1, 1]>,
    ////
    //// VST1lnu
    //InstrItinData<IIC_VST1lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1]>,
    ////
    //// VST2
    //InstrItinData<IIC_VST2,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [1, 1, 1, 1]>,
    ////
    //// VST2x2
    //InstrItinData<IIC_VST2x2,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2, 2]>,
    ////
    //// VST2u
    //InstrItinData<IIC_VST2u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1]>,
    ////
    //// VST2x2u
    //InstrItinData<IIC_VST2x2u,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2, 2]>,
    ////
    //// VST2ln
    //InstrItinData<IIC_VST2ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [1, 1, 1, 1]>,
    ////
    //// VST2lnu
    //InstrItinData<IIC_VST2lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1]>,
    ////
    //// VST3
    //InstrItinData<IIC_VST3,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2]>,
    ////
    //// VST3u
    //InstrItinData<IIC_VST3u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2]>,
    ////
    //// VST3ln
    //InstrItinData<IIC_VST3ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2]>,
    ////
    //// VST3lnu
    //InstrItinData<IIC_VST3lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<3, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2]>,
    ////
    //// VST4
    //InstrItinData<IIC_VST4,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2, 2]>,
    ////
    //// VST4u
    //InstrItinData<IIC_VST4u,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2, 2]>,
    ////
    //// VST4ln
    //InstrItinData<IIC_VST4ln,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [1, 1, 1, 1, 2, 2]>,
    ////
    //// VST4lnu
    //InstrItinData<IIC_VST4lnu,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<4, [A53_PipeStr]>],
    //                            [2, 1, 1, 1, 1, 1, 2, 2]>,
    ////
    //// Double-register FP Unary
    //InstrItinData<IIC_VUNAD,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [5, 2]>,
    ////
    //// Quad-register FP Unary
    //// Result written in N5, but that is relative to the last cycle of multicycle,
    //// so we use 6 for those cases
    //InstrItinData<IIC_VUNAQ,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [6, 2]>,
    ////
    //// Double-register FP Binary
    //InstrItinData<IIC_VBIND,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [5, 2, 2]>,
    ////
    //// VPADD, etc.
    //InstrItinData<IIC_VPBIND,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [5, 2, 2]>,
    ////
    //// Double-register FP VMUL
    //InstrItinData<IIC_VFMULD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [5, 2, 1]>,

    ////
    //// Quad-register FP Binary
    //// Result written in N5, but that is relative to the last cycle of multicycle,
    //// so we use 6 for those cases
    //InstrItinData<IIC_VBINQ,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [6, 2, 2]>,
    ////
    //// Quad-register FP VMUL
    //InstrItinData<IIC_VFMULQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [6, 2, 1]>,
    ////
    //// Move
    //InstrItinData<IIC_VMOV,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [1, 1]>,
    ////
    //// Move Immediate
    //InstrItinData<IIC_VMOVImm,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3]>,
    ////
    //// Double-register Permute Move
    //InstrItinData<IIC_VMOVD,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [2, 1]>,
    ////
    //// Quad-register Permute Move
    //// Result written in N2, but that is relative to the last cycle of multicycle,
    //// so we use 3 for those cases
    //InstrItinData<IIC_VMOVQ,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 1]>,
    ////
    //// Integer to Single-precision Move
    //InstrItinData<IIC_VMOVIS ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [2, 1]>,
    ////
    //// Integer to Double-precision Move
    //InstrItinData<IIC_VMOVID ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [2, 1, 1]>,
    ////
    //// Single-precision to Integer Move
    //InstrItinData<IIC_VMOVSI ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [20, 1]>,
    ////
    //// Double-precision to Integer Move
    //InstrItinData<IIC_VMOVDI ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [20, 20, 1]>,
    ////
    //// Integer to Lane Move
    //InstrItinData<IIC_VMOVISL , [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 1, 1]>,
    ////
    //// Vector narrow move
    //InstrItinData<IIC_VMOVN   , [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [2, 1]>,
    ////
    //// Double-register Permute
    //InstrItinData<IIC_VPERMD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [2, 2, 1, 1]>,
    ////
    //// Quad-register Permute
    //// Result written in N2, but that is relative to the last cycle of multicycle,
    //// so we use 3 for those cases
    //InstrItinData<IIC_VPERMQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 3, 1, 1]>,
    ////
    //// Quad-register Permute (3 cycle issue)
    //// Result written in N2, but that is relative to the last cycle of multicycle,
    //// so we use 4 for those cases
    //InstrItinData<IIC_VPERMQ3,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>,
    //                             InstrStage<1, [A53_NPipeAlu], 0>,
    //                             InstrStage<2, [A53_PipeStr]>], [4, 4, 1, 1]>,
    ////
    //// Double-register FP Multiple-Accumulate
    //InstrItinData<IIC_VMACD,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [9, 3, 2, 2]>,
    ////
    //// Quad-register FP Multiple-Accumulate
    //// Result written in N9, but that is relative to the last cycle of multicycle,
    //// so we use 10 for those cases
    //InstrItinData<IIC_VMACQ,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [10, 3, 2, 2]>,
    ////
    //// Double-register Fused FP Multiple-Accumulate
    //InstrItinData<IIC_VFMACD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [9, 3, 2, 2]>,
    ////
    //// Quad-register Fused FP Multiple-Accumulate
    //// Result written in N9, but that is relative to the last cycle of multicycle,
    //// so we use 10 for those cases
    //InstrItinData<IIC_VFMACQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [10, 3, 2, 2]>,
    ////
    //// Double-register Reciprical Step
    //InstrItinData<IIC_VRECSD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [9, 2, 2]>,
    ////
    //// Quad-register Reciprical Step
    //InstrItinData<IIC_VRECSQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [10, 2, 2]>,
    ////
    //// Double-register Integer Count
    //InstrItinData<IIC_VCNTiD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3, 2, 2]>,
    ////
    //// Quad-register Integer Count
    //// Result written in N3, but that is relative to the last cycle of multicycle,
    //// so we use 4 for those cases
    //InstrItinData<IIC_VCNTiQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [4, 2, 2]>,
    ////
    //// Double-register Integer Unary
    //InstrItinData<IIC_VUNAiD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 2]>,
    ////
    //// Quad-register Integer Unary
    //InstrItinData<IIC_VUNAiQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 2]>,
    ////
    //// Double-register Integer Q-Unary
    //InstrItinData<IIC_VQUNAiD,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 1]>,
    ////
    //// Quad-register Integer CountQ-Unary
    //InstrItinData<IIC_VQUNAiQ,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 1]>,
    ////
    //// Double-register Integer Binary
    //InstrItinData<IIC_VBINiD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3, 2, 2]>,
    ////
    //// Quad-register Integer Binary
    //InstrItinData<IIC_VBINiQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3, 2, 2]>,
    ////
    //// Double-register Integer Binary (4 cycle)
    //InstrItinData<IIC_VBINi4D,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 2, 1]>,
    ////
    //// Quad-register Integer Binary (4 cycle)
    //InstrItinData<IIC_VBINi4Q,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 2, 1]>,

    ////
    //// Double-register Integer Subtract
    //InstrItinData<IIC_VSUBiD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3, 2, 1]>,
    ////
    //// Quad-register Integer Subtract
    //InstrItinData<IIC_VSUBiQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3, 2, 1]>,
    ////
    //// Double-register Integer Subtract
    //InstrItinData<IIC_VSUBi4D,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 2, 1]>,
    ////
    //// Quad-register Integer Subtract
    //InstrItinData<IIC_VSUBi4Q,  [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 2, 1]>,
    ////
    //// Double-register Integer Shift
    //InstrItinData<IIC_VSHLiD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [3, 1, 1]>,
    ////
    //// Quad-register Integer Shift
    //InstrItinData<IIC_VSHLiQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [4, 1, 1]>,
    ////
    //// Double-register Integer Shift (4 cycle)
    //InstrItinData<IIC_VSHLi4D,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [4, 1, 1]>,
    ////
    //// Quad-register Integer Shift (4 cycle)
    //InstrItinData<IIC_VSHLi4Q,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [5, 1, 1]>,
    ////
    //// Double-register Integer Pair Add Long
    //InstrItinData<IIC_VPALiD,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [6, 3, 1]>,
    ////
    //// Quad-register Integer Pair Add Long
    //InstrItinData<IIC_VPALiQ,   [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [7, 3, 1]>,
    ////
    //// Double-register Absolute Difference and Accumulate
    //InstrItinData<IIC_VABAD,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [6, 3, 2, 1]>,
    ////
    //// Quad-register Absolute Difference and Accumulate
    //InstrItinData<IIC_VABAQ,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [6, 3, 2, 1]>,

    ////
    //// Double-register Integer Multiply (.8, .16)
    //InstrItinData<IIC_VMULi16D, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [6, 2, 2]>,
    ////
    //// Double-register Integer Multiply (.32)
    //InstrItinData<IIC_VMULi32D, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [7, 2, 1]>,
    ////
    //// Quad-register Integer Multiply (.8, .16)
    //InstrItinData<IIC_VMULi16Q, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [7, 2, 2]>,
    ////
    //// Quad-register Integer Multiply (.32)
    //InstrItinData<IIC_VMULi32Q, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>,
    //                             InstrStage<2, [A53_PipeStr], 0>,
    //                             InstrStage<3, [A53_NPipeAlu]>], [9, 2, 1]>,
    ////
    //// Double-register Integer Multiply-Accumulate (.8, .16)
    //InstrItinData<IIC_VMACi16D, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>], [6, 3, 2, 2]>,
    ////
    //// Double-register Integer Multiply-Accumulate (.32)
    //InstrItinData<IIC_VMACi32D, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [7, 3, 2, 1]>,
    ////
    //// Quad-register Integer Multiply-Accumulate (.8, .16)
    //InstrItinData<IIC_VMACi16Q, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_NPipeAlu]>], [7, 3, 2, 2]>,
    ////
    //// Quad-register Integer Multiply-Accumulate (.32)
    //InstrItinData<IIC_VMACi32Q, [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_NPipeAlu]>,
    //                             InstrStage<2, [A53_PipeStr], 0>,
    //                             InstrStage<3, [A53_NPipeAlu]>], [9, 3, 2, 1]>,
    ////
    //// Double-register VEXT
    //InstrItinData<IIC_VEXTD,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>], [2, 1, 1]>,
    ////
    //// Quad-register VEXT
    //InstrItinData<IIC_VEXTQ,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 1, 1]>,
    ////
    //// VTB
    //InstrItinData<IIC_VTB1,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 2, 1]>,
    //InstrItinData<IIC_VTB2,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 2, 2, 1]>,
    //InstrItinData<IIC_VTB3,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>,
    //                             InstrStage<1, [A53_NPipeAlu], 0>,
    //                             InstrStage<2, [A53_PipeStr]>], [4, 2, 2, 3, 1]>,
    //InstrItinData<IIC_VTB4,     [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>,
    //                             InstrStage<1, [A53_NPipeAlu], 0>,
    //                             InstrStage<2, [A53_PipeStr]>],[4, 2, 2, 3, 3, 1]>,
    ////
    //// VTBX
    //InstrItinData<IIC_VTBX1,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 1, 2, 1]>,
    //InstrItinData<IIC_VTBX2,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<2, [A53_PipeStr]>], [3, 1, 2, 2, 1]>,
    //InstrItinData<IIC_VTBX3,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>,
    //                             InstrStage<1, [A53_NPipeAlu], 0>,
    //                             InstrStage<2, [A53_PipeStr]>],[4, 1, 2, 2, 3, 1]>,
    //InstrItinData<IIC_VTBX4,    [InstrStage<1, [A53_Pipe0, A53_Pipe1]>,
    //                             InstrStage<1, [A53_PipeStr]>,
    //                             InstrStage<1, [A53_NPipeAlu], 0>,
    //                             InstrStage<2, [A53_PipeStr]>], [4, 1, 2, 2, 3, 3, 1]>
]>;

// ===---------------------------------------------------------------------===//
// This following definitions describe the simple machine model which
// will replace itineraries.

// Cortex-A53 machine model for scheduling and other instruction cost heuristics.
def CortexA53Model : SchedMachineModel {
  let MicroOpBufferSize = 0; // Explicitly set to zero since A53 is in-order.
  let IssueWidth = 2; // 2 micro-ops are dispatched per cycle.
  let MinLatency = 1; // OperandCycles are interpreted as MinLatency.
  let LoadLatency = 3; // Optimistic load latency assuming bypass.
                       // This is overriden by OperandCycles if the
                       // Itineraries are queried instead.
  let MispredictPenalty = 9; // Based on estimate of pipeline depth.

  let Itineraries = CortexA53Itineraries;
}
