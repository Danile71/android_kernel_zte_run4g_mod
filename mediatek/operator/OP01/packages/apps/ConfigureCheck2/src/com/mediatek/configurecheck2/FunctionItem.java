package com.mediatek.configurecheck2;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import android.telephony.TelephonyManager;

//APN
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.os.Handler;
import android.os.Bundle;
import android.os.Message;
import android.os.UserHandle;
import android.provider.Settings;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import java.util.*;
import android.os.SystemProperties;
import com.android.internal.telephony.TelephonyProperties;

// SUPL
import com.mediatek.lbs.em2.utils.AgpsInterface;
import com.mediatek.lbs.em2.utils.AgpsConfig;
import com.mediatek.lbs.em2.utils.SuplProfile;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

// BT
import android.bluetooth.BluetoothAdapter;

// MMS Roaming
import android.content.ComponentName;
import android.content.Intent;

class CheckLabAPN extends CheckItemBase {
    private List<SimInfoRecord> mSimInfoList = null;
    private Set<Integer> mSimNeedSetAPN = new HashSet<Integer>();

    private final static int CHECK_BASE = 0x000;
    private final static int CHECK_MMS = 0x001;
    private final static int CHECK_TYPE = 0x002;

    private final static String TAG = "CheckLabAPN";

    private int mCheckProperty = CHECK_BASE;
        
