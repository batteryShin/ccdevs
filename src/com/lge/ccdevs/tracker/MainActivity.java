
package com.lge.ccdevs.tracker;

import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.Socket;
import java.util.Enumeration;

import org.apache.http.conn.util.InetAddressUtils;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {
    public static final int SERVERPORT = 8080;
    
    private TrackerServer mBoundService;
    private Messenger mServerMessenger = null;
    
    private boolean connected = false;
    private boolean mISServiceRunning = false;
    
    private String mServerIP;
    private String clientMsg = "";
    private String serverIpAddress = "";
    
    
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mBoundService = ((TrackerServer.LocalBinder)service).getService();
            mServerMessenger = new Messenger(service);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            mBoundService = null;
        }
        
    };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        TextView text_ip = (TextView)findViewById(R.id.text_ip);
        mServerIP = getLocalIpAddress();
        text_ip.setText("server IP: " + mServerIP);

        Button btn_watcher = (Button) findViewById(R.id.btn_watcher);
        btn_watcher.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MainActivity.this, TrackerServer.class);
                intent.putExtra("com.lge.ccdevs.tracker.IP", mServerIP);
                //startService(intent);
                bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
                mISServiceRunning = true;
            }});
        
        Button btn_target = (Button) findViewById(R.id.btn_target);
        btn_target.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MainActivity.this, CameraActivity.class);
                startActivity(intent);
            }});
        
        
        // Client Mode
        Button connectPhones = (Button) findViewById(R.id.connect_phones);
        connectPhones.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!connected) {
                    serverIpAddress = ((EditText)findViewById(R.id.server_ip)).getText().toString();
                    if (!serverIpAddress.equals("")) {
                        Thread cThread = new Thread(new ClientThread());
                        cThread.start();
                    }
                }
            }
        });
        
        Button btn_send = (Button)findViewById(R.id.send_msg);
        btn_send.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // get message
                EditText edit_msg = (EditText)findViewById(R.id.message);
                String message = edit_msg.getText().toString();
                
                if (!message.isEmpty()) {
                    clientMsg = message;
                }
            }});
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    
    
    /*
    //ip = getDottedDecimalIP(getLocalIPAddress());
    private byte[] getLocalIPAddress() {
        try { 
            Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
            
            Log.e("AndroidNetworkAddressFactory", "getLocalIPAddress()"); 
            
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) { 
                NetworkInterface intf = en.nextElement(); 
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) { 
                    InetAddress inetAddress = enumIpAddr.nextElement(); 
                    if (!inetAddress.isLoopbackAddress()) { 
                        if (inetAddress instanceof Inet4Address) { // fix for Galaxy Nexus. IPv4 is easy to use :-) 
                            return inetAddress.getAddress(); 
                        } 
                        //return inetAddress.getHostAddress().toString(); // Galaxy Nexus returns IPv6 
                    } 
                } 
            }
        } catch (SocketException ex) { 
            //Log.e("AndroidNetworkAddressFactory", "getLocalIPAddress()", ex); 
        } catch (NullPointerException ex) { 
            //Log.e("AndroidNetworkAddressFactory", "getLocalIPAddress()", ex); 
        } 
        return null; 
    }
    
    private String getDottedDecimalIP(byte[] ipAddr) {
        //convert to dotted decimal notation:
        String ipAddrStr = "";
        for (int i=0; i<ipAddr.length; i++) {
            if (i > 0) {
                ipAddrStr += ".";
            }
            ipAddrStr += ipAddr[i]&0xFF;
        }
        return ipAddrStr;
    }
    */
        
    /*public String getLocalIpAddress()
    {
        final String IP_NONE = "N/A";
        final String WIFI_DEVICE_PREFIX = "eth";
     
        String LocalIP = IP_NONE;
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();           
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        if( LocalIP.equals(IP_NONE) )
                            LocalIP = inetAddress.getHostAddress().toString();
                        else if( intf.getName().startsWith(WIFI_DEVICE_PREFIX) )
                            LocalIP = inetAddress.getHostAddress().toString();
                        }
                    }
                }
        } catch (SocketException e) {
            Log.e("MainActivity", "getLocalIpAddress Exception:"+e.toString());
        }
        return LocalIP;
    }*/
    
    @Override
    protected void onDestroy() {
        if (mISServiceRunning) {
            unbindService(mConnection);
            mISServiceRunning = false;
        }
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
    
    public class ClientThread implements Runnable {

        public void run() {
            try {
                InetAddress serverAddr = InetAddress.getByName(serverIpAddress);
                Log.d("ClientActivity", "C: Connecting...");
                Socket socket = new Socket(serverAddr, SERVERPORT);
                connected = true;
                while (connected) {
                    try {
                        if (!clientMsg.isEmpty()) {
                            //Log.d("ClientActivity", "C: Sending command.");
                            PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket
                                        .getOutputStream())), true);
                                // where you issue the commands
                                out.println(clientMsg);
                            //Log.d("ClientActivity", "C: Sent.");
                            
                            clientMsg = "";
                        }
                    } catch (Exception e) {
                        Log.e("ClientActivity", "S: Error", e);
                    }
                }
                socket.close();
                Log.d("ClientActivity", "C: Closed.");
            } catch (Exception e) {
                Log.e("ClientActivity", "C: Error", e);
                connected = false;
            }
        }
    }
    
    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            /*switch (msg.what) {
                case TrackerServer.MSG_FROM_SERVER:
                    //set text
                    break;
                default:                        
            }*/
        }
    };
}
