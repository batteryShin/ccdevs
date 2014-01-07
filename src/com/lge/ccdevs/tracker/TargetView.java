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
    private PointF mPtTargetCenter;
    private int mPtTargetRadius;
    private float[] mHmatTarget;

    private RectF mRectBoundary;

    private Paint p;
    
    public TargetView(Context context) {
        super(context);
        
        mPtTargetStart = new PointF();
        mPtTargetEnd = new PointF();
        
        mPtTargetCenter = new PointF();
        mHmatTarget = new float[8];

        mRectBoundary = new RectF();

        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.MAGENTA);
        p.setStyle(Paint.Style.STROKE);
    }
    
    public TargetView(Context context, AttributeSet attrs) {
        super(context, attrs);
        
        mPtTargetStart = new PointF();
        mPtTargetEnd = new PointF();

        mPtTargetCenter = new PointF();
        mHmatTarget = new float[8];
        
        mRectBoundary = new RectF();

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

        if (mPtTargetCenter != null) {
            p.setColor(Color.CYAN);
            canvas.drawCircle(mPtTargetCenter.x, mPtTargetCenter.y, mPtTargetRadius, p);
        }

        if (mRectBoundary!=null) {
            p.setColor(Color.RED);
            p.setAlpha(128);
            p.setStyle(Paint.Style.FILL);
            canvas.drawRoundRect(mRectBoundary, 100, 100, p);

            p.setAlpha(255);
            p.setStyle(Paint.Style.STROKE);
        }
        super.onDraw(canvas);
    }
    
    public void drawTarget(RectF target) {
        Log.i(TAG, "TargetView::drawRect = (" + target.left + ", " + target.top + ", " + target.right + ", " + target.bottom);
        mPtTargetStart.x = target.left;
        mPtTargetStart.y = target.top;
        
        mPtTargetEnd.x = target.right;
        mPtTargetEnd.y = target.bottom;
        
        this.invalidate();
    }

    public void drawTarget(PointF target, int radius) {
        Log.i(TAG, "TargetView::drawCircle = (" + target.x + ", " + target.y + "), size=" + radius);
        mPtTargetCenter = target;
        mPtTargetRadius = radius;
    }

    public void drawTarget(float[] target) {
        Log.i(TAG, "TargetView::drawLines = (" +
                target[0] + ", " + target[1] + "), (" +
                target[2] + ", " + target[3] + "), (" +
                target[4] + ", " + target[5] + "), (" +
                target[6] + ", " + target[7] + ")");

        mHmatTarget = target;

        this.invalidate();
    }

    public void drawBoundary(RectF boundary) {
        Log.i(TAG, "TargetView::drawBoundary = (" + boundary.left + ", " + boundary.top + ", " + boundary.right + ", " + boundary.bottom);
        mRectBoundary = boundary;

        this.invalidate();
    }
}
