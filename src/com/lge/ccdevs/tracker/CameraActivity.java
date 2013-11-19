package com.lge.ccdevs.tracker;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.app.Activity;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;


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
}
