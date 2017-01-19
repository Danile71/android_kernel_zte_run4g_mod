package com.mediatek.op.telephony;

import android.util.Log;
import android.content.Context;
import com.mediatek.common.telephony.IServiceStateExt;
import android.telephony.ServiceState;

public class ServiceStateExtOP07 extends ServiceStateExt {
    private Context mContext;
    static final String LOG_TAG = "GSM";
	
    public ServiceStateExtOP07() {
    }
	
    public ServiceStateExtOP07(Context context) {
        mContext = context;
    }
	
    public boolean isImeiLocked(){
        return true;
    }		

    public boolean needBrodcastACMT(int error_type,int error_cause) {
        /* <error_type>: integer
           0  UNDEFINED
           1  MM reject code received during a MM procedure
           2  CM reject code received during a CM procedure
           3  GMM reject code received during a non-combined GMM procedure for GPRS services
           4  SM reject code
           5  GMM reject code received during a combined GMM procedure for non-GPRS services
           6  GMM reject code received during a combined GMM procedure for GPRS and non-GPRS services
           7  EMM reject code received during a non-combined EMM procedure for EPS services (not supported,RFU)
           8  EMM reject code received during a combined EMM procedure for non-EPS services (not supported,RFU)
           9  EMM reject code received during a combined EMM procedure for EPS and non-EPS services (not supported,RFU)
           10 ESM reject code received during a ESM procedure (not supported,RFU)
           11 GMM reject code received during a GMM Service procedure (not supported,RFU)
           12 GMM reject code received during a GMM MT Detach procedure (not supported,RFU)           
        */    	
            if(((error_type == 1)&&((error_cause == 17)||(error_cause == 22)))||
               ((error_type == 2)&&((error_cause == 17)||(error_cause == 22)))||
               ((error_type == 3)&&((error_cause == 7)||(error_cause == 8)||(error_cause == 10)||(error_cause == 17)||(error_cause == 22)))||        
               ((error_type == 4)&&((error_cause == 27)||(error_cause == 28)||(error_cause == 30)||(error_cause == 31)||(error_cause == 33)||(error_cause == 36)||(error_cause == 38)))||           
               ((error_type == 5)&&((error_cause == 17)||(error_cause == 22)))||
               ((error_type == 6)&&((error_cause == 7)||(error_cause == 8)||(error_cause == 10)||(error_cause == 17)||(error_cause == 22)))||
               ((error_type == 11)&&((error_cause == 7)||(error_cause == 10)||(error_cause == 17)||(error_cause == 22)||(error_cause == 40))))
            {
                Log.w(LOG_TAG,"needBrodcastACMT return false. type="+error_type+"cause="+error_cause);
                return true;        	
            }        	    
	
        Log.w(LOG_TAG,"needBrodcastACMT return false. type="+error_type+"cause="+error_cause);	
        return false;
    } 

    public boolean needRejectCauseNotification(int cause)
    {
        boolean needNotification = false;
        Log.i(LOG_TAG,"needRejectCauseNotification cause:"+cause);   

        /* ALPS00283696 CDR-NWS-241 */				        
        switch(cause)   	
        {
            case 2:       
            case 3:
            case 5:		   	
            case 6:	
                Log.w(LOG_TAG,"needRejectCauseNotification return true");   		   	
                needNotification = true;
                break;			   
            default:
                break;		   	
        } 	   
        return needNotification;
    }	

   public boolean needIgnoredState(int state,int new_state,int cause){
        if((state == ServiceState.STATE_IN_SERVICE) && (new_state == 2)){
            /* Don't update for searching state, there shall be final registered state update later */						
            Log.i(LOG_TAG,"set dontUpdateNetworkStateFlag for searching state");                  
            return true;
        }	   

        /* -1 means modem didn't provide <cause> information. */
        if(cause != -1){
            if((state == ServiceState.STATE_IN_SERVICE)  && (new_state == 3) && (cause != 0)){
                /* This is likely temporarily network failure, don't update for better UX */			
                Log.i(LOG_TAG,"set dontUpdateNetworkStateFlag for REG_DENIED with cause");                  
                return true;
            }
        }	
		
        Log.i(LOG_TAG,"clear dontUpdateNetworkStateFlag");       	   

        return false;
    }

   public boolean ignoreDomesticRoaming()
   {
       /* ALPS00296372 */   
       return true;   
   }

    public int setEmergencyCallsOnly(int state,int cid,int lac){
        /* ALPS00283717 CDR-NWS-190 */
        if((cid == -1) || (state == 4)){
            /* state(4) is 'unknown'  and cid(-1) means cid was not provided in +creg URC */                
            Log.i(LOG_TAG,"No valid info to distinguish limited service and no service");                                        
            return -1;			
        }				
        else if(((state ==0)||(state == 3)) && ((cid & 0xffff)!=0)){
            return 1;
        }				
        else{
            return 0;
        }		   
    }

    public boolean isSupportRatBalancing(){
        return true;
    }
}
