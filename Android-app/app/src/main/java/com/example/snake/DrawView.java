package com.example.snake;

import android.app.ActionBar;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
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
import android.widget.Toast;

import java.lang.reflect.Array;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class DrawView extends View {
    ViewGroup.LayoutParams params;
    private Path path = new Path();
    private Paint brush = new Paint();
    private Paint appleBrush = new Paint();
    private static final String TAG = "MyActivity";
    private List<Rect> undrawnRects = new ArrayList<>();
    private List<Rect> drawnRects = new ArrayList<>();
    private int maxDrawnRects = 8;
    private Rect[] rectsToDraw = new Rect[maxDrawnRects + 1];
    private BTConnection connection;
    private List<int[]> apples = new ArrayList<>();

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

        appleBrush.setAntiAlias(true);
        appleBrush.setColor(Color.RED);
        appleBrush.setStyle(Paint.Style.STROKE);
        appleBrush.setStrokeJoin(Paint.Join.ROUND);
        appleBrush.setStrokeWidth(8f);

        params = new ViewGroup.LayoutParams(ActionBar.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);

        BluetoothDevice device = MainActivity.getHC06();
        if (device == null) {
            Log.e(TAG, "No device found!");
            return;
        }
        connection = new BTConnection(device);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float pointX = event.getX();
        float pointY = event.getY();
        //Log.i(TAG, "Before connection read");
        int appleX = connection.read();
        //Log.i(TAG, "Connection read: " + appleX);
        if (appleX != -1) {
            int appleY = connection.read();
            int[] apple = {appleX, appleY};
            Log.i(TAG, "Adding apple: " + apple[0] + ", " + apple[1]);
            apples.add(apple);
        }
        if (pointX < 0 || pointY < 0 || pointX > 1023 || pointY > 1023) {
            return false;
        }
        Rect rectangle = getRectangle(pointX, pointY);
        undrawnRects.add(rectangle);

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
        int modX = roundedX % 32;
        int modY = roundedY % 32;
        roundedX -= modX;
        roundedY -= modY;
        Rect rectangle = new Rect(roundedX, roundedY, roundedX + 31, roundedY + 31);
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
                int left = rec.left / 32;
                int top = rec.top / 32;
                byte[] msgBytes = new byte[] {(byte) left, (byte) top };
                //Log.i(TAG, "left: " + left + ", top: " + top);
                connection.write(msgBytes);
                drawnRects.add(rec);
            }
        }
        int i = 0;
        for (Rect drawnRec : drawnRects) {
            rectsToDraw[i] = drawnRec;
            i++;
            //canvas.drawRect(drawnRec, brush);
        }
        if (!apples.isEmpty()) {
            int[] app = {-1, -1};
            while (!apples.isEmpty()) {
                app = apples.remove(0);
            }
            if (app[0] != -1 && app[1] != -1) {
                int left = app[0] * 32;
                int top = app[1] * 32;
                Log.i(TAG, "About to print apple: " + left + ", " + top);
                Rect appRect = new Rect(left, top, left + 31, top + 31);
                rectsToDraw[rectsToDraw.length - 1] = appRect;
                //canvas.drawRect(left, top, left + 31, top + 31, appleBrush);
            }
        }
        for (int count = 0; count < rectsToDraw.length; count++) {
            if (rectsToDraw[count] == null) {
                continue;
            }
            if (count == rectsToDraw.length - 1) {
                canvas.drawRect(rectsToDraw[count], appleBrush);
            } else {
                canvas.drawRect(rectsToDraw[count], brush);
            }
        }
    }
}
