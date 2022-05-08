package com.example.snake;

import android.app.ActionBar;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;
import java.util.List;

public class DrawView extends View {
    ViewGroup.LayoutParams params;
    private Path path = new Path();
    private Paint brush = new Paint();
    private static final String TAG = "MyActivity";
    private List<Rect> undrawnRects = new ArrayList<>();
    private List<Rect> drawnRects = new ArrayList<>();
    private int maxDrawnRects = 20;

    public DrawView(Context context) {
        super(context);
        init();
    }

    public DrawView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public void init() {
        brush.setAntiAlias(true);
        brush.setColor(Color.BLACK);
        brush.setStyle(Paint.Style.STROKE);
        brush.setStrokeJoin(Paint.Join.ROUND);
        brush.setStrokeWidth(8f);

        params = new ViewGroup.LayoutParams(ActionBar.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float pointX = event.getX();
        float pointY = event.getY();
        if (pointX < 0 || pointY < 0 || pointX > 1023 || pointY > 1023) {
            return false;
        }
        Rect rectangle = getRectangle(pointX, pointY);
        undrawnRects.add(rectangle);
        //Log.i(TAG, "Touched point: (" + pointX + ", " + pointY + ")");


        switch(event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                path.moveTo(pointX, pointY);
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_MOVE:
                path.lineTo(pointX, pointY);
                //Log.i(TAG, "Point: (" + pointX + ", " + pointY + ")");
                break;
            default:
                return false;
        }
        postInvalidate();
        return false;
    }

    public Rect getRectangle(float x, float y) {
        int roundedX = (int) x;
        int roundedY = (int) y;
        //Log.i(TAG, "Float point: (" + x + ", " + y + "), Rounded point: (" + roundedX + ", " + roundedY + ")");
        int modX = roundedX % 8;
        int modY = roundedY % 8;
        roundedX -= modX;
        roundedY -= modY;
        Rect rectangle = new Rect(roundedX, roundedY, roundedX + 7, roundedY + 7);
        return rectangle;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        while (!undrawnRects.isEmpty()) {
            Rect rec = undrawnRects.remove(0);
            boolean rectDrawn = false;
            for (Rect drawnRec : drawnRects) {
                if (rec.equals(drawnRec)) {
                    rectDrawn = true;
                    break;
                }
            }
            if (!rectDrawn) {
                if (drawnRects.size() >= maxDrawnRects) {
                    drawnRects.remove(0);
                }
                drawnRects.add(rec);
            }
        }
        for (Rect drawnRec : drawnRects) {
            canvas.drawRect(drawnRec, brush);
        }
    }
}
