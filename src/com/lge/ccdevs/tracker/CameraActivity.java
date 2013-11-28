package com.lge.ccdevs.tracker;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import com.lge.ccdevs.tracker.CameraPreview.IOnDrawTargetListener;

import android.app.Activity;
import android.content.Context;
import android.graphics.PixelFormat;
import android.graphics.RectF;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;


public class CameraActivity extends Activity {
    private static final String TAG = "CameraActivity";;
    // Called when the activity is first created.
    Camera _camera;
    int fcount = 0;
    private final static int ID_SET_TARGET = 0;
    private final static int ID_SHOW_TARGET = ID_SET_TARGET + 1;
    
    private FrameLayout mTargetLayer;
    private FrameLayout mTargetSettingLayer;
    private TargetSettingView mTargetSettingView = null;
    private TargetView mTargetView = null;
    private Context mContext;
    private CameraPreview mPreview;
    private RectF mInitialTargetRect;
    private boolean mShowTarget = false;
    
    static {
//        System.loadLibrary("Tracker_jni");
		System.load("/data/data/com.lge.ccdevs.tracker/lib/libTracker_jni.so");
        native_init();
    }

    private static native final void native_init();
    
    private IOnDrawTargetListener mOnDrawTargetListener = new IOnDrawTargetListener() {
        @Override
        public void onDrawTarget(RectF target) {
            Log.d(TAG, "onDrawTarget()");
            if (mShowTarget) {
                if (target != null && !target.isEmpty()) {
                    mTargetView.drawTarget(target);
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;        
        
        mInitialTargetRect = null;        
        
        // set preview display
        mPreview = new CameraPreview(mContext);        
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(mPreview);
        
        //set OnDrawTargetListener
        mPreview.setOnDrawTargetListener(mOnDrawTargetListener);

        // set target setting window
        mTargetLayer = (FrameLayout)LayoutInflater.from(mContext).inflate(R.layout.target_setting, null);
        mTargetSettingLayer = (FrameLayout)mTargetLayer.findViewById(R.id.target_setting_layout);
        mTargetSettingView = (TargetSettingView)mTargetLayer.findViewById(R.id.target_setting_view);
        mTargetView = (TargetView)mTargetLayer.findViewById(R.id.target_view);
        
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        DisplayMetrics metrics = new DisplayMetrics();
        wm.getDefaultDisplay().getMetrics(metrics);

        int width = metrics.widthPixels;
        int height = metrics.heightPixels;

        WindowManager.LayoutParams wmParams = new WindowManager.LayoutParams(width,
                height, 0, 0,
                LayoutParams.TYPE_PHONE,
                LayoutParams.FLAG_NOT_FOCUSABLE | LayoutParams.FLAG_NOT_TOUCH_MODAL |
                        LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH |
                        LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                PixelFormat.RGBA_8888);

        wmParams.gravity = Gravity.LEFT | Gravity.TOP;
        wmParams.setTitle("TargetSetting");
        wmParams.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING;

        wm.addView(mTargetLayer, wmParams);
        
        Button btn_done = (Button)mTargetLayer.findViewById(R.id.btn_done);
        btn_done.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                mTargetSettingLayer.setVisibility(View.GONE);
                mTargetView.setVisibility(View.VISIBLE);
                
                mInitialTargetRect = mTargetSettingView.getTargetRect();
                
                mPreview.setTarget(mInitialTargetRect);
                
                mShowTarget = true;                
            }});

        InputStream is = null;
        FileOutputStream fos = null;
        File outDir = new File(getString(R.string.face_db_dir));
        outDir.mkdirs();

        try {
            is = getAssets().open("haarcascade_frontalface_alt.xml");
            int size = is.available();
            byte[] buffer = new byte[size];
            File outfile = new File(outDir + "/" + "haarcascade_frontalface_alt.xml");
            fos = new FileOutputStream(outfile);
            for (int c = is.read(buffer); c != -1; c = is.read(buffer)){
                fos.write(buffer, 0, c);
            }
            is.close();
            fos.close();

            is = getAssets().open("haarcascade_eye_tree_eyeglasses.xml");
            outfile = new File(outDir + "/" + "haarcascade_eye_tree_eyeglasses.xml");
            fos = new FileOutputStream(outfile);
            for (int c = is.read(buffer); c != -1; c = is.read(buffer)){
                fos.write(buffer, 0, c);
            }
            is.close();
            fos.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
        
    @Override
    protected void onPause() {
        Log.d(TAG, "onPause()");
        
        mShowTarget = false;
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        wm.removeView(mTargetLayer);
        
        super.onPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, ID_SET_TARGET, 0, "set");
        menu.add(0, ID_SHOW_TARGET, 0, "hide target");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case ID_SET_TARGET :
                startTargetSetting();      
                break;
            case ID_SHOW_TARGET :
                if (item.getTitle().equals("show target")) {
                    showTarget();
                    item.setTitle("hide target");
                } else if (item.getTitle().equals("hide target")) {
                    hideTarget();
                    item.setTitle("show target");
                }
                break;
        }
        return super.onOptionsItemSelected(item);
    }
    
    private void startTargetSetting() {
        mTargetLayer.setVisibility(View.VISIBLE);
        mTargetSettingLayer.setVisibility(View.VISIBLE);
        mTargetView.setVisibility(View.GONE);        
    }
    
    private void showTarget() {
        mTargetLayer.setVisibility(View.VISIBLE);
        mTargetSettingView.setVisibility(View.VISIBLE);
        mShowTarget = true;
    }
    
    private void hideTarget() {
        mTargetLayer.setVisibility(View.GONE);
        mTargetSettingView.setVisibility(View.GONE);
        mShowTarget = false;
    }
}
