package com.lge.ccdevs.tracker;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;

import com.lge.ccdevs.tracker.CameraPreview.IOnDrawTargetListener;
import com.lge.ccdevs.tracker.CameraPreview.IOnRecordingStopListener;
import com.lge.ccdevs.tracker.CameraPreview.IOnTrackResultListener;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.graphics.PointF;
import android.graphics.RectF;
import android.hardware.Camera;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
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
import android.widget.Toast;

public class CameraActivity extends Activity {
    private static final String TAG = "CameraActivity";;
    // Called when the activity is first created.
    Camera _camera;
    int fcount = 0;
    private final static int ID_SET_TARGET = 0;
    private final static int ID_SET_BOUNDARY = ID_SET_TARGET + 1;

    //for record test
    private final static int ID_START_RECORD = ID_SET_TARGET + 2;
    private final static int ID_STOP_RECORD = ID_SET_TARGET + 3;

    private WindowManager.LayoutParams wmParams;
    private FrameLayout mTargetLayer;
    private FrameLayout mTargetSettingLayer;
    private TargetSettingView mTargetSettingView = null;
    private TargetView mTargetView = null;
    private Context mContext;
    private CameraPreview mPreview;
    private RectF mInitialTargetRect;
    private RectF mInitialBoundaryRect;
    private boolean mShowTarget = false;
    private boolean mIsRecording = false;


    private int mMonitorMode;

    // Sensor related start
    private SensorEventListener mAccSensorListener = null;
    private SensorEventListener mLightSensorListener = null;
    private boolean mReliableAccSensor = true;
    private boolean mReliableLightSensor = true;
    private boolean mIsFirst = true;
    private boolean mFlash = false;
    private float mPrevGx;
    private float mPrevGy;
    private float mPrevGz;
    // Sensor related end

    private PowerManager.WakeLock mWL;

    static {
//        System.loadLibrary("Tracker_jni");
		System.load("/data/data/com.lge.ccdevs.tracker/lib/libTracker_jni.so");
    }

    private IOnDrawTargetListener mOnDrawTargetListener = new IOnDrawTargetListener() {
        @Override
        public void onDrawTarget(RectF target) {
            if (mShowTarget) {
                if (target != null && !target.isEmpty()) {
                    mTargetView.drawTarget(target);
                }
            }
        }

        @Override
        public void onDrawTarget(PointF target, int radius) {
            if (mShowTarget) {
                if (target != null) {
                    mTargetView.drawTarget(target, radius);
                }
            }
        }

        @Override
        public void onDrawTarget(float[] target) {
            if (mShowTarget) {
                if (target != null) {
                    mTargetView.drawTarget(target);
                }
            }
        }

        @Override
        public void onDrawBoundary(RectF boundary) {
            if (mShowTarget) {
                if (boundary != null) {
                    mTargetView.drawBoundary(boundary);
                }
            }
        }
    };

    private IOnRecordingStopListener mOnRecordingStopListener = new IOnRecordingStopListener() {
        @Override
        public void onRecordingStopped() {
            if (mIsRecording) {
                mIsRecording = false;
                mPreview.stopRecording();
            }
        }
    };

