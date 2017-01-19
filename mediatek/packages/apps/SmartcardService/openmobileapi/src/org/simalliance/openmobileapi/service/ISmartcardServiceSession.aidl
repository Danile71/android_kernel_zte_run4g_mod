/*
 * Copyright (C) 2011, The Android Open Source Project
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
/*
 * Contributed by: Giesecke & Devrient GmbH.
 */

package org.simalliance.openmobileapi.service;

import org.simalliance.openmobileapi.service.ISmartcardServiceChannel;
import org.simalliance.openmobileapi.service.ISmartcardServiceReader;
import org.simalliance.openmobileapi.service.ISmartcardServiceCallback;
import org.simalliance.openmobileapi.service.SmartcardError;

interface ISmartcardServiceSession {

    /**
     * Get the reader that provides this session.
     * 
     * @return The Reader object.
     */
    ISmartcardServiceReader getReader();

   	/**
	 * Returns the ATR of the connected card or null if the ATR is not available.
	 */
    byte[] getAtr();

    /**
     * Close the connection with the Secure Element. This will close any
     * channels opened by this application with this Secure Element.
     */
    void close(out SmartcardError error);
    
    /**
     * Close any channel opened on this session.
     */
    void closeChannels(out SmartcardError error);
    
    
    /**
     * Tells if this session is closed.
     * 
     * @return <code>true</code> if the session is closed, false otherwise.
     */
    boolean isClosed();

    /**
     * Opens a connection using the basic channel of the card in the
     * specified reader and returns a channel handle.
     * Logical channels cannot be opened with this connection.
     * Use interface method openLogicalChannel() to open a logical channel.
     */
    ISmartcardServiceChannel openBasicChannel(ISmartcardServiceCallback callback, out SmartcardError error);

    /**
     * Opens a connection using the basic channel of the card in the
     * specified reader and returns a channel handle. Selects the specified applet.
     * Logical channels cannot be opened with this connection.
     * Selection of other applets with this connection is not supported.
     * Use interface method openLogicalChannel() to open a logical channel.
     */
    ISmartcardServiceChannel openBasicChannelAid(in byte[] aid, ISmartcardServiceCallback callback, out SmartcardError error);

    /**
     * Opens a connection using the next free logical channel of the card in the
     * specified reader. Selects the specified applet.
     * Selection of other applets with this connection is not supported.
     */
    ISmartcardServiceChannel openLogicalChannel(in byte[] aid, ISmartcardServiceCallback callback, out SmartcardError error);
}