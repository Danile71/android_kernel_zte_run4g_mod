package com.mediatek.widget;


import java.util.Iterator;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

import com.mediatek.widget.RenderThreadEx.OnDrawMavFrameListener;
import com.mediatek.widget.MpoDecodeHelper;

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Process;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.FloatMath;
import android.util.Log;
import android.util.LruCache;
import android.view.View;
import android.widget.ImageView;
import android.widget.QuickContactBadge;
import android.widget.AbsListView.OnScrollListener;
import android.graphics.Canvas;
import android.app.Activity;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.SystemProperties;

public class QuickContactBadgeEx extends QuickContactBadge {
    private static final String TAG = "QuickContactBadgeEx";
    public static boolean sAutoPlayBack = false;
    public static boolean DEBUG = false;
    public static boolean DEBUG_FOR_LOG = false;
    public String mUri = null;
    private int mType = 0;
    private int mVisiabltiy = View.INVISIBLE;
    public int mCurrentFrameIndex = -1;
    public static final int SIZE_LRUCACHE = 4*100*Config.SIZE_MICROTHUMBNAIL*Config.SIZE_MICROTHUMBNAIL; // MaxSize of LruCahce
    public static final Bitmap sDefaultBitmap = Bitmap.createBitmap(Config.SIZE_MICROTHUMBNAIL, Config.SIZE_MICROTHUMBNAIL, Bitmap.Config.ARGB_8888);
    public static BitmapLoadThreadEx sMavLoadThread = null;
    public static RenderThreadEx sMavRenderThread = null;
    public static final DequeEx<DataAdapterEx> sLoadInputQueue = new DequeEx<DataAdapterEx>();

    public static ConcurrentHashMap<QuickContactBadgeEx, DataAdapterEx> mAllQuickContactBadgeEx =
            new ConcurrentHashMap<QuickContactBadgeEx, DataAdapterEx>();
    public static int mCount;

    public static final String MINITYPE_MPO = ".mpo";
    public static int sListViewState = 0;
    private Bitmap mMiddleFrameStaticBitmap = null;
    
    private boolean mHasAssginedUri = false;
    
    public static final int TYPE_NO_INIT = 0;
    public static final int TYPE_DYNAMIC_MICROTHUMBNAIL = 1;
    public static final int TYPE_DYNAMIC_MINITHUMBNAIL = 2;
    public static final int TYPE_DYNAMIC_THUMBNAIL = 3;
    
    public static final int TYPE_STATIC_MICROTHUMBNAIL = 4;
    public static final int TYPE_STATIC_MINITHUMBNAIL = 5;
    public static final int TYPE_STATIC_THUMBNAIL = 6;
    
    private boolean mHasDefinitionOrder = false;
    private boolean mOrdered = true; //The order is: Frist assgin Contont Uri ,Second setUri.
    
    private int mFristFrameIndex = -1;
    
    private static Context sContext; 
    public static final class Config {
        public static final int SIZE_MICROTHUMBNAIL = 128;
        public static final int SIZE_MINITHUMBNAIL = 256;
        public static final int SIZE_THUMBNAIL = 512;
        public static final int MAX_PIXEL_COUNT_MICROTHUMBNAIL = 65536;
        public static final int MAX_PIXEL_COUNT_MINITHUMBNAIL = 65536*4;
        
        private static int SLEEP_TIME_INTERVAL = 1000;
        private static int SLEEP_TIME_MICROTHUMBNAIL = 40;
        private static int SLEEP_TIME_THUMBNAIL = 20;
        
        
        private static String sOnClickFilePath = null;
        private static int sCurrentIndex = -1;
        private static boolean isOnClicked = false;

        public static int getTargetSize(int type) {
        int size = SIZE_MICROTHUMBNAIL;;
            switch (type) {
            case TYPE_DYNAMIC_MICROTHUMBNAIL:
            case TYPE_STATIC_MICROTHUMBNAIL:
                size = SIZE_MICROTHUMBNAIL;
            break;
            case TYPE_DYNAMIC_MINITHUMBNAIL:
            case TYPE_STATIC_MINITHUMBNAIL:
                size = SIZE_MINITHUMBNAIL;
            break;
            case TYPE_DYNAMIC_THUMBNAIL:
            case TYPE_STATIC_THUMBNAIL:
                size = SIZE_THUMBNAIL;
                break;
                default:
                    throw new RuntimeException(
                        "getTargetSize type="+type);
            }
            return size;
        }
        
        private static boolean hideOverLayer (int type) {
            return type == TYPE_DYNAMIC_THUMBNAIL 
                || type == TYPE_DYNAMIC_MINITHUMBNAIL;
        }

