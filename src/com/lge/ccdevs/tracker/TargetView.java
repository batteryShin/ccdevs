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
    
    private Paint p;
    
    public TargetView(Context context) {
        super(context);
        
        mPtTargetStart = new PointF();
        mPtTargetEnd = new PointF();
        
        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.MAGENTA);
        p.setStyle(Paint.Style.STROKE);
    }
    
    public TargetView(Context context, AttributeSet attrs) {
        super(context, attrs);
        
        mPtTargetStart = new PointF();
        mPtTargetEnd = new PointF();
        
        p = new Paint();
        p.setStrokeWidth(5);
        p.setColor(Color.MAGENTA);
        p.setStyle(Paint.Style.STROKE);
    }
    
    @Override
    protected void onDraw(Canvas canvas) {
        if (mPtTargetStart != null && mPtTargetEnd != null) {
            p.setColor(Color.MAGENTA);
            canvas.drawRect(mPtTargetStart.x, mPtTargetStart.y, mPtTargetEnd.x, mPtTargetEnd.y, p);
        }
        super.onDraw(canvas);
    }
    
    public void drawTarget(RectF target) {
        Log.d(TAG, "TargetView::drawTarget = (" + target.left + ", " + target.top + ", " + target.right + ", " + target.bottom);
        mPtTargetStart.x = target.left;
        mPtTargetStart.y = target.top;
        
        mPtTargetEnd.x = target.right;
        mPtTargetEnd.y = target.bottom;
        
        this.invalidate();
    }
}
