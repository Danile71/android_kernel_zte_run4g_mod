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

package com.orangelabs.rcs.core.ims.service;

import java.util.List;

import org.gsma.joyn.capability.CapabilityService;
import org.gsma.joyn.session.MultimediaMessageIntent;
import org.gsma.joyn.session.MultimediaSessionIntent;

import javax2.sip.message.Request;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;

import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * SIP intent manager
 * 
 * @author Jean-Marc AUFFRET
 */
public class SipIntentManager {
	/**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
	 * Constructor
	 */
	public SipIntentManager() {
	}
	
	/**
	 * Is the SIP request may be resolved
	 * 
	 * @param request Incoming request
	 * @return Resolved intent or null if not resolved
	 */
	public Intent isSipRequestResolved(SipRequest request) {
		Intent result = null;
		List<String> tags = request.getFeatureTags();
		for(int i=0; i < tags.size(); i++) {
			String featureTag = tags.get(i);
			Intent intent = generateSipIntent(request, featureTag);
			if (logger.isActivated()) {
				logger.debug("SIP intent: " + intent.getAction() + ", " + intent.getType());
			}
			if (isSipIntentResolvedByBroadcastReceiver(intent)) {
				result = intent;
				break;
			}
		}
		return result;
	}	

	/**
	 * Generate a SIP Intent
	 * 
	 * @param request SIP request
	 * @param featureTag Feature tag
	 */
	private Intent generateSipIntent(SipRequest request, String featureTag) {
		String mime = formatIntentMimeType(featureTag);
		String action = formatIntentAction(request.getMethod());
		Intent intent = new Intent(action);
		intent.addCategory(Intent.CATEGORY_DEFAULT);
		intent.setType(mime.toLowerCase());
		return intent;
	}

	/**
	 * Is the SIP intent may be resolved by at least broadcast receiver
	 * 
	 * @param intent The Intent to resolve
	 * @return Returns true if the intent has been resolved, else returns false
	 */
	private boolean isSipIntentResolvedByBroadcastReceiver(Intent intent) {
		PackageManager packageManager = AndroidFactory.getApplicationContext().getPackageManager();
		List<ResolveInfo> list = packageManager.queryBroadcastReceivers(intent,
				PackageManager.MATCH_DEFAULT_ONLY);
		return (list.size() > 0);
	}
	
	/**
	 * Format intent action
	 * 
	 * @param request Request method name
	 * @return Intent action
	 */
	private String formatIntentAction(String request) { 
		String action;
		if (request.equals(Request.MESSAGE)) {
			action = MultimediaMessageIntent.ACTION_NEW_MESSAGE;
		} else {
			action = MultimediaSessionIntent.ACTION_NEW_INVITATION;
		}
		return action;
	}
	
	/**
	 * Format intent MIME type
	 * 
	 * @param featureTag Feature tag
	 * @return Intent MIME type
	 */
	private String formatIntentMimeType(String featureTag) { 
		String serviceId = CapabilityUtils.extractServiceId(featureTag);
		return CapabilityService.EXTENSION_MIME_TYPE + "/" + serviceId;
	}
}
