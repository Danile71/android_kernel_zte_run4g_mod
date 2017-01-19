package com.mediatek.keyguard.plugin;

import android.content.Context;
import android.provider.Telephony.SimInfo;

import com.mediatek.keyguard.ext.DefaultOperatorSIMString;
import com.mediatek.keyguard.ext.IOperatorSIMString.SIMChangedTag;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.xlog.Xlog;

import java.util.Locale;

public class OP09OperatorSIMStringImp extends DefaultOperatorSIMString {
    private static final String TAG = "OperatorSIMStringImp";
    private static final String SIM = "SIM";
    private static final String UIMSIM = "UIM/SIM";
    private static final String UIM = "UIM";

    @Override
    public String getOperatorSIMString(String sourceStr, int slotId,
            SIMChangedTag simChangedTag, Context context) {
        String retStr = sourceStr;
        Xlog.d(TAG, "getOperatorSIMString, slotId = " + slotId
                + " simChangedTag = " + simChangedTag + " sourceStr = "
                + sourceStr);
        if (isNeedProcessByLanguage()) {
            if ((simChangedTag == SIMChangedTag.SIMTOUIM)
                    && isOP09Card(slotId, context)) {
                retStr = retStr.replace(SIM, UIM);
            } else if (simChangedTag == SIMChangedTag.UIMSIM) {
                retStr = retStr.replace(SIM, UIMSIM);
            } else if (simChangedTag == SIMChangedTag.DELSIM) {
                retStr = delSim(retStr);
            }
        }

        Xlog
                .d(TAG, "getOperatorSIMString, processed string retStr = "
                        + retStr);
        return retStr;
    }

    public String getOperatorSIMStringForSIMDetection(String sourceStr,
            int newSimSlot, int newSimNumber, Context context) {
        String retStr = sourceStr;
        Xlog.d(TAG, "getOperatorSIMStringForSIMDetection, newSimSlot = "
                + newSimSlot + " sourceStr = " + sourceStr);
        if (isNeedProcessByLanguage()) {
            boolean hasUIM = false;
            int slotId = 0;
            while (newSimSlot != 0) {
                if ((newSimSlot & 0x01) != 0 && isOP09Card(slotId, context)) {
                    hasUIM = true;
                }
                slotId++;
                newSimSlot = newSimSlot >>> 1;
            }
            if (hasUIM) {
                if (newSimNumber == 1) {
                    retStr = retStr.replace(SIM, UIM);
                } else if (newSimNumber > 1) {
                    retStr = retStr.replace(SIM, UIMSIM);
                }
            }
        }

        Xlog.d(TAG,
                "getOperatorSIMStringForSIMDetection, processed string retStr = "
                        + retStr);
        return retStr;
    }

    /**
     * Whether the card in the slotId is main card.
     * 
     * @param slotId
     * @return
     */
    private boolean isOP09Card(int slotId, Context context) {

        if (null == SimInfoManager.getSimInfoBySlot(context, slotId)) {
            return false;
        }
        String strOperator = SimInfoManager.getSimInfoBySlot(context, slotId).mOperator;
        if (SimInfo.OPERATOR_OP09.equals(strOperator)) {
            return true;
        }
        return false;
    }

    /**
     * Delete the substring Sim from the sourceStr, according to the
     * local(English and simplified Chinese and traditional Chinese).
     */
    private String delSim(String sourceStr) {
        String retStr = sourceStr;
        Locale locale = Locale.getDefault();
        String countryStr = locale.getCountry();
        Xlog.d(TAG, " delSimFromStr, County = " + countryStr);
        if (isReplacedWithSpace(countryStr)) {
            retStr = sourceStr.replaceAll(" *" + SIM + " *", " ");
            retStr = toUppercaseFirstLetter(retStr);
        } else if (isReplaceToEmpyt(countryStr)) {
            retStr = retStr.replaceAll(" *" + SIM + " *", "");
        }
        retStr = retStr.trim();
        return retStr;
    }

    /**
     * Whether replace the string with space.
     * 
     * @param countryStr
     *            the country code
     * @return
     */
    private boolean isReplacedWithSpace(String countryStr) {
        return ("US".equals(countryStr) || "UK".equals(countryStr));
    }

    /**
     * Whether replace the string to empty.
     * 
     * @param countryStr
     *            the country code
     * @return
     */
    private boolean isReplaceToEmpyt(String countryStr) {
        return ("CN".equals(countryStr) || "TW".equals(countryStr));
    }

    /**
     * Replace the first letter of the word to Upper case.
     * 
     * @param sourceStr
     * @return
     */
    private String toUppercaseFirstLetter(String sourceStr) {
        String retStr = sourceStr.trim();
        if (retStr.length() > 1) {
            retStr = retStr.substring(0, 1).toUpperCase() + retStr.substring(1);
        } else if (retStr.length() == 1) {
            retStr = retStr.substring(0, 1).toUpperCase();
        }
        return retStr;
    }

    /**
     * Whether the language need to process
     * 
     * @return
     */
    private boolean isNeedProcessByLanguage() {
        Locale locale = Locale.getDefault();
        String countryStr = locale.getCountry();
        Xlog.d(TAG, " isNeedProcessByLanguage, County = " + countryStr);
        return ("CN".equals(countryStr) || "TW".equals(countryStr)
                || "US".equals(countryStr) || "UK".equals(countryStr));

    }
}
