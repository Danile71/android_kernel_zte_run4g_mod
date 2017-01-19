/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
package org.gsma.joyn.ft;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.gsma.joyn.JoynContactFormatException;
import org.gsma.joyn.JoynService;
import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.JoynServiceListener;
import org.gsma.joyn.JoynServiceNotAvailableException;
import org.gsma.joyn.Permissions;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.IInterface;

import com.mediatek.rcse.api.Logger;

/**
 * This class offers the main entry point to transfer files and to
 * receive files. Several applications may connect/disconnect to the API.
 * 
 * The parameter contact in the API supports the following formats:
 * MSISDN in national or international format, SIP address, SIP-URI
 * or Tel-URI.
 * 
 * @author Jean-Marc AUFFRET 
 */
public class FileTransferService extends JoynService {
	/**
	 * API
	 */
	private IFileTransferService api = null;
	
	public static final String TAG = "TAPI-FileTransferService";
	
    /**
     * Constructor
     * 
     * @param ctx Application context
     * @param listener Service listener
     */
    public FileTransferService(Context ctx, JoynServiceListener listener) {
    	super(ctx, listener);
    }

    /**
     * Connects to the API
     */
    public void connect() {
    	Logger.i(TAG, "FileTransfer connect() entry");    	
    	ctx.bindService(new Intent(IFileTransferService.class.getName()), apiConnection, 0);
    }
    
    /**
     * Disconnects from the API
     */
    public void disconnect() {
    	try {
    		Logger.i(TAG, "FileTransfer disconnect() entry");    
    		ctx.unbindService(apiConnection);
        } catch(IllegalArgumentException e) {
        	// Nothing to do
        }
    }

	/**
	 * Set API interface
	 * 
	 * @param api API interface
	 */
    protected void setApi(IInterface api) {
    	super.setApi(api);
    	
        this.api = (IFileTransferService)api;
    }
    
