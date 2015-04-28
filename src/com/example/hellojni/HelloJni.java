/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.hellojni;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.os.Bundle;


public class HelloJni extends Activity
{
	private static Context sContext;
	private TextView mTitle;
	private Button mGrayButton;
	private Button mEdgeButton;
	private Button mNormalButton;
	private ImageView mImage;
	private Bitmap mBitmapOrig;
	private Bitmap mBitmapNew;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_layout);

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        sContext = this;
        
        mTitle = (TextView) findViewById(R.layout.main_layout);
        mGrayButton = (Button) findViewById(R.id.gray_button);
        mEdgeButton = (Button) findViewById(R.id.edge_button);
        mNormalButton = (Button) findViewById(R.id.normal_button);
        mImage = (ImageView) findViewById(R.id.image);
        
     // load bitmap from resources
        BitmapFactory.Options options = new BitmapFactory.Options();
        // Make sure it is 24 bit color as our image processing algorithm 
		// expects this format
        options.inPreferredConfig = Config.ARGB_8888;
        mBitmapOrig = BitmapFactory.decodeResource(this.getResources(), R.drawable.campanile, options);
        if (mBitmapOrig != null) {
            mImage.setImageBitmap(mBitmapOrig);
        }
        
        mNormalButton.setOnClickListener(new OnClickListener() {

			@SuppressLint("NewApi")
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Drawable originalImage = sContext.getDrawable(R.drawable.campanile);
				mImage.setImageResource(R.drawable.campanile);
			}
        	
        });
        
        mGrayButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
//				getString();
				mBitmapNew = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				convertToGray(mBitmapOrig, mBitmapNew);
				mImage.setImageBitmap(mBitmapNew);
			}
        	
        });
        
        mEdgeButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Bitmap greyBitmap = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				convertToGray(mBitmapOrig, greyBitmap);
				mBitmapNew = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				findEdges(greyBitmap, mBitmapNew);
				mImage.setImageBitmap(mBitmapNew);
			}
        	
        });
    }

    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.hellojni/lib/libhello-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("hello-jni");
    }
    
    /* A native method that is implemented by the
     * 'hello-jni' native library, which is packaged
     * with this application.
     */
    public native String  stringFromJNI();
    public native String  getString();
    public native void convertToGray(Bitmap bitmapIn,Bitmap bitmapOut);
    public native void findEdges(Bitmap bitmapIn, Bitmap bitmapOut);
}
