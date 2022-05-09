package com.example.snake;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class BTConnection {
    private BluetoothSocket socket;
    private InputStream inStream;
    private OutputStream outStream;
    private static final String TAG = "MyActivity";

    public BTConnection(BluetoothDevice device) {
        BluetoothSocket socket = null;
        try {
            socket = device.createRfcommSocketToServiceRecord(UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"));
        } catch (IOException e) {
            Log.e(TAG, "Socket's create() method failed", e);
            return;
        }
        try {
            socket.connect();
        } catch (IOException connectException) {
            try {
                socket.close();
                Log.v(TAG, "SOCKET HAD TO CLOSE");
            } catch (IOException closeException) {
                Log.e(TAG, "Could not close the client socket", closeException);
                return;
            }
        }
        this.socket = socket;
        InputStream tmpIn = null;
        OutputStream tmpOut = null;
        try {
            tmpIn = this.socket.getInputStream();
        } catch (IOException e) {
            Log.e(TAG, "Error occurred when creating input stream", e);
        }
        try {
            tmpOut = this.socket.getOutputStream();
        } catch (IOException e) {
            Log.e(TAG, "Error occurred when creating output stream", e);
        }

        this.inStream = tmpIn;
        this.outStream = tmpOut;
    }

    public void write(byte[] bytes) {
        try {
            this.outStream.write(bytes);
            Log.v(TAG, "MESSAGE WRITTEN SUCCESSFULLY");
        }
        catch (IOException e) {
            Log.e(TAG, "Error occurred when writing to the output stream", e);
        }
    }

    public int read() {
        try {
            if (this.inStream.available() > 0) {
                return this.inStream.read();
            }
        } catch (IOException e) {
            Log.e(TAG, "Error occurred when reading from the input stream", e);
        }
        return -1;
    }
}