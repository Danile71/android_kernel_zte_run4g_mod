package com.orangelabs.rcs.core.ims.service.im.filetransfer;

import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ListOfParticipant;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.StorageUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.mediatek.rcse.settings.SettingUtils;

/**
 * Abstract file sharing session 
 * 
 * @author jexa7410
 */
public abstract class FileSharingSession extends ImsServiceSession {
    /**
     * Contribution ID
     */
    private String contributionId = null;	
    
    /**
     * Contversation ID
     */
    private String contversationId = null;	

	  /**
     * Contversation ID
     */
    private String inReplyId = null;	
    
    /**
	 * Content to be shared
	 */
	protected MmContent content;
	
	private String thumbUrl = null;
	
	/**
	 * File transfered
	 */
	private boolean fileTransfered = false;

    /**
     * List of participants
     */
    protected ListOfParticipant participants = new ListOfParticipant();

    /**
	 * Thumbnail
	 */
	private byte[] thumbnail = null;
	
	/**
	 * Display Name
	 */
	private String displayName = null;
	
	/**
	 * File transfer paused
	 */
	private boolean fileTransferPaused = false;

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger(FileSharingSession.class.getName());

    /**
	 * Constructor
	 * 
	 * @param parent IMS service
	 * @param content Content to be shared
	 * @param contact Remote contact
	 * @param thumbnail Thumbnail
	 */
	public FileSharingSession(ImsService parent, MmContent content, String contact, byte[] thumbnail) {
		super(parent, contact);
		
		this.content = content;
		this.thumbnail = thumbnail;
	}

	/**
	 * Return the contribution ID
	 * 
	 * @return Contribution ID
	 */
	public String getContributionID() {
		return contributionId;
	}	
	
	/**
	 * Set the contribution ID
	 * 
	 * @param id Contribution ID
	 */
	public void setContributionID(String id) {
		this.contributionId = id;
	}
	
	/**
	 * Return the conversation ID
	 * 
	 * @return conversation ID
	 */
	public String getConversationID() {
		return contversationId;
	}	
	
	/**
	 * Set the conversation ID
	 * 
	 * @param id conversation ID
	 */
	public void setConversationID(String id) {
		this.contversationId = id;
	}

	/**
	 * Return the conversation ID
	 * 
	 * @return conversation ID
	 */
	public String getInReplyID() {
		return inReplyId;
	}	
	
	/**
	 * Set the conversation ID
	 * 
	 * @param id conversation ID
	 */
	public void setInReplyID(String id) {
		this.inReplyId = id;
	}
	
	/**
	 * Return the contribution ID
	 * 
	 * @return Contribution ID
	 */
	public String getDisplayName() {
		return displayName;
	}	
	
	/**
	 * Set the contribution ID
	 * 
	 * @param id Contribution ID
	 */
	public void setDisplayName(String displayName) {
		this.displayName = displayName;
	}
	
	/**
	 * Returns the content
	 * 
	 * @return Content 
	 */
	public MmContent getContent() {
		return content;
	}
	
	/**
	 * Returns the list of participants involved in the transfer
	 * 
	 * @return List of participants 
	 */
	public ListOfParticipant getParticipants() {
		return participants;
	}
	
	/**
	 * Set the content
	 * 
	 * @param content Content  
	 */
	public void setContent(MmContent content) {
		this.content = content;
	}	
	
	/**
	 * Returns the "file-transfer-id" attribute
	 * 
	 * @return String
	 */
	public String getFileTransferId() {
		return "" + System.currentTimeMillis();
	}	
	
	/**
	 * File has been transfered
	 */
	public void fileTransfered() {
		this.fileTransfered = true;
		
	}
	
	/**
	 * Is file transfered
	 * 
	 * @return Boolean
	 */
	public boolean isFileTransfered() {
		return fileTransfered; 
	}
	
	/**
	 * File has been paused
	 */
	public void fileTransferPaused() {
		this.fileTransferPaused = true;
	}
	
	/**
	 * File is resuming
	 */
	public void fileTransferResumed() {
		this.fileTransferPaused = false;
	}
	
	/**
	 * Is file transfer paused
	 * 
	 * @return fileTransferPaused
	 */
	public boolean isFileTransferPaused() {
		return fileTransferPaused; 
	}

	/**
	 * Returns max file sharing size
	 * 
	 * @return Size in bytes
	 */
	public static int getMaxFileSharingSize() {
		return RcsSettings.getInstance().getMaxFileTransferSize()*1024;
	}

    /** M: ftAutAccept @{ */
    public boolean shouldAutoAccept() {
    	if( RcsSettings.getInstance().isFileTransferAutoAccepted()){
			boolean isRoaming = SettingUtils.isRoaming(AndroidFactory
                .getApplicationContext());
	        // whether ftAutAccept is enabled if roaming.
	        if (isRoaming) {
	            return RcsSettings.getInstance()
	                    .isEnableFtAutoAcceptWhenRoaming();
	        } else {
	            return RcsSettings.getInstance()
	                    .isEnableFtAutoAcceptWhenNoRoaming();
	        }
    	}
		else{
        	return false;
		}
    }

    /** @} */

    /**
     * Returns the thumbnail
     * 
     * @return Thumbnail
     */
    public byte[] getThumbnail() {
    	return thumbnail;
    }

    /**
     * Set the thumbnail
     *
     * @param Thumbnail
     */
    public void setThumbnail(byte[] thumbnail) {
        this.thumbnail = thumbnail;
    }

/**
     * Returns the thumbnail
     * 
     * @return Thumbnail
     */
    public String getThumbUrl() {
    	return thumbUrl;
    }

  /**
     * Set the thumbnail
     *
     * @param Thumbnail
     */
    public void setThumbUrl(String thumbUrl) {
        this.thumbUrl = thumbUrl;
    }

	/**
	 * Check if file capacity is acceptable
	 * 
	 * @param fileSize File size in bytes
	 * @return Error or null if file capacity is acceptable
	 */
	public static FileSharingError isFileCapacityAcceptable(long fileSize) {
		boolean fileIsToBig = (FileSharingSession.getMaxFileSharingSize() > 0) ? fileSize > FileSharingSession.getMaxFileSharingSize() : false;
		boolean storageIsTooSmall = (StorageUtils.getExternalStorageFreeSpace() > 0) ? fileSize > StorageUtils.getExternalStorageFreeSpace() : false;
		if (fileIsToBig) {
			if (logger.isActivated()) {
				logger.warn("File is too big, reject the file transfer");
			}
			return new FileSharingError(FileSharingError.MEDIA_SIZE_TOO_BIG);
		} else {
			if (storageIsTooSmall) {
				if (logger.isActivated()) {
					logger.warn("Not enough storage capacity, reject the file transfer");
				}
				return new FileSharingError(FileSharingError.NOT_ENOUGH_STORAGE_SPACE);
			}
		}
		return null;
	}    
}
