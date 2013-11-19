package com.lge.ccdevs.tracker;

import java.io.IOException;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;


public class CameraActivity extends Activity {
    // Called when the activity is first created.
    Camera _camera;
    int fcount = 0;
    
	static {
		System.loadLibrary("Tracker_jni");
		native_init();
	}
	
	private static native final void native_init();

	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(new CameraPreview(this));
    } 
    
}

/*
public class CameraActivity extends Activity {
    private Camera mMyCam = null;
    private CameraPreview mPreview;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.camera_main);
        
        int camNum = Camera.getNumberOfCameras();
        Log.d("test", "# of available cams = " + camNum);
        mMyCam = getCameraInstance(0);
        
        mPreview = new CameraPreview(this, mMyCam);
        
        FrameLayout preview = (FrameLayout) findViewById(R.id.preview);
        preview.addView(mPreview);
        
        
        
        Camera.Parameters params = mMyCam.getParameters();
        int imgformat = params.getPreviewFormat();
        int bitsperpixel = ImageFormat.getBitsPerPixel(imgformat);
        int byteperpixel = bitsperpixel / 8;
        Camera.Size size = params.getPreviewSize();
        
        Log.d("test", "size = " + size.width + " / " + size.height);
        int frame_size = (size.width * size.height) * byteperpixel;
                
        mMyCam.setPreviewCallbackWithBuffer(prevCallback);
        mMyCam.addCallbackBuffer(new byte[frame_size]);
        

        
        Button btn_capture = (Button) findViewById(R.id.btn_capture);
        btn_capture.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                //mMyCam.takePicture(null, mPicture, null);
            }});        
    }
    
    PictureCallback mPicture = new PictureCallback() {
        public void onPictureTaken(byte[] data, Camera camera) {
            Log.d("test", "take picture!!");
        }
    };
    
    
    
    @Override
    protected void onPause() {
        releaseCamera();
        super.onPause();
    }

    private Camera getCameraInstance(int camIndex) {
        Camera c = null;
        try {
            c = Camera.open();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return c;
    }
    
    private void releaseCamera() {
        if (mMyCam != null) {
            mMyCam.release();
            mMyCam = null;
        }
    }
    
    Camera.PreviewCallback prevCallback = new Camera.PreviewCallback() {
        
        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            Log.d("test", "get preview frame!!");
            //camera.addCallbackBuffer(data);
        }
    };
    
    public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
        private SurfaceHolder mHolder;
        private Camera mCamera;
        
        public CameraPreview(Context context, Camera camera) {
            super(context);
            mCamera = camera;
            
            mHolder = getHolder();
            mHolder.addCallback(this);
            
            if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.HONEYCOMB) {
                mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
            if (mHolder.getSurface() == null) {
                return;
            }
            
            // stop preview before changing orientation, resize, reformatting..
            try {
                mCamera.stopPreview();
            } catch (Exception e) {
                e.printStackTrace();
            }
            
            setPictureFormat(format);
            setPreviewSize(w, h);
            
            try {
                mCamera.setPreviewDisplay(mHolder);
                mCamera.startPreview();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            try {
                mCamera.setPreviewDisplay(holder);
                mCamera.startPreview();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            // TODO Auto-generated method stub
            
        }
        
        private void setPictureFormat(int format) {
            Camera.Parameters params = mCamera.getParameters();
            List<Integer> supported = params.getSupportedPictureFormats();
            if (supported != null) {
                for (int i : supported) {
                    if (i == format) {
                        params.setPreviewFormat(format);
                        mCamera.setParameters(params);
                        break;
                    }
                }
            }
        }
        
        private void setPreviewSize(int width, int height) {
            Camera.Parameters params = mCamera.getParameters();
            List<Camera.Size> supported = params.getSupportedPreviewSizes();
            if (supported != null) {
                for (Camera.Size size : supported) {
                    if (size.width <= width && size.height <= height) {
                        params.setPreviewSize(size.width, size.height);
                        mCamera.setParameters(params);
                        break;
                    }
                }
            }
        }
    }
}
*/