    private IOnTrackResultListener mIOnTrackResultListener = new IOnTrackResultListener() {
        @Override
        public void onResultChanged(PointF pt, int dist) {
            PointF ipt = new PointF();

            switch(mMonitorMode) {
                case MonitorModeActivity.MONITOR_MODE_BABY :
                    ipt.x = (mInitialTargetRect.left + mInitialTargetRect.right) / 2.f;
                    ipt.y = (mInitialTargetRect.top + mInitialTargetRect.bottom) / 2.f;
                    if( ((ipt.x-pt.x)*(ipt.x-pt.x)+(ipt.y-pt.y)*(ipt.y-pt.y)) > (dist*dist) ) {
                        Log.d(TAG, "object moved!!");
                        String clientMsg = "baby movement detected!!";
                        
                        Intent i = new Intent();
                        i.putExtra("message", clientMsg);
                        i.setAction(MessagingService.PROCESS_MSG);
                        sendBroadcast(i);

                        if (!mIsRecording) {
                            mIsRecording = mPreview.startRecording();
                        }
                    }
                    break;

                case MonitorModeActivity.MONITOR_MODE_PET :
                    ipt.x = (mInitialBoundaryRect.left + mInitialBoundaryRect.right) / 2.f;
                    ipt.y = (mInitialBoundaryRect.top + mInitialBoundaryRect.bottom) / 2.f;

                    if( ((ipt.x-pt.x)*(ipt.x-pt.x)+(ipt.y-pt.y)*(ipt.y-pt.y)) < (dist*dist) ) {
                        Log.d(TAG, "object moved!!");
                        String clientMsg = "pet abnormal movement detected!!";
                        
                        Intent i = new Intent();
                        i.putExtra("message", clientMsg);
                        i.setAction(MessagingService.PROCESS_MSG);
                        sendBroadcast(i);

                        if (!mIsRecording) {
                            mIsRecording = mPreview.startRecording();
                        }
                    }
                    break;
            }

        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;

        Intent i = getIntent();
        mMonitorMode = i.getExtras().getInt("mode");


        if (mMonitorMode == MonitorModeActivity.MONITOR_MODE_VEHICLE) {
            setModeVehicle();
        } else if (mMonitorMode == MonitorModeActivity.MONITOR_MODE_BABY) {
            setModeBaby();
        } else if (mMonitorMode == MonitorModeActivity.MONITOR_MODE_PET) {
            setModePet();
        }

        mInitialTargetRect = null;
        mInitialBoundaryRect = null;

        // set preview display
        mPreview = new CameraPreview(mContext);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(mPreview);

        // Listeners...
        mPreview.setOnDrawTargetListener(mOnDrawTargetListener);
        mPreview.setOnRecordingStopListener(mOnRecordingStopListener);
        mPreview.setOnTrackResultListener(mIOnTrackResultListener);

        SensorManager sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        Sensor light = sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
        mLightSensorListener = new SensorEventListener() {
            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
                Log.d(TAG,"Accuracy of light sensor is changed to "+ accuracy);
                if( accuracy==SensorManager.SENSOR_STATUS_ACCURACY_HIGH ) {
                    mReliableLightSensor = true;
                } else {
                    mReliableLightSensor = false;
                }
            }

            @Override
            public void onSensorChanged(SensorEvent event) {
                if( !mReliableLightSensor ) {
                    return;
                }

                Log.d(TAG,"Light value(lux) is changed to "+event.values[0]); 
                if( event.values[0] < CameraPreview.THRESHOLD_FLASHON_LUX ) {
                    mFlash = true;
                    mPreview.setCameraFlash(mFlash);
                } else {
                    mFlash = false;
                    mPreview.setCameraFlash(mFlash);
                }
            }
        };

        sensorManager.registerListener(mLightSensorListener, light, SensorManager.SENSOR_DELAY_NORMAL);

        // set target setting window
        mTargetLayer = (FrameLayout)LayoutInflater.from(mContext).inflate(R.layout.target_setting, null);
        mTargetSettingLayer = (FrameLayout)mTargetLayer.findViewById(R.id.target_setting_layout);
        mTargetSettingView = (TargetSettingView)mTargetLayer.findViewById(R.id.target_setting_view);
        mTargetView = (TargetView)mTargetLayer.findViewById(R.id.target_view);

        /*Point dispSize = new Point();
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealSize(dispSize);

        int width = dispSize.x;
        int height = dispSize.y;*/
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        DisplayMetrics metrics = new DisplayMetrics();
        wm.getDefaultDisplay().getMetrics(metrics);

        int width = metrics.widthPixels;
        int height = metrics.heightPixels;

        wmParams = new WindowManager.LayoutParams(width,
                height, 0, 0,
                LayoutParams.TYPE_APPLICATION,
                LayoutParams.FLAG_NOT_FOCUSABLE | LayoutParams.FLAG_NOT_TOUCH_MODAL |
                        LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH |
                        LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                PixelFormat.RGBA_8888);

        wmParams.gravity = Gravity.LEFT | Gravity.TOP;
        wmParams.setTitle("TargetSetting");
        wmParams.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING;

        Button btn_done = (Button)mTargetLayer.findViewById(R.id.btn_done);
        btn_done.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                mTargetSettingLayer.setVisibility(View.GONE);
                mTargetView.setVisibility(View.VISIBLE);

                if(mTargetSettingView.getMode()==TargetSettingView.TARGET_SETTING) {
                    mInitialTargetRect = mTargetSettingView.getTargetRect();
                    mPreview.setTarget(mInitialTargetRect, mMonitorMode);
                } else if(mTargetSettingView.getMode()==TargetSettingView.BOUNDARY_SETTING) {
                    mInitialBoundaryRect = mTargetSettingView.getBoundaryRect();
                    mPreview.setBoundary(mInitialBoundaryRect, mMonitorMode);
                }
                mShowTarget = true;

            }});

        Button btn_cancel = (Button)mTargetLayer.findViewById(R.id.btn_cancel);
        btn_cancel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                mTargetSettingLayer.setVisibility(View.GONE);
                mTargetView.setVisibility(View.VISIBLE);
            }});

        InputStream is = null;
        FileOutputStream fos = null;
        File outDir = new File(getString(R.string.face_db_dir));
        if( outDir.mkdirs() ) {
            Log.d(TAG, "Make [face_db_dir] !!");

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
                e.printStackTrace();
            }
        }

        String path = Environment.getExternalStorageDirectory().getAbsolutePath();
        path += File.separator + "tracker";

        outDir = new File(path);
        if( outDir.mkdirs() ) {
            Log.d(TAG, "Make [track_record_dir] !!");
        }

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWL = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "DoNotDIMScreen");
    }

    @Override
    protected void onResume() {
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        wm.addView(mTargetLayer, wmParams);
        super.onResume();

        mWL.acquire();
    }


    @Override
    protected void onPause() {
        Log.d(TAG, "onPause()");

        mShowTarget = false;
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        wm.removeView(mTargetLayer);

        if (mIsRecording) {
            mIsRecording = false;
            mPreview.stopRecording();
        }

        super.onPause();

        mWL.release();
    }

    @Override
    protected void onDestroy() {
        SensorManager sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        if (mAccSensorListener != null) {
            sensorManager.unregisterListener(mAccSensorListener);
            mAccSensorListener = null;
            mIsFirst = true;
        }

        if (mLightSensorListener != null) {
            sensorManager.unregisterListener(mLightSensorListener);
            mLightSensorListener = null;
            mFlash = false;
            mPreview.setCameraFlash(mFlash);
        }
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, ID_SET_TARGET, 0, "set target");
        if( mMonitorMode==MonitorModeActivity.MONITOR_MODE_PET ) {
            menu.add(0, ID_SET_BOUNDARY, 0, "set boundary");
        }
        menu.add(0, ID_START_RECORD, 0, "start recording");
        menu.add(0, ID_STOP_RECORD, 0, "stop recording");

        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if( !mIsRecording ) {
            menu.findItem(ID_START_RECORD).setVisible(true);
            menu.findItem(ID_STOP_RECORD).setVisible(false);
        } else {
            menu.findItem(ID_START_RECORD).setVisible(false);
            menu.findItem(ID_STOP_RECORD).setVisible(true);
        }

        if( mTargetLayer.getVisibility()==View.VISIBLE ) {
            mTargetLayer.setVisibility(View.GONE);
        }

        if( mTargetSettingLayer.getVisibility()==View.VISIBLE) {
            mTargetSettingLayer.setVisibility(View.GONE);
        }

        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case ID_SET_TARGET :
                startTargetSetting();
                break;
            case ID_SET_BOUNDARY :
                startBoundarySetting();
                break;
            case ID_START_RECORD :
                if (!mIsRecording) {
                    mIsRecording = mPreview.startRecording();
                }
                break;
            case ID_STOP_RECORD :
                if (mIsRecording) {
                    mIsRecording = false;
                    mPreview.stopRecording();
                }
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void startTargetSetting() {
        mTargetSettingView.setMode(TargetSettingView.TARGET_SETTING);
        mTargetLayer.setVisibility(View.VISIBLE);
        mTargetSettingLayer.setVisibility(View.VISIBLE);
        mTargetView.setVisibility(View.GONE);
    }

    private void startBoundarySetting() {
        mTargetSettingView.setMode(TargetSettingView.BOUNDARY_SETTING);
        mTargetLayer.setVisibility(View.VISIBLE);
        mTargetSettingLayer.setVisibility(View.VISIBLE);
        mTargetView.setVisibility(View.GONE);
    }