    CheckLabAPN (Context c, String key) {
        super(c, key);
        setTitle(R.string.apn_check_title);
        
        StringBuilder noteStr = new StringBuilder();
        if (key.equals(CheckItemKeySet.CI_APN_IMS2)) {
            noteStr.append(getContext().getString(R.string.set_apn) + "\n")
                   .append(getContext().getString(R.string.apn_name) + "\n")
                   .append(getContext().getString(R.string.apn_apn_lab) + "ims2" + "\n")
                   .append(getContext().getString(R.string.SOP_REFER))
                   .append(getContext().getString(R.string.SOP_LTE_NV_IOT));
        } else if (key.equals(CheckItemKeySet.CI_APN_APN)) {
            noteStr.append(getContext().getString(R.string.set_apn) + "\n")
                   .append(getContext().getString(R.string.apn_name) + "\n")
                   .append(getContext().getString(R.string.apn_apn_lab) + "apn")
                   .append(getContext().getString(R.string.SOP_REFER))
                   .append(getContext().getString(R.string.SOP_LTE_NS_IOT));
        } else if (key.equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
            noteStr.append(getContext().getString(R.string.set_apn) + "\n")
                   .append(getContext().getString(R.string.apn_name) + "\n")
                   .append(getContext().getString(R.string.apn_apn_lab) + "9cmri.com" + "\n")
                   .append(getContext().getString(R.string.apn_protocol) + "\n")
                   .append(getContext().getString(R.string.SOP_REFER))
                   .append(getContext().getString(R.string.SOP_IPV6));
        } else if (key.equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
            noteStr.append(getContext().getString(R.string.set_apn) + "\n")
                   .append(getContext().getString(R.string.note_bip_case))
                   .append(getContext().getString(R.string.apn_type_supl) + "\n")
                   .append(getContext().getString(R.string.SOP_REFER))
                   .append(getContext().getString(R.string.SOP_LTE_UICC_USIM));
        } else {
            noteStr.append(getContext().getString(R.string.set_apn) + "\n")
                   .append(getContext().getString(R.string.apn_name) + "\n")
                   .append(getContext().getString(R.string.apn_apn_lab) + "labwap3" + "\n")
                   .append(getContext().getString(R.string.apn_proxy_lab) + "\n")
                   .append(getContext().getString(R.string.apn_port_lab) + "\n");
            if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)) {
                mCheckProperty = mCheckProperty | CHECK_MMS;
                noteStr.append(getContext().getString(R.string.apn_mms) + "\n")
                       .append(getContext().getString(R.string.apn_mms_proxy) + "\n")
                       .append(getContext().getString(R.string.apn_mms_port) + "\n");
            } else if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_TYPE)) {
                mCheckProperty = mCheckProperty | CHECK_TYPE;
                noteStr.append(getContext().getString(R.string.apn_type) + "\n");
            } else if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
                mCheckProperty = mCheckProperty | CHECK_MMS;
                mCheckProperty = mCheckProperty | CHECK_TYPE;
                noteStr.append(getContext().getString(R.string.apn_mms) + "\n")
                       .append(getContext().getString(R.string.apn_mms_proxy) + "\n")
                       .append(getContext().getString(R.string.apn_mms_port) + "\n")
                       .append(getContext().getString(R.string.apn_type) + "\n");
            } else {
                mCheckProperty = CHECK_BASE;
            }
            if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)) {
                noteStr.append(getContext().getString(R.string.agps_doc) + "\n")
                       .append(getContext().getString(R.string.card_doc));
            } else if (key.equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
                noteStr.append(getContext().getString(R.string.iot_doc));
            }
        }

        setNote(noteStr.toString());
        if (key.equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
            setProperty(PROPERTY_CLEAR);//QA's suggestion
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }
        
        mSimInfoList = SimInfoManager.getInsertedSimInfoList(c);
        if (mSimInfoList.isEmpty()) {
            setProperty(PROPERTY_AUTO_CHECK);
        }
    }
    
    public check_result getCheckResult() {
        CTSCLog.v(TAG, "getCheckResult()");
        if (!isCheckable()) {
            return super.getCheckResult();
        }

        if (mSimInfoList.isEmpty()) {
            setValue(R.string.string_sim);
            return check_result.UNKNOWN;
        }

        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            StringBuilder valueStr = new StringBuilder();
            check_result result = check_result.RIGHT;
            
            for (SimInfoRecord simInfo : mSimInfoList) {
                int simNo = simInfo.mSimSlotId + 1;
                Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                            null, null, null, null);

                if (cursor_APN == null || !cursor_APN.moveToNext()) {
                    if (cursor_APN != null) {
                        cursor_APN.close();
                    }
                    valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_setted));
                    result = check_result.WRONG;
                    mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                } else {
                    String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));
                    String curAPNProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PROXY));
                    String curAPNPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PORT));
                    String curAPNMMS = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSC));
                    String curAPNType = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.TYPE));
                    String curAPNMMSProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPROXY));
                    String curAPNMMSPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPORT));
                    String curAPNProtocol = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PROTOCOL));

                    cursor_APN.close();
                    CTSCLog.v(TAG, "curAPN = " + curAPN);
                    if (getKey().equals(CheckItemKeySet.CI_APN_IMS2)) {
                        if (null==curAPN || !curAPN.equalsIgnoreCase("ims2")) {
                            result = check_result.WRONG;
                            mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                            if(valueStr.toString().length() != 0) {
                                valueStr.append("\n");
                            }
                            valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                        }
                        continue;
                    }
                    if (getKey().equals(CheckItemKeySet.CI_APN_APN)) {
                        if (null==curAPN || !curAPN.equalsIgnoreCase("apn")) {
                            result = check_result.WRONG;
                            mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                            if(valueStr.toString().length() != 0) {
                                valueStr.append("\n");
                            }
                            valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                        }
                        continue;
                    }
                    if (getKey().equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
                        if (null==curAPNProtocol || !curAPNProtocol.equalsIgnoreCase("IPV4V6")) {
                            result = check_result.WRONG;
                            mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                            if(valueStr.toString().length() != 0) {
                                valueStr.append("\n");
                            }
                            valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                        }
                        continue;
                    }
                    if (getKey().equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
                        if ((null==curAPNType || !curAPNType.contains("supl"))
                                || (null==curAPN || !curAPN.equalsIgnoreCase("test"))) {
                            result = check_result.WRONG;
                            mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                            if(valueStr.toString().length() != 0) {
                                valueStr.append("\n");
                            }
                            valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                        }
                        continue;
                    }
                    
                    if ( (null!=curAPN && curAPN.equalsIgnoreCase("labwap3"))
                            && (null!=curAPNProxy && curAPNProxy.equalsIgnoreCase("192.168.230.8"))
                            && (null!=curAPNPort && curAPNPort.equalsIgnoreCase("9028")) ) {
                        if ((mCheckProperty & CHECK_MMS) != 0) {
                            if ( (null==curAPNMMS || !curAPNMMS.equalsIgnoreCase("http://218.206.176.175:8181/was"))
                                    || (null==curAPNMMSProxy || !curAPNMMSProxy.equalsIgnoreCase("192.168.230.8"))
                                    || (null==curAPNMMSPort || !curAPNMMSPort.equalsIgnoreCase("9028")) ) {
                                result = check_result.WRONG;
                                mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                                if(valueStr.toString().length() != 0) {
                                    valueStr.append("\n");
                                }
                                valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                            }
                        }

                        if ((mCheckProperty & CHECK_TYPE) != 0) {
                            if (null==curAPNType || !curAPNType.equals("default,mms,net")) {
                                result = check_result.WRONG;
                                mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                                if(valueStr.toString().length() != 0) {
                                    valueStr.append("\n");
                                }
                                valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                            }
                        }
                    } else {
                        result = check_result.WRONG;
                        mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                        if(valueStr.toString().length() != 0) {
                            valueStr.append("\n");
                        }
                        valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                    }
                }
            }

            setValue(valueStr.toString());
            return result;
        } else {
            Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers/preferapn"),
                                            null, null, null, null);
            if (cursor_APN == null || !cursor_APN.moveToNext()) {
                if (cursor_APN != null) {
                    cursor_APN.close();
                }
            
                setValue(R.string.apn_not_setted);
                return check_result.WRONG;
            } else {
                String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));
                String curAPNProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PROXY));
                String curAPNPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PORT));
                String curAPNMMS = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSC));
                String curAPNType = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.TYPE));
                String curAPNMMSProxy = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPROXY));
                String curAPNMMSPort = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.MMSPORT));
                String curAPNProtocol = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.PROTOCOL));
                    
                cursor_APN.close();
                CTSCLog.v(TAG, "curAPN = " + curAPN);
                if (getKey().equals(CheckItemKeySet.CI_APN_IMS2)) {
                    if (null==curAPN || !curAPN.equalsIgnoreCase("ims2")) {
                        setValue(R.string.apn_not_correct);
                        return check_result.WRONG;
                    } else {
                        setValue("");
                        return check_result.RIGHT;
                    }
                }
                if (getKey().equals(CheckItemKeySet.CI_APN_APN)) {
                    if (null==curAPN || !curAPN.equalsIgnoreCase("apn")) {
                        setValue(R.string.apn_not_correct);
                        return check_result.WRONG;
                    } else {
                        setValue("");
                        return check_result.RIGHT;
                    }
                }
                if (getKey().equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
                    if (null==curAPNProtocol || !curAPNProtocol.equalsIgnoreCase("IPV4V6")) {
                        setValue(R.string.apn_not_correct);
                        return check_result.WRONG;
                    } else {
                        setValue("");
                        return check_result.RIGHT;
                    }
                }
                if (getKey().equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
                    if ((null==curAPNType || !curAPNType.contains("supl"))
                            || (null==curAPN || !curAPN.equalsIgnoreCase("test"))) {
                        setValue(R.string.apn_not_correct);
                        return check_result.WRONG;
                    } else {
                        setValue("");
                        return check_result.RIGHT;
                    }
                }

                if ( (null!=curAPN && curAPN.equalsIgnoreCase("labwap3"))
                        && (null!=curAPNProxy && curAPNProxy.equalsIgnoreCase("192.168.230.8"))
                        && (null!=curAPNPort && curAPNPort.equalsIgnoreCase("9028")) ) {
                    if ((mCheckProperty & CHECK_MMS) != 0) {
                        if ( (null==curAPNMMS || !curAPNMMS.equalsIgnoreCase("http://218.206.176.175:8181/was"))
                                || (null==curAPNMMSProxy || !curAPNMMSProxy.equalsIgnoreCase("192.168.230.8"))
                                || (null==curAPNMMSPort || !curAPNMMSPort.equalsIgnoreCase("9028")) ){
                            setValue(R.string.apn_not_correct);
                            return check_result.WRONG;
                        }
                    }

                    if ((mCheckProperty & CHECK_TYPE) != 0) {
                        if (null == curAPNType || !curAPNType.equals("default,mms,net")) {
                            setValue(R.string.apn_not_correct);
                            return check_result.WRONG;
                        }
                    }

                    setValue("");
                    return check_result.RIGHT;
                } else {
                    setValue(R.string.apn_not_correct);
                    return check_result.WRONG;
                }
            }
        }

    }    

    public boolean onReset() {
        CTSCLog.v(TAG, "onReset()");
        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            for (Integer simId : mSimNeedSetAPN) {
                String where = new String();
                APNBuilder apnBuilder = null;
                if (getKey().equals(CheckItemKeySet.CI_APN_IMS2)) {
                    where = "apn=\'ims2\'";
                    apnBuilder = new APNBuilder(getContext(), simId.intValue(), "ims2", "ims2");
                } else if (getKey().equals(CheckItemKeySet.CI_APN_APN)) {
                    where = "apn=\'apn\'";
                    apnBuilder = new APNBuilder(getContext(), simId.intValue(), "apn", "apn");
                } else if (getKey().equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
                    where = "protocol=\'IPV4V6\'";
                    apnBuilder = new APNBuilder(getContext(), simId.intValue(), "apn", "apn");
                } else if (getKey().equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
                    where = "type like \'%supl%\' and apn=\'test\'";
                    apnBuilder = new APNBuilder(getContext(), simId.intValue(), "test", "test");
                } else {
                    where = "apn=\'labwap3\'";
                    apnBuilder = new APNBuilder(getContext(), simId.intValue(), "labwap3", "labwap3");
                }
                where += " and mcc=\'" + apnBuilder.getMCC() + "\'"
                       + " and mnc=\'" + apnBuilder.getMNC() + "\'"
                       + " and numeric=\'" + apnBuilder.getSimOperator() + "\'";

                CTSCLog.v(TAG, "where = " + where);
                int simNo = simId.intValue() + 1;
                Uri uri= Uri.parse("content://telephony/carriers_sim" + simNo);
                Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "numeric"}, where, null, null);
                
                if ((cursor != null) && (true == cursor.moveToFirst())) {
                    cursor.moveToFirst();
                    int index = cursor.getColumnIndex("_id");
                    String apnId = cursor.getString(index);
                    
                    if (getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)
                            || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_TYPE)
                            || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
                        ContentValues values = new ContentValues();
                        values.put(Telephony.Carriers.NAME, "labwap3");
                        values.put(Telephony.Carriers.PROXY, "192.168.230.8");
                        values.put(Telephony.Carriers.PORT, "9028");

                        if ((mCheckProperty & CHECK_MMS) != 0) {
                            values.put(Telephony.Carriers.MMSC, "http://218.206.176.175:8181/was");
                            values.put(Telephony.Carriers.MMSPROXY, "192.168.230.8");
                            values.put(Telephony.Carriers.MMSPORT, "9028");
                        }

                        if ((mCheckProperty & CHECK_TYPE) != 0) {
                            values.put(Telephony.Carriers.TYPE, "default,mms,net");
                        }
                        
                        String whereUpdate = "_id=\"" + apnId + "\"";

                        getContext().getContentResolver().update(
                                uri,
                                values, 
                                whereUpdate, 
                                null);
                    }
                    
                    ContentValues values_prefer = new ContentValues();
                    values_prefer.put("apn_id", apnId);
                    getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                    values_prefer, 
                                    null, 
                                    null);

                } else {
                    if (getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)
                            || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_TYPE)
                            || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
                        apnBuilder.setProxy("192.168.230.8")
                                  .setPort("9028");
                        if ((mCheckProperty & CHECK_MMS) != 0) {
                            apnBuilder.setMMSC("http://218.206.176.175:8181/was");
                            apnBuilder.setMMSProxy("192.168.230.8");
                            apnBuilder.setMMSPort("9028");
                        }
                        if ((mCheckProperty & CHECK_TYPE) != 0) {
                            apnBuilder.setApnType("default,mms,net");
                        }
                    }
                    if (getKey().equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
                        apnBuilder.setApnProtocol("IPV4V6");
                    }
                    if (getKey().equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
                        apnBuilder.setApnType("supl");
                    }
                    apnBuilder.build()
                              .setAsCurrent();
                }

                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            String where = new String();
            APNBuilder apnBuilder = null;
            if (getKey().equals(CheckItemKeySet.CI_APN_IMS2)) {
                where = "apn=\'ims2\'";
                apnBuilder = new APNBuilder(getContext(), "ims2", "ims2");
            } else if (getKey().equals(CheckItemKeySet.CI_APN_APN)) {
                where = "apn=\'apn\'";
                apnBuilder = new APNBuilder(getContext(), "apn", "apn");
            } else if (getKey().equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
                where = "protocol=\'IPV4V6\'";
                apnBuilder = new APNBuilder(getContext(), "apn", "apn");
            } else if (getKey().equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
                where = "type like \'%supl%\' and apn=\'test\'";
                apnBuilder = new APNBuilder(getContext(), "test", "test");
            } else {
                where = "apn=\'labwap3\'";
                apnBuilder = new APNBuilder(getContext(), "labwap3", "labwap3");
            }
            where += " and mcc=\'" + apnBuilder.getMCC() + "\'"
                    + " and mnc=\'" + apnBuilder.getMNC() + "\'"
                    + " and numeric=\'" + apnBuilder.getSimOperator() + "\'";
            
            CTSCLog.v(TAG, "where = " + where);
            
            Uri uri = Uri.parse("content://telephony/carriers");
            Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "numeric"}, where, null, null);

            if ((cursor != null) && (true == cursor.moveToFirst())) {
                cursor.moveToFirst();

                int index = cursor.getColumnIndex("_id");
                String apnId = cursor.getString(index);
                
                if (getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)
                        || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_TYPE)
                        || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
                    ContentValues values = new ContentValues();
                    values.put(Telephony.Carriers.NAME, "labwap3");
                    values.put(Telephony.Carriers.PROXY, "192.168.230.8");
                    values.put(Telephony.Carriers.PORT, "9028");

                    if ((mCheckProperty & CHECK_MMS) != 0) {
                        values.put(Telephony.Carriers.MMSC, "http://218.206.176.175:8181/was");
                        values.put(Telephony.Carriers.MMSPROXY, "192.168.230.8");
                        values.put(Telephony.Carriers.MMSPORT, "9028");
                    }

                    if ((mCheckProperty & CHECK_TYPE) != 0) {
                        values.put(Telephony.Carriers.TYPE, "default,mms,net");
                    }
                    
                    String whereUpdate = "_id=\"" + apnId + "\"";
                    getContext().getContentResolver().update(
                            uri,
                            values, 
                            whereUpdate, 
                            null);
                }

                ContentValues values_prefer = new ContentValues();
                values_prefer.put("apn_id", apnId);
                getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers/preferapn"),
                                    values_prefer, 
                                    null, 
                                    null);
            } else {
                if (getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS)
                        || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_TYPE)
                        || getKey().equals(CheckItemKeySet.CI_LABAPN_CHECK_MMS_TYPE)) {
                    apnBuilder.setProxy("192.168.230.8")
                              .setPort("9028");
                    if ((mCheckProperty & CHECK_MMS) != 0) {
                        apnBuilder.setMMSC("http://218.206.176.175:8181/was");
                        apnBuilder.setMMSProxy("192.168.230.8");
                        apnBuilder.setMMSPort("9028");
                    }
                    if ((mCheckProperty & CHECK_TYPE) != 0) {
                        apnBuilder.setApnType("default,mms,net");
                    }
                }
                if (getKey().equals(CheckItemKeySet.CI_APN_PROTOCOL)) {
                    apnBuilder.setApnProtocol("IPV4V6");
                }
                if (getKey().equals(CheckItemKeySet.CI_APN_TYPE_SUPL)) {
                    apnBuilder.setApnType("supl");
                }
                apnBuilder.build()
                          .setAsCurrent();
            }
            
            if (null != cursor) {
                cursor.close();
            }
        }

        return true;
    }
}

