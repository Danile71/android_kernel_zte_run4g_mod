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
package com.mediatek.rcse.ipcall;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import org.gsma.joyn.JoynServiceException;
import org.gsma.joyn.ipcall.AudioCodec;
import org.gsma.joyn.ipcall.IIPCallPlayerListener;
import org.gsma.joyn.ipcall.IPCallPlayer;
import org.gsma.joyn.ipcall.IPCallPlayerListener;
import org.gsma.joyn.ipcall.VideoCodec;
import com.orangelabs.rcs.core.ims.protocol.rtp.MediaRegistry;
import com.orangelabs.rcs.core.ims.protocol.rtp.MediaRtpSender;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.audio.AudioFormat;
import com.orangelabs.rcs.core.ims.protocol.rtp.media.MediaException;
import com.orangelabs.rcs.core.ims.protocol.rtp.media.MediaInput;
import com.orangelabs.rcs.core.ims.protocol.rtp.media.MediaSample;
import android.hardware.Camera;
import android.media.MediaRecorder;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import com.orangelabs.rcs.platform.network.DatagramConnection;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.utils.CodecsUtils;
import com.orangelabs.rcs.utils.FifoBuffer;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;

import com.orangelabs.rcs.core.ims.protocol.rtp.stream.RtpStreamListener;

/**
 * IP call player offers an interface to manage the IP call player instance
 * independently of the joyn service. The IP call player is implemented in
 * the application side. The IP call player captures the audio/video from the
 * device micro/camera, encodes the audio/video into the selected formats,
 * streams the encoded audio samples and video frames over the network in RTP.
 *  
 * @author Jean-Marc AUFFRET
 */
public  class LiveIPCallPlayer extends IPCallPlayer implements Camera.PreviewCallback, RtpStreamListener {
    
	 /**
     * List of supported audio codecs
     */
    private AudioCodec[] supportedAudioCodecs = null; 
    
    /**
     * Selected audio codec
     */
    private AudioCodec selectedAudioCodec = null;
    
    /**
     * Temporary connection to reserve the port
     */
    private DatagramConnection temporaryConnection = null; 
    
    /**
     * Local RTP port
     */
    private int localAudioRtpPort;
    
    /**
     * Local RTP port
     */
    private int localVideoRtpPort;
    
    /**
     * Local socket sender
     */
    private LocalSocket localSocketSender;
    
    /**
     * Local socket receiver
     */
    private LocalSocket localSocketReceiver;
    
    /**
     * Local socket
     */
    private LocalServerSocket localServerSocket;
    
    /**
     * Timestamp increment
     */
    private int timestampInc = 100; // calculate it ?
    
    /***
     * Current time stamp
     */
    private long timeStamp = 0;
    
    /**
     * Local socket endpoint
     */    
    private static final String LOCAL_SOCKET = "com.orangelabs.rcs.service.api.client.media.audio.socket.player";
    
    /**
     * RTP sender session
     */
    private MediaRtpSender rtpSender = null;

    /**
     * Audio format
     */
    private AudioFormat audioFormat;
    
    /**
     * Local MediaRecorder object to capture mic and encode the stream
     */
    private MediaRecorder mediaRecorder;
    
	/**
     * AudioRenderer for RTP stream sharing
     */
    private LiveIPCallRenderer audioRenderer = null;        
    
	
    /**
     * RTP audio input
     */
    private AudioRtpInput rtpInput = null;
    
    /**
     * Is player opened
     */
    private boolean opened = false;

    /**
     * Is player started
     */
    private boolean started = false;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
	
    /**
     * IP call player error
     */
    public static class Error {
    	/**
    	 * Internal error
    	 */
    	public final static int INTERNAL_ERROR = 0;
    	
    	/**
    	 * Network connection failed
    	 */
    	public final static int NETWORK_FAILURE = 1;
    	
        private Error() {
        }    	
    }

    /**
     * IP call player event listeners
     */
    private Set<IPCallPlayerListener> listeners = new HashSet<IPCallPlayerListener>();
    