        private static boolean isDynamicThumbNail (int type) {
            boolean isDynamic = false;
            if (!(type >= TYPE_DYNAMIC_MICROTHUMBNAIL && type <= TYPE_STATIC_THUMBNAIL)) {
                return false;
            }
            int animationType = getAnimationType(type);
            if (animationType == AnimationEx.TYPE_ANIMATION_PLAYBACK
                    || animationType == AnimationEx.TYPE_ANIMATION_CONTINUOUS
                    || animationType == AnimationEx.TYPE_ANIMATION_INTERVAL) {
                isDynamic = true;
            }
            return isDynamic;
        }
        
        public static boolean isOutOfLimitation (float pix, int type) {
            if (type == TYPE_DYNAMIC_MICROTHUMBNAIL || type == TYPE_STATIC_MICROTHUMBNAIL ) {
                return pix > MAX_PIXEL_COUNT_MICROTHUMBNAIL;
            } else {
                return pix > MAX_PIXEL_COUNT_MINITHUMBNAIL;
            }
        }
        
        public static float caculateScale (int type, int width, int height) {
            int limitation ;
            if (type == TYPE_DYNAMIC_MICROTHUMBNAIL || type == TYPE_STATIC_MICROTHUMBNAIL) {
                limitation = MAX_PIXEL_COUNT_MICROTHUMBNAIL;
            } else {
                limitation = MAX_PIXEL_COUNT_MINITHUMBNAIL;
            }
            return FloatMath.sqrt((float) limitation / (width * height));
        }
        
        public static boolean needCenterCropProcess(int type) {
            return type == TYPE_DYNAMIC_MICROTHUMBNAIL 
                || type == TYPE_STATIC_MICROTHUMBNAIL
                || type == TYPE_DYNAMIC_MINITHUMBNAIL
                || type == TYPE_STATIC_MINITHUMBNAIL;
        }
        
        public static int getIntervalTime() {
            return SLEEP_TIME_INTERVAL;
        }

        private static int getSleepTime(int type) {
            int time = SLEEP_TIME_MICROTHUMBNAIL;
            switch (type) {
                case TYPE_DYNAMIC_MICROTHUMBNAIL:
                    time = SLEEP_TIME_MICROTHUMBNAIL;
                    break;
                case TYPE_DYNAMIC_MINITHUMBNAIL:
                case TYPE_DYNAMIC_THUMBNAIL:
                    time = SLEEP_TIME_THUMBNAIL;
                    break;
                default:
                    throw new RuntimeException(
                        "getSleepTime for type="+type);
            }
            return time;
        }
        
        private static boolean isAbleGyroSensor (int type) {
            if (sAutoPlayBack) return true;
            boolean ableGyroSensor = false;
            switch (type) {
                case TYPE_DYNAMIC_MICROTHUMBNAIL:
                    ableGyroSensor = false;
                    break;
                case TYPE_DYNAMIC_MINITHUMBNAIL:
                case TYPE_DYNAMIC_THUMBNAIL:
                    ableGyroSensor = true;
                    break;
                default:
                    throw new RuntimeException(
                        "isAbleGyroSensor for type="+type);
            }
            return ableGyroSensor;
        }

        private static int getAnimationType (int type) {
            if (sAutoPlayBack) return AnimationEx.TYPE_ANIMATION_PLAYBACK;
            int animationType = AnimationEx.TYPE_ANIMATION_PLAYBACK;
            switch (type) {
                case TYPE_DYNAMIC_MICROTHUMBNAIL:
                    animationType =  AnimationEx.TYPE_ANIMATION_PLAYBACK;
                    break;
                case TYPE_DYNAMIC_MINITHUMBNAIL:
                case TYPE_DYNAMIC_THUMBNAIL:
                    animationType =  AnimationEx.TYPE_ANIMATION_CONTINUOUS;
                    break;
                case TYPE_STATIC_MICROTHUMBNAIL:
                case TYPE_STATIC_MINITHUMBNAIL:
                case TYPE_STATIC_THUMBNAIL:
                    animationType = AnimationEx.TYPE_ANIMATION_SINGLEIMAGE;
                break;
                default:
                throw new RuntimeException(
                        "getAnimationType for type="+type);
            }
            return animationType;
        }
        
        public static boolean needSavePath(int type) {
            return (type == TYPE_DYNAMIC_MICROTHUMBNAIL ||
                    type == TYPE_DYNAMIC_MINITHUMBNAIL);
        }
        
        public static int getSavedIndex () {
            return sCurrentIndex;
        }
        
