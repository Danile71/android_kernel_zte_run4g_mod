package com.discretix.drmactions;

import android.content.Context;
import android.util.Log;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

public class GetMiracastOpl extends DrmActionSync {

		static 
		{			
			int currentapiVersion = android.os.Build.VERSION.SDK_INT;
			if (currentapiVersion >= 19) // KK4.4 and above
			{
				System.loadLibrary("DrmAssistJNI44");
			} 
			else if (currentapiVersion == 18) // JB4.3 and above
			{
				System.loadLibrary("DrmAssistJNI43");
			} 
			else
			{
				System.loadLibrary("DrmAssistJNI42"); // 4.2 and lower
			}
		}
			/*
			 * (non-Javadoc)
			 * @see com.discretix.drmassist.DrmActionBase#getName()
			 */
			public String getName() {
				return "Get Miracast & OPL level";
			}

			/*
			 * (non-Javadoc)
			 * @see com.discretix.drmassist.DrmActionBase#isSupported(java.lang.String, java.lang.String)
			 */
			public boolean isSupported(String extension, String filePath) {
				return Utilities.isPlayreadyDrm(extension);
			}
			
			/*
			 * (non-Javadoc)
			 * @see com.discretix.drmassist.DrmActionSync#perform(android.content.Context, java.lang.String, java.lang.String)
			 */
			protected String perform(Context context, String filePath, String extension) {		
				
				DecryptSession ds = new DecryptSession();
				// run all test for current file
				//String res = GetOplMiracastFromFile(1, filePath);
				String res = ds.OplTest(4, filePath);
				Log.d("Decrypt Session", "Get Miracast & OPL level for local file returned " + res);
							
				return res;
				
			}
			
			
			//private native String GetOplMiracastFromFile(int testnum, String filename);
		}