    /**
     * IP call player event listeners
     */
    private Set<IIPCallPlayerListener> ilisteners = new HashSet<IIPCallPlayerListener>();
    
    /**
     * Constructor
     */
    public LiveIPCallPlayer() {
    	
    	// Set the local RTP port
        localAudioRtpPort = NetworkRessourceManager.generateLocalRtpPort();
        reservePort(localAudioRtpPort);
        
     // Set the local RTP port
        localVideoRtpPort = NetworkRessourceManager.generateLocalRtpPort();
        reservePort(localVideoRtpPort);
        
        // Init codecs
        supportedAudioCodecs = CodecsUtils.getSupportedAudioCodecList();   

        // Set the default media codec
        if (supportedAudioCodecs.length > 0) {
            setAudioCodec(supportedAudioCodecs[0]);
        }
    }
    
    /**
     * Release the reserved port.
     */
    private void releasePort() {
        if (temporaryConnection != null) {
            try {
                temporaryConnection.close();
            } catch (IOException e) {
                temporaryConnection = null;
            }
        }
    }

    /**
     * Set audio codec
     *
     * @param mediaCodec Audio codec
     */
	public void setAudioCodec(AudioCodec mediaCodec) {
        if (checkAudioCodec(supportedAudioCodecs, mediaCodec)) {
            selectedAudioCodec = mediaCodec;
            audioFormat = (AudioFormat) MediaRegistry.generateFormat(mediaCodec.getEncoding());
        } else {
            //notifyPlayerEventError("Codec not supported");
        }           
	}
	
	 public static boolean checkAudioCodec(AudioCodec[] supportedCodecs, AudioCodec codec) {
	        for (int i = 0; i < supportedCodecs.length; i++) {
	            if (codec.compare(supportedCodecs[i])) {
	                return true;
	            }
	        }
	        return false;
    }

    /**
	 * Opens the player and prepares resources (e.g. encoder, micro, camera)
	 * 
	 * @param audiocodec Audio codec
	 * @param videocodec Video codec
	 * @param remoteHost Remote RTP host
	 * @param remoteAudioPort Remote audio RTP port
	 * @param remoteVideoPort Remote video RTP port
	 */
	public void open(AudioCodec audiocodec, VideoCodec videocodec, String remoteHost, int remoteAudioPort, int remoteVideoPort) {
		
		 if (opened) {
	            // Already opened
	        	if (logger.isActivated()) {
	        		logger.info("audioplayer open : already opened");
	        	}
	            return;
	        }		
	        
			if (logger.isActivated()) {
				logger.info("open the audioplayer");
			}
	        
	        // Init the socket listener thread
	        new LiveAudioPlayerSocketListener().start();
	        
	        // Init the RTP layer
	        try {
	            releasePort();
	            rtpSender = new MediaRtpSender(audioFormat, localAudioRtpPort);
	            rtpInput = new AudioRtpInput();
	            rtpInput.open();
	            if ( audioRenderer != null ) {
	            	// The audio renderer is supposed to be opened and so we used its RTP stream
	            	if (logger.isActivated()) {
	            		logger.info("audioplayer share the audio renderer rtp stream on same port");
	            	}
	            	rtpSender.prepareSession(rtpInput, remoteHost, remoteAudioPort, audioRenderer.getRtpInputStream(), this);
	            } else { 
	            	// The audio renderer doesn't exist and so we create a new RTP stream
	            	rtpSender.prepareSession(rtpInput, remoteHost, remoteAudioPort, this);
	            }
	        } catch (Exception e) {
	            //notifyPlayerEventError(e.getMessage());
	            return;
	        }
	        
	        // Player is opened
	        opened = true;
	}
	

	/**
     * Audio RTP input
     */
    private static class AudioRtpInput implements MediaInput {
        /**
         * Received frames
         */
    private FifoBuffer fifo = null;
    /**
     * Constructor
     */
    public AudioRtpInput() {
    }

    /**
     * Add a new audio sample
     *
     * @param data Data
     * @param timestamp Timestamp
     * @param marker Marker bit 
     */
    public void addSample(byte[] data, long timestamp) {
        if (fifo != null) {
            MediaSample sample = new MediaSample(data, timestamp);
            fifo.addObject(sample);
        }
    }