class CheckSUPL extends CheckItemBase {
    
    protected AgpsInterface agpsInterface;
    
    CheckSUPL (Context c, String key) {
        super(c, key);

        setTitle(R.string.supl_check_title);

        if (key.equals(CheckItemKeySet.CI_SUPL_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }

        StringBuilder suplStr = new StringBuilder();
        suplStr.append(getContext().getString(R.string.supl_address) + "\n");
        suplStr.append(getContext().getString(R.string.supl_port) + "\n");
        suplStr.append(getContext().getString(R.string.supl_tls) + "\n");
        suplStr.append(getContext().getString(R.string.supl_net_request) + "\n");
        suplStr.append(getContext().getString(R.string.supl_log_to_file) + "\n");
        suplStr.append(getContext().getString(R.string.agps_doc));
        setNote(suplStr.toString());
        
        initAgpsInterface();
    }
    
   	protected void initAgpsInterface() {
		    try {
			      agpsInterface = new AgpsInterface();
		    } catch (Exception e) {
			      CTSCLog.e("CheckSUPL", "agps interface connection failure");
			      e.printStackTrace();
		    }
 	  }
    
    public check_result getCheckResult() {
    	  CTSCLog.i("CheckSUPL", "getCheckResult()");
        if (agpsInterface == null) {
        	  CTSCLog.e("CheckSUPL", "agpsInterface is null");
			      return check_result.UNKNOWN;
		    }
		    
        AgpsConfig config = agpsInterface.getAgpsConfig();
        CTSCLog.i("CheckSUPL", "niRequest=" + config.getUpSetting().niRequest
            + " suplLog=" + config.getUpSetting().suplLog);
        if (!config.getUpSetting().niRequest || !config.getUpSetting().suplLog) {
            return check_result.WRONG;
        }
        
        CTSCLog.i("CheckSUPL", "addr=" + config.getCurSuplProfile().addr
            + " port=" + config.getCurSuplProfile().port
            + " tls=" + config.getCurSuplProfile().tls);
        if (config.getCurSuplProfile().addr.equalsIgnoreCase("218.206.176.50") &&  
            config.getCurSuplProfile().port == 7275 &&
            config.getCurSuplProfile().tls) {
            return check_result.RIGHT;
        } else {
            return check_result.WRONG;
        }
    }    
    
    public boolean onReset() {
    	  CTSCLog.i("CheckSUPL", "onReset()");
        return setSUPL(getContext());
    }

    private boolean setSUPL(Context context) {
        if (agpsInterface == null) {
        	  CTSCLog.e("CheckSUPL", "agpsInterface is null");
			      return false;
		    }
		    
		    // set net work request
		    agpsInterface.setAllowNI(true);
		    agpsInterface.setSupl2file(true);

        // look for cmcc lab profile
        //for (MtkAgpsProfile profile: profileMgr.getAllProfile()) {
        //    if(profile.name.equalsIgnoreCase("Lab - CMCC")) {
        //        agpsMgr.setProfile(profile);
        //        return true;
         //   }
        //}
        
        try {
        	  SuplProfile profile = agpsInterface.getAgpsConfig().getCurSuplProfile();
        	  profile.addr = "218.206.176.50";
        	  profile.port = 7275;
        	  profile.tls = true;
        	  agpsInterface.setSuplProfile(profile);
		    } catch (Exception e) {
			      e.printStackTrace();
		    }

        return true;
    }

}


class APNBuilder {
    private final static String TAG = "CheckLabAPN";
    private Context mContext = null;
    private APN mAPN = null;
    private Uri mUri = null;
    private int mSlot = -1;

