package com.discretix.drmactions;

import com.discretix.drmassist.DrmActionStore;

/**
 * Auxiliary class used for register DRM actions to UI 
 */
public class DrmRigistry {
	
	/**
	 * register DRM actions to UI
	 */
	public static void register(){
		DrmActionStore store = DrmActionStore.getInstance();
		
		//store.registerAction(new ExecuteInitiator());
		store.registerAction(new AcquireRights());
		store.registerAction(new AcquireRightsByApp());
		store.registerAction(new RemoveRights());
		store.registerAction(new RemoveExpiredRights());
		store.registerAction(new RemoveAllPRRights());
		store.registerAction(new DomainJoin());
		store.registerAction(new DomainLeave());
		store.registerAction(new CanHandle());
		store.registerAction(new PlayFile());
		store.registerAction(new CheckRights());
		store.registerAction(new GetConstraints());
		store.registerAction(new GetMetadata());
		store.registerAction(new GetOriginalMimeType());
		store.registerAction(new RemoveAllRights());
		store.registerAction(new DecryptSession());
		store.registerAction(new Performance_challenge());
		store.registerAction(new Performance_Response());
		store.registerAction(new RemoveAllPLicesesByAcquireDrmInfo());
		store.registerAction(new GetMiracastOpl());
	}
}
