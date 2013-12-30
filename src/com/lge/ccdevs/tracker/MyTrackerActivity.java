package com.lge.ccdevs.tracker;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MyTrackerActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.mytracker_main);
        
        Button btn_watcher = (Button)findViewById(R.id.btn_watcher);
        btn_watcher.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MyTrackerActivity.this, WatcherModeActivity.class);
                startActivity(intent);
            }});
        
        Button btn_monitor = (Button)findViewById(R.id.btn_monitor);
        btn_monitor.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                Intent intent = new Intent(MyTrackerActivity.this, MonitorModeActivity.class);
                startActivity(intent);
            }});
    }

}