    public APNBuilder(Context context, String apnName, String apn) {
        CTSCLog.v(TAG, "APNBuilder: apnName = " + apnName + " apn = " + apn);
        mContext = context;

        mAPN = new APN();
        mAPN.name = apnName;
        mAPN.apn = apn;
    }

    public APNBuilder(Context context, int slotId, String apnName, String apn) {
        CTSCLog.v(TAG, "APNBuilder: apnName = " + apnName + " apn = " + apn);
        mContext = context;

        mAPN = new APN();
        mAPN.name = apnName;
        mAPN.apn = apn;

        mSlot = slotId;
    }

    // for gemini
    public APNBuilder build() {
        CTSCLog.v(TAG, "build()");
        ContentValues values = new ContentValues();
        setParams(values);

        if(mSlot == -1) {
            mUri = mContext.getContentResolver().insert(
                                 Uri.parse("content://telephony/carriers"),
                                 values
                                 );
        } else {
            int simNo = mSlot + 1;
            mUri = mContext.getContentResolver().insert(
                                     Uri.parse("content://telephony/carriers_sim" + simNo),
                                     values
                                     );
        }

        return this;
    }

    private void setParams(ContentValues values) {
        CTSCLog.v(TAG, "setParams: mAPN.name = " + mAPN.name + " mAPN.apn = " + mAPN.apn);
        values.put(Telephony.Carriers.NAME, mAPN.name);
        values.put(Telephony.Carriers.APN, mAPN.apn);
        
        values.put(Telephony.Carriers.MCC, getMCC());
        values.put(Telephony.Carriers.MNC, getMNC());
        values.put(Telephony.Carriers.NUMERIC, getSimOperator());

        if(mAPN.proxy != null) {
            values.put(Telephony.Carriers.PROXY, mAPN.proxy);
        }

        if(mAPN.port != null) {
            values.put(Telephony.Carriers.PORT, mAPN.port);
        }

        if(mAPN.uName != null) {
            values.put(Telephony.Carriers.USER, mAPN.uName);
        }

        if(mAPN.pwd != null) {
            values.put(Telephony.Carriers.PASSWORD, mAPN.pwd);
        }

        if(mAPN.server != null) {
            values.put(Telephony.Carriers.SERVER, mAPN.server);
        }

        if(mAPN.mmsc != null) {
            values.put(Telephony.Carriers.MMSC, mAPN.mmsc);
        }

        if(mAPN.mmsproxy != null) {
            values.put(Telephony.Carriers.MMSPROXY, mAPN.mmsproxy);
        }

        if(mAPN.mmsport != null) {
            values.put(Telephony.Carriers.MMSPORT, mAPN.mmsport);
        }

        // TODO: check if type is String
        if(mAPN.authtype != null) {
            values.put(Telephony.Carriers.AUTH_TYPE, mAPN.authtype);
        }

        if(mAPN.apntype != null) {
            values.put(Telephony.Carriers.TYPE, mAPN.apntype);
        }
        
        if(mAPN.protocol != null) {
            values.put(Telephony.Carriers.PROTOCOL, mAPN.protocol);
        }
    }