        public static boolean hasRight (String mewPath, int type) {
            if ((getClickPath() != null && mewPath.equals(getClickPath()))
                    && (isOnClicked) && type == TYPE_DYNAMIC_THUMBNAIL) {
                return true;
            } else {
                return false;
            }
        }
       
        public static String getClickPath () {
            return sOnClickFilePath;
        }
        
        public static void setClick (String uri, int frameIndex) {
            sOnClickFilePath = uri;
            isOnClicked = true;
            sCurrentIndex = frameIndex;
        }
        public static void clearClick () {
            sOnClickFilePath = null;
            sCurrentIndex = -1;
            isOnClicked = false;
        }
    }
    
    public static final class Action {
        private QuickContactBadgeEx mView;
        private Bitmap mBitmap;
        public Action(QuickContactBadgeEx view, Bitmap bitmap){
            mView = view;
            mBitmap = bitmap;
        }
        public QuickContactBadgeEx getView() {
            return mView;
        }
        
        public Bitmap getBitmap() {
            return mBitmap;
        }
    }
    
    public boolean useLruCache(int type) {
        boolean isAbleCache = false;
        switch (type) {
        case TYPE_DYNAMIC_MICROTHUMBNAIL:
        case TYPE_DYNAMIC_MINITHUMBNAIL:
            isAbleCache = true;
            break;
        case TYPE_DYNAMIC_THUMBNAIL:
        case TYPE_STATIC_MICROTHUMBNAIL:
        case TYPE_STATIC_MINITHUMBNAIL:
        case TYPE_STATIC_THUMBNAIL:
            isAbleCache = false;
            break;
        default:
        throw new RuntimeException(
                "isAbleCache for type="+type);
        }
        return isAbleCache;
    }
    
    public static LruCache<String, Action> sBitmapCache = new LruCache<String ,Action>(SIZE_LRUCACHE) {
        @Override
        public void entryRemoved(boolean evicted, String key, Action oldValue, Action newValue) {
            if (key != null && oldValue != null) {
                if (DEBUG_FOR_LOG) Log.d(TAG,"sBitmapCache  key=="+key+ " entryRemoved  oldValue="+oldValue+"  :::"+oldValue.getBitmap().getHeight()
                        +" newValue="+newValue);
                oldValue.getBitmap().recycle();
            }
        }
        @Override
        protected int sizeOf(String key, Action oldValue) {
            if (key != null && oldValue != null) {
                return 4*oldValue.getBitmap().getWidth()*oldValue.getBitmap().getHeight();
            } else {
                return 0;
            }
        }
    };
    