    /**
	 * Service connection
	 */
	private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
        	Logger.i(TAG, "onServiceConnected entry " + className);
        	setApi(IFileTransferService.Stub.asInterface(service));
        	if (serviceListener != null) {
        		serviceListener.onServiceConnected();
        	}
        }

        public void onServiceDisconnected(ComponentName className) {
        	Logger.i(TAG, "onServiceDisconnected entry " + className);
        	setApi(null);
        	if (serviceListener != null) {
        		serviceListener.onServiceDisconnected(JoynService.Error.CONNECTION_LOST);
        	}
        }
    };

    /**
     * Returns the configuration of the file transfer service
     * 
     * @return Configuration
     * @throws JoynServiceException
     */
    public FileTransferServiceConfiguration getConfiguration() throws JoynServiceException {
    	Logger.i(TAG, "getConfiguration() entry " + api);
		if (api != null) {
			try {
				return api.getConfiguration();
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

    /**
     * Transfers a file to a contact. The parameter filename contains the complete
     * path of the file to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     * 
     * @param contact 
     * @param filename Filename to transfer
     * @param listener File transfer event listener
     * @return File transfer
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
	 */
    public FileTransfer transferFile(String contact, String filename, FileTransferListener listener) throws JoynServiceException, JoynContactFormatException {
    	return transferFile(contact, filename, null, listener);
    }    
    
    /**
     * Transfers a file to a contact. The parameter filename contains the complete
     * path of the file to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     * 
     * @param contact 
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listener File transfer event listener
     * @return File transfer
     * @throws JoynServiceException
	 * @throws JoynContactFormatException
     */
    public FileTransfer transferFile(String contact, String filename, String fileicon, FileTransferListener listener) throws JoynServiceException, JoynContactFormatException {
    	/*if (ctx.checkCallingOrSelfPermission(Permissions.RCS_FILETRANSFER_SEND) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_FILETRANSFER_SEND");
		}
    	*/
    	Logger.i(TAG, "transferFile() entry contact= " + contact + " filename="+ filename +" fileicon = " +fileicon+ " listener="+listener);
    	if (api != null) {
    		try {
				IFileTransfer ftIntf = api.transferFile(contact, filename, fileicon, listener);
				if (ftIntf != null) {
					return new FileTransfer(ftIntf);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}
    
    /**
     * transfers a file to a group of contacts outside of a current group chat. The
     * parameter file contains the complete filename including the path to be transferred.
     * See also the method GroupChat.sendFile() of the Chat API to send a file from an
     * existing group chat conversation
     * 
     * @param set of contacts 
     * @param filename Filename to transfer
     * @param listener File transfer event listener
     * @return File transfer
     * @throws JoynServiceException
     */
    public FileTransfer transferFileToGroup(String chatId,Set<String> contacts, String filename, FileTransferListener listener) throws JoynServiceException
    {
    	return transferFileToGroup(chatId,contacts, filename, null, listener);
    }
    
    /**
     * transfers a file to a group of contacts with an optional file icon.
     * @param set of contacts 
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listener File transfer event listener
     * @return File transfer
     * @throws JoynServiceException
     */
    public FileTransfer transferFileToGroup(String chatId,Set<String> contacts, String filename, String fileicon, FileTransferListener listener) throws JoynServiceException
    {
    	/*if (ctx.checkCallingOrSelfPermission(Permissions.RCS_FILETRANSFER_SEND) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_FILETRANSFER_SEND");
		}*/
    	Logger.i(TAG, "transferFileToGroup() entry contact= " + contacts +" chatId="+ chatId+ " filename="+ filename +" fileicon = " +fileicon+ " listener="+listener);
    	if (api != null) {
    		try {
    			List<String> contactsList = new ArrayList<String>(contacts);
				IFileTransfer ftIntf = api.transferFileToGroup(chatId, contactsList, filename, fileicon, listener);
				if (ftIntf != null) {
					return new FileTransfer(ftIntf);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }
    /**
     * Returns the list of file transfers in progress
     * 
     * @return List of file transfers
     * @throws JoynServiceException
     */
    public Set<FileTransfer> getFileTransfers() throws JoynServiceException {
    	Logger.i(TAG, "getFileTransfers() entry " + api);
		if (api != null) {
			try {
	    		Set<FileTransfer> result = new HashSet<FileTransfer>();
				List<IBinder> ftList = api.getFileTransfers();
				for (IBinder binder : ftList) {
					FileTransfer ft = new FileTransfer(IFileTransfer.Stub.asInterface(binder));
					result.add(ft);
				}
				return result;
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }    

    /**
     * Returns a current file transfer from its unique ID
     * 
     * @return File transfer or null if not found
     * @throws JoynServiceException
     */
    public FileTransfer getFileTransfer(String transferId) throws JoynServiceException {
    	Logger.i(TAG, "getFileTransfer() entry " + transferId + " api="+api);
		if (api != null) {
			try {
				IFileTransfer ftIntf = api.getFileTransfer(transferId);
				if (ftIntf != null) {
					return new FileTransfer(ftIntf);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }    
    
    /**
     * Returns a current file transfer from its invitation Intent
     * 
     * @param intent Invitation intent
     * @return File transfer or null if not found
     * @throws JoynServiceException
     */
    public FileTransfer getFileTransferFor(Intent intent) throws JoynServiceException {
    	Logger.i(TAG, "getFileTransferFor() entry " + intent + " api="+api);
		if (api != null) {
			try {
				String transferId = intent.getStringExtra(FileTransferIntent.EXTRA_TRANSFER_ID);
				if (transferId != null) {
					return getFileTransfer(transferId);
				} else {
					return null;
				}
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
    }     
    
    /**
	 * Registers a file transfer invitation listener
	 * 
	 * @param listener New file transfer listener
	 * @throws JoynServiceException
	 */
	public void addNewFileTransferListener(NewFileTransferListener listener) throws JoynServiceException {
		/*if (ctx.checkCallingOrSelfPermission(Permissions.RCS_FILETRANSFER_RECEIVE) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_FILETRANSFER_RECEIVE");
		}*/
		Logger.i(TAG, "addNewFileTransferListener() entry " + listener + " api="+api);
		if (api != null) {
			try {
				api.addNewFileTransferListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}

	/**
	 * Unregisters a file transfer invitation listener
	 * 
	 * @param listener New file transfer listener
	 * @throws JoynServiceException
	 */
	public void removeNewFileTransferListener(NewFileTransferListener listener) throws JoynServiceException {
		/*if (ctx.checkCallingOrSelfPermission(Permissions.RCS_FILETRANSFER_RECEIVE) != PackageManager.PERMISSION_GRANTED) {
			throw new SecurityException(" Required permission RCS_FILETRANSFER_RECEIVE");
		}*/
		Logger.i(TAG, "removeNewFileTransferListener() entry " + listener + " api="+api);
		if (api != null) {
			try {
				api.removeNewFileTransferListener(listener);
			} catch(Exception e) {
				throw new JoynServiceException(e.getMessage());
			}
		} else {
			throw new JoynServiceNotAvailableException();
		}
	}
}
