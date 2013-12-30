package com.lge.ccdevs.tracker;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class TargetView extends View {
    private static final String TAG = "TargetView";;
    private PointF mPtTargetStart;
    private PointF mPtTargetEnd;

    private float[] mHmatTarget;
    
    private Paint p;
    
    public TargetView(Context context) {
        super(context);
        
        mPtTargetStart = new PointF();
        mPtTargetEnd = new PointF();
        
        mHmatTarget = new float[8];

        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.MAGENTA);
        p.setStyle(Paint.Style.STROKE);
    }
    
    public TargetView(Context context, AttributeSet attrs) {
        super(context, attrs);
        
        mPtTargetStart = new PointF();
        mPtTargetEnd = new PointF();

        mHmatTarget = new float[8];
        
        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.MAGENTA);
        p.setStyle(Paint.Style.STROKE);
    }
    
    @Override
    protected void onDraw(Canvas canvas) {
        if (mHmatTarget[0]>0 && mHmatTarget[1]>0) {
            p.setColor(Color.MAGENTA);
            canvas.drawLine(mHmatTarget[0], mHmatTarget[1], mHmatTarget[2], mHmatTarget[3], p);
            canvas.drawLine(mHmatTarget[2], mHmatTarget[3], mHmatTarget[4], mHmatTarget[5], p);
            canvas.drawLine(mHmatTarget[4], mHmatTarget[5], mHmatTarget[6], mHmatTarget[7], p);
            canvas.drawLine(mHmatTarget[6], mHmatTarget[7], mHmatTarget[0], mHmatTarget[1], p);
        } 

        if (mPtTargetStart != null && mPtTargetEnd != null) {
            p.setColor(Color.CYAN);
            canvas.drawRect(mPtTargetStart.x, mPtTargetStart.y, mPtTargetEnd.x, mPtTargetEnd.y, p);
        }
        super.onDraw(canvas);
    }
    
    public void drawTarget(RectF target) {
        Log.i(TAG, "TargetView::drawTarget = (" + target.left + ", " + target.top + ", " + target.right + ", " + target.bottom);
        mPtTargetStart.x = target.left;
        mPtTargetStart.y = target.top;
        
        mPtTargetEnd.x = target.right;
        mPtTargetEnd.y = target.bottom;
        
        this.invalidate();
    }

    public void drawTarget(float[] target) {
        Log.i(TAG, "TargetView::drawTarget = (" +
                target[0] + ", " + target[1] + "), (" +
                target[2] + ", " + target[3] + "), (" +
                target[4] + ", " + target[5] + "), (" +
                target[6] + ", " + target[7] + ")");

        mHmatTarget = target;

        this.invalidate();
    }
}