    public void setAsCurrent() {
        CTSCLog.v(TAG, "setAsCurrent()");
        if (mUri == null) {
            return;
        }
        Cursor cur = mContext.getContentResolver().query(mUri, null, null, null, null);
        cur.moveToFirst();
        int index = cur.getColumnIndex("_id");
        String apnId = cur.getString(index);
        cur.close();

        ContentValues values = new ContentValues();
        values.put("apn_id", apnId);
        CTSCLog.v(TAG, "setAsCurrent: apn_id = " + apnId);
        if (mSlot == -1) {
            mContext.getContentResolver().update(
                                Uri.parse("content://telephony/carriers/preferapn"), 
                                values, 
                                null, 
                                null);
        } else {
            int simNo = mSlot + 1;
            mContext.getContentResolver().update(
                                Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                values, 
                                null, 
                                null);
        }
    }

    public APNBuilder setProxy(String proxy) {
        mAPN.proxy = proxy;

        return this;
    }

    public APNBuilder setPort(String port) {
        mAPN.port = port;

        return this;
    }

    public APNBuilder setUserName(String name) {
        mAPN.uName = name;

        return this;
    }

    public APNBuilder setPassword(String pwd) {
        mAPN.pwd = pwd;

        return this;
    }

    public APNBuilder setServer(String server) {
        mAPN.server = server;

        return this;
    }

    public APNBuilder setMMSC(String mmsc) {
        mAPN.mmsc = mmsc;

        return this;
    }

