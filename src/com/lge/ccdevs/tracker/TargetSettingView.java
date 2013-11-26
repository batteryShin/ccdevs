package com.lge.ccdevs.tracker;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class TargetSettingView extends View {
    private static final String TAG = "TargetSettingView";;
    private PointF mPtStart;
    private PointF mPtEnd;
       
    private Paint p;

    public TargetSettingView(Context context) {
        super(context);
        
        mPtStart = null;
        mPtEnd = null;
                
        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.CYAN);
        p.setStyle(Paint.Style.STROKE);
    }
    
    public TargetSettingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        
        mPtStart = null;
        mPtEnd = null;
                
        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.CYAN);
        p.setStyle(Paint.Style.STROKE);
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        Log.d(TAG, "onTouchEvent()");
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN :
                mPtStart = new PointF();
                mPtStart.x = event.getX();
                mPtStart.y = event.getY();
                Log.d(TAG, "onTouchEvent:down = " + mPtStart.x + " / " + mPtStart.y);
                return true;
            case MotionEvent.ACTION_MOVE :
                if (mPtEnd == null) {
                    mPtEnd = new PointF();
                }
                mPtEnd.x = event.getX();
                mPtEnd.y = event.getY();
                this.invalidate();
                return true;
            case MotionEvent.ACTION_UP :
                if (mPtEnd == null) {
                    mPtEnd = new PointF();
                }
                mPtEnd.x = event.getX();
                mPtEnd.y = event.getY();
                Log.d(TAG, "onTouchEvent:up = " + mPtEnd.x + " / " + mPtEnd.y);
                
                this.invalidate();
                return true;
            default:
        }
        return super.onTouchEvent(event);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mPtStart != null && mPtEnd != null) {
            Log.d(TAG, "onDraw");
            
            p.setColor(Color.CYAN);
            canvas.drawRect(mPtStart.x, mPtStart.y, mPtEnd.x, mPtEnd.y, p);
        }
        super.onDraw(canvas);
    }

    public RectF getTargetRect() {
        return new RectF(mPtStart.x,mPtStart.y,mPtEnd.x,mPtEnd.y);
    }
}
