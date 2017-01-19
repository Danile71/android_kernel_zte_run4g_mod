/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.passpoint;

import android.util.Log;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;

import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerConfigurationException;


import com.mediatek.common.passpoint.WifiTree;

import org.w3c.dom.*;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;



/**
 * A helper class for accessing the raw data in the intent extra and handling
 * certificates.
 */
class PasspointPpsmoParser {
    private static final String TAG = "PasspointPpsmoParser";

    private String UpdateIdentifier;
    private WifiTree wifiTree; 
    private WifiTreeHelper treeHelper = new WifiTreeHelper();       

    PasspointPpsmoParser() {
        UpdateIdentifier = "0";
        wifiTree = new WifiTree();                    
    }

    PasspointPpsmoParser(WifiTree tree) {
        UpdateIdentifier = "0";
        wifiTree = tree;                    
    }
  
    public static Document getDocument(String XML){

        try{        
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setCoalescing(true);
            factory.setNamespaceAware(true);
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = null;

            XML = tearDownSpecialChars(XML);
            Log.d(TAG, "parseXML:" + XML);

            StringReader sr = new StringReader(XML);
            InputSource is = new InputSource(sr);

            try{
                doc=(Document) builder.parse(is);
            } catch (IOException e) {
                Log.e(TAG, "getDocument IOException:" + e);
            } catch (SAXException e) {
                Log.e(TAG, "getDocument SAXException:" + e);
            }

            return doc;
        }catch(Exception e){
            Log.e(TAG, "getDocument err:" + e);
        }

        return null;
    }

    public static Vector<Document> getSPPNodes(Document doc, String namespace, String sTag){
        Vector<Document> sppNodes = new Vector<Document>();
        NodeList tagElements = doc.getElementsByTagNameNS(namespace, sTag); 
        if(tagElements.getLength() != 0){
            try{
                for(int i = 0; i < tagElements.getLength(); i++){
                    Node nNode = tagElements.item(i);
                    Document newXmlDocument = DocumentBuilderFactory.newInstance()
                            .newDocumentBuilder().newDocument();
                    Element root = newXmlDocument.createElementNS(namespace, "root");
                    newXmlDocument.appendChild(root);
                                   
                    Node copyNode = newXmlDocument.importNode(nNode, true);
                    root.appendChild(copyNode);
                    
                    sppNodes.add(newXmlDocument);
                }                
            }catch(Exception e){
                Log.e(TAG, "getSPPNodes err:" + e);
            }            
            return sppNodes;
        }
        return null;
    }

    private static String tearDownSpecialChars(String XML){
        //restore escaping symbol first (format v6)
        XML = XML.replaceAll("&lt;","<");
        XML = XML.replaceAll("&gt;",">");
        XML = XML.replaceAll("\\Q<![CDATA[\\E","");
        XML = XML.replaceAll("\\Q]]>\\E","");

        return XML;
    }

    public static String getTagValue(String sTag, Element eElement) {        
        try{
            NodeList tagElements = eElement.getElementsByTagName(sTag);
            if(tagElements != null && tagElements.item(0) != null){
                NodeList nlList = tagElements.item(0).getChildNodes();
                Node nValue = (Node) nlList.item(0);
                
                return nValue.getNodeValue();
            }else{
                return null;
            }

        }catch(Exception e){
            Log.e(TAG, "getTagValue err:" + e);
        }        
        return null;
    }

	public static String getTagValue(String namespace, String sTag, Element eElement) {
        try{
            NodeList tagElements = eElement.getElementsByTagNameNS(namespace, sTag); 
            if(tagElements != null && tagElements.item(0) != null){
                NodeList nlList = tagElements.item(0).getChildNodes();
                Node nValue = (Node) nlList.item(0);
                
                return nValue.getNodeValue();
            }else{
                return null;
            }

        }catch(Exception e){
            Log.e(TAG, "getTagValue err:" + e);
        }
        return null;
    }

    public static Document extractMgmtTree(String XML){
        try{
            Document doc = getDocument(XML);
            Document newXmlDocument = DocumentBuilderFactory.newInstance()
                    .newDocumentBuilder().newDocument();
            Element root = newXmlDocument.createElement("MgmtTree");
            newXmlDocument.appendChild(root);

            NodeList verDtd = doc.getElementsByTagName("VerDTD");
            Node copyNode = newXmlDocument.importNode(verDtd.item(0), true);
            root.appendChild(copyNode);

            NodeList nodes = doc.getElementsByTagName("Node");
            copyNode = newXmlDocument.importNode(nodes.item(0), true);
            root.appendChild(copyNode);

            return newXmlDocument;
        }catch(Exception e){
            Log.e(TAG, "extractMgmtTree err:" + e);
        }
        return null;
    }

    public static Document extractMgmtTree(Document doc){
        try{
            Document newXmlDocument = DocumentBuilderFactory.newInstance()
                    .newDocumentBuilder().newDocument();
            Element root = newXmlDocument.createElement("MgmtTree");
            newXmlDocument.appendChild(root);

            NodeList verDtd = doc.getElementsByTagName("VerDTD");
            Node copyNode = newXmlDocument.importNode(verDtd.item(0), true);
            root.appendChild(copyNode);

            NodeList nodes = doc.getElementsByTagName("Node");
            copyNode = newXmlDocument.importNode(nodes.item(0), true);
            root.appendChild(copyNode);

            return newXmlDocument;
        }catch(Exception e){
            Log.e(TAG, "extractMgmtTree err:" + e);
        }
        return null;
    }   

	public static String xmlToString(Document doc) {
        try {
            TransformerFactory tf = TransformerFactory.newInstance();
            Transformer transformer = tf.newTransformer();
            transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
            StringWriter writer = new StringWriter();
            transformer.transform(new DOMSource(doc), new StreamResult(writer));
            String output = removeComment(writer.getBuffer().toString().replaceAll("\n|\r", ""));
            return output;
        } catch (TransformerConfigurationException e) {
            Log.e(TAG, "xmlToString TransformerConfigurationException:" + e);
        } catch (TransformerException e) {
            Log.e(TAG, "xmlToString TransformerException:" + e);
        }
        return null;
    }

    private static String removeComment(String XML){
        XML = XML.replaceAll("(?s)<!--.*?-->", "");

        return XML;
    }
}
