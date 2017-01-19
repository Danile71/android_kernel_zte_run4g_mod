package com.discretix.drmactions;

import android.content.Context;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionUI;
import com.discretix.drmassist.VideoPlayer;

/**
 * Play Media file. In case of encrypted files license should be acquired prior performing this action 
 */
public class PlayFile extends DrmActionUI {
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#getName()
	 */
	public String getName() {
		return "Play file";
	}
	
	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
	 */
	public boolean isSupported(final String extension, final String filePath) {
		return Utilities.isPlayreadyDrm(extension) || Utilities.isMediaFile(extension);
	}

	/*
	 * (non-Javadoc)
	 * @see com.discretix.drmassist.DrmActionUI#perform(android.content.Context, java.lang.String, java.lang.String)
	 */
	protected void perform(final Context context, final String filePath, final String extension) {

		VideoPlayer.play(context, filePath);
		
	}

}