    /**
     * Open the player
     */
    public void open() {
        fifo = new FifoBuffer();
    }

    /**
     * Close the player
     */
    public void close() {
        if (fifo != null) {
            fifo.close();
            fifo = null;
        }
    }

    /**
     * Read an media sample (blocking method)
     *
     * @return Media sample
     * @throws MediaException
     */
    public MediaSample readSample() throws MediaException {
        try {
            if (fifo != null) {
                return (MediaSample)fifo.getObject();
            } else {
                throw new MediaException("Media audio input not opened");
            }
        } catch (Exception e) {
            throw new MediaException("Can't read media audio sample");
        }
    }    
}

/**
 * Thread that listen from local socket connection and read bytes from it to add in RTP stream
 *
 */
class LiveAudioPlayerSocketListener extends Thread {
    
    public LiveAudioPlayerSocketListener() {
    }
    
    @Override
    public void run() {

        try {
            localServerSocket = new LocalServerSocket(LOCAL_SOCKET);
            
            while (true) {
            	
                localSocketReceiver = localServerSocket.accept();

                if (localSocketReceiver != null) { 
                    
                    // Reading bytes from the socket
                	
        	 		InputStream in = localSocketReceiver.getInputStream();
        	        int len = 0; 
        	        byte[] b = new byte[1024];
        	        byte[] buffer;
        	        
        	        if (logger.isActivated()) {
        	        	logger.info("start reading inputstream in localsocket server");
        	        }
        	        
        	        while ((len = in.read(b)) >= 0) {
        	        	buffer = new byte[len];
        	        	for (int j = 0; j < len; j++) {
        	            	buffer[j] = b[j];
        	            }
        	            rtpInput.addSample(buffer, timeStamp);          
//            	        if (logger.isActivated()) {
//            	        	logger.info("addSample to rtp input: " + buffer.length);
//            	            StringBuilder sb = new StringBuilder();
//            	            for (int y = 0; y < buffer.length; y++) {
//            	            	sb.append(" "+Byte.valueOf(buffer[y]).toString());        	            	
//            	            }
//            	        	logger.info("addSample to rtp input: " + sb.toString());
//            	        }
        	            timeStamp += timestampInc; // needed ?
        	            
        	        }
        	        if (logger.isActivated()) {
        	        	logger.info("stop reading inputstream in localsocket server");
        	        }
                }
            }
        } catch (IOException e) {
           // Log.e(getClass().getName(), e.getMessage());
        }
    }
	}
	
	/**
	 * Closes the player and deallocates resources
	 * 
	 * @throws JoynServiceException
	 */
	public void close() {
	}
	
	/**
	 * Starts the player
	 */
	public void start() {
		
		 if (!opened) {
	            // Player not opened
	        	if (logger.isActivated()) {
	        		logger.info("audioplayer start : not opened");
	        	}
	            return;
	        }

	        if (started) {
	            // Already started
	        	if (logger.isActivated()) {
	        		logger.info("audioplayer start : already started");
	        	}
	            return;
	        }
	        
			if (logger.isActivated()) {
				logger.info("start the LiveAudioPlayer");
			}
	        
	        // Set and start the media recorder
	        startMediaRecorder();
	    	
	    	// Start the RTP sender
	    	rtpSender.startSession();
	    	
	    	// Start the media recorder
			mediaRecorder.start();
			if (logger.isActivated()) {
				logger.info("start MediaRecorder");
			}
			
	        // Player is started
	        started = true;
	        notifyPlayerEventStarted();
	}
	
