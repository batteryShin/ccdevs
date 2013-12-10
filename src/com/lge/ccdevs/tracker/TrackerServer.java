package com.lge.ccdevs.tracker;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Enumeration;

import org.apache.http.conn.util.InetAddressUtils;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Messenger;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

public class TrackerServer extends Service {
    private static final int NOTI_ID = 5;
    private final IBinder mBinder = new LocalBinder();
    
    private NotificationManager mNM;
    private String mMessage = "";
    
    // default ip
    public static String SERVERIP;

    // designate a port
    public static final int SERVERPORT = 8080;

    private Handler handler = new Handler();

    private ServerSocket serverSocket;
    
    private Context mContext;
    
    
    public class LocalBinder extends Binder {
        TrackerServer getService() {
            return TrackerServer.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
    
    @Override
    public void onCreate() {
        Toast.makeText(getApplicationContext(), "start watching..", Toast.LENGTH_SHORT).show();
        mContext = this;

        mNM = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        SERVERIP = intent.getStringExtra("com.lge.ccdevs.tracker.IP");
                
        Thread fst = new Thread(new ServerThread());
        fst.start();

        return START_STICKY;
    }
    
    @Override
    public void onDestroy() {
        Toast.makeText(getApplicationContext(), "stop watching..", Toast.LENGTH_SHORT).show();
        
        try {
            // make sure you close the socket upon exiting
            serverSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        mNM.cancel(NOTI_ID);
        super.onDestroy();
    }
    
    public class ServerThread implements Runnable {

        public void run() {
            try {
                if (SERVERIP != null) {
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(mContext, "Listening on IP: " + SERVERIP, Toast.LENGTH_SHORT).show();
                        }
                    });
                    serverSocket = new ServerSocket(SERVERPORT);
                    while (true) {
                        // listen for incoming clients
                        Socket client = serverSocket.accept();
                        handler.post(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(mContext, "Connected..", Toast.LENGTH_SHORT).show();
                            }
                        });

                        try {
                            BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream()));
                            String line = null;
                            while ((line = in.readLine()) != null) {
                                Log.d("ServerActivity", line);
                                mMessage = line;
                                
                                handler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        //clientMsg.append(mMessage + "\n");
                                        showNotification(R.drawable.ic_launcher, mMessage);
                                    }
                                });
                            }
                            break;
                        } catch (Exception e) {
                            handler.post(new Runnable() {
                                @Override
                                public void run() {
                                    Toast.makeText(mContext, "Oops. Connection interrupted. Please reconnect your phones.", Toast.LENGTH_SHORT).show();
                                }
                            });
                            e.printStackTrace();
                        }
                    }
                } else {
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(mContext, "Couldn't detect internet connection.", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
            } catch (Exception e) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(mContext, "Error!!", Toast.LENGTH_SHORT).show();
                    }
                });
                e.printStackTrace();
            }
        }
    }
    
    private void showNotification(int iconId, String msg) {
        PendingIntent pi = PendingIntent.getActivity(this, 0, new Intent(this, MainActivity.class), 0);
        NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
        
        builder.setSmallIcon(iconId)
            .setAutoCancel(true)
            .setContentTitle("title")
            .setContentInfo("info")
            .setContentText(msg)
            .setTicker(msg) // null
            .setContentIntent(pi);
        
        builder.setContentIntent(pi);
        mNM.notify(R.layout.activity_main, builder.build());
    }
}
