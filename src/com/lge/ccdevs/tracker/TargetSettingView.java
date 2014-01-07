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
    private static final String TAG = "TargetSettingView";
    
    public static final int TARGET_SETTING = 1;
    public static final int BOUNDARY_SETTING = 2;

    private int mMode;

    private PointF mPtStart;
    private PointF mPtEnd;
    //private RectF mDrawRect;

    private PointF mPtBdStart;
    private PointF mPtBdEnd;
       
    private Paint p;

    public TargetSettingView(Context context) {
        super(context);
        
        mPtStart = new PointF();
        mPtEnd = new PointF();
                
        mPtBdStart = new PointF();
        mPtBdEnd = new PointF();

        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.CYAN);
        p.setStyle(Paint.Style.STROKE);
    }
    
    public TargetSettingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        
        mPtStart = new PointF();
        mPtEnd = new PointF();
                
        mPtBdStart = new PointF();
        mPtBdEnd = new PointF();

        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.CYAN);
        p.setStyle(Paint.Style.STROKE);
    }

    public void setMode(int mode) {
        mMode = mode;
    }

    public int getMode() {
        return mMode;
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if(mMode==TARGET_SETTING) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN :
                    mPtStart.x = event.getX();
                    mPtStart.y = event.getY();
                    Log.i(TAG, "onTouchEvent:down = " + mPtStart.x + " / " + mPtStart.y);
                    return true;
                case MotionEvent.ACTION_MOVE :
                    mPtEnd.x = event.getX();
                    mPtEnd.y = event.getY();
                    this.invalidate();
                    return true;
                case MotionEvent.ACTION_UP :
                    mPtEnd.x = event.getX();
                    mPtEnd.y = event.getY();
                    Log.i(TAG, "onTouchEvent:up = " + mPtEnd.x + " / " + mPtEnd.y);

                    this.invalidate();
                    return true;
                default:
            }
        } else if(mMode==BOUNDARY_SETTING) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN :
                    mPtBdStart.x = event.getX();
                    mPtBdStart.y = event.getY();
                    Log.i(TAG, "onTouchEvent:down = " + mPtBdStart.x + " / " + mPtBdStart.y);
                    return true;
                case MotionEvent.ACTION_MOVE :
                    mPtBdEnd.x = event.getX();
                    mPtBdEnd.y = event.getY();
                    this.invalidate();
                    return true;
                case MotionEvent.ACTION_UP :
                    mPtBdEnd.x = event.getX();
                    mPtBdEnd.y = event.getY();
                    Log.i(TAG, "onTouchEvent:up = " + mPtBdEnd.x + " / " + mPtBdEnd.y);

                    this.invalidate();
                    return true;
                default:
            }
        }
        return super.onTouchEvent(event);
    }


    @Override
    protected void onDraw(Canvas canvas) {
        if (mPtStart != null && mPtEnd != null) {
            p.setColor(Color.CYAN);

            canvas.drawLine(mPtStart.x, mPtStart.y, mPtStart.x, mPtEnd.y, p);
            canvas.drawLine(mPtEnd.x, mPtStart.y, mPtEnd.x, mPtEnd.y, p);
            canvas.drawLine(mPtStart.x, mPtStart.y, mPtEnd.x, mPtStart.y, p);
            canvas.drawLine(mPtStart.x, mPtEnd.y, mPtEnd.x, mPtEnd.y, p);
        }

        if (mPtBdStart != null && mPtBdEnd != null) {
            p.setColor(Color.MAGENTA);

            canvas.drawLine(mPtBdStart.x, mPtBdStart.y, mPtBdStart.x, mPtBdEnd.y, p);
            canvas.drawLine(mPtBdEnd.x, mPtBdStart.y, mPtBdEnd.x, mPtBdEnd.y, p);
            canvas.drawLine(mPtBdStart.x, mPtBdStart.y, mPtBdEnd.x, mPtBdStart.y, p);
            canvas.drawLine(mPtBdStart.x, mPtBdEnd.y, mPtBdEnd.x, mPtBdEnd.y, p);
        }
        super.onDraw(canvas);
    }

    public RectF getTargetRect() {
        return new RectF(mPtStart.x,mPtStart.y,mPtEnd.x,mPtEnd.y);
    }

    public RectF getBoundaryRect() {
        return new RectF(mPtBdStart.x,mPtBdStart.y,mPtBdEnd.x,mPtBdEnd.y);
    }
}
