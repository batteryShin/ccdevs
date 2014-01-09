package com.lge.ccdevs.tracker;

import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Enumeration;

import org.apache.http.conn.util.InetAddressUtils;

import com.lge.ccdevs.tracker.MessagingService.MessagingEventReceiver;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ResultReceiver;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MonitorModeActivity extends Activity {
    // Mode selection
    public static final int MONITOR_MODE_VEHICLE = 0;
    public static final int MONITOR_MODE_BABY = 1;
    public static final int MONITOR_MODE_PET = 2;

    private static final int MSG_WAITING = 0;
    private static final int MSG_CONNECTED = 1;
    private static final int MSG_NO_INTERNET = 2;
    private static final int MSG_ERROR = 3;

    public static final int SERVERPORT = 5555;
    private String mServerIP = null;

    private MonitorEventReceiver mEventReceiver = null;
    private static TextView mTextStatus;
    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.monitor_mode);

        mContext = this;
        mServerIP = getLocalIpAddress();

        TextView text_ip = (TextView)findViewById(R.id.text_ip);
        text_ip.setText("Server IP: " + mServerIP);

        mTextStatus  = (TextView)findViewById(R.id.text_status);

        if (mServerIP == null) {
            Toast.makeText(mContext, "network not connected!!", Toast.LENGTH_SHORT).show();
        } else {
            Intent intent = new Intent("MessagingService");
            intent.putExtra("ServerIP", mServerIP);
            startService(intent);
            
            
            mEventReceiver = new MonitorEventReceiver();
            IntentFilter eventFilter = new IntentFilter();
            eventFilter.addAction("MSG_WAITING");
            eventFilter.addAction("MSG_CONNECTED");
            eventFilter.addAction("MSG_NO_INTERNET");
            eventFilter.addAction("MSG_ERROR");
            registerReceiver(mEventReceiver, eventFilter);
        }


        Button btn_mode_pet = (Button)findViewById(R.id.btn_mode_pet);
        btn_mode_pet.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                if (mServerIP != null) {
                    // user can set target(pet) & warning area => alarm when the target is in the warning area
                    Intent intent = new Intent(MonitorModeActivity.this, CameraActivity.class);
                    intent.putExtra("mode", MONITOR_MODE_PET);
                    intent.putExtra("ServerIP", mServerIP);

                    startActivity(intent);
                } else {
                    Toast.makeText(mContext, "network not connected!!", Toast.LENGTH_SHORT).show();
                }
            }});

        Button btn_mode_baby = (Button)findViewById(R.id.btn_mode_baby);
        btn_mode_baby.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                if (mServerIP != null) {
                    // user can set target => alarm when movement occur
                    Intent intent = new Intent(MonitorModeActivity.this, CameraActivity.class);
                    intent.putExtra("mode", MONITOR_MODE_BABY);
                    intent.putExtra("ServerIP", mServerIP);

                    startActivity(intent);
                } else {
                    Toast.makeText(mContext, "network not connected!!", Toast.LENGTH_SHORT).show();
                }
            }});

        Button btn_mode_vehicle = (Button)findViewById(R.id.btn_mode_vehicle);
        btn_mode_vehicle.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                if (mServerIP != null) {
                    // alarm when vehicle movement is detected
                    Intent intent = new Intent(MonitorModeActivity.this, CameraActivity.class);
                    intent.putExtra("mode", MONITOR_MODE_VEHICLE);
                    intent.putExtra("ServerIP", mServerIP);

                    startActivity(intent);
                } else {
                    Toast.makeText(mContext, "network not connected!!", Toast.LENGTH_SHORT).show();
                }
            }});
    }

    @Override
    protected void onDestroy() {
        Intent intent = new Intent("MessagingService");
        stopService(intent);
        unregisterReceiver(mEventReceiver);
        
        super.onDestroy();
    }

    public String getLocalIpAddress() {
        String ipv4;

        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface
                    .getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf
                        .getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();

                    // for getting IPV4 format
                    if (!inetAddress.isLoopbackAddress() && InetAddressUtils.isIPv4Address(ipv4 = inetAddress.getHostAddress())) {
                        return ipv4;
                    }
                }
            }
        } catch (Exception ex) {
            Log.e("IP Address", ex.toString());
        }
        return null;
    }

    static Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            String status = "";
            switch (msg.what) {
                case MSG_WAITING:
                    status = "Waiting for the device..";
                    break;
                case MSG_CONNECTED:
                    status = "Connected..";
                    break;
                case MSG_NO_INTERNET:
                    status = "Couldn't detect internet connection.";
                    break;
                case MSG_ERROR:
                    status = "ERROR!!";
                    break;
                default:
                    break;
            }

            mTextStatus.setText("Server Status: ");
            if (!status.isEmpty()) {
                mTextStatus.append(status);
            }
        }
    };

    class MonitorEventReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction() == "MSG_WAITING") {
                handler.sendEmptyMessage(MSG_WAITING);
            } else if (intent.getAction() == "MSG_CONNECTED") {
                handler.sendEmptyMessage(MSG_CONNECTED);
            } else if (intent.getAction() == "MSG_NO_INTERNET") {
                handler.sendEmptyMessage(MSG_NO_INTERNET);
            } else if (intent.getAction() == "MSG_ERROR") {
                handler.sendEmptyMessage(MSG_ERROR);
            }
        }
    }
}
