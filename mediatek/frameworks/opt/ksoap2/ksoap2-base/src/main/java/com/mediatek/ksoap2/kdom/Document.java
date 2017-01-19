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
 */

/* Copyright (c) 2002,2003, Stefan Haustein, Oberhausen, Rhld., Germany
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The  above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE. */


package com.mediatek.ksoap2.kdom;

import java.io.*;

import org.xmlpull.v1.*;
/** The document consists of some legacy events and a single root
    element. This class basically adds some consistency checks to
    Node. */

public class Document extends Node {

    protected int rootIndex = -1;
    String encoding;
    Boolean standalone;

    /** returns "#document" */

    public String getEncoding () {
        return encoding;
    }

    public void setEncoding(String enc) {
        this.encoding = enc;
    }

    public void setStandalone (Boolean standalone) {
        this.standalone = standalone;
    }

    public Boolean getStandalone() {
        return standalone;
    }


    public String getName() {
        return "#document";
    }

    /** Adds a child at the given index position. Throws
    an exception when a second root element is added */

    public void addChild(int index, int type, Object child) {
        if (type == ELEMENT) {
         //   if (rootIndex != -1)
           //     throw new RuntimeException("Only one document root element allowed");

            rootIndex = index;
        }
        else if (rootIndex >= index)
            rootIndex++;

        super.addChild(index, type, child);
    }

    /** reads the document and checks if the last event
    is END_DOCUMENT. If not, an exception is thrown.
    The end event is consumed. For parsing partial
        XML structures, consider using Node.parse (). */

    public void parse(XmlPullParser parser)
        throws IOException, XmlPullParserException {

        parser.require(XmlPullParser.START_DOCUMENT, null, null);
        parser.nextToken ();

        encoding = parser.getInputEncoding();
        standalone = (Boolean)parser.getProperty ("http://xmlpull.org/v1/doc/properties.html#xmldecl-standalone");

        super.parse(parser);

        if (parser.getEventType() != XmlPullParser.END_DOCUMENT)
            throw new RuntimeException("Document end expected!");

    }

    public void removeChild(int index) {
        if (index == rootIndex)
            rootIndex = -1;
        else if (index < rootIndex)
            rootIndex--;

        super.removeChild(index);
    }

    /** returns the root element of this document. */

    public Element getRootElement() {
        if (rootIndex == -1)
            throw new RuntimeException("Document has no root element!");

        return (Element) getChild(rootIndex);
    }


    /** Writes this node to the given XmlWriter. For node and document,
        this method is identical to writeChildren, except that the
        stream is flushed automatically. */

    public void write(XmlSerializer writer)
        throws IOException {

        writer.startDocument(encoding, standalone);
        writeChildren(writer);
        writer.endDocument();
    }


}