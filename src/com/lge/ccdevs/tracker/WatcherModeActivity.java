package com.lge.ccdevs.tracker;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class WatcherModeActivity extends Activity {
    public static final int SERVERPORT = 5555;    
    private String mServerIP;
    private EditText mEditTextIP;
    private Button mButtonConnect;
        
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.watcher_mode);
                
        // connect to the server
        mEditTextIP = (EditText)findViewById(R.id.server_ip);
        mButtonConnect = (Button)findViewById(R.id.connect_phones);
        mButtonConnect.setTag(0);
        mButtonConnect.setText("con");
        mButtonConnect.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if( (Integer)v.getTag()==1 ) {
                    Intent intent = new Intent(WatcherModeActivity.this, TrackerServer.class);
                    if( stopService(intent) ) {
                        mButtonConnect.setText("con");
                        mEditTextIP.setTextColor(Color.parseColor("#000000"));
                        mEditTextIP.setEnabled(true);
                        mEditTextIP.setFocusableInTouchMode(true);
                        v.setTag(0);
                    }
                    return;
                }

                mServerIP = mEditTextIP.getText().toString();
                if (mServerIP.isEmpty()) {
                    Toast.makeText(getApplicationContext(), "Server IP is not set!!", Toast.LENGTH_SHORT).show();
                    return;
                }

                // check if service is running
                if (!isServiceRunning()) {
                    // start service
                    Intent intent = new Intent(WatcherModeActivity.this, TrackerServer.class);
                    intent.putExtra("com.lge.ccdevs.tracker.IP", mServerIP);
                    if( startService(intent)!=null ) {
                        mButtonConnect.setText("discon");
                        mEditTextIP.setTextColor(Color.parseColor("#888888"));
                        mEditTextIP.setEnabled(false);
                        mEditTextIP.setFocusable(false);
                        v.setTag(1);
                    }
                }
            }
        });
    }

    @Override
    protected void onResume() {
        if( !isServiceRunning() ) {
            mButtonConnect.setText("con");
            mEditTextIP.setTextColor(Color.parseColor("#000000"));
            mEditTextIP.setEnabled(true);
            mEditTextIP.setFocusableInTouchMode(true);
        } else {
            mButtonConnect.setText("discon");
            mEditTextIP.setText("???");
            mEditTextIP.setTextColor(Color.parseColor("#888888"));
            mEditTextIP.setEnabled(false);
            mEditTextIP.setFocusable(false);
        }
        super.onResume();
    }


    @Override
    protected void onPause() {
        mServerIP = mEditTextIP.getText().toString();
        super.onPause();
    }

    private boolean isServiceRunning() {
        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (TrackerServer.class.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }
}