	/**
     * Notify player event started
     */
    private void notifyPlayerEventStarted() {
        if (logger.isActivated()) {
            logger.info("Player is started");
        }
        Iterator<IIPCallPlayerListener> ite = ilisteners.iterator();
        while (ite.hasNext()) {
            try {
                ((IIPCallPlayerListener)ite.next()).onPlayerStarted();
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
    }
	 /**
     * Create, prepare and start the media recorder
     */
	public void startMediaRecorder() {
		// Create media recorder
		mediaRecorder = new MediaRecorder();
		if (logger.isActivated()) {
			logger.info("create MediaRecorder");
		}
		
		// Set media recorder listener 
		mediaRecorder.setOnErrorListener(new MediaRecorder.OnErrorListener() {

			@Override
			public void onError(MediaRecorder mr, int what, int extra) {
				if (logger.isActivated()) {
					logger.error("mediaRecorder error : reason=" + what);
				}
			}
		});
		
		// Set media recorder audio source, output format and audio encoder
    	mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);    	
    	mediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.AMR_WB);
    	mediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_WB);
    	if (logger.isActivated()) {
    		logger.info("set mediaRecorder source=MIC outputformat=AMR_WB audioencoder=AMR_WB");
    	}
		
		// Set output in a local socket
		localSocketSender = new LocalSocket(); 
		if (logger.isActivated()) {
			logger.info("new localSenderSocket");
		}

		try {
			 localSocketSender.connect(new LocalSocketAddress(LOCAL_SOCKET));
			 if (logger.isActivated()) {
				 logger.info("localSenderSocket connect locally to the thread");
			 }
		} catch (IOException e) {
			e.printStackTrace();
		}

		mediaRecorder.setOutputFile(localSocketSender.getFileDescriptor());
		if (logger.isActivated()) {
			logger.info("mediaRecorder local socket sender endpoint = " + LOCAL_SOCKET);
		}
    	
    	// Prepare the media recorder
    	try {
			mediaRecorder.prepare();
			if (logger.isActivated()) {
				logger.info("prepare mediaRecorder");
			}
		} catch (IllegalStateException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	
	/**
     * Reserve a port.
     *
     * @param port Port to reserve
     */
    private void reservePort(int port) {
        if (temporaryConnection == null) {
            try {
                temporaryConnection = NetworkFactory.getFactory().createDatagramConnection();
                temporaryConnection.open(port);
            } catch (IOException e) {
                temporaryConnection = null;
            }
        }
	}
	
	/**
	 * Stops the player
	 */
	public void stop() {
	}
	
	/**
	 * Returns the local RTP port used to stream audio
	 * 
	 * @return Port number
	 */
	public int getLocalAudioRtpPort() {
		return localAudioRtpPort;
	}
	
	/**
	 * Returns the current audio codec
	 * 
	 * @return Audo codec
	 */
	public AudioCodec getAudioCodec() {
		 if (selectedAudioCodec == null)
		return null;
	        else
	            return selectedAudioCodec;
	}

	/**
	 * Returns the list of audio codecs supported by the player
	 * 
	 * @return List of audio codecs
	 */
	public AudioCodec[] getSupportedAudioCodecs() {
		return supportedAudioCodecs;
	}

	/**
	 * Returns the local RTP port used to stream video
	 * 
	 * @return Port number
	 */
	public int getLocalVideoRtpPort() {
		return 0;
	}
	
	/**
	 * Returns the current video codec
	 * 
	 * @return Video codec
	 */
	public VideoCodec getVideoCodec() {
		return null;
	}

	/**
	 * Returns the list of video codecs supported by the player
	 * 
	 * @return List of video codecs
	 */
	public VideoCodec[] getSupportedVideoCodecs() {
		return CodecsUtils.getIPCallPlayerCodecList();
	}

	/**
	 * Returns the list of player event listeners
	 * 
	 * @return Listeners
	 */
	public Set<IPCallPlayerListener> getEventListeners() {
		return listeners;
	}

	/**
	 * Adds a listener on player events
	 * 
	 * @param listener Listener
	 */
	public void addEventListener(IIPCallPlayerListener listener) {
		ilisteners.add(listener);
	}

	
	@Override
	public void removeEventListener(IIPCallPlayerListener listener) {
		ilisteners.remove(listener);
	}
	
	/**
	 * Removes all listeners from player events
	 */
	public void removeAllEventListeners() {
		listeners.clear();
	}

	@Override
	public void rtpStreamAborted() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		// TODO Auto-generated method stub
		
	}
}
