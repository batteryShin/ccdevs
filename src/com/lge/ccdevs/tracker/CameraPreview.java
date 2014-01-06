package com.lge.ccdevs.tracker;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.media.MediaRecorder.OnInfoListener;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.Toast;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "CameraPreview";

    private Context mContext;

    Camera mCamera;    
    private byte[]              mBuffer;
    private int[]               mRGB;
    private int                 mFrameSize;
    private Bitmap              mBitmap;
    private Matrix              mTMat;
    private Matrix              mTIMat;
    private boolean             mCameraIsInitialized;
    private int                 mFrameWidth;
    private int                 mFrameHeight;
    SurfaceHolder               holder;

    private boolean mRun = false;
    private boolean mTargetSet = false;
    private boolean  mCVInitialized = false;
    private boolean mCheckTR = false;
    
    private static int d = 0;
    private static final int FRAME_COUNT = 5;
    private static final int THRESHOLD_PX_DISTANCE = 100;

    private static int mFrameCount = 0;
    private RectF mScaledTargetRect;
    private RectF mDetectedRect;

    private PointF mDetectedPoint;

    private float[] mTargetPts;
    private float[] mScaledTargetPts;
    private float[] mDetectedPts;

    private static int mDispWidth;
    private static int mDispHeight;

    // recording
    private MediaRecorder mMediaRecorder = null;
    private static final int MAX_RECORDING_MSEC = 60000;
    private static int mVideoNum;

    // ###dc### Native call for CV process..
	private native final void native_cv_facex(Bitmap bmp);
    private native final void native_cv_init(Bitmap base_img, RectF rgn);
    private native final void native_cv_init(Bitmap base_img, float[] pts);
	private native final RectF native_cv_track(Bitmap img);
	private native final float[] native_cv_match(Bitmap img);

	static public void test_capture(Bitmap img, String fname) {
        try {
            FileOutputStream out = new FileOutputStream("/sdcard/mytracker" + File.separator + fname);
            img.compress(Bitmap.CompressFormat.JPEG, 100, out);
        } catch (FileNotFoundException e) {
        }
    }

    static public void decodeYUV420SP(int[] rgb, byte[] yuv420sp, int width, int height) {
        final int frameSize = width * height;

        for (int j = 0, yp = 0; j < height; j++) {
            int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
            for (int i = 0; i < width; i++, yp++) {
                int y = (0xff & ((int) yuv420sp[yp])) - 16;
                if (y < 0) y = 0;
                if ((i & 1) == 0) {
                    v = (0xff & yuv420sp[uvp++]) - 128;
                    u = (0xff & yuv420sp[uvp++]) - 128;
                }
                int y1192 = 1192 * y;
                int r = (y1192 + 1634 * v);
                int g = (y1192 - 833 * v - 400 * u);
                int b = (y1192 + 2066 * u);

                if (r < 0) r = 0; else if (r > 262143) r = 262143;
                if (g < 0) g = 0; else if (g > 262143) g = 262143;
                if (b < 0) b = 0; else if (b > 262143) b = 262143;

                rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
            }
        }
    }

	public interface IOnDrawTargetListener {
	    public void onDrawTarget(RectF target);
	    public void onDrawTarget(PointF target, int radius);
        public void onDrawTarget(float[] target);
	}

	private IOnDrawTargetListener mIOnDrawTargetListener = null;	
	public void setOnDrawTargetListener(IOnDrawTargetListener listener) {
	    mIOnDrawTargetListener = listener;
	}


	public interface IOnRecordingStopListener {
        public void onRecordingStopped();
    }

    private IOnRecordingStopListener mIOnRecordingStopListener = null;
    public void setOnRecordingStopListener(IOnRecordingStopListener listener) {
        mIOnRecordingStopListener = listener;
    }

    public interface IOnTrackResultListener {
        public void onResultChanged(PointF pt, int dist);
    }
    private IOnTrackResultListener mIOnTrackResultListener = null;
    public void setOnTrackResultListener(IOnTrackResultListener listener) {
        mIOnTrackResultListener = listener;
    }


    public CameraPreview(Context context) {
        super(context);
        mContext = context;

        holder = getHolder();
        holder.addCallback(this);
        
        mBuffer = null;
        mRGB = null;

        mDetectedRect = new RectF();
        mScaledTargetRect = new RectF();
        mDetectedPoint = new PointF();
        mDetectedPts = new float[8];
        mTargetPts = new float[8];
        mScaledTargetPts = new float[8];
                
        WindowManager wm = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics metrics = new DisplayMetrics();
        wm.getDefaultDisplay().getMetrics(metrics);

        mDispWidth = metrics.widthPixels;
        mDispHeight = metrics.heightPixels;
        Log.i(TAG, "Display (" + mDispWidth + " x " + mDispHeight + ")");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setupCamera(width, height);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        openCamera();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        releaseCamera();
    }

    public void setPreview() throws IOException {
        mCamera.setPreviewDisplay(holder);
    }

    protected void onPreviewStarted(int previewWidth, int previewHeight) {
//        mFrameSize = previewWidth * previewHeight;
//        mBitmap = Bitmap.createBitmap(previewWidth, previewHeight, Bitmap.Config.ARGB_8888);
    }
    
    public void setupCamera(int width, int height) {
        Log.i(TAG, "setupCamera(" + width + " x " + height + ")");
        synchronized (this) {
            if (mCamera != null) {
                setPreviewSize(width/2, height/2);
                
                int size = mFrameWidth * mFrameHeight;
                size = size * ImageFormat.getBitsPerPixel(mCamera.getParameters().getPreviewFormat()) / 8;
                
                mRGB = new int[size];
                mBuffer = new byte[size];
                mCamera.addCallbackBuffer(mBuffer);

                try {
                    setPreview();
                } catch (IOException e) {
                    Log.e(TAG, "mCamera.setPreviewDisplay/setPreviewTexture fails: " + e);
                }

                onPreviewStarted(mFrameWidth, mFrameHeight);

                mCamera.startPreview();
            }
        }
    }

    protected void processFrame(byte[] data){
        decodeYUV420SP(mRGB, data, mFrameWidth, mFrameHeight);
        mBitmap = Bitmap.createBitmap(mRGB, mFrameWidth, mFrameHeight, Config.ARGB_8888); 

        // native process using opencv
//        native_cv_facex(bmp);
        if( mTargetSet ) {
            if( !mCVInitialized ) {
                Log.i(TAG, "native_cv_init() call");
                native_cv_init(mBitmap, mScaledTargetRect);
                native_cv_init(mBitmap, mScaledTargetPts);
                mCVInitialized = true;
                return;
            }
            Log.i(TAG, "scaled rect = ("
                    + mScaledTargetRect.left + "," + mScaledTargetRect.top + "), (" + mScaledTargetRect.right + "," + mScaledTargetRect.bottom + ")");

            Log.i(TAG, "native_cv_track() call");
            mScaledTargetRect = native_cv_track(mBitmap);
            Log.i(TAG, "native_cv_match() call");
            mScaledTargetPts = native_cv_match(mBitmap);
        }

        // get detected rect and put it in mDetectedRect
        if( mTIMat.mapRect(mDetectedRect,mScaledTargetRect) ) {
            Log.i(TAG, "draw rect = (" + mDetectedRect.left + 
                    "," + mDetectedRect.top + "), (" + 
                    mDetectedRect.right + "," + mDetectedRect.bottom + ")");
//            mIOnDrawTargetListener.onDrawTarget(mDetectedRect);
            mDetectedPoint.x = (mDetectedRect.left+mDetectedRect.right)/2.f;
            mDetectedPoint.y = (mDetectedRect.top+mDetectedRect.bottom)/2.f;
            mIOnDrawTargetListener.onDrawTarget(mDetectedPoint, 100);

            if( mCheckTR ) {
                mIOnTrackResultListener.onResultChanged(mDetectedPoint, THRESHOLD_PX_DISTANCE);
            }
        }

        mTIMat.mapPoints(mDetectedPts,mScaledTargetPts);
        Log.i(TAG, "draw pts = (" + mDetectedPts[0] + "," + mDetectedPts[1] +
                "), (" + mDetectedPts[2] + "," + mDetectedPts[3] + "), (" +
                "), (" + mDetectedPts[4] + "," + mDetectedPts[5] + "), (" +
                "), (" + mDetectedPts[6] + "," + mDetectedPts[7] + ")");
        mIOnDrawTargetListener.onDrawTarget(mDetectedPts);

        return;
    }
    
    public boolean openCamera() {
        Log.i(TAG, "openCamera");
        releaseCamera();
        mCamera = Camera.open();
        if (mCamera == null) {
            Log.e(TAG, "Can't open camera!");
            return false;
        }
//        mCamera.setDisplayOrientation(90);
        mCamera.setPreviewCallback(new Camera.PreviewCallback(){
              
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                if (mTargetSet) {
                    mFrameCount++;
                    
                    if (!mRun) {
                        mCameraIsInitialized = true;
                        if (mFrameCount >= FRAME_COUNT) {
                            mFrameCount = 0;
                            DoImageProcessing(data);
                        }
                    }
                }
            }
        });
        
        return true;
    }

    public void releaseCamera() {
        synchronized (this) {
            if (mCamera != null) {
                Parameters params = mCamera.getParameters();
                params.setFlashMode(Parameters.FLASH_MODE_OFF);
                mCamera.setParameters(params);

                mCamera.stopPreview();
                mCamera.setPreviewCallback(null);
                mCamera.release();
                mCamera = null;
                
                mCameraIsInitialized = false;
            }
        }
        if(mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }

        if(mTMat != null) {
            mTMat = null;
        }
        if(mTIMat != null) {
            mTIMat = null;
        }
    }

    private void DoImageProcessing(byte[] data) 
    {
        mRun = true;
        try {
            while (mRun && !mCameraIsInitialized) {
                synchronized (this) {
                    wait(100);  // wait 100 milliseconds before trying again.
                }
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        synchronized (this) {            
            processFrame(data);
        }

        mRun = false;
    }
    
    private void setPreviewSize(int width, int height) {
        Camera.Parameters params = mCamera.getParameters();
        List<Camera.Size> supported = params.getSupportedPreviewSizes();
        if (supported != null) {
            for (Camera.Size size : supported) {
                if (size.width <= width && size.height <= height) {
                    mFrameWidth = size.width;
                    mFrameHeight = size.height;
                    
                    // camera is 90 degree rotated
                    float sx = (float)mFrameWidth / (float)mDispWidth;
                    float sy = (float)mFrameHeight / (float)mDispHeight; 
                    Log.i(TAG, "scaling param = " + sx + "," + sy);

                    if( Math.abs(sx-sy)<0.01 ) {
                        mTMat = new Matrix();
                        mTMat.postScale(sx,sy);

                        mTIMat = new Matrix();
                        mTIMat.postScale(1.f/sx,1.f/sy);
                        break;
                    }
                }
            }
            params.setPreviewSize(mFrameWidth, mFrameHeight);
            params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);

            if( getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FLASH) ) {
                params.setFlashMode(Parameters.FLASH_MODE_TORCH);
            }
            mCamera.setParameters(params);
        }
    }
    
    public void setTarget(RectF target, int mode) {
//        if( mTMat.mapRect(mScaledTargetRect,target) ) {
        mTMat.mapRect(mScaledTargetRect,target);
            mScaledTargetPts[0] = mScaledTargetRect.left;
            mScaledTargetPts[1] = mScaledTargetRect.top;
            mScaledTargetPts[2] = mScaledTargetRect.right;
            mScaledTargetPts[3] = mScaledTargetRect.top;
            mScaledTargetPts[4] = mScaledTargetRect.right;
            mScaledTargetPts[5] = mScaledTargetRect.bottom;
            mScaledTargetPts[6] = mScaledTargetRect.left;
            mScaledTargetPts[7] = mScaledTargetRect.bottom;

            Log.i(TAG, "target region = (" + mScaledTargetPts[0] + "," + mScaledTargetPts[1] + 
                    "), (" + mScaledTargetPts[4] + "," + mScaledTargetPts[5] + ")");
            mTargetSet = true;
            if( mode==MonitorModeActivity.MONITOR_MODE_BABY ) {
                mCheckTR = true;
            }
//        }
    }
    
    private boolean prepareVideoRecorder(){
        mMediaRecorder = new MediaRecorder();

        // Step 1: Unlock and set camera to MediaRecorder
        mCamera.unlock();
        mMediaRecorder.setCamera(mCamera);

        // Step 2: Set sources
        mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.CAMCORDER);
        mMediaRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);

        // Step 3: Set a CamcorderProfile (requires API Level 8 or higher)
        mMediaRecorder.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_HIGH));

        // Step 4: Set output file
        String path = mContext.getString(R.string.track_record_dir);
        path += "video" + (mVideoNum++) + ".mp4";
        mMediaRecorder.setOutputFile(path);

        // Step 5: Set the preview output
        mMediaRecorder.setPreviewDisplay(this.getHolder().getSurface());


        //testing max limit
        mMediaRecorder.setMaxDuration(MAX_RECORDING_MSEC);
        mMediaRecorder.setOnInfoListener(new OnInfoListener() {

            @Override
            public void onInfo(MediaRecorder mr, int what, int extra) {
                if (what == MediaRecorder.MEDIA_RECORDER_INFO_MAX_DURATION_REACHED) {
                    if (mIOnRecordingStopListener != null) {
                        mIOnRecordingStopListener.onRecordingStopped();
                    }
                }
            }});


        // Step 6: Prepare configured MediaRecorder
        try {
            mMediaRecorder.prepare();
        } catch (IllegalStateException e) {
            Log.d(TAG, "IllegalStateException preparing MediaRecorder: " + e.getMessage());
            releaseMediaRecorder();
            return false;
        } catch (IOException e) {
            Log.d(TAG, "IOException preparing MediaRecorder: " + e.getMessage());
            releaseMediaRecorder();
            return false;
        }
        return true;
    }
    
    public boolean startRecording() {
     // initialize video camera
        if (prepareVideoRecorder()) {
            // Camera is available and unlocked, MediaRecorder is prepared,
            // now you can start recording
            mMediaRecorder.start();

            // inform the user that recording has started
            Toast.makeText(getContext(), "Recording start!!", Toast.LENGTH_SHORT).show();
            return true;
        } else {
            // prepare didn't work, release the camera
            releaseMediaRecorder();
            Toast.makeText(getContext(), "Recording failed!! Try again!!", Toast.LENGTH_SHORT).show();
        }        
        return false;
    }
    
    public void stopRecording() {
     // stop recording and release camera
        mMediaRecorder.stop();  // stop the recording
        releaseMediaRecorder(); // release the MediaRecorder object
        mCamera.lock();         // take camera access back from MediaRecorder

        // inform the user that recording has stopped
        Toast.makeText(getContext(), "Recording stopped", Toast.LENGTH_SHORT).show();
    }
    
    private void releaseMediaRecorder() {
        if (mMediaRecorder != null) {
            mMediaRecorder.reset();   // clear recorder configuration
            mMediaRecorder.release(); // release the recorder object
            mMediaRecorder = null;
            mCamera.lock();           // lock camera for later use
        }
    }
}
