package com.lge.ccdevs.tracker;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;

import com.lge.ccdevs.tracker.CameraActivity.ClientThread;

import android.app.IntentService;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

public class MessagingService extends IntentService {
        public static final int SERVERPORT = 5555;
    private static final int SHOW_TOAST = 0;
    private static final int CONNECT = 1;
    
    private TrackerServer mBoundService;
    private boolean mServerConnected = false;
    private Socket mSocket;
    private String mServerIP;
    private String clientMsg = "hi";
    private String mServerIpAddress = "";
    
    public MessagingService(String name) {
        super(name);
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Log.d("MessagingService", "onHandleIntent");
        Message msg = new Message();
        msg.what = SHOW_TOAST;
        msg.obj = "onHandleIntent";
        handler.sendMessage(msg);
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        Log.d("MessagingService", "onBind");
        
        Message msg = new Message();
        msg.what = SHOW_TOAST;
        msg.obj = "onBind";
        handler.sendMessage(msg);
        
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("MessagingService", "onStartCommand");
        Message msg = new Message();
        msg.what = SHOW_TOAST;
        msg.obj = "onStartCommand";
        handler.sendMessage(msg);
        
        mServerIpAddress = intent.getExtras().getString("ServerIP");
        
        /*if (mServerIpAddress==null || mServerIpAddress.equals("")) {
            //Toast.makeText(getApplicationContext(), "Cannot connect to the Server!!", Toast.LENGTH_SHORT);
            Log.d("MessagingService", "Cannot connect to the Server!!");
        } else {
            if (!mServerConnected) {
                Thread cThread = new Thread(new ClientThread());
                cThread.start();
                
                Message msg2 = new Message();
                msg2.what = CONNECT;
                handler.sendMessageDelayed(msg2, 1000);
            } else {
                Message msg2 = new Message();
                msg2.what = CONNECT;
                handler.sendMessage(msg2);
            }
        }*/
        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.d("MessagingService", "onDestroy");
        
        Message msg = new Message();
        msg.what = SHOW_TOAST;
        msg.obj = "onDestroy";
        handler.sendMessage(msg);
        
        if (mServerConnected) {
            try {
                mSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            Log.d("MessagingService", "C: Closed.");
        }
        super.onDestroy();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d("MessagingService", "onUnbind");
        return super.onUnbind(intent);
    }

    private Handler handler = new Handler() {
      @Override
      public void handleMessage(Message msg) {
          super.handleMessage(msg);
          switch (msg.what) {
              case SHOW_TOAST :
                  String toast = (String)msg.obj;
                  Toast.makeText(getApplicationContext(), toast, Toast.LENGTH_SHORT).show();
                  break;
              case CONNECT :
                  try {
                      if (!clientMsg.isEmpty()) {
                          Log.d("MessagingService", "C: Sending command.");
                          PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(mSocket.getOutputStream())), true);
                          out.println(clientMsg);
                          Log.d("MessagingService", "C: Sent.");
                          
                          clientMsg = "";
                      }
                  } catch (Exception e) {
                      Log.e("MessagingService", "S: Error", e);
                  }
                  break;
              default : break;
          }
      }
    };
    
    public class ClientThread implements Runnable {

        public void run() {
            try {
                InetAddress serverAddr = InetAddress.getByName(mServerIpAddress);
                Log.d("MessagingService", "C: Connecting...");
                mSocket = new Socket(serverAddr, SERVERPORT);
                mServerConnected = true;
            } catch (Exception e) {
                Log.e("MessagingService", "C: Error", e);
                mServerConnected = false;
            }
        }
    }

    
}
