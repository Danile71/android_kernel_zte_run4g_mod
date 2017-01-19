package com.discretix.drmassist;

import java.util.ArrayList;
import java.util.List;

/**
 * DRM Actions repository
 */
final public class DrmActionStore {
	
	static private DrmActionStore instance = null;
	
	private List<DrmActionBase> drmActions = null;
	
	/**
	 * create singleton instance
	 */
	static {
		instance = new DrmActionStore();
	}
	
	/**
	 * private constructor
	 */
	private DrmActionStore(){
		drmActions = new ArrayList<DrmActionBase>();
	}
	
	/**
	 * @return DrmActionStore instance
	 */
	public static DrmActionStore getInstance(){
		return instance;
	}
	
	/**
	 * add {@link DrmActionBase} to repository
	 * @param action
	 */
	public void registerAction(DrmActionBase action){
		drmActions.add(action);
	}
	

	
	/**
	 * Get {@link List} of supported {@link DrmActionBase} instances
	 * @param extension file extension that will be involved in DRM action
	 * @param filePath absolute file path that will be involved in DRM action
	 * @return {@link List} of supported {@link DrmActionBase} instances
	 */
	List<DrmActionBase> getActions(String extension, String filePath){
		List<DrmActionBase> suitableDrmActions = new ArrayList<DrmActionBase>();
		for (DrmActionBase drmAction : drmActions) {
			if (drmAction.isSupported(extension, filePath)){
				suitableDrmActions.add(drmAction);
			}
		}
		return (suitableDrmActions.isEmpty()) ? null: suitableDrmActions;
	}
	
	

}	
