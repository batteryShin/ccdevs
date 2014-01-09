package com.lge.ccdevs.tracker;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

import android.app.IntentService;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ResultReceiver;
import android.util.Log;
import android.widget.Toast;

public class MessagingService extends Service {
    public static final int SERVERPORT = 5555;
    
    public static final String PROCESS_MSG = "com.lge.ccdevs.tracker.process_message"; 
    
    private static final int SHOW_TOAST = 0;
    private static final int SEND_MSG = 1;
    
    private ServerSocket mServerSocket = null;
    private Socket mClient = null;
    private String mServerIp = "";
    
    private MessagingEventReceiver mEventReceiver;


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("MessagingService", "onStartCommand");

        Bundle b = intent.getExtras();
        if (b == null) {
            Log.d("MessagingService", "onStartCommand::smt. wrong, cannot start service!!");
            return -1;
        }
        mServerIp = b.getString("ServerIP");

        if (mServerIp == null || mServerIp.equals("")) {
            Log.d("MessagingService", "Cannot connect to the Server, invalid IP!!");
            return -1;
        }

        Thread fst = new Thread(new ServerThread());
        fst.start();

        mEventReceiver = new MessagingEventReceiver();
        IntentFilter eventFilter = new IntentFilter();
        eventFilter.addAction(PROCESS_MSG);
        registerReceiver(mEventReceiver, eventFilter);
        
        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.d("MessagingService", "onDestroy");

        if (mServerSocket != null) {
            try {
                mServerSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mServerSocket = null;
        }
        
        if (mClient != null) {
            try {
                mClient.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mClient = null;
        }

        unregisterReceiver(mEventReceiver);
        super.onDestroy();
    }

    Handler handler = new Handler() {
      @Override
      public void handleMessage(Message msg) {
          super.handleMessage(msg);
          switch (msg.what) {
              case SHOW_TOAST :
                  String toast = (String)msg.obj;
                  Toast.makeText(getApplicationContext(), toast, Toast.LENGTH_SHORT).show();
                  break;
              case SEND_MSG :
                  try {
                      Log.d("MessagingService", "C: Sending command.");

                      PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(mClient.getOutputStream())), true);
                      String message = (String)msg.obj;
                      out.println(message);

                      Log.d("MessagingService", "C: Sent.");
                  } catch (Exception e) {
                      Log.e("MessagingService", "S: Error", e);
                      
                      Intent intent3 = new Intent();
                      intent3.setAction("ERROR");
                      sendBroadcast(intent3);
                      
                      e.printStackTrace();
                  }
                  break;
              default : break;
          }
      }
    };

    public void sendMessage(String msg) {
        Log.d("MessagingService", "sendMessage:" + msg);
                
        Message message = new Message();
        message.what = SEND_MSG;
        message.obj = msg;
        handler.sendMessage(message);
    }
    
    class MessagingEventReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(PROCESS_MSG)) {
                String msg = intent.getStringExtra("message");
                sendMessage(msg);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public class ServerThread implements Runnable {
        public void run() {
            try {
                if (mServerIp != null) {
                    Intent intent = new Intent();
                    intent.setAction("MSG_WAITING");
                    sendBroadcast(intent);
                    
                    mServerSocket = new ServerSocket(SERVERPORT);
                    while (true) {
                        // listen for incoming clients
                        mClient = mServerSocket.accept();

                        Intent intent2 = new Intent();
                        intent2.setAction("MSG_CONNECTED");
                        sendBroadcast(intent2);

                        Message message = new Message();
                        message.what = SEND_MSG;
                        message.obj = "hello, this is server.. you are connected!";
                        handler.sendMessage(message);
                    }
                } else {
                    Intent intent = new Intent();
                    intent.setAction("MSG_NO_INTERNET");
                    sendBroadcast(intent);
                }
            } catch (Exception e) {
                Intent intent = new Intent();
                intent.setAction("MSG_ERROR");
                sendBroadcast(intent);
                
                e.printStackTrace();
            }
        }
    }
    
    
     
}
