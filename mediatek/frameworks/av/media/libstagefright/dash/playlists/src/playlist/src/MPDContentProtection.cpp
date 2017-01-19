/*----------------------------------------------------------------------------*
* Copyright Statement:                                                       *
*                                                                            *
*   This software/firmware and related documentation ("MediaTek Software")   *
* are protected under international and related jurisdictions'copyright laws *
* as unpublished works. The information contained herein is confidential and *
* proprietary to MediaTek Inc. Without the prior written permission of       *
* MediaTek Inc., any reproduction, modification, use or disclosure of        *
* MediaTek Software, and information contained herein, in whole or in part,  *
* shall be strictly prohibited.                                              *
* MediaTek Inc. Copyright (C) 2010. All rights reserved.                     *
*                                                                            *
*   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND     *
* AGREES TO THE FOLLOWING:                                                   *
*                                                                            *
*   1)Any and all intellectual property rights (including without            *
* limitation, patent, copyright, and trade secrets) in and to this           *
* Software/firmware and related documentation ("MediaTek Software") shall    *
* remain the exclusive property of MediaTek Inc. Any and all intellectual    *
* property rights (including without limitation, patent, copyright, and      *
* trade secrets) in and to any modifications and derivatives to MediaTek     *
* Software, whoever made, shall also remain the exclusive property of        *
* MediaTek Inc.  Nothing herein shall be construed as any transfer of any    *
* title to any intellectual property right in MediaTek Software to Receiver. *
*                                                                            *
*   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its *
* representatives is provided to Receiver on an "AS IS" basis only.          *
* MediaTek Inc. expressly disclaims all warranties, expressed or implied,    *
* including but not limited to any implied warranties of merchantability,    *
* non-infringement and fitness for a particular purpose and any warranties   *
* arising out of course of performance, course of dealing or usage of trade. *
* MediaTek Inc. does not provide any warranty whatsoever with respect to the *
* software of any third party which may be used by, incorporated in, or      *
* supplied with the MediaTek Software, and Receiver agrees to look only to   *
* such third parties for any warranty claim relating thereto.  Receiver      *
* expressly acknowledges that it is Receiver's sole responsibility to obtain *
* from any third party all proper licenses contained in or delivered with    *
* MediaTek Software.  MediaTek is not responsible for any MediaTek Software  *
* releases made to Receiver's specifications or to conform to a particular   *
* standard or open forum.                                                    *
*                                                                            *
*   3)Receiver further acknowledge that Receiver may, either presently       *
* and/or in the future, instruct MediaTek Inc. to assist it in the           *
* development and the implementation, in accordance with Receiver's designs, *
* of certain softwares relating to Receiver's product(s) (the "Services").   *
* Except as may be otherwise agreed to in writing, no warranties of any      *
* kind, whether express or implied, are given by MediaTek Inc. with respect  *
* to the Services provided, and the Services are provided on an "AS IS"      *
* basis. Receiver further acknowledges that the Services may contain errors  *
* that testing is important and it is solely responsible for fully testing   *
* the Services and/or derivatives thereof before they are used, sublicensed  *
* or distributed. Should there be any third party action brought against     *
* MediaTek Inc. arising out of or relating to the Services, Receiver agree   *
* to fully indemnify and hold MediaTek Inc. harmless.  If the parties        *
* mutually agree to enter into or continue a business relationship or other  *
* arrangement, the terms and conditions set forth herein shall remain        *
* effective and, unless explicitly stated otherwise, shall prevail in the    *
* event of a conflict in the terms in any agreements entered into between    *
* the parties.                                                               *
*                                                                            *
*   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and    *
* cumulative liability with respect to MediaTek Software released hereunder  *
* will be, at MediaTek Inc.'s sole discretion, to replace or revise the      *
* MediaTek Software at issue.                                                *
*                                                                            *
*   5)The transaction contemplated hereunder shall be construed in           *
* accordance with the laws of Singapore, excluding its conflict of laws      *
* principles.  Any disputes, controversies or claims arising thereof and     *
* related thereto shall be settled via arbitration in Singapore, under the   *
* then current rules of the International Chamber of Commerce (ICC).  The    *
* arbitration shall be conducted in English. The awards of the arbitration   *
* shall be final and binding upon both parties and shall be entered and      *
* enforceable in any court of competent jurisdiction.                        *
*---------------------------------------------------------------------------*/

