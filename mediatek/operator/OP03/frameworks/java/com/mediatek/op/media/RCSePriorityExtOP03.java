package com.mediatek.op.media;

import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;
import android.content.pm.LabeledIntent;
import com.mediatek.op.media.DefaultRCSePriority;
import java.util.List;
import java.util.ArrayList;



public class RCSePriorityExtOP03 extends
        DefaultRCSePriority {
    
    private  Context mContext;
    private static final String TAG = "RCSePriorityExt";

    public RCSePriorityExtOP03() {
	Log.i(TAG, "Constructor default called");
		}
		
    public RCSePriorityExtOP03(ArrayList<String> defaultList) {
	Log.i(TAG, "Constructor with parametres as list");
		   
    }
   
    public RCSePriorityExtOP03(Context context) {
        Log.i(TAG, " Constructor called with argument context");
    }
    
    public int sortTheListForRCSe(ArrayList<String> activities) {
          Log.i(TAG, "sortTheListForRCSe ActivityResolveInfo" );
          int rcseIndex = -1;
          int activityCount = activities.size();         
          for (int i = 0; i < activityCount; i++) {         
          if(activities.get(i).equals("com.orangelabs.rcs"))
            {
                Log.i(TAG, "RCSE App Found: " );
                //rcseActResolveInfo = actResolveInfo;
		rcseIndex = i;                    
            }
          }                
        
         Log.i(TAG, "sortTheListForRCSe ActivityResolveInfo exit index is" + rcseIndex );	 	 
         return rcseIndex;
     }

   
}

