package com.lge.ccdevs.tracker;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {

    private static final String TAG = "test";//"MyPerviewCam::SurfaceView";
    Camera mCamera;    
    private byte[]              mFrame;
    private byte[]              mBuffer;
    private int                 mFrameSize;
    private Bitmap              mBitmap;
    private int[]               mRGBA;
    private boolean             mCameraIsInitialized;
    private int                 mFrameWidth;
    private int                 mFrameHeight;
    SurfaceHolder               holder;
    private boolean mRun = false;
    
    private static int mFrameCount = 0;

    // ###dc### Native call for CV process..
	private native final void native_cv_facex(Bitmap bmp);
//	private native final File native_cv_merge(ArrayList<Bitmap> imgs);

    public CameraPreview(Context context) {
        super(context);
        holder = getHolder();
        holder.addCallback(this);
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
        /*if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
            mCamera.setPreviewTexture( new SurfaceTexture(10) );
        else
            mCamera.setPreviewDisplay(null);*/
        mCamera.setPreviewDisplay(holder);
    }

    protected void onPreviewStarted(int previewWidth, int previewHeight) {
        mFrameSize = previewWidth * previewHeight;
        mRGBA = new int[mFrameSize];
        mBitmap = Bitmap.createBitmap(previewWidth, previewHeight, Bitmap.Config.ARGB_8888);        
    }
    
    public void setupCamera(int width, int height) {
        Log.i(TAG, "setupCamera(" + width + "x" + height + ")");
        synchronized (this) {
            if (mCamera != null) {
                Camera.Parameters params = mCamera.getParameters();
                setPreviewSize(width, height);
                //mFrameWidth = width;
                //mFrameHeight = height;
                params.setPreviewSize(mFrameWidth, mFrameHeight);                
                mCamera.setParameters(params);
                params = mCamera.getParameters();
                Log.i(TAG, "Chosen Camera Preview Size: " + params.getPreviewSize().width + "x" + params.getPreviewSize().height);
                int size = params.getPreviewSize().width * params.getPreviewSize().height;
                size = size * ImageFormat.getBitsPerPixel(params.getPreviewFormat()) / 8;
                mBuffer = new byte[size];
                mFrame = new byte [size];
                mCamera.addCallbackBuffer(mBuffer);

                try {
                    setPreview();
                } catch (IOException e) {
                    Log.e(TAG, "mCamera.setPreviewDisplay/setPreviewTexture fails: " + e);
                }

                onPreviewStarted(params.getPreviewSize().width, params.getPreviewSize().height);

                mCamera.startPreview();
            }
        }
    }

    protected Bitmap processFrame(byte[] data){
        int[] rgba = mRGBA;
        Bitmap bmp = mBitmap;

        bmp.setPixels(rgba, 0, mBitmap.getWidth() , 0, 0, mBitmap.getWidth(), mBitmap.getHeight());

        // ###dc### native process..(opencv)
        native_cv_facex(bmp);

        return bmp;
    }
    
    public boolean openCamera() {
        Log.i(TAG, "openCamera");
        releaseCamera();
        mCamera = Camera.open();
        if(mCamera == null) {
            Log.e(TAG, "Can't open camera!");
                return false;
        }
        mCamera.setPreviewCallback(new Camera.PreviewCallback(){
              
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                mFrameCount++;
//                Log.i("test", "onPreviewFrame");
                
                if ( !mRun ) {
                    mFrame = data;
                    mCameraIsInitialized = true;
                    if( mFrameCount>=60 ) {
                        mFrameCount = 0;
                        DoImageProcessing();
                    }
                }
            }
        });
        return true;
    }

    public void releaseCamera() {
        Log.i(TAG, "releaseCamera");
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
        mRGBA = null;
    }

    private void DoImageProcessing() 
    {
//        Log.i("MyRealTimeImageProcessing", "DoImageProcessing():");
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
        Bitmap bmp = null;
        Bitmap largebmp;
        synchronized (this) {            
            bmp = processFrame(mFrame);
        }
        Canvas c = null;
        try{
            c = holder.lockCanvas(null);
            //c.drawColor(Color.BLACK);
            synchronized(holder){
                if (bmp != null) {
                    if (c != null) {
                        largebmp = Bitmap.createScaledBitmap(bmp, mFrameWidth, mFrameHeight, false);
                        c.drawBitmap(largebmp,0,0, null);
                    }
                }
            }
        }
        finally{
            if(c != null){
                holder.unlockCanvasAndPost(c);
            }
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
                    break;
                }
            }
        }
    }
}
