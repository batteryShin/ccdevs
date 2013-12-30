package com.lge.ccdevs.tracker;

import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;

import com.lge.ccdevs.tracker.MainActivity.ClientThread;

import android.app.Activity;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Messenger;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MonitorModeActivity extends Activity {
    // Mode selection
    public static final int MONITOR_MODE_PET = 0;
    public static final int MONITOR_MODE_BABY = 1;
    public static final int MONITOR_MODE_VEHICLE = 2;
    
    private String mServerIpAddress;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.monitor_mode);
        
        // connect to the server
        Button btn_connect = (Button) findViewById(R.id.connect_phones);
        btn_connect.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {                
                mServerIpAddress = ((EditText)findViewById(R.id.server_ip)).getText().toString();
            }
        });
        
        
        Button btn_mode_pet = (Button)findViewById(R.id.btn_mode_pet);
        btn_mode_pet.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // user can set target(pet) & warning area => alarm when the target is in the warning area
                Intent intent = new Intent(MonitorModeActivity.this, CameraActivity.class);
                intent.putExtra("mode", MONITOR_MODE_PET);

                mServerIpAddress = ((EditText)findViewById(R.id.server_ip)).getText().toString();
                intent.putExtra("ServerIP", mServerIpAddress);

                startActivity(intent);
            }});
        
        Button btn_mode_baby = (Button)findViewById(R.id.btn_mode_baby);
        btn_mode_baby.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // user can set target => alarm when movement occur
                Intent intent = new Intent(MonitorModeActivity.this, CameraActivity.class);
                intent.putExtra("mode", MONITOR_MODE_BABY);

                mServerIpAddress = ((EditText)findViewById(R.id.server_ip)).getText().toString();
                intent.putExtra("ServerIP", mServerIpAddress);

                startActivity(intent);
            }});
        
        Button btn_mode_vehicle = (Button)findViewById(R.id.btn_mode_vehicle);
        btn_mode_vehicle.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // alarm when vehicle movement is detected
                Intent intent = new Intent(MonitorModeActivity.this, CameraActivity.class);
                intent.putExtra("mode", MONITOR_MODE_VEHICLE);
                
                mServerIpAddress = ((EditText)findViewById(R.id.server_ip)).getText().toString();
                intent.putExtra("ServerIP", mServerIpAddress);
                
                startActivity(intent);
            }});
    }
    
}
