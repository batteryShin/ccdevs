package com.lge.ccdevs.tracker;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

import org.apache.http.conn.util.InetAddressUtils;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.widget.TextView;

public class WatcherModeActivity extends Activity {
    public static final int SERVERPORT = 5555;
    
    private TrackerServer mBoundService;    
    private String mServerIP;
    
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mBoundService = ((TrackerServer.LocalBinder)service).getService();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mBoundService = null;
        }
        
    };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.watcher_mode);
        
        TextView text_ip = (TextView)findViewById(R.id.text_ip);
        mServerIP = getLocalIpAddress();
        text_ip.setText("server IP: " + mServerIP);
        
        
        // check if service is running
        if (!isServiceRunning()) {
            // start service
            Intent intent = new Intent(WatcherModeActivity.this, TrackerServer.class);
            intent.putExtra("com.lge.ccdevs.tracker.IP", mServerIP);
            startService(intent);
        }
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
