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
package org.gsma.joyn.vsh;


/**
 * New video sharing invitation event listener
 * 
 * @author Jean-Marc AUFFRET
 */
public abstract class NewVideoSharingListener extends INewVideoSharingListener.Stub {
	/**
	 * Callback called when a new video sharing invitation has been received
	 * 
	 * @param sharingId Sharing ID
	 */
	public abstract void onNewVideoSharing(String sharingId);

}
