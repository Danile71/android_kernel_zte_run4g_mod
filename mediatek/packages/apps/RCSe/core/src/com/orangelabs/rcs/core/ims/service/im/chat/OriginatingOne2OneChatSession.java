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

package com.orangelabs.rcs.core.ims.service.im.chat;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.geoloc.GeolocInfoDocument;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpInfoDocument;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.utils.logger.Logger;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;

/**
 * Originating one-to-one chat session
 * 
 * @author jexa7410
 */
public class OriginatingOne2OneChatSession extends OneOneChatSession {	
	/**
	 * Boundary tag
	 */
	private final static String BOUNDARY_TAG = "boundary1";

	/**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
	 * Constructor
	 * 
	 * @param parent IMS service
	 * @param contact Remote contact
	 * @param msg First message of the session
	 */
	public OriginatingOne2OneChatSession(ImsService parent, String contact, InstantMessage msg) {
        super(parent, contact);

        // Set first message
        setFirstMesssage(msg);

        // Create dialog path
        createOriginatingDialogPath();

        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);

		if(RcsSettings.getInstance().isCPMSupported()){
			if (logger.isActivated()) {
	    		logger.info("CPMS OriginatingOne2OneChatSession");
	    	}
			// Set conversation ID
	        String ConversationId = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
	        setConversationID(ConversationId);
		}
	}

	/**
	 * Background processing
	 */
	public void run() {
		try {
	    	if (logger.isActivated()) {
	    		logger.info("Initiate a new 1-1 chat session as originating");
	    	}
            /**
             * M: Modified to resolve the 403 error issue @{
             */
	    	SipRequest invite = createSipInvite();
	    	/**
	    	 * @}
	    	 */
	        // Send INVITE request
	        sendInvite(invite);	        
		} catch(Exception e) {
        	if (logger.isActivated()) {
        		logger.error("Session initiation has failed", e);
        	}

        	// Unexpected error
			handleError(new ChatError(ChatError.UNEXPECTED_EXCEPTION, e.getMessage()));
		}		
	}

        	/**
     * M: Modified to resolve the 403 error issue @{
     */
    /**
     * @return A sip invite request
     */
    protected SipRequest createSipInvite(String callId) {
        logger.debug("createSipInvite(), callId = " + callId);
        createOriginatingDialogPath(callId);
        return createSipInvite();
    }

    private SipRequest createSipInvite() {
        logger.debug("createSipInvite()");
    		// Set setup mode
	    	String localSetup = createSetupOffer();
            if (logger.isActivated()){
				logger.debug("Local setup attribute is " + localSetup);
			}

            // Set local port
            int localMsrpPort;
            if ("active".equals(localSetup)) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = getMsrpMgr().getLocalMsrpPort();
            }

	    	// Build SDP part
	    	String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
	    	String ipAddress = getDialogPath().getSipStack().getLocalIpAddress();
			String sdp = null;
			 logger.debug("isSecureProtocolMessage()");
			if(isSecureProtocolMessage()){
	    	 	sdp =
	    		"v=0" + SipUtils.CRLF +
	            "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            "s=-" + SipUtils.CRLF +
				"c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            "t=0 0" + SipUtils.CRLF +			
	            "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
	            "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
	            "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF +
	            "a=setup:" + localSetup + SipUtils.CRLF +
	    		"a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
	            "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
	    		"a=sendrecv" + SipUtils.CRLF;
			}
			else{
				sdp =
	    		"v=0" + SipUtils.CRLF +
	            "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            "s=-" + SipUtils.CRLF +
				"c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
	            "t=0 0" + SipUtils.CRLF +			
	            "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
	            "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
	            "a=setup:" + localSetup + SipUtils.CRLF +
	    		"a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
	            "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
	    		"a=sendrecv" + SipUtils.CRLF;
			}
	    	
	    	// If there is a first message then builds a multipart content else builds a SDP content
	    	if (getFirstMessage() != null) {
		    	// Build CPIM part
				String from = ChatUtils.ANOMYNOUS_URI;
				String to = ChatUtils.ANOMYNOUS_URI;

				boolean useImdn = getImdnManager().isImdnActivated();
				String formattedMsg;
				String mime;
				if (getFirstMessage() instanceof GeolocMessage) {
					GeolocMessage geolocMsg = (GeolocMessage)getFirstMessage();
					formattedMsg = ChatUtils.buildGeolocDocument(geolocMsg.getGeoloc(),
							ImsModule.IMS_USER_PROFILE.getPublicUri(),
							getFirstMessage().getMessageId());
					mime = GeolocInfoDocument.MIME_TYPE;
				} else
				if (getFirstMessage() instanceof FileTransferMessage) {
					FileTransferMessage fileMsg = (FileTransferMessage)getFirstMessage();
					formattedMsg = fileMsg.getFileInfo();
					mime = FileTransferHttpInfoDocument.MIME_TYPE;
				} else {
					formattedMsg = getFirstMessage().getTextMessage();
					mime = InstantMessage.MIME_TYPE;
				}
				
				String cpim;
				if (useImdn) {
					// Send message in CPIM + IMDN
					cpim = ChatUtils.buildCpimMessageWithImdn(
	    					from,
	    					to,
		        			getFirstMessage().getMessageId(),
		        			StringUtils.encodeUTF8(formattedMsg),
		        			mime);
				} else {
					// Send message in CPIM
					cpim = ChatUtils.buildCpimMessage(
	    					from,
	    					to,
		        			StringUtils.encodeUTF8(formattedMsg),
		        			mime);
				}

		    	// Build multipart
		        String multipart = 
		        	Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
	    			"Content-Type: application/sdp" + SipUtils.CRLF +
	    			"Content-Length: " + sdp.getBytes().length + SipUtils.CRLF +
	    			SipUtils.CRLF +
	    			sdp + SipUtils.CRLF + 
	    			Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
	    			"Content-Type: " + CpimMessage.MIME_TYPE + SipUtils.CRLF +
	    			"Content-Length: "+ cpim.getBytes().length + SipUtils.CRLF +
	    			SipUtils.CRLF +
	    			cpim + SipUtils.CRLF +
	    			Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + Multipart.BOUNDARY_DELIMITER;

				// Set the local SDP part in the dialog path
		    	getDialogPath().setLocalContent(multipart);
	    	} else {
				// Set the local SDP part in the dialog path
		    	getDialogPath().setLocalContent(sdp);
	    	}
            try {
            SipRequest invite = createInvite();

	        // Set the Authorization header
	        getAuthenticationAgent().setAuthorizationHeader(invite);

	        // Set initial request in the dialog path
	        getDialogPath().setInvite(invite);

	         return invite;
		} catch (SipException e) {
                e.printStackTrace();
            } catch (CoreException e) {
                e.printStackTrace();
        	}

        logger.error("Create sip invite failed, retrn null.");
        return null;
	}
}
 /**
     * @}
     */
