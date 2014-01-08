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

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
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
    private ServerSocket serverSocket;

    private static TextView mTextStatus;
    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.monitor_mode);

        mContext = this;
        mServerIP = getWifiIpAddress();

        TextView text_ip = (TextView)findViewById(R.id.text_ip);
        text_ip.setText("Server IP: " + mServerIP);

        mTextStatus  = (TextView)findViewById(R.id.text_status);

        if (mServerIP == null) {
            Toast.makeText(mContext, "network not connected!!", Toast.LENGTH_SHORT).show();
        } else {
            Thread fst = new Thread(new ServerThread());
            fst.start();
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

    public String getWifiIpAddress() {
        WifiManager wifiManager = (WifiManager) getSystemService(WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        int ipAddress = wifiInfo.getIpAddress();
        return String.format("%d.%d.%d.%d",
                (ipAddress & 0xff), (ipAddress >> 8 & 0xff),
                (ipAddress >> 16 & 0xff), (ipAddress >> 24 & 0xff));
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


    public class ServerThread implements Runnable {

        public void run() {
            try {
                if (mServerIP != null) {
                    handler.sendEmptyMessage(MSG_WAITING);
                    serverSocket = new ServerSocket(SERVERPORT);
                    while (true) {
                        // listen for incoming clients
                        Socket client = serverSocket.accept();
                        handler.sendEmptyMessage(MSG_CONNECTED);

                        //
                        try {
                            Log.d("MessagingService", "C: Sending command.");
                            PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(client.getOutputStream())), true);
                            out.println("hello, this is server.. you are connected!");
                            Log.d("MessagingService", "C: Sent.");
                        } catch (Exception e) {
                            Log.e("MessagingService", "S: Error", e);
                        }
                        //
                    }
                } else {
                    handler.sendEmptyMessage(MSG_NO_INTERNET);
                }
            } catch (Exception e) {
                handler.sendEmptyMessage(MSG_ERROR);
                e.printStackTrace();
            }
        }
    }
}
