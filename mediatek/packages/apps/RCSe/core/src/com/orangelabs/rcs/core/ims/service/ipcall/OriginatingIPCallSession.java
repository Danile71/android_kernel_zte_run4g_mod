/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.service.ipcall;

import org.gsma.joyn.ipcall.IIPCallPlayer;
import org.gsma.joyn.ipcall.IIPCallRenderer;

import com.orangelabs.rcs.core.content.AudioContent;
import com.orangelabs.rcs.core.content.VideoContent;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Originating IP call session
 *
 * @author opob7414
 */
public class OriginatingIPCallSession extends IPCallSession {
	
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * Constructor
     *
     * @param parent IMS service
     * @param contact Remote contact
     * @param audioContent Audio content
     * @param videoContent Video content
     * @param player IP call player
     * @param renderer IP call renderer
     */
    public OriginatingIPCallSession(ImsService parent, String contact, AudioContent audioContent,
    		VideoContent videoContent, IIPCallPlayer player, IIPCallRenderer renderer) {
    	super(parent, contact, audioContent, videoContent);
    	
        // Set the player
        setPlayer(player);
        
        // Set the renderer
        setRenderer(renderer);

        // Create dialog path
        createOriginatingDialogPath();
    }
    
    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Initiate a new IP call session as originating");
            }

            // Check audio parameters 
            if (getAudioContent() == null) {
                handleError(new IPCallError(IPCallError.UNSUPPORTED_AUDIO_TYPE, "Audio codec not supported"));
                return;
            }
            
            // Build SDP proposal
            String sdp = buildAudioVideoSdpProposal();

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp); 

            // Create an INVITE request
            if (logger.isActivated()) {
                logger.info("Send INVITE");
            }
            SipRequest invite;  
            if (getVideoContent() == null) {
            	// Voice call
            	invite = SipMessageFactory.createInvite(getDialogPath(), IPCallService.FEATURE_TAGS_IP_VOICE_CALL, sdp);
            } else {
            	// Visio call
            	invite = SipMessageFactory.createInvite(getDialogPath(), IPCallService.FEATURE_TAGS_IP_VIDEO_CALL, sdp);
            } 

	        // Set the Authorization header
	        getAuthenticationAgent().setAuthorizationHeader(invite);

	        // Set initial request in the dialog path
            getDialogPath().setInvite(invite);

            // Send INVITE request
            sendInvite(invite);
            
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }
            
            // Unexpected error
            handleError(new IPCallError(IPCallError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }
    }
}