    public APNBuilder setMMSProxy(String mmsproxy) {
        mAPN.mmsproxy = mmsproxy;

        return this;
    }

    public APNBuilder setMMSPort(String mmsport) {
        mAPN.mmsport = mmsport;

        return this;
    }

    public APNBuilder setAuthType(String authtype) {
        mAPN.authtype = authtype;

        return this;
    }

    public APNBuilder setApnType(String apntype) {
        mAPN.apntype = apntype;

        return this;
    }
    
    public APNBuilder setApnProtocol(String protocol) {
        mAPN.protocol = protocol;

        return this;
    }

    String getMCC() {
        String oprator = getSimOperator();
        if (null == oprator) {
            return "";
        }
        return oprator.substring(0,3);
    }

    String getMNC() {
        String oprator = getSimOperator();
        if (null == oprator) {
            return "";
        }
        return oprator.substring(3);
    }

    String getSimOperator() {
        String oprator = null;
        
        switch(mSlot) {
            case -1:
            case 0:
                oprator = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
                break;
            case 1:
                oprator = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2, "-1");
                break;
            default:
                break;
        }

        return oprator;
    }

    class APN {
        String name;
        String apn;
        String proxy;
        String port;
        String uName;
        String pwd;
        String server;
        String mmsc;
        String mmsproxy;
        String mmsport;
        String mcc;
        String mnc;
        String authtype;
        String apntype;
        String protocol;
    }
}

class CheckSMSC extends CheckItemBase {
    private boolean mSyncDone = false;
    private boolean mNeedNotify = false;

    private List<SimInfoRecord> mSimInfoList = null;

    private static final int MSG_GET_SMSC_DONE = 1;
    private static final int MSG_SET_SMSC_DONE = 2;
    
    private final Handler mSMSCHandler = new Handler() {
        public final void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_GET_SMSC_DONE:
                    Bundle bd = (Bundle)msg.obj;
                    if(FeatureOption.MTK_GEMINI_SUPPORT == true) {

                        mResult = check_result.RIGHT;
                        StringBuilder smscStr = new StringBuilder();
                        for (SimInfoRecord simInfo : mSimInfoList) {
                            String curStr = bd.getString("SMSC" + simInfo.mSimSlotId);

                            CTSCLog.v("CheckSMSC", "SMSC" + simInfo.mSimSlotId + " " + curStr);
                            if (curStr != null && false == curStr.equalsIgnoreCase("+8613800100569")) {
                                if(smscStr.toString().length() != 0) {
                                    smscStr.append("\n");
                                }
                                smscStr.append("Sim" + (simInfo.mSimSlotId+1) + getContext().getString(R.string.smsc_not_correct));
                                mResult = check_result.WRONG;
                            }
                        }
                        setValue(smscStr.toString());
                        
                    } else {
                        String smscStr = bd.getString("SMSC");
                    
                        if (smscStr != null && smscStr.equalsIgnoreCase("+8613800100569")) {
                            setValue("");
                            mResult = check_result.RIGHT;
                        } else {
                            setValue(R.string.smsc_not_correct);
                            mResult = check_result.WRONG;
                        }

                    }
                    
                    mSyncDone = true;
                    if(true == mNeedNotify) {
                        sendBroadcast();
                    }
                    break;
                case MSG_SET_SMSC_DONE:
                    CTSCLog.v("SMSC", " set done");
                    sendBroadcast();
                    break;
                default:
                    return;
            }
        }
    };
    
    CheckSMSC(Context c, String key) {
        super(c, key);

        setTitle(R.string.smsc_check_title);

        if (key.equals(CheckItemKeySet.CI_SMSC_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
        }

        mSimInfoList = SimInfoManager.getInsertedSimInfoList(c);

        if (mSimInfoList.isEmpty()) {
            setProperty(PROPERTY_AUTO_CHECK);
        }

        StringBuilder smscNote = new StringBuilder();
        smscNote.append(getContext().getString(R.string.smsc_number));
        smscNote.append("\n" + getContext().getString(R.string.agps_doc));
        
        setNote(smscNote.toString());
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }

    public boolean onCheck() {
        if (mSimInfoList.isEmpty()) {
                setValue(R.string.string_sim);
                return false;
        }
        
        new Thread(new Runnable() {
                public void run() {
                    TelephonyManagerEx telephonyMgr = TelephonyManagerEx.getDefault();
                            
                    if(FeatureOption.MTK_GEMINI_SUPPORT == true) {
                        Bundle bd = new Bundle();
                            
                        for (SimInfoRecord simInfo : mSimInfoList) {
                            int slotId = simInfo.mSimSlotId;
                            String smscStr = telephonyMgr.getScAddress(slotId);
                            bd.putString("SMSC" + slotId, smscStr);

                            CTSCLog.v("CheckSMSC", "Get Result SMSC" + slotId + " " + smscStr);
                        }
                        
                        mSMSCHandler.sendMessage(mSMSCHandler.obtainMessage(MSG_GET_SMSC_DONE, 0, 0, bd));
                    } else {
                        String smscStr = telephonyMgr.getScAddress(0);
                        Bundle bd = new Bundle();
                        bd.putString("SMSC", smscStr);
                        mSMSCHandler.sendMessage(mSMSCHandler.obtainMessage(MSG_GET_SMSC_DONE, 0, 0, bd));
                    }
                }
            }).start();
        mSyncDone = false;

        return super.onCheck();
    }
    
    public check_result getCheckResult() {
        
        if (!isCheckable()) {
            return super.getCheckResult();
        }

        if (mSimInfoList.isEmpty()) {
                setValue(R.string.string_sim);
                return check_result.UNKNOWN;
        }

        if (mSyncDone == false) {
            mResult = check_result.UNKNOWN;
            mNeedNotify = true;
            setValue(R.string.string_checking);
            return mResult;
        }

        mNeedNotify = false;
        return mResult;
    }    

    public boolean onReset() {
        if(!isCheckable()) {
            return false;
        }

        new Thread(new Runnable() {
            public void run() {
                TelephonyManagerEx telephonyMgr = TelephonyManagerEx.getDefault();
        
                if(FeatureOption.MTK_GEMINI_SUPPORT == true) {
                    for (SimInfoRecord simInfo : mSimInfoList) {
                        telephonyMgr.setScAddress("+8613800100569", simInfo.mSimSlotId);
                    }
                } else {
                    telephonyMgr.setScAddress("+8613800100569", 0);
                }
                
                mSMSCHandler.sendEmptyMessage(MSG_SET_SMSC_DONE);
            }
        }).start();
        
        return true;
    }
}

