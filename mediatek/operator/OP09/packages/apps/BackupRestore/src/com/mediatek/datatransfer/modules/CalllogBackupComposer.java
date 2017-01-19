package com.mediatek.datatransfer.modules;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Utils;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CallLog;

public class CalllogBackupComposer extends Composer {
	
	private static final String CLASS_TAG = MyLogger.LOG_TAG + "/CalllogBackupComposer";
	private static final Uri mCalllogUri = CallLog.Calls.CONTENT_URI;
	private Cursor callLogs = null;
	private Writer mWriter = null;
	public CalllogBackupComposer(Context context) {
		super(context);
	}

	@Override
	public boolean init() {
		// TODO Auto-generated method stub
		boolean result = false;
		callLogs = mContext.getContentResolver().query(mCalllogUri, null, null,
                null, "date ASC");
		if(callLogs!=null){
			callLogs.moveToFirst();
			result = true;
		}
		MyLogger.logD(CLASS_TAG, "init():" + (result?"OK!!!":"FAILED!!!") + ",count:" + getCount());
		return result;
	}

	@Override
	public int getModuleType() {
		// TODO Auto-generated method stub
		return ModuleType.TYPE_CALLLOG;
	}

	
	
	@Override
	public void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		MyLogger.logD(CLASS_TAG, "onStart():mParentFolderPath:" + mParentFolderPath);
		if(getCount() > 0) {
            File path = new File(mParentFolderPath + File.separator + Constants.ModulePath.FOLDER_CALLLOG);
            if (!path.exists()) {
                path.mkdirs();
            }

            File file = new File(path.getAbsolutePath() + File.separator + Constants.ModulePath.NAME_CALLLOG);
            if (!file.exists()) {
                try {
                    file.createNewFile();
                } catch (Exception e) {
                    MyLogger.logE(CLASS_TAG, "onStart():file:" + file.getAbsolutePath());
                    MyLogger.logE(CLASS_TAG, "onStart():create file failed");
                }
            }

            try {
                mWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
            } catch (Exception e) {
                MyLogger.logE(CLASS_TAG, "new BufferedWriter failed");
            }
        }
	}
	
	/**
     * Describe <code>onEnd</code> method here.
     * 
     */
    public final void onEnd() {
        super.onEnd();
        try {
            MyLogger.logD(CLASS_TAG, "CalllogBackupComposer onEnd");
            if (mWriter != null) {
                MyLogger.logE(CLASS_TAG, "mWriter.close()");
                mWriter.close();
            }
        } catch (Exception e) {
            MyLogger.logE(CLASS_TAG, "mWriter.close() failed");
        }

        if (callLogs != null) {
        	callLogs.close();
        	callLogs = null;
        }
    }
    
	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		if(callLogs!=null&& !callLogs.isClosed()){
			return callLogs.getCount();
		}
		return -1;
	}

	@Override
	public boolean isAfterLast() {
		// TODO Auto-generated method stub
		boolean result = true;
		if (callLogs != null && !callLogs.isAfterLast()) {
            result = false;
        }
		return result;
	}

	@Override
	protected boolean implementComposeOneEntity() {
		// TODO Auto-generated method stub
		MyLogger.logD(CLASS_TAG, "implementComposeOneEntity() GO!");
		boolean result = false;
		if (callLogs != null && !callLogs.isAfterLast()) {
			CallLogsData callLogsData = new CallLogsData();
			callLogsData.id=callLogs.getInt(callLogs.getColumnIndex("_id"));
			callLogsData.new_Type = callLogs.getInt(callLogs.getColumnIndex(CallLog.Calls.NEW));
			callLogsData.type = callLogs.getInt(callLogs.getColumnIndex(CallLog.Calls.TYPE));
			callLogsData.name = callLogs.getString(callLogs.getColumnIndex(CallLog.Calls.CACHED_NAME));;
			callLogsData.date = callLogs.getLong(callLogs.getColumnIndex(CallLog.Calls.DATE));
			callLogsData.number = callLogs.getString(callLogs.getColumnIndex(CallLog.Calls.NUMBER));
			callLogsData.duration = callLogs.getLong(callLogs.getColumnIndex(CallLog.Calls.DURATION));
			callLogsData.number_type = callLogs.getInt(callLogs.getColumnIndex(CallLog.Calls.CACHED_NUMBER_TYPE));
			callLogsData.simid = Utils.simId2Slot(callLogs.getLong(callLogs.getColumnIndex(CallLog.Calls.SIM_ID)), mContext);
			MyLogger.logD(CLASS_TAG, "implementComposeOneEntity()  ==---CONTENT---==  "+callLogsData.toString());
			try {
				mWriter.write(combineVclwithSim(callLogsData));
				result = true;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}finally{
				callLogs.moveToNext();
			}
		}
		return result;
	}
	
	private String combineVclwithSim(CallLogsData callLogsData) {
		// TODO Auto-generated method stub
		StringBuilder mBuilder = new StringBuilder();
		mBuilder.append(CallLogsData.BEGIN_VCALL);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.SIMID+callLogsData.simid);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
//		mBuilder.append(CallLogsData.NEW+callLogsData.new_Type);
//		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.TYPE+callLogsData.type);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		String dateString = Utils.parseDate(callLogsData.date);
		MyLogger.logD(CLASS_TAG,"startsWith(DATE) = "+ dateString);
		mBuilder.append(CallLogsData.DATE+dateString);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.NUMBER+callLogsData.number);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.DURATION+callLogsData.duration);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.END_VCALL);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		return mBuilder.toString();
	}
	
	/**
	 * @deprecated refer {@link #combineVclwithSim}
	 * @param callLogsData
	 * @return
	 */
	private String combineVcl(CallLogsData callLogsData) {
		// TODO Auto-generated method stub
		StringBuilder mBuilder = new StringBuilder();
		mBuilder.append(CallLogsData.BEGIN_VCALL);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.ID+callLogsData.id);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.NEW+callLogsData.new_Type);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.TYPE+callLogsData.type);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.DATE+callLogsData.date);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.NAME+callLogsData.name);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.NUMBER+callLogsData.number);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.DURATION+callLogsData.duration);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.NMUBER_TYPE+callLogsData.number_type);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		mBuilder.append(CallLogsData.END_VCALL);
		mBuilder.append(CallLogsData.VCL_END_OF_LINE);
		return mBuilder.toString();
	}

}
