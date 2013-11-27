package com.lge.ccdevs.tracker;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.RectF;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "CameraPreview";;
    Camera mCamera;    
    private byte[]              mBuffer;
    private int                 mFrameSize;
    private Bitmap              mBitmap;
    private boolean             mCameraIsInitialized;
    private int                 mFrameWidth;
    private int                 mFrameHeight;
    SurfaceHolder               holder;
    private boolean mRun = false;
    private boolean mTargetSet = false;
    
    
    private static final int FRAME_COUNT = 30;
    private static int mFrameCount = 0;
    private RectF mTargetRect;
    private RectF mDetectedRect;
    private Bitmap mPrevBMP;

    // ###dc### Native call for CV process..
	private native final void native_cv_facex(Bitmap bmp);
	private native final void native_cv_track(Bitmap simg, Bitmap dimg, RectF rgn);
//	private native final File native_cv_merge(ArrayList<Bitmap> imgs);

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
        
        mDetectedRect = new RectF();
        mTargetRect = new RectF();
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
        mPrevBMP = Bitmap.createBitmap(previewWidth, previewHeight, Bitmap.Config.ARGB_8888);
    }
    
    public void setupCamera(int width, int height) {
        Log.i(TAG, "setupCamera(" + width + "x" + height + ")");
        synchronized (this) {
            if (mCamera != null) {
                setPreviewSize(width, height);
                
                int size = mFrameWidth * mFrameHeight;
                size = size * ImageFormat.getBitsPerPixel(mCamera.getParameters().getPreviewFormat()) / 8;
                
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
        mBitmap = BitmapFactory.decodeByteArray(data, 0, data.length);

        // native process using opencv
//        native_cv_facex(bmp);
        if( mPrevBMP!=null && !mTargetRect.isEmpty()) {
            Log.d(TAG, "native_cv_track call");
            native_cv_track(mBitmap, mPrevBMP, mTargetRect);
        }
        //mPrevBMP = mBitmap.copy(Bitmap.Config.ARGB_8888, true);
        
        
        // get detected rect and put it in mDetectedRect
        mDetectedRect = mTargetRect;
        Log.d(TAG, "processFrame(), target rect = ( " + mTargetRect.left + ", "
                        + mTargetRect.top + ", " + mTargetRect.right + ", " + mTargetRect.bottom + " )");
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
                    break;
                }
            }
        }
    }
    
    public void setTarget(RectF target) {
        mTargetRect = target;
        
        float top = mTargetRect.top;
        float left = mTargetRect.left;
        float bottom = mTargetRect.bottom;
        float right = mTargetRect.right;        
        
        Log.d(TAG, "target info : (top, left, bottom, right) = ("
                + top + ", " + left + ", " + bottom + ", " + right + ")");
        
        mTargetSet = true;
    }
}