    public static LruCache<String, Action> sBigBitmapCache = new LruCache<String ,Action>(1) {
        @Override
        public void entryRemoved(boolean evicted, String key, Action oldValue, Action newValue) {
            if (key != null && oldValue != null) {
                if (DEBUG_FOR_LOG) Log.d(TAG,"!!!sBigBitmapCache  key=="+key+ " entryRemoved  oldValue="+oldValue+"  :::"+oldValue.getBitmap().getHeight()
                        +" newValue="+newValue);
                oldValue.getBitmap().recycle();
            }
        }
        @Override
        protected int sizeOf(String key, Action oldValue) {
            if (key != null && oldValue != null) {
                return 1;
            } else {
                return 0;
            }
        }
    };
    public static ConcurrentHashMap<String ,String> sCacheOfFrameIndex = new ConcurrentHashMap<String ,String>();
    public AnimationEx mAnimation = null;
    public Bitmap mCurrentFrame = null;
    private GyroSensorEx mGyroSensor = null;
    private static final int MSG_UPDATE_MAV_FRAME = 5;
    private static final int MSG_UPDATE_MAV_ONE_FRAME = 6;
    private int sResourceId = -1; 
    private int position = -1;
    private final static Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_MAV_FRAME:{
                    draw();
                    break;
                }
                case MSG_UPDATE_MAV_ONE_FRAME:{
                    ((QuickContactBadgeEx)msg.obj).drawOneFrame();
                    break;
                }
                default: throw new AssertionError();
            }
        }
    };

    private void drawOneFrame () {
        if (mType == TYPE_NO_INIT || mUri == null) return ;
        setMavImageBitmap (mUri, mType);
        sMavRenderThread.setRenderRequester(isOnIdleState());
    }
    public QuickContactBadgeEx(Context context, AttributeSet attrs) {
        super(context, attrs, 0);
        setQuickContactBadgeEx(context);
        if (DEBUG_FOR_LOG) Log.d(TAG,"QuickContactBadgeEx(Context context, AttributeSet attrs)");
    }
    public QuickContactBadgeEx(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setQuickContactBadgeEx(context);
        if (DEBUG_FOR_LOG) Log.d(TAG,"QuickContactBadgeEx(Context context, AttributeSet attrs, int defStyle)");
    }
    
    public QuickContactBadgeEx(Context context) {
        super(context);
        setQuickContactBadgeEx(context);
        if (DEBUG_FOR_LOG) Log.d(TAG," QuickContactBadgeEx(Context context");
    }

    @Override
    protected void drawableStateChanged() {
        super.drawableStateChanged();
        if (DEBUG) Log.d(TAG,"drawableStateChanged postion="+this.position +"  IsVisbale==="+this.isActivated()
                +" getVisibility::"+this.getVisibility() +" "+this.isEnabled() +" "+this.getWindowVisibility() 
                +" "+this.getSystemUiVisibility()+
                "   "+this.getKeepScreenOn()+" "
                +"   " +this);
    }
    
    @Override
    protected void onDraw(Canvas canvas) {
        try {
           super.onDraw(canvas);
        } catch (RuntimeException e) {
            setMavImageBitmap(mUri, mType);
            Log.d(TAG, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!RuntimeException==!!!!!!!!"+ 
                    "   this"+this +"   this.mFilePath=="+this.mUri +" postion="+this.position  +" data.getState()="+mAllQuickContactBadgeEx.get(this)) ;
        }
    }

    @Override
    public void setImageToDefault() {
        if (getContext() == null) return;
        if (sResourceId > 0) {
            setImageDrawable(BitmapDrawableEx.getBitmapDrawableEx(getResources(), sResourceId));
        } /*else {
            ((ImageView)this).setImageDrawable(getResources().getDrawable(R.drawable.ic_contact_picture));
        }*/
    }

    public void setQuickContactBadgeEx(Context context) {
        if (sContext != null && sContext != context) {
            releasAllResource();
            Log.d(TAG, "Have a new comtext! ,so release All resource first!!");
        }
        sContext = context;
        if (sMavLoadThread == null) {
            sMavLoadThread = new BitmapLoadThreadEx(getContext());
            sMavLoadThread.start();
        }
        if (sMavRenderThread == null) {
            sMavRenderThread = new RenderThreadEx(getContext(), new GyroSensorEx(getContext()));
            sMavRenderThread.setOnDrawMavFrameListener(new OnDrawMavFrameListener(){
                public void drawMavFrame() {
                    mHandler.removeMessages(MSG_UPDATE_MAV_FRAME);
                    Message m = mHandler.obtainMessage(MSG_UPDATE_MAV_FRAME);
                    m.sendToTarget();
                }

                public boolean advanceAnimation(int targetFrame, int type) {
                    boolean isfinished = false;
                    Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
                    while (it!= null && it.hasNext()) {
                        ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
                        //QuickContactBadgeEx mav= entry.getKey();
                        DataAdapterEx adapter= entry.getValue();
                        QuickContactBadgeEx mavTemp= adapter.getImageView();
                        if (mavTemp.mHasWindowFocus == false) continue; 
                        if (mavTemp.mAnimation == null) {
                            if (adapter.getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME) {
                                mavTemp.mAnimation = mavTemp.createAnimation(adapter);
                            }
                        } else if (adapter.getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME){
                            isfinished &= mavTemp.mAnimation.advanceAnimation();
                        }
                    }
                    return isfinished;
                }

                public void initAnimation(int targetFrame, int animationType) {
                    Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
                    while (it!= null && it.hasNext()) {
                        ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
                        //QuickContactBadgeEx mav= entry.getKey();
                        DataAdapterEx adapter= entry.getValue();
                        QuickContactBadgeEx mavTemp= adapter.getImageView();
                        if (mavTemp.mHasWindowFocus == false || !Config.isAbleGyroSensor(mavTemp.mType)) continue; 
                        if (adapter.getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME && mavTemp.mAnimation != null) {
                            if (DEBUG_FOR_LOG) Log.d(TAG, "adapter.getBitmap()[msg.arg1]=============targetFrame="+targetFrame
                                    +"  animationType=="+animationType +" mavTemp.mType="+mavTemp.mType);
                            mavTemp.mAnimation.initAnimation(targetFrame, animationType);
                        }
                        if (sMavRenderThread != null) {
                            sMavRenderThread.setRenderRequester(QuickContactBadgeEx.isOnIdleState());
                        }
                    }
                }

                public void changeAnimationType() {
                    if (!sAutoPlayBack) return ; 
                    if (DEBUG_FOR_LOG) Log.d(TAG," mAllQuickContactBadgeEx size="+mAllQuickContactBadgeEx.size());
                    Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
                    while (it!= null && it.hasNext()) {
                        ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
                        //QuickContactMav mav= entry.getKey();
                        DataAdapterEx adapter= entry.getValue();
                        QuickContactBadgeEx mavTemp= adapter.getImageView();
                        if (DEBUG_FOR_LOG && mavTemp.mAnimation != null)
                            Log.d(TAG," mavTemp.mHasWindowFocus="+mavTemp.mHasWindowFocus
                                +" mavTemp.mType="+mavTemp.mType
                                +" sAutoPlayBack="+sAutoPlayBack
                                +" mavTemp.mAnimation.isFinished()="+mavTemp.mAnimation.isFinished()
                                +"  mavTemp.mAnimation.getType()="+
                                mavTemp.mAnimation.getType() +" adapter.getState()="+adapter.getState());
                        if (mavTemp.mHasWindowFocus == false || mavTemp.mAnimation == null) continue; 
                        if (adapter.getState() == DataAdapterEx.STATE_LOADED_ALL_FRAME && mavTemp.mAnimation != null) {
                            mavTemp.mAnimation.nextStepAnimation();
                            sMavRenderThread.setRenderRequester(isOnIdleState());
                        }
                    }
                }

                public int getSleepTime() {
                    // TODO Auto-generated method stub
                    Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
                    int numberOfMicrothumbnail = 0;
                    int numberOfThumbnail = 0;
                    while (it!= null && it.hasNext()) {
                        ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
                        DataAdapterEx adapter= entry.getValue();
                        QuickContactBadgeEx mavTemp= adapter.getImageView();
                        if (mavTemp.mAnimation != null && mavTemp.mHasWindowFocus) {
                            if (mavTemp.mType == TYPE_DYNAMIC_MICROTHUMBNAIL) {
                                numberOfMicrothumbnail++;
                            } else if(mavTemp.mType == TYPE_DYNAMIC_THUMBNAIL) {
                                numberOfThumbnail++;
                            }
                        }
                    }
                    if (numberOfThumbnail > 0) {
                        return Config.getSleepTime(TYPE_DYNAMIC_THUMBNAIL);
                    } else {
                        return Config.getSleepTime(TYPE_DYNAMIC_MICROTHUMBNAIL);
                    }
                }
                
            });
            sMavRenderThread.start();
        }
    }

    
    private void reAssignResource () {
        if (!mHasDefinitionOrder && mHasAssginedUri) {
            mOrdered = false;
        }
        if (!mOrdered && !mHasAssginedUri) {
            if (releaseResource(this)) {
                setImageToDefault();
                init(null, TYPE_NO_INIT);
            }
        } else if (mOrdered && mHasDefinitionOrder) {
            if (releaseResource(this)) {
                setImageToDefault();
                init(null, TYPE_NO_INIT);
            }
        }
        mHasDefinitionOrder = true;
        mHasAssginedUri = false;
    }
    @Override 
    public void assignContactUri (Uri contactUri) {
        super.assignContactUri(contactUri);
        reAssignResource();
    }

    @Override
    public void assignContactFromEmail (String emailAddress, boolean lazyLookup) {
        super.assignContactFromEmail(emailAddress, lazyLookup);
        reAssignResource();
    }
    
    @Override 
    public void assignContactFromPhone(String phoneNumber, boolean lazyLookup) {
        super.assignContactFromPhone(phoneNumber, lazyLookup);
        reAssignResource();
    }
    
    public boolean hasAssginedFrameIndex () {
        if (mUri != null && isMPOUri(mUri) && mType == TYPE_DYNAMIC_THUMBNAIL ) {
            if (mFristFrameIndex >= 0 && mFristFrameIndex < 25) {
                return true;
            }
        }
        return false;
    }

    public int getCurrentFrame () {
        return mFristFrameIndex;
    }

    private void init(String uri, int type) {
        mUri = uri;
        mType = type;
    }
    public DataAdapterEx setUri( String uri, int type) {
        init(uri, type);
        if (DEBUG_FOR_LOG) Log.d(TAG,"setUri  uri=="+uri +" type="+type);
        DataAdapterEx adapter = null;
        synchronized (sLoadInputQueue) {
            adapter = new DataAdapterEx(this, getContext(), uri, type);
            if (adapter.mMarkFrame != null) {
                adapter.setState(DataAdapterEx.STATE_LOADED_MARK_FRAME);
                sLoadInputQueue.addFirst(adapter);
            } else {
                adapter.setState(DataAdapterEx.STATE_QUEUED);
                sLoadInputQueue.addFirst(adapter);
            }

            sLoadInputQueue.notify();
        }
        return adapter;
    }


    public void setUri( String uri, int type, int resourceId, int frameIndex) {
        mHasAssginedUri = true;
        mFristFrameIndex = frameIndex;
        setUri(uri, type, resourceId);
    }
    
    public int getCurrentFrameIndex () {
        DataAdapterEx adapter = mAllQuickContactBadgeEx.get(this);
        if (adapter != null) {
           return adapter.getCurrentFrameIndex();
        } else {
            return 13;
        }
    }
    
    public Bitmap getBitmap (int type) {
        Bitmap bitmap = MpoDecodeHelper.loadOneBitmap(getContext(), mUri, type);
        if (bitmap != null) {
            mMiddleFrameStaticBitmap = bitmap;
           return bitmap;
        } else {
            if (sResourceId > 0) {
                return ((BitmapDrawable)getContext().getResources().getDrawable(sResourceId)).getBitmap();
            } else {
                return sDefaultBitmap;
            }
        }
    }
    
    public void setUri( String uri, int type, int resourceId) {
        DEBUG = SystemProperties.getInt("MavView_DEBUG", 0) == 1 ? true : false;
        DEBUG_FOR_LOG = SystemProperties.getInt("MavView_DFL", 0) == 1 ? true : false;
        if (DEBUG) {
            int intervalTime = SystemProperties.getInt("MV_INT_MT", 50);
            if (intervalTime > 0 && intervalTime < 1000) {
                Config.SLEEP_TIME_MICROTHUMBNAIL = intervalTime;
            }
            intervalTime = SystemProperties.getInt("MV_INT_T", 20);
            if (intervalTime > 0 && intervalTime < 1000) {
                Config.SLEEP_TIME_THUMBNAIL = intervalTime;
            }
            sAutoPlayBack = SystemProperties.getInt("MV_PLAYBACK", 0) == 1 ? true : false;
        }
        if (mUri == null && uri != null) mHasWindowFocus = true;
        if (DEBUG_FOR_LOG) Log.d(TAG,"!!setUri postion="+this.position +"  IsVisbale==="+this.isActivated()
                +" getVisibility::"+this.getVisibility() +" "+this.isEnabled() +" "+this.getWindowVisibility() 
                +" "+this.getSystemUiVisibility()+
                "   "+this.getKeepScreenOn()+" "
                +"   " +this +"  uri=="+uri +" mUri="+mUri +"  type="+type +" sAutoPlayBack="+sAutoPlayBack);
        if (uri == null || sMavRenderThread == null || sMavLoadThread == null) return ;
        sResourceId= resourceId;
        mType = type;
        if(!isMPOUri(uri)) {
            releaseResource(this);
            setImageToDefault();
            return;
        }
        this.mVisiabltiy = View.VISIBLE;
        if (!Config.isDynamicThumbNail(mType)) {
            init(uri, type);
            return;
        }
        if (Config.hasRight(uri, type)) {
            mFristFrameIndex = Config.getSavedIndex();
        }
        Config.clearClick();
        //if (mType != TYPE_DYNAMIC_MICROTHUMBNAIL) setOverLayState(true);
        if (Config.hideOverLayer(mType)) mHideOverLay = true;

        if (mAllQuickContactBadgeEx != null && mAllQuickContactBadgeEx.get(this) != null) {
            if(uri != null && uri.equals(mUri)) {
                return ;
            } else if(uri != null && !uri.equals(mUri) && mUri != null) {
                releaseResource(this);
            } else if (mUri == null) {
               // setImageToDefault();
            }
        }
        mAllQuickContactBadgeEx.put(this, setUri(uri, type));
        sMavRenderThread.setRenderRequester(isOnIdleState());
    }

    public static void draw() {
        if (mAllQuickContactBadgeEx == null) return ; 
        Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
        while (it!= null && it.hasNext()) {
            ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
            QuickContactBadgeEx mav= entry.getKey();
            DataAdapterEx adapter= entry.getValue();
            if (mav.mHasWindowFocus == false || adapter.isInterrupte()) {
                if (DEBUG_FOR_LOG) Log.d("wjxdraw"," postion="+mav.position +"  IsVisbale==="+mav.isActivated());
                continue; 
            }
            adapter.setCurrentFrame();
        }
        if (mAllQuickContactBadgeEx != null && mAllQuickContactBadgeEx.size() == 0 && sMavRenderThread != null) {
            sMavRenderThread.setRenderRequester(false);
        }
    }

    public static void onPause() {
        if (mAllQuickContactBadgeEx == null) return ;
        Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
        while (it!= null && it.hasNext()) {
            ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
            QuickContactBadgeEx mav= entry.getKey();
            mav.setImageBitmap(null);
            DataAdapterEx adapter= entry.getValue();
            adapter.setInterrupte(true);
            adapter.setState(DataAdapterEx.STATE_RELEASE_ALL_FRAME);
            adapter.freeFrame();
        }
        mAllQuickContactBadgeEx.clear();
        mAllQuickContactBadgeEx = null;
    }
    
    public static void onDestroy() {
        if (sMavLoadThread != null ) {
            sMavLoadThread.stopLoop();
            sMavLoadThread = null;
        }
        if (sMavRenderThread != null) {
            sMavRenderThread.interrupt();
            sMavRenderThread.stopActive();
            sMavRenderThread = null;
        }
        if (sBitmapCache != null) {
            sBitmapCache.evictAll();
            sBitmapCache = null;
        }
    }
    
    public static void recycle() {
    }

    public static void scrollStateChange (int state) {
        sListViewState = state;
        if (sMavRenderThread != null) sMavRenderThread.setRenderRequester(isOnIdleState());
        if (DEBUG_FOR_LOG) Log.d(TAG, "MavScrollListener state="+state);
    }
    public boolean mHasWindowFocus = false;
    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        super.onWindowFocusChanged(hasWindowFocus);
        if (DEBUG_FOR_LOG) Log.d("wjxwindowsFocusChanged", "hasWindowFocus="+hasWindowFocus +" mUri=="+this.mUri +" mType= "+this.mType
                +" postion="+this.position +"  IsVisbale==="+this.isActivated()
                +" getVisibility::"+this.getVisibility() +" "+this.isEnabled() +" "+this.getWindowVisibility() 
                +" "+this.getSystemUiVisibility()+
                "   "+this.getKeepScreenOn()+" mVisiabltiy=" +this.mVisiabltiy +"getFilterTouchesWhenObscured=" +this.getFilterTouchesWhenObscured()
                +"   isShown=" +this.isShown()+" isDirty="+this.isDirty() +" isDrawingCacheEnabled="+this.isDrawingCacheEnabled() +"  isFocusableInTouchMode="+ this.isFocusableInTouchMode()
                +"  "+" mUri="+this.mUri +"  type="+this.mType);
        mHasWindowFocus = hasWindowFocus;
        if (sLoadInputQueue == null) return; 
        synchronized (sLoadInputQueue) {
            DataAdapterEx adapter = mAllQuickContactBadgeEx.get(this);
            if (!hasWindowFocus) {
                releaseResource(this, true);
                if (sMavRenderThread != null) sMavRenderThread.setRenderRequester(isOnStopRenderState(hasWindowFocus));
            } else if (hasWindowFocus && adapter == null && mUri != null 
                    && (Config.isDynamicThumbNail(mType))) {
                mAllQuickContactBadgeEx.put(this, setUri(mUri, mType));
                if (sMavRenderThread != null) sMavRenderThread.setRenderRequester(isOnIdleState());
                mAnimation = null;
            }
        }
    }
    
    public AnimationEx createAnimation (DataAdapterEx adapter) {
        int endframe ;
        if (Config.getAnimationType(mType) == AnimationEx.TYPE_ANIMATION_PLAYBACK) {
            endframe = adapter.mMarkFrameIndex > adapter.getmFrameCount()/2 ? 0:adapter.getmFrameCount()-1;
        } else {
            endframe = adapter.mMarkFrameIndex;
        }
        if (DEBUG_FOR_LOG) Log.d("createAnimation"," MavAnimation uri=="+mUri +" type="+mType +" mavTemp postion=="+position);
        return new AnimationEx(adapter.getmFrameCount(),
                adapter.mMarkFrameIndex, endframe , Config.getAnimationType(mType), Config.getIntervalTime());
    }

    private boolean isMPOUri(String uri) {
        if (uri != null && uri.endsWith(MINITYPE_MPO)){
            return true;
        } else 
            return false;
    }

    private boolean releaseResource(QuickContactBadgeEx mView) {
        return releaseResource(mView, true);
    }
    
    private void releasAllResource () {
        if (mAllQuickContactBadgeEx == null) return ;
        Iterator< Entry<QuickContactBadgeEx, DataAdapterEx> > it = mAllQuickContactBadgeEx.entrySet().iterator();
        while (it!= null && it.hasNext()) {
            ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx> entry = (ConcurrentHashMap.Entry<QuickContactBadgeEx, DataAdapterEx>)it.next();
            QuickContactBadgeEx mav= entry.getKey();
            releaseResource(mav);
        }
    }
    
    private boolean releaseResource(QuickContactBadgeEx mView, boolean remove) {
        if (!Config.isDynamicThumbNail(mType)) {
            if (mMiddleFrameStaticBitmap != null && !mMiddleFrameStaticBitmap.isRecycled()){
                mMiddleFrameStaticBitmap.recycle();
                mMiddleFrameStaticBitmap = null;
                return true;
            }
        } else {
            DataAdapterEx adapter = mAllQuickContactBadgeEx.get(this);
            if (adapter != null) {
                if (useLruCache(mType)) {
                    adapter.setInterrupte(true);
                    Bitmap bitmap = adapter.getCurrentBitmap(adapter.getCurrentFrameIndex());
                    if (bitmap != null) {
                        setImage(bitmap);
                        sBitmapCache.put(adapter.getFilePath()+adapter.getType(), new QuickContactBadgeEx.Action(this, bitmap));
                        sCacheOfFrameIndex.put(adapter.getFilePath()+adapter.getType(), Integer.toString(adapter.getCurrentFrameIndex()));
                    }
                }/* else {
                   sBigBitmapCache.remove(adapter.getFilePath()+adapter.getType());
                }*/
                adapter.setState(DataAdapterEx.STATE_RELEASE_ALL_FRAME);
                synchronized (sLoadInputQueue) {
                    sLoadInputQueue.addLast(adapter);
                    sLoadInputQueue.notify();
                }
                mAnimation = null;
                if (remove) {
                    mAllQuickContactBadgeEx.remove(this);
                }
                return true;
            }
        }
        return false;
    }
    
    private boolean isOnStopRenderState(boolean loastWindowsFocus) {
        return isOnIdleState() && loastWindowsFocus;
    }
    
    public static boolean isOnIdleState() {
        return QuickContactBadgeEx.sListViewState == OnScrollListener.SCROLL_STATE_IDLE
        && (mAllQuickContactBadgeEx != null && mAllQuickContactBadgeEx.size() != 0) ;
    }

    public void setMavImageBitmap (String filePath, int type) {
        Action mAction = null;
        Bitmap bitmap = null;
        if (useLruCache(type) && sCacheOfFrameIndex.get(filePath+type) != null) {
            mAction = sBitmapCache.get(filePath+type);
        } else if (!useLruCache(type)){
            mAction = sBigBitmapCache.get(filePath+type);
        }

        if (mAction != null) {
            bitmap = mAction.getBitmap();
        }
        if (bitmap != null && !bitmap.isRecycled()) {
            //this.setImageBitmap(bitmap);
            setImage(bitmap);
        } else {
            setImageToDefault();
        }
    }

    @Override
    public void onClick(View v) {
        if (Config.needSavePath(mType)) {
            mFristFrameIndex = -1;
            Config.setClick(mUri,getCurrentFrameIndex());

        }
        super.onClick(v);
        
    }
