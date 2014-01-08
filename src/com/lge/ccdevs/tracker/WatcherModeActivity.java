package com.lge.ccdevs.tracker;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;


public class WatcherModeActivity extends Activity {
    public static final int SERVERPORT = 5555;    
    private String mServerIP;
        
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.watcher_mode);
                
        // connect to the server
        Button btn_connect = (Button) findViewById(R.id.connect_phones);
        btn_connect.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {                
                mServerIP = ((EditText)findViewById(R.id.server_ip)).getText().toString();
                
                if (mServerIP.isEmpty()) {
                    Toast.makeText(getApplicationContext(), "Server IP is not set!!", Toast.LENGTH_SHORT).show();
                    return;
                }
                
                // check if service is running
                if (!isServiceRunning()) {
                    // start service
                    Intent intent = new Intent(WatcherModeActivity.this, TrackerServer.class);
                    intent.putExtra("com.lge.ccdevs.tracker.IP", mServerIP);
                    startService(intent);
                }
            }
        });
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