/*
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
*/

    private void setModeVehicle() {
        SensorManager sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        Sensor acc = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        mAccSensorListener = new SensorEventListener() {

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
                Log.d(TAG,"Accuracy of light sensor is changed to "+ accuracy);
                if( accuracy==SensorManager.SENSOR_STATUS_ACCURACY_HIGH ) {
                    mReliableAccSensor = true;
                } else {
                    mReliableAccSensor = false;
                }
            }

            @Override
            public void onSensorChanged(SensorEvent event) {
                if( !mReliableAccSensor ) {
                    return;
                }

                float Gx = event.values[0];
                float Gy = event.values[1];
                float Gz = event.values[2];

                if (mIsFirst) {
                    mPrevGx = Gx;
                    mPrevGy = Gy;
                    mPrevGz = Gz;
                    mIsFirst = false;
                    return;
                }


                if ( Math.abs(mPrevGx - Gx) > CameraPreview.THRESHOLD_SENSOR_MOVEMENT || 
                     Math.abs(mPrevGy - Gy) > CameraPreview.THRESHOLD_SENSOR_MOVEMENT ||
                     Math.abs(mPrevGz - Gz) > CameraPreview.THRESHOLD_SENSOR_MOVEMENT ) {
                    Log.i(TAG, "sensor changed!!");
                    String clientMsg = "vehicle movement detected!!";

                    if (!mIsRecording) {
                        mIsRecording = mPreview.startRecording();
                    }
                    mPrevGx = Gx;
                    mPrevGy = Gy;
                    mPrevGz = Gz;

                    Log.i(TAG, "send message to MessagingService!");
                    Intent i = new Intent();
                    i.putExtra("message", clientMsg);
                    i.setAction(MessagingService.PROCESS_MSG);
                    sendBroadcast(i);
                }
            }};

        sensorManager.registerListener(mAccSensorListener, acc, SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void setModeBaby() {
        // Baby mode
    }

    private void setModePet() {
        // Pet mode
    }
}
