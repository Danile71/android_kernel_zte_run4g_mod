/*----------------------------------------------------------------------------*
 * No Warranty                                                                *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MTK with respect to any MTK *
 * Deliverables or any use thereof, and MTK Deliverables are provided on an   *
 * "AS IS" basis.  MTK hereby expressly disclaims all such warranties,        *
 * including any implied warranties of merchantability, non-infringement and  *
 * fitness for a particular purpose and any warranties arising out of course  *
 * of performance, course of dealing or usage of trade.  Parties further      *
 * acknowledge that Company may, either presently and/or in the future,       *
 * instruct MTK to assist it in the development and the implementation, in    *
 * accordance with Company's designs, of certain softwares relating to        *
 * Company's product(s) (the "Services").  Except as may be otherwise agreed  *
 * to in writing, no warranties of any kind, whether express or implied, are  *
 * given by MTK with respect to the Services provided, and the Services are   *
 * provided on an "AS IS" basis.  Company further acknowledges that the       *
 * Services may contain errors, that testing is important and Company is      *
 * solely responsible for fully testing the Services and/or derivatives       *
 * thereof before they are used, sublicensed or distributed.  Should there be *
 * any third party action brought against MTK, arising out of or relating to  *
 * the Services, Company agree to fully indemnify and hold MTK harmless.      *
 * If the parties mutually agree to enter into or continue a business         *
 * relationship or other arrangement, the terms and conditions set forth      *
 * hereunder shall remain effective and, unless explicitly stated otherwise,  *
 * shall prevail in the event of a conflict in the terms in any agreements    *
 * entered into between the parties.                                          *
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Copyright (c) 2008, MediaTek, Inc.
 * All rights reserved.
 *
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclosure of this information in whole or in part is prohibited.
 *-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/

/** @file cli_command.c
 *  Add your description here.
 */

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

#include "cli.h"

//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------
//#define MAXCHARS	512
// char print_buf[MAXCHARS];
#if 0
#define I1BUFLEN 512
static char i1Buf[I1BUFLEN];
#endif
//-----------------------------------------------------------------------------
// global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Imported variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Imported functions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Static functions
//-----------------------------------------------------------------------------
/*
UINT32 StrToInt(const char* pszStr)
{
	return strtol(pszStr, NULL, 0);
}
*/