class CheckCurAPN extends CheckItemBase {
    private List<SimInfoRecord> mSimInfoList = null;
    private Set<Integer> mSimNeedSetAPN = new HashSet<Integer>();
    private String checkAPN = null;
    
    // numeric list  key for gemini
    //copy from ApnUtils.java (file of apn in Settings app)
    private static final String NUMERIC_LIST[] = {
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC,
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_2,
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_3,
            TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC_4
     };
    
    CheckCurAPN(Context c, String key) {
        super(c, key);

        setTitle(R.string.apn_check_title);

        if (key.equals(CheckItemKeySet.CI_CMWAP_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
            checkAPN = "cmwap";
            setNote(R.string.apn_cmwap);
            StringBuilder apn_cmwap = new StringBuilder();
            apn_cmwap.append(getContext().getString(R.string.apn_cmwap));
            apn_cmwap.append("\n" + getContext().getString(R.string.internet_doc));
        
            setNote(apn_cmwap.toString());
        } else if (key.equals(CheckItemKeySet.CI_CMWAP_AUTO_CONFG) || key.equals(CheckItemKeySet.CI_CMWAP)){
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
            checkAPN = "cmwap";
            StringBuilder apn_cmwap = new StringBuilder();
            apn_cmwap.append(getContext().getString(R.string.apn_cmwap));
            apn_cmwap.append("\n" + getContext().getString(R.string.internet_doc));
        
            setNote(apn_cmwap.toString());
        } else if (key.equals(CheckItemKeySet.CI_CMNET_CHECK_ONLY)) {
            setProperty(PROPERTY_AUTO_CHECK);
            checkAPN = "cmnet";
            StringBuilder apn_cmnet = new StringBuilder();
            apn_cmnet.append(getContext().getString(R.string.apn_cmnet));
            apn_cmnet.append("\n" + getContext().getString(R.string.streaming_doc));
        
            setNote(apn_cmnet.toString());
        } else {
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);
            checkAPN = "cmnet";
            StringBuilder apn_cmnet = new StringBuilder();
            apn_cmnet.append(getContext().getString(R.string.apn_cmnet));
            apn_cmnet.append("\n" + getContext().getString(R.string.streaming_doc));
        
            setNote(apn_cmnet.toString());
        }

        mSimInfoList = SimInfoManager.getInsertedSimInfoList(c);

        if (mSimInfoList.isEmpty()) {
            setProperty(PROPERTY_AUTO_CHECK);
        }
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }

    public check_result getCheckResult() {
        if (!isCheckable()) {
            return super.getCheckResult();
        }

        if (mSimInfoList.isEmpty()) {
            setValue(R.string.string_sim);
            return check_result.UNKNOWN;
        }

        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            StringBuilder valueStr = new StringBuilder();
            check_result result = check_result.RIGHT;
            
            for (SimInfoRecord simInfo : mSimInfoList) {
                int simNo = simInfo.mSimSlotId + 1;
                Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                            null, null, null, null);

                if (cursor_APN == null || !cursor_APN.moveToNext()) {
                    if (cursor_APN != null) {
                        cursor_APN.close();
                    }

                    if(valueStr.toString().length() != 0) {
                        valueStr.append("\n");
                    }                    
                    valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_setted));
                    result = check_result.WRONG;
                    mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                } else {
                    String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));

                    cursor_APN.close();
                    
                    if (curAPN.equalsIgnoreCase(checkAPN)) {

                    } else {
                        result = check_result.WRONG;
                        if(valueStr.toString().length() != 0) {
                            valueStr.append("\n");
                        }
                        valueStr.append("Sim" + simNo + getContext().getString(R.string.apn_not_correct));
                        mSimNeedSetAPN.add(new Integer(simInfo.mSimSlotId));
                    }
                }
            }

            setValue(valueStr.toString());
            return result;
        } else {
            Cursor cursor_APN = getContext().getContentResolver().query(
                                            Uri.parse("content://telephony/carriers/preferapn"),
                                            null, null, null, null);
            if (cursor_APN == null || !cursor_APN.moveToNext()) {
                if (cursor_APN != null) {
                    cursor_APN.close();
                }
            
                setValue(R.string.apn_not_setted);
                return check_result.WRONG;
            } else {
                String curAPN = cursor_APN.getString(cursor_APN.getColumnIndex(Telephony.Carriers.APN));
                cursor_APN.close();

                if (curAPN.equalsIgnoreCase(checkAPN)) {
                    setValue("");
                    return check_result.RIGHT;
                } else {
                    setValue(R.string.apn_not_correct);
                    return check_result.WRONG;
                }
            }
        }
    }

    public boolean onReset() {
        if (true == FeatureOption.MTK_GEMINI_SUPPORT) {
            for (Integer simSlotId : mSimNeedSetAPN) {
                String numeric = SystemProperties.get(NUMERIC_LIST[simSlotId.intValue()], "-1");
                String where = "apn=\"" + checkAPN + "\"" + " and numeric=\"" + numeric + "\"";
                CTSCLog.v("CheckCurAPN", "where = " + where);
                
                int simNo = simSlotId.intValue() + 1;
                Uri uri= Uri.parse("content://telephony/carriers_sim" + simNo);
                Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "name"}, where, null, null);

                if ((cursor != null) && (true == cursor.moveToFirst())) {
                    cursor.moveToFirst();
                    int index = cursor.getColumnIndex("_id");
                    String apnId = cursor.getString(index);
                    CTSCLog.v("CheckCurAPN", "set apnId = " + apnId);

                    ContentValues values = new ContentValues();
                    values.put("apn_id", apnId);

                    getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers_sim" + simNo +"/preferapn"),
                                    values, 
                                    null, 
                                    null);
                }

                if (null != cursor) {
                    cursor.close();
                }
            }
        } else {
            CTSCLog.v("CheckCurAPN","Not support GEMINI");
            String numeric = SystemProperties.get(TelephonyProperties.PROPERTY_ICC_OPERATOR_NUMERIC, "-1");
            String where = "apn=\"" + checkAPN + "\"" + " and numeric=\"" + numeric + "\"";
            CTSCLog.v("CheckCurAPN", "where = " + where);
            
            Uri uri= Uri.parse("content://telephony/carriers");
            Cursor cursor = getContext().getContentResolver().query(uri, new String[] {
                                        "_id", "name"}, where, null, null);

            if ((cursor != null) && (true == cursor.moveToFirst())) {
                    cursor.moveToFirst();
                    int index = cursor.getColumnIndex("_id");
                    String apnId = cursor.getString(index);
                    CTSCLog.v("CheckCurAPN", "set apnId = " + apnId);
                    cursor.close();

                    ContentValues values = new ContentValues();
                    values.put("apn_id", apnId);

                    getContext().getContentResolver().update(
                                    Uri.parse("content://telephony/carriers/preferapn"),
                                    values, 
                                    null, 
                                    null);
            }
            if (null != cursor) {
                cursor.close();
            }
        }
        
        return true;
    }
}

