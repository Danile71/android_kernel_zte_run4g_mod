package com.mediatek.voiceextension.swip;

public interface ISwipCallback {

    public void onSwipMessageNotify(String swipSetName, int apiType, int result,
            int extraMsg, Object extraObj);

}
