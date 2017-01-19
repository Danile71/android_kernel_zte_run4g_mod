/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#include "mpdschema_prefix.h"
#include <string.h>
#include "MPDSchema.h"
#include <algorithm>
 
 
#include "XercesString.h"
#include "dash_static.h"
#include "MPDParser.h"
 
 
 
 
 // turn off warning: "this" used in base initializer list

// init xerces.
bool MPDParser::init()
{
    if(doc != NULL || mpd_ex != NULL)
    {
        MPD_PARSER_DBG(("destory then init\n"));
        return false;
    }
    
    try
    {
        xercesc::XMLPlatformUtils::Initialize();

        MPD_PARSER_DBG(("init ok\n"));
        return true;
    }
    catch (CXmlException& e)
    {
        MPD_PARSER_DBG(("Error: %s\n", e.GetInfo().c_str()));
        return false;

    }
    catch (xercesc::XMLException& e)
    {
        MPD_PARSER_DBG(("Error: %s %s %s\n", e.getSrcFile(), e.getSrcLine(), e.getMessage()));
        return false;
    }
    catch (xercesc::DOMException& e)
    {
        MPD_PARSER_DBG(("Error %d %s\n", e.code, e.msg));
        return false;
    }
}

// terminate xerces
MPDParser::~MPDParser()
{
    if(doc != NULL)
    {

        delete doc;
        doc = NULL;
    }

    if(mpd_ex  != NULL)
    {
        delete mpd_ex;
        mpd_ex = NULL;
    }

}

bool MPDParser::destory()
{
    
    try
    {
        if(doc != NULL )
        {

            doc->DestroyDocument();
            delete doc;
            doc = NULL;
        }

        if(mpd_ex  != NULL)
        {
            delete mpd_ex;
            mpd_ex = NULL;
        }
    
        xercesc::XMLPlatformUtils::Terminate();
        MPD_PARSER_DBG(("destroy ok\n"));
        return true;
    }
    catch (CXmlException& e)
    {
        MPD_PARSER_DBG(("Error: %s\n", e.GetInfo().c_str()));
        return false;

    }
    catch (xercesc::XMLException& e)
    {
        MPD_PARSER_DBG(("Error: %s %s %s\n", e.getSrcFile(), e.getSrcLine(), e.getMessage()));
        return false;
    }
    catch (xercesc::DOMException& e)
    {
        MPD_PARSER_DBG(("Error %d %s\n", e.code, e.msg));
        return false;
    }
    catch (altova::Exception& exception)
    {
        MPD_PARSER_DBG(("Error: %s\n",exception.message().c_str()));
        return false;
    }
    catch (altova::Exception& exception)
    {
        MPD_PARSER_DBG(("Error: %s\n",exception.message().c_str()));
        return false;
    }
    catch (altova::Error& exception)
    {
        MPD_PARSER_DBG(("Error: %s\n",exception.message().c_str()));
        return false;
    }
    catch (std::exception& e)
    {
        MPD_PARSER_DBG(("Error: %s\n",e.what()));
        return false;
    }
    catch (...)
    {
        MPD_PARSER_DBG(("unknow error\n"));
        return false;
    }
}



unsigned int MPDParser::get_miniupdatetime()
{
    if(mpd_ex == NULL)
    {
        MPD_PARSER_DBG(("error no mpd_ex pointer\n"));
        return 0;
    }

    if(mpd_ex->miniupdatetime_exists)
    {
        return mpd_ex->miniupdatetime_ex;
    }

    return 0;
}

unsigned int MPDParser::get_minbuffertime()
{
    if(mpd_ex == NULL)
    {
        MPD_PARSER_DBG(("error no mpd_ex pointer\n"));
        return 0;
    }

    if(mpd_ex->minbuffertime_exists)
    {
        return mpd_ex->minbuffertime_ex;
    }

    return 0;
}