class CheckBT extends CheckItemBase {
    private BluetoothAdapter mAdapter = null;
    
    CheckBT(Context c, String key) {
        super(c, key);

        setTitle(R.string.bt_name_check_title);

        mAdapter = BluetoothAdapter.getDefaultAdapter();

        StringBuilder sb = new StringBuilder();
        sb.append(getContext().getString(R.string.bt_cmcc_req));
        //sb.append("\n");
        //sb.append(getContext().getString(R.string.bt_doc));

        setNote(sb.toString());
    }

    public boolean onCheck() {
        if (mAdapter.isEnabled()) {
            StringBuilder sb = new StringBuilder();
            sb.append(getContext().getString(R.string.bt_name_current));
            sb.append(mAdapter.getName());
            
            setValue(sb.toString());
        } else {
            setValue(R.string.bt_open_device);
        }

        return true;
    }

    public boolean setCheckResult(check_result result) {
        //do something yourself, and check if allow user set
        return super.setCheckResult(result);
    }

    public check_result getCheckResult() {
        return super.getCheckResult();
    }

}

class CheckMMSRoaming extends CheckItemBase {
    CheckMMSRoaming(Context c, String key) {
        super(c, key);

        setProperty(PROPERTY_AUTO_FWD);
        setTitle(R.string.mms_roaming_title);

        StringBuilder sb = new StringBuilder();
        sb.append(getContext().getString(R.string.mms_roaming_setting));
        sb.append("\n");
        sb.append(getContext().getString(R.string.iot_doc));
        
        setNote(sb.toString());        
    }

    public Intent getForwardIntent() {
        Intent intent = new Intent();
        intent.setComponent(new ComponentName("com.android.mms", 
                "com.android.mms.ui.BootActivity"));

        if (getContext().getPackageManager().resolveActivity(intent, 0) != null) {
            return intent;
        } else {
            throw new RuntimeException(
                    "Can't find such activity: com.android.mms, " +
                    "com.android.mms.ui.BootActivity");
        }
    }
}

class CheckIPO extends CheckItemBase {
    private static final String TAG = " FunctionItem CheckIPO";
    private boolean mIpoEnabled = false;

    CheckIPO(Context c, String key) {
        super(c, key);

        setProperty(PROPERTY_AUTO_CHECK);
        setTitle(R.string.IPO_title);
        setNote(R.string.IPO_note);
    }

        @Override
    public boolean onCheck() {
        if (!FeatureOption.MTK_IPO_SUPPORT
                || UserHandle.myUserId() != UserHandle.USER_OWNER) {
            setValue(R.string.IPO_no_support_value);
            mResult = check_result.WRONG;
            CTSCLog.d("CheckIPO", "Not support IPO!");
            return true;
        } else {
            mIpoEnabled = Settings.System.getInt(getContext().getContentResolver(),
                    Settings.System.IPO_SETTING, 1) == 1;
            setProperty(PROPERTY_AUTO_CHECK|PROPERTY_AUTO_CONFG);            
            CTSCLog.d("CheckIPO", "Feature option is exists and query result is " + mIpoEnabled);
            if (mIpoEnabled) {
                setValue(R.string.IPO_on_value);
                mResult = check_result.RIGHT;
            } else {
                setValue(R.string.IPO_off_value);
                mResult = check_result.WRONG;
            }
            return true;
        }
    }
    
    public boolean onReset() {
        CTSCLog.i(TAG, "onReset");
        if (!isConfigurable()) {
            return false;
        }
        Settings.System.putInt(getContext().getContentResolver(), Settings.System.IPO_SETTING, 1); 
        setValue(R.string.IPO_on_value);
        mResult = check_result.RIGHT;        
        return true;
    }
}
