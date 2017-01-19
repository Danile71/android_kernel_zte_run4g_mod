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

package com.orangelabs.rcs.core.ims.userprofile;

import com.orangelabs.rcs.provider.settings.RcsSettings;

/**
 * User profile read from RCS settings database
 * 
 * @author JM. Auffret
 */
public class VolteUserProfileInterface extends UserProfileInterface {
	/**
	 * Constructor
	 */
	public VolteUserProfileInterface() {
		super();
	}
	
	/**
	 * Read the user profile
	 * 
	 * @return User profile
	 */
	public UserProfile read() {
		
		//TODO: READ FROM THE RCS UA manager
		
		// Read profile info from the database settings
		String username = RcsSettings.getInstance().getUserProfileImsUserName(); 
		String homeDomain = RcsSettings.getInstance().getUserProfileImsDomain();
		String privateID = RcsSettings.getInstance().getUserProfileImsPrivateId();
		String password = RcsSettings.getInstance().getUserProfileImsPassword();
		String realm = RcsSettings.getInstance().getUserProfileImsRealm();
		String xdmServer = RcsSettings.getInstance().getXdmServer();
		String xdmLogin = RcsSettings.getInstance().getXdmLogin();
		String xdmPassword = RcsSettings.getInstance().getXdmPassword();
		String imConfUri = RcsSettings.getInstance().getImConferenceUri();
		String username_full = RcsSettings.getInstance().getUserProfileImsUserName_full();// @tct-stack wuquan add for full public user identity but not only number part 

		return new UserProfile(username, homeDomain, privateID, password, realm,
				xdmServer, xdmLogin, xdmPassword, imConfUri,username_full);
	}
}