#if 0
/******************************************************************************
* Function      : _CmdMemRead
* Description   : memory read, word
******************************************************************************/
static INT32 _CmdMemRead(INT32 i4Argc, const char ** szArgv)
{
    UINT32 u4SrcAddr;
    UINT32 u4Len;
    UINT32 u4Idx;

    if ((i4Argc < 3) || (szArgv == NULL) || (szArgv[1] == NULL) || (szArgv[2] == NULL))
    {
        cli_print("r addr byte_nums, ex: r 0x80010000 0x100\r\n");
        return 0;
    }

    //u4SrcAddr = StrToInt(szArgv[1]);
    //u4SrcAddr = strtol(szArgv[1], NULL, 0);
    //u4Len = StrToInt(szArgv[2]);
    u4SrcAddr = strtoul(szArgv[1], NULL, 16);
    u4Len = strtoul(szArgv[2], NULL, 16);
//for debug
//    cli_print("szArgv[1]=%s, szArgv[2]=%s, u4SrcAddr=%x, u4Len=%x\r\n",szArgv[1],szArgv[2], u4SrcAddr, u4Len);
    if (u4Len == 0)
    {
        return 0;
    }
    if (u4Len > 0x1000)
    {
        u4Len = 0x1000;
    }

    // DW alignment
    u4SrcAddr &= 0xFFFFFFFC; 

    memset(&i1Buf[0],0, I1BUFLEN);
    for (u4Idx = 0; u4Idx < u4Len; u4Idx += 16)
    {
        //lint -e{613}

        //fue_cli_print("%08x | %08x %08x %08x %08x\r\n"
        sprintf(&i1Buf[0], "0x%08x | %08x %08x %08x %08x\r\n"
            , (u4SrcAddr + u4Idx)
            , DRV_Reg32(u4SrcAddr + u4Idx + 0)
            , DRV_Reg32(u4SrcAddr + u4Idx + 4)
            , DRV_Reg32(u4SrcAddr + u4Idx + 8)
            , DRV_Reg32(u4SrcAddr + u4Idx + 12));
        //cli_print("%s",&i1Buf[0]);
        cli_print("%s",&i1Buf[0]);
    }

    return 0;

}
/******************************************************************************
* Function      : _CmdMemWrite
* Description   : memory write, word
******************************************************************************/
static INT32 _CmdMemWrite(INT32 i4Argc, const char ** szArgv)
{
    UINT32 u4DestAddr;
    UINT32 u4Value;

    if ((i4Argc < 3) || (szArgv == NULL) || (szArgv[1] == NULL) || (szArgv[2] == NULL))
    {
        cli_print("w addr value, ex: w 0x80010708 0x60\r\n");
        return 0;
    }

    //u4DestAddr = StrToInt(szArgv[1]);
    //u4Value = StrToInt(szArgv[2]);
    u4DestAddr = strtoul(szArgv[1], NULL, 16);
    u4Value = strtoul(szArgv[2], NULL, 16);
//for debug
//     cli_print("szArgv[1]=%s, szArgv[2]=%s, u4DestAddr=%x, u4Value=%x\r\n",szArgv[1],szArgv[2], u4DestAddr, u4Value);

    // DW alignment
    u4DestAddr &= 0xFFFFFFFC;

    //ASSERT(u4DestAddr != NULL);
    assert(u4DestAddr != 0);
    DRV_WriteReg32(u4DestAddr, u4Value);
    //IO_WRITE32(0, u4DestAddr, u4Value);

    return 0;

}
#endif


//----------------------------------------------------------------------------- 
/** 
 * 
 *  @param  
 *  @retval   
 */
//-----------------------------------------------------------------------------
static INT32 _CmdDoNothing(INT32 i4Argc, const char ** szArgv)
{
    //UNUSED(i4Argc);
    //UNUSED(szArgv);
    return 0;
}


//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------



/******************************************************************************
* Variable      : cli default table
******************************************************************************/
//const CLI_EXEC_T _arDefaultCmdTbl[] CLI_MAIN_COMMAND =
const CLI_EXEC_T _arDefaultCmdTbl[] =
{
#if 0
    {
        "read",                     //pszCmdStr
        "r",
        _CmdMemRead,                    //execution function
        NULL,
        "Memory read(word)",
        CLI_GUEST
    },
    {
        "write",                    //pszCmdStr
        "w",
        _CmdMemWrite,               //execution function
        NULL,
        "Memory write(word)",
        CLI_GUEST
    },
#endif
#if 0
    {
        "basic_",                    //pszCmdStr
        "b",
        NULL,                       //execution function
        (CLI_EXEC_T *)_arBasicCmdTbl,
        "basic command",
        CLI_GUEST
    },
#endif    
    {
         "//",
         ";",
         _CmdDoNothing,
        NULL,
         "Line comment",
        CLI_SUPERVISOR
    },
       // last cli command record, NULL
    {
        NULL, NULL, NULL, NULL, NULL, CLI_SUPERVISOR
    }
};

//----------------------------------------------------------------------------- 
/** 
 *  
 *  @param  
 *  @param 
 *  @param  
 *  @retval  
 *  @retval 
 */
//-----------------------------------------------------------------------------
/******************************************************************************
* Function      : CLI_GetDefaultCmdTbl
* Description   : retrun default command table
******************************************************************************/
CLI_EXEC_T* CLI_GetDefaultCmdTbl(void)
{
    //??? UNUSED(_arNullCmdTbl);
    return (CLI_EXEC_T *)_arDefaultCmdTbl;
}