/*
    @Override
    public void setImageDrawable(Drawable drawable) {
        //reAssignResource();
        if (!(drawable instanceof BitmapDrawableEx)) {
            if (mAllQuickContactBadgeEx.get(this) != null) {
                if (DEBUG) Log.d(TAG," setImageDrawable~  mAllQuickContactBadgeEx type=="+mAllQuickContactBadgeEx.get(this).getState());
            } else {
                if (DEBUG) Log.d(TAG," setImageDrawable~");
            }
            reAssignResource();
        }
        
        super.setImageDrawable(drawable);
    }
    */
    @Override 
    public void setImageBitmap (Bitmap bm) {
        super.setImageBitmap(bm);
    }
    

    public void setImage (Bitmap frame) {
        //Log.d(TAG, "setImage = "+frame);
        setImageDrawable(new BitmapDrawableEx(getContext().getResources(), frame));
    }
    
    private static class BitmapDrawableEx extends BitmapDrawable {

        public static BitmapDrawableEx getBitmapDrawableEx (Resources resource, int sResourceId) {
            return getBitmapDrawableEx (resource, resource.getDrawable(sResourceId));
        }
        public static BitmapDrawableEx getBitmapDrawableEx (Resources resource, Drawable drawable) {
            Bitmap bitmap = ((BitmapDrawable)drawable).getBitmap();
            return new BitmapDrawableEx(resource, bitmap);
        }

        public BitmapDrawableEx(Resources res, Bitmap bitmap) {
            super(res, bitmap);
        }
    }

    public void notifyImageChange() {
        // TODO Auto-generated method stub
        if (mType == TYPE_NO_INIT || mUri == null) return ;
        mHandler.removeMessages(MSG_UPDATE_MAV_ONE_FRAME);
        Message m = mHandler.obtainMessage(MSG_UPDATE_MAV_ONE_FRAME, this);
        m.sendToTarget();
    }
}