#include "MPDContentProtection.h"
#include <stdlib.h>
using namespace android_dash;
using namespace xercesc;

MPDContentProtection::MPDContentProtection()
{
    mType = CP_DRM_NONE;
    mIsEncrypted = false;
    mIVSize = 0;
	mIsTemplate = false;
}

MPDContentProtection::~MPDContentProtection()
{
    mKids.clear();
}

bool MPDContentProtection::parserPc(MPDSchema::CContentProtectionType *content)
{
    assert(content != NULL);

    // firstly for ML DRM information setup
    if(content->MarlinBroadband.exists())
    {
    	MPD_ERROR("BB");
        mType = CP_DRM_ML_BB;
		assert(content->MarlinBroadband.count() > 0);
		MPDSchema::CMarlinBroadbandType *ptr = &content->MarlinBroadband[0];
		if(ptr->PreviewRightsUrl.exists())
		{
		    mPreviewUrl = (std::string)ptr->PreviewRightsUrl[0];
		}

		if(ptr->SilentRightsUrl.exists())
		{
		    mSilentUrl = (std::string)ptr->SilentRightsUrl[0];
		}

		if(ptr->RightsIssuerUrl.exists())
		{
		    mRightUrl = (std::string)ptr->RightsIssuerUrl[0];
		}
    }
    else if(content->MS3.exists())
    {
    	MPD_ERROR("MS3");
        mType = CP_DRM_ML_MS3;
		assert(content->MS3.count() > 0);
		MPDSchema::CMS3Type *ptr = &content->MS3[0];
		if(ptr->URIsAreTemplate.exists())
		{
			MPD_ERROR("template");
		    std::string temp = (std::string)ptr->URIsAreTemplate[0];
			MPD_ERROR("template is %s", temp.c_str());
			if(temp.compare("true") == 0)
			{
			    mIsTemplate = true;
			}
		}
    }
	else if(content->MarlinContentIds.exists())
	{
	    MPD_ERROR("kid exists for default BB");
		mType = CP_DRM_ML_BB;
	}

    if(mType != CP_DRM_NONE)
    {
        if(content->MarlinContentIds.exists()
            && content->MarlinContentIds[0].MarlinContentId.exists())
        {
            uint32_t count = content->MarlinContentIds[0].MarlinContentId.count();
            uint32_t index = 0;

            for(; index < count; ++ index)
            {
                std::string data = (std::string)content->MarlinContentIds[0].MarlinContentId[index];
                mKids.push_back(data);
            }

            if(content->schemeIdUri.exists())
            {
                mScheme = (std::string)content->schemeIdUri;
            }

            return true;
        }
		else
        {
            MPD_ERROR("it is impossible, check MPD xml");
            return false;
        }
    }


    // OH playready data check
    DOMNode* ContentProtectionNode = content->GetNode();
    XMLCh* TAG_ContentProtection = XMLString::transcode("ContentProtection");
    XMLCh* ATTR_schemeIdUri = XMLString::transcode("schemeIdUri");
    XMLCh* TAG_msprpro1 = XMLString::transcode("mspr:pro");
    XMLCh* TAG_msprpro2 = XMLString::transcode("mspr:prheader");
    XMLCh* TAG_IVsize = XMLString::transcode("mspr:IV_size");
    XMLCh* TAG_IsEncrypted = XMLString::transcode("mspr:IsEncrypted");
    XMLCh* TAG_kid = XMLString::transcode("mspr:kid");

    char *scheme = NULL;
    char *prodata = NULL;
    
    std::string playready_uuid_1_0 = std::string("urn:uuid:79f0049a-4098-8642-ab92-e65be0885f95");
    std::string playready_uuid_1_1 = std::string("urn:uuid:9a04f079-9840-4286-ab92-e65be0885f95");
    
    if(ContentProtectionNode->getNodeType() && 
        ContentProtectionNode->getNodeType() == DOMNode::ELEMENT_NODE)
    {
        xercesc::DOMElement* ContentProtectionElement
                    = (xercesc::DOMElement*)ContentProtectionNode;

        if(XMLString::equals(ContentProtectionElement->getTagName(), TAG_ContentProtection))
        {
            const XMLCh* xmlch_schemeIdUri
                 = ContentProtectionElement->getAttribute(ATTR_schemeIdUri);
            char *scheme = XMLString::transcode(xmlch_schemeIdUri);
            mScheme = std::string(scheme);

            //for playready_20
            if (mScheme.compare(playready_uuid_1_0) == 0 
                || mScheme.compare(playready_uuid_1_1) == 0)
            {
                MPD_ERROR("Playready");
                mType = CP_DRM_PR;
                DOMNodeList* prochildrenlist = ContentProtectionElement->getChildNodes();
                XMLSize_t nodeCount = prochildrenlist->getLength();
                for(XMLSize_t i = 0; i < nodeCount; ++i)
                {
                    xercesc::DOMNode* proElementNode = prochildrenlist->item(i);
                    if( proElementNode->getNodeType() &&  // true is not NULL
                        proElementNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
                    {
                        xercesc::DOMElement* proElement = (xercesc::DOMElement*)proElementNode;
                        if(XMLString::equals(proElement->getTagName(), TAG_msprpro1) 
                            || XMLString::equals(proElement->getTagName(), TAG_msprpro2))
                        {
                            const XMLCh* xmlPro = proElement->getTextContent();
                            prodata = XMLString::transcode(xmlPro);
                            
                            mPro = std::string(prodata);
                            
                            if(prodata != NULL)
                            {
                                XMLString::release( &prodata);
                            }
                        }
                        else if(XMLString::equals(proElement->getTagName(), TAG_IVsize))
                        {
                            const XMLCh* xmlPro = proElement->getTextContent();
                            prodata = XMLString::transcode(xmlPro);
                            mIVSize = (uint32_t)atoi(prodata);

                            if(prodata != NULL)
                            {
                                XMLString::release( &prodata);
                            }
                        }
                        else if(XMLString::equals(proElement->getTagName(), TAG_IsEncrypted))
                        {
                            const XMLCh* xmlPro = proElement->getTextContent();
                            prodata = XMLString::transcode(xmlPro);
                            mIsEncrypted = (uint32_t)atoi(prodata);
                            if(prodata != NULL)
                            {
                                XMLString::release( &prodata);
                            }
                        }
                        else if(XMLString::equals(proElement->getTagName(), TAG_kid))
                        {
                            const XMLCh* xmlPro = proElement->getTextContent();
                            prodata = XMLString::transcode(xmlPro);
                            std::string data = std::string(prodata);
                            mKids.push_back(data);
                            if(prodata != NULL)
                            {
                                XMLString::release( &prodata);
                            }
                        }
                    }
                }
            }
        }
    }

    try
    {
        XMLString::release( &TAG_ContentProtection);
        XMLString::release( &ATTR_schemeIdUri);
        XMLString::release( &TAG_msprpro1);
        XMLString::release( &TAG_msprpro2);
        XMLString::release( &TAG_IsEncrypted);
        XMLString::release( &TAG_IVsize);
        XMLString::release( &TAG_kid);
        
        if(scheme != NULL)
        {
            XMLString::release( &scheme);
        }
    }
    catch( ... )
    {
        MPD_PARSER_DBG(("Unknown exception encountered in TagNamesdtor \n"));

        return false;
    }

	///TODO:: how handle widevine?? no spec found

    return true;
}


#if MPD_PLAYLISTS_DEBUG
void MPDContentProtection::cpDebug()
{
	MPD_ERROR("drm debug");
    if(mType == CP_DRM_PR)
    {
	    MPD_DEBUG("type = %d isEncrypted = %d",
			mType, mIsEncrypted);

		MPD_DEBUG("iv size = %d scheme = %s", mIVSize, mScheme.c_str());
		MPD_DEBUG("pro = %s", mPro.c_str());
    }

	uint32_t count = mKids.size();
	uint32_t index = 0;

	for(; index < count ; ++ index)
	{
	    MPD_DEBUG("kid = %s", mKids[index].c_str());
	}

	if(mType == CP_DRM_ML_BB)
	{
	    MPD_DEBUG("silent = %s, preview = %s, right = %s",
			mSilentUrl.c_str(), mPreviewUrl.c_str(), mRightUrl.c_str());
	}

	if(mType == CP_DRM_ML_MS3)
	{
	    MPD_DEBUG("tempelate = %d", mIsTemplate);
	}
}
#endif

