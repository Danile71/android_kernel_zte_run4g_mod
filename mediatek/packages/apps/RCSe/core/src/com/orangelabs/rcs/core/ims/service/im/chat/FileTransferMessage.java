package com.orangelabs.rcs.core.ims.service.im.chat;

import java.util.Date;

import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpInfoDocument;

/**
 * File transfer message
 * 
 * @author Jean-Marc AUFFRET
 */ 
public class FileTransferMessage extends InstantMessage {
	/**
	 * MIME type
	 */
	public static final String MIME_TYPE = FileTransferHttpInfoDocument.MIME_TYPE;

	/**
	 * File info
	 */
	private String file = null;
		
    /**
     * Constructor for outgoing message
     * 
     * @param messageId Message Id
     * @param remote Remote user
     * @param file File info
     * @param imdnDisplayedRequested Flag indicating that an IMDN "displayed" is requested
	 */
	public FileTransferMessage(String messageId, String remote, String file, boolean imdnDisplayedRequested) {
		super(messageId, remote, null, imdnDisplayedRequested,null);
		
		this.file = file;
	}
	
	/**
     * Constructor for incoming message
     * 
     * @param messageId Message Id
     * @param remote Remote user
     * @param file File info
     * @param imdnDisplayedRequested Flag indicating that an IMDN "displayed" is requested
	 * @param serverReceiptAt Receipt date of the message on the server
	 */
	public FileTransferMessage(String messageId, String remote, String file, boolean imdnDisplayedRequested, Date serverReceiptAt) {
		super(messageId, remote, null, imdnDisplayedRequested, serverReceiptAt,null);
		
		this.file = file;
	}

    /**
	 * Get file info
	 * 
	 * @return File info
	 */
	public String getFileInfo() {
		return file;
	}
}
