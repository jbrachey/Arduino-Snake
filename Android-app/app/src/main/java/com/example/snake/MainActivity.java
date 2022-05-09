package com.example.snake;

import androidx.appcompat.app.AppCompatActivity;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;
import java.util.Set;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MyActivity";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout);
        Button connectButton = (Button) findViewById(R.id.button);
        connectButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                if (bluetoothAdapter == null) {
                    // Device doesn't support Bluetooth
                    Toast.makeText(getApplicationContext(), "BT not supported :(",
                            Toast.LENGTH_LONG).show();
                    return;
                }
                if (!bluetoothAdapter.isEnabled()) {
                    Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                    startActivityForResult(enableBtIntent, 6);
                }

                LocationManager locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
                boolean isGpsEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
                Log.v(TAG, "GPS enabled? " + isGpsEnabled);
                if (!isGpsEnabled) {
                    startActivityForResult(new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS), 3);
                }

                Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
                if (pairedDevices.size() > 0) {
                    boolean foundDevice = false;
                    if (getHC06() != null) {
                        foundDevice = true;
                        Intent i = new Intent(MainActivity.this, SnakeActivity.class);
                        startActivity(i);
                    }
                    if (!foundDevice) {
                        Toast.makeText(getApplicationContext(), "Connect to HC06 in BT Settings",
                                Toast.LENGTH_LONG).show();
                    }
                }
            }
        });
    }
    public static BluetoothDevice getHC06() {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
        for (BluetoothDevice device : pairedDevices) {
            if (device.getName().equals("HC-06")) {
                return device;
            }
        }
        return null;
    }
}