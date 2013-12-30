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

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.RectF;
import android.hardware.Camera;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
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
import android.widget.Toast;


public class CameraActivity extends Activity {
    private static final String TAG = "CameraActivity";;
    // Called when the activity is first created.
    Camera _camera;
    int fcount = 0;
    private final static int ID_SET_TARGET = 0;
    private final static int ID_SHOW_TARGET = ID_SET_TARGET + 1;
    
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
    private boolean mShowTarget = false;
    private boolean mIsRecording = false;
    
    
    private int mMonitorMode;
    
    // Vehicle Mode Settings_start
    private SensorEventListener mSensorListener = null;    
    private boolean mIsFirst = true;
    private float mPrevGx;
    private float mPrevGz;
    // Vehicle Mode Settings_end
    
    // Server/Client connection_start
    public static final int SERVERPORT = 5555;
    private TrackerServer mBoundService;
    private boolean connected = false;
    private boolean mISServiceRunning = false;
    private String mServerIP;
    private String clientMsg = "";
    private String mServerIpAddress = "";
 // Server/Client connection_end
    
    static {
//        System.loadLibrary("Tracker_jni");
		System.load("/data/data/com.lge.ccdevs.tracker/lib/libTracker_jni.so");
    }

    private IOnDrawTargetListener mOnDrawTargetListener = new IOnDrawTargetListener() {
        @Override
        public void onDrawTarget(RectF target) {
            Log.i(TAG, "onDrawTarget()");
            if (mShowTarget) {
                if (target != null && !target.isEmpty()) {
                    mTargetView.drawTarget(target);
                }
            }
        }

        @Override
        public void onDrawTarget(float[] target) {
            Log.i(TAG, "onDrawTarget()");
            if (mShowTarget) {
                if (target != null) {
                    mTargetView.drawTarget(target);
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

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;
        
        Intent i = getIntent();
        mServerIpAddress = i.getExtras().getString("ServerIP");
        mMonitorMode = i.getExtras().getInt("mode");
        
        if (mServerIpAddress==null || mServerIpAddress.equals("")) {
            Toast.makeText(mContext, "Cannot connect to the Server!!", Toast.LENGTH_SHORT);
            return;
        } else {
            Thread cThread = new Thread(new ClientThread());
            cThread.start();
        }
        
        if (mMonitorMode == MonitorModeActivity.MONITOR_MODE_VEHICLE) {
            setModeVehicle();
        }
        
        mInitialTargetRect = null;        
        
        // set preview display
        mPreview = new CameraPreview(mContext);        
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(mPreview);
        
        //set OnDrawTargetListener
        mPreview.setOnDrawTargetListener(mOnDrawTargetListener);
        mPreview.setOnRecordingStopListener(mOnRecordingStopListener);

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
                LayoutParams.TYPE_PHONE,
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
            e.printStackTrace();
        }
    }
        
    @Override
    protected void onResume() {
        WindowManager wm = (WindowManager)getSystemService(WINDOW_SERVICE);
        wm.addView(mTargetLayer, wmParams);
        super.onResume();
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
    }

    @Override
    protected void onDestroy() {
        if (mSensorListener != null) {
            SensorManager sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
            sensorManager.unregisterListener(mSensorListener);
            mSensorListener = null;
            mIsFirst = true;
        }
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, ID_SET_TARGET, 0, "set");
        menu.add(0, ID_SHOW_TARGET, 0, "hide target");
        menu.add(0, ID_START_RECORD, 0, "start recording");
        menu.add(0, ID_STOP_RECORD, 0, "stop recording");
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
    
    private void setMonitorMode() {
        switch (mMonitorMode) {
            case MonitorModeActivity.MONITOR_MODE_PET :
                break;
            case MonitorModeActivity.MONITOR_MODE_BABY :
                break;
            case MonitorModeActivity.MONITOR_MODE_VEHICLE :
                break;
            default :
                break;
                
        }
    }
    
    private void setModeVehicle() {
        SensorManager sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        Sensor acc = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        
        
        mSensorListener = new SensorEventListener() {

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
                // TODO Auto-generated method stub
                
            }

            @Override
            public void onSensorChanged(SensorEvent event) {
                float Gx = event.values[0];
                float Gy = event.values[1];
                float Gz = event.values[2];

                if (mIsFirst) {
                    mPrevGx = Gx;
                    mPrevGz = Gz;
                    mIsFirst = false;
                    return;
                }


                if (Math.abs(mPrevGx - Gx) > 2 || Math.abs(mPrevGz - Gz) > 2) {
                    Log.d("test", "sensor changed!!");
                    clientMsg = "vehicle movement detected!!";

                    if (!mIsRecording) {
                        mIsRecording = mPreview.startRecording();
                    }

                    mPrevGx = Gx;
                    mPrevGz = Gz;
                }                        
            }};
            
        sensorManager.registerListener(mSensorListener, acc, SensorManager.SENSOR_DELAY_NORMAL);
    }
    
    public class ClientThread implements Runnable {

        public void run() {
            try {
                InetAddress serverAddr = InetAddress.getByName(mServerIpAddress);
                Log.d("ClientActivity", "C: Connecting...");
                Socket socket = new Socket(serverAddr, SERVERPORT);
                connected = true;
                while (connected) {
                    try {
                        if (!clientMsg.isEmpty()) {
                            Log.d("ClientActivity", "C: Sending command.");
                            PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket
                                        .getOutputStream())), true);
                                // where you issue the commands
                                out.println(clientMsg);
                            Log.d("ClientActivity", "C: Sent.");
                            
                            clientMsg = "";
                        }
                    } catch (Exception e) {
                        Log.e("ClientActivity", "S: Error", e);
                    }
                }
                socket.close();
                Log.d("ClientActivity", "C: Closed.");
            } catch (Exception e) {
                Log.e("ClientActivity", "C: Error", e);
                connected = false;
            }
        }
    }
}