std::string MPDParser::get_location()
{
    if(mpd_ex == NULL)
    {
        MPD_PARSER_DBG(("error no mpd_ex pointer\n"));
        return std::string("");
    }

    if(mpd_ex->location_exists)
    {
        return mpd_ex->location_ex;
    }

    return std::string("");
    
}


bool MPDParser::is_dynamic()
{
    if(mpd_ex == NULL)
    {
        return false;
    }

    if(mpd_ex->dynamic_type != dash_enum::MPD_TYPE_STATIC)
    {
        return true;
    }

    else
    {
        return false;
    }
}

bool MPDParser::is_specialDynamic()
{
    if(mpd_ex == NULL)
    {
        return false;
    }

    return mpd_ex->isSpecialDynamic;    
}

int MPDParser::get_mpdprofile()
{
    if(mpd_ex == NULL)
    {
        return 0;
    }

    return mpd_ex->profile_type;
}

mpd_parser_result MPDParser::parser(std::string& mpd_path, DataType type)
{
    if(mpd_path.length() == 0)
    {
        MPD_PARSER_DBG(("no mpd path to parse\n"));
        return mpd_parser_error;
    }

    // for dynamic parser .....

    try
    {
        if(doc != NULL)
        {

            doc->DestroyDocument();
            delete doc;
            doc = NULL;
        }

        if(mpd_ex  != NULL)
        {
            delete mpd_ex;
            mpd_ex = NULL;
        }

        if(type == MPD_FILE_PATH)
        {
            doc = MPDSchema::CMPDSchema::LoadFromFile(mpd_path);
        }
        else if(type == MPD_DATA_STRING)
        {
            doc = MPDSchema::CMPDSchema::LoadFromString(mpd_path);
        }

        if(doc == NULL)
        {
            MPD_PARSER_DBG(("xml parser error\n"));
            return mpd_parser_error;
        }
        
        if(!doc->MPD.exists())
        {
            MPD_PARSER_DBG(("mpd Parser failed\n"));
            return mpd_parser_error;
        }

        MPDSchema::CMPDtype root = doc->MPD.first();
        
        mpd_ex = new MPDSchema::CMPDtype_ex();
        mpd_ex->mpd_init(&root);
        mpd_ex->mpd_process();
        if(mpd_ex->empty())
        {
            return mpd_parser_empty;
        }
        
#if (MPD_PASER_DEBUG > 1)
        mpd_ex->mpd_verify(&root);
        std::string save_file = mpd_path.append("save");
        doc->SaveToFile(save_file, true);
#endif

       return mpd_parser_ok;
    }
    catch (CXmlException& e)
    {
        MPD_PARSER_DBG(("Error: %s\n", e.GetInfo().c_str()));
        return mpd_parser_error;

    }
    catch (xercesc::XMLException& e)
    {
        MPD_PARSER_DBG(("Error: %s %s %s\n", e.getSrcFile(), e.getSrcLine(), e.getMessage()));
        return mpd_parser_error;
    }
    catch (xercesc::DOMException& e)
    {
        MPD_PARSER_DBG(("Error %d %s\n", e.code, e.msg));
        return mpd_parser_error;
    }
    catch (altova::Exception& exception)
    {
        MPD_PARSER_DBG(("Error: %s\n",exception.message().c_str()));
        return mpd_parser_error;
    }
    catch (altova::Exception& exception)
    {
        MPD_PARSER_DBG(("Error: %s\n",exception.message().c_str()));
        return mpd_parser_error;
    }
    catch (altova::Error& exception)
    {
        MPD_PARSER_DBG(("Error: %s\n",exception.message().c_str()));
        return mpd_parser_error;
    }
    catch (std::exception& e)
    {
        MPD_PARSER_DBG(("Error: %s\n",e.what()));
        return mpd_parser_error;
    }
    catch (all_exceptions& e)
    {
        MPD_PARSER_DBG(("Error: %s\n",e.get_message().c_str()));
        return mpd_parser_error;
    }
    catch (...)
    {
        MPD_PARSER_DBG(("unknow error\n"));
        return mpd_parser_error;
    }
}