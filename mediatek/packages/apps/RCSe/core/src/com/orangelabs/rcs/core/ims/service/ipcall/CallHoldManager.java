package com.orangelabs.rcs.core.ims.service.ipcall;

import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Super class for IP Call Hold Manager 
 * 
 * @author O. Magnon
 */
public abstract class  CallHoldManager {
	/**
	 * Constant values for IPCall Hold states
	 */
	static final int IDLE = 0; 
	static final int HOLD_INPROGRESS = 1;
	static final int HOLD = 2;
	static final int UNHOLD_INPROGRESS = 3;
	
	// Hold state
	int state;
	
	// session handled by Hold manager
	IPCallSession session ; 
	
	/**
	 * The logger
	 */
	protected Logger logger = Logger.getLogger(this.getClass().getName());
	
	
	public CallHoldManager(IPCallSession session){
		if (logger.isActivated()){
			logger.info("IPCall_Hold()");
		}
		this.state = CallHoldManager.IDLE;
		this.session = session;
	}
	
	public abstract void setCallHold(boolean callHoldAction);
	
	
	public abstract void setCallHold(boolean callHoldAction, SipRequest reInvite);
		
	
	public abstract void prepareSession();

	public boolean isCallHold() {
		return ((state == HOLD_INPROGRESS) || (state == HOLD));
	}
}
