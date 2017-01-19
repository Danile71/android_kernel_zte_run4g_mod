package com.discretix.drmactions;

import android.content.Context;
import android.util.Log;

import com.discretix.drmactions.auxillary.Utilities;
import com.discretix.drmassist.DrmActionSync;

public class DecryptSession extends DrmActionSync {

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
				return "Decrypt session tests";
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
				
				
				// run all test for current file
				String res = Decrypt(3, filePath);
				Log.d("Decrypt Session", "Decrypt session for local file returned " + res);
							
				return res;
				
			}
			
			public String OplTest(int testnum, String filename)
			{
				String res = Decrypt(4, filename);
				Log.d("Decrypt Session", "Decrypt session for local file returned " + res);
							
				return res;
			}
			
			private native String Decrypt(int testnum, String filename);
			//public String OplTest(int testnum, String filename);
		}
