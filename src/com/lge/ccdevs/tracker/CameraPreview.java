package com.lge.ccdevs.tracker;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.RectF;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.Toast;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "CameraPreview";;
    Camera mCamera;    
    private byte[]              mBuffer;
    private int[]               mRGB;
    private int                 mFrameSize;
    private Bitmap              mBitmap;
    private boolean             mCameraIsInitialized;
    private int                 mFrameWidth;
    private int                 mFrameHeight;
    SurfaceHolder               holder;
    private boolean mRun = false;
    private boolean mTargetSet = false;
    private boolean  mCVInitialized = false;
    
    private static int d = 0;
    private static final int FRAME_COUNT = 30;

    private static int mFrameCount = 0;
    private RectF mTargetRect;
    private RectF mScaledTargetRect;
    private RectF mDetectedRect;
    private static int mDispWidth;
    private static int mDispHeight;
    private float mScaleX;
    private float mScaleY;
    // recording
    private MediaRecorder mMediaRecorder = null;

    // ###dc### Native call for CV process..
	private native final void native_cv_facex(Bitmap bmp);
    private native final void native_cv_init(Bitmap base_img, RectF rgn);
	private native final RectF native_cv_track(Bitmap img);
//	private native final File native_cv_merge(ArrayList<Bitmap> imgs);
//
	static public void test_capture(Bitmap img, String fname) {
        try {
            FileOutputStream out = new FileOutputStream("/sdcard/"+fname);
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
	}
	
	private IOnDrawTargetListener mIOnDrawTargetListener = null;	
	public void setOnDrawTargetListener(IOnDrawTargetListener listener) {
	    mIOnDrawTargetListener = listener;
	}
	
    public CameraPreview(Context context) {
        super(context);
        holder = getHolder();
        holder.addCallback(this);
        
        mBuffer = null;
        mRGB = null;
        
        mDetectedRect = new RectF();
        mTargetRect = new RectF();
        mScaledTargetRect = new RectF();
                
        Point dispSize = new Point();
        WindowManager wm = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
        wm.getDefaultDisplay().getRealSize(dispSize);

        mDispWidth = dispSize.x;
        mDispHeight = dispSize.y;
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
        mFrameSize = previewWidth * previewHeight;
        mBitmap = Bitmap.createBitmap(previewWidth, previewHeight, Bitmap.Config.ARGB_8888);        
    }
    
    public void setupCamera(int width, int height) {
        Log.i(TAG, "setupCamera(" + width + "x" + height + ")");
        synchronized (this) {
            if (mCamera != null) {
                setPreviewSize(width, height);
                
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
                mCVInitialized = true;
                return;
            }
//            test_capture(mBitmap, "capture"+ (d++) +".jpg");
            Log.i(TAG, "native_cv_track() call");
            mScaledTargetRect = native_cv_track(mBitmap);



            Log.i(TAG, "scaled rect = ("
                    + mScaledTargetRect.left + ", " + mScaledTargetRect.top + ") - (" + mScaledTargetRect.right + ", " + mScaledTargetRect.bottom + ")");
        }
        
        // get detected rect and put it in mDetectedRect
        mDetectedRect.set(mScaledTargetRect.top / mScaleX,
                            mScaledTargetRect.left / mScaleY,
                            mScaledTargetRect.bottom / mScaleX,
                            mScaledTargetRect.right / mScaleY);
        Log.i(TAG, "draw rect = (" + mDetectedRect.left + 
                    ", " + mDetectedRect.top + ") - (" + 
                    mDetectedRect.right + ", " + mDetectedRect.bottom + ")");
        mIOnDrawTargetListener.onDrawTarget(mDetectedRect);

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
        mCamera.setDisplayOrientation(90);
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
        
        
        Camera.Parameters params = mCamera.getParameters();
        params.setFocusMode(Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
        mCamera.setParameters(params);
        return true;
    }

    public void releaseCamera() {
        synchronized (this) {
            if (mCamera != null) {
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
                    
                    params.setPreviewSize(mFrameWidth, mFrameHeight);                
                    mCamera.setParameters(params);
                    
                    // camera is 90 degree rotated
                    mScaleX = (float)mFrameWidth / (float)mDispHeight;
                    mScaleY = (float)mFrameHeight / (float)mDispWidth;
                    Log.i(TAG, "scaling param = " + mScaleX + ", " + mScaleY);
                    if( mScaleX==mScaleY )
                        break;
                }
            }
        }
    }
    
    public void setTarget(RectF target) {
        mTargetRect = target;
        
        mScaledTargetRect.set(mTargetRect.top * mScaleX,
                                mTargetRect.left * mScaleY,
                                mTargetRect.bottom * mScaleX,
                                mTargetRect.right * mScaleY);

        float top = mScaledTargetRect.top;
        float left = mScaledTargetRect.left;
        float right = mScaledTargetRect.right;
        float bottom = mScaledTargetRect.bottom;
        Log.i(TAG, "target region = (" + top + ", " + left + 
                    ") - (" + right + ", " + bottom + ")");
        mTargetSet = true;
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
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();
        path += File.separator + "mytracker";
        String filepath = path + File.separator + "myvideo.mp4";
        mMediaRecorder.setOutputFile(filepath);

        // Step 5: Set the preview output
        mMediaRecorder.setPreviewDisplay(this.getHolder().getSurface());

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
    
    private void releaseMediaRecorder(){
        if (mMediaRecorder != null) {
            mMediaRecorder.reset();   // clear recorder configuration
            mMediaRecorder.release(); // release the recorder object
            mMediaRecorder = null;
            mCamera.lock();           // lock camera for later use
        }
    }

}


