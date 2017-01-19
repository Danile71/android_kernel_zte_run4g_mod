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

import org.simalliance.openmobileapi.service.ISmartcardServiceSession;
import org.simalliance.openmobileapi.service.ISmartcardServiceCallback;
import org.simalliance.openmobileapi.service.SmartcardError;

interface ISmartcardServiceReader {
	/**
     * Return the user-friendly name of this reader.
     * <ul>
	 * <li>If this reader is a SIM reader, then its name must start with the "SIM" prefix.</li>
	 * <li>If the reader is a SD or micro SD reader, then its name must start with the "SD" prefix</li>
	 * <li>If the reader is a embedded SE reader, then its name must start with the "eSE" prefix</li>
	 * <ul>
     * 
     * @return name of this Reader
     */
    String getName(out SmartcardError error);

    /**
     * Returns true if a card is present in the specified reader.
     * Returns false if a card is not present in the specified reader.
     */
    boolean isSecureElementPresent(out SmartcardError error);


    /**
     * Connects to a secure element in this reader. <br>
     * This method prepares (initialises) the Secure Element for communication
     * before the Session object is returned (e.g. powers the Secure Element by
     * ICC ON if its not already on). There might be multiple sessions opened at
     * the same time on the same reader. The system ensures the interleaving of
     * APDUs between the respective sessions.
     * 
     * @return a Session object to be used to create Channels.
     */
    ISmartcardServiceSession openSession(out SmartcardError error);
    
    /**
     * Close all the sessions opened on this reader. All the channels opened by
     * all these sessions will be closed.
     */
    void closeSessions(out SmartcardError error);
    
}