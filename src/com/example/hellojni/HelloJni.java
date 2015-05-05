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

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;


public class HelloJni extends Activity
{
	private static final int REQUEST_IMAGE_CAPTURE = 1;

	private static Context sContext;
	private TextView mTitle;
	private Button mGrayButton;
	private Button mEdgeButton;
	private Button mNormalButton;
	private Button mCameraButton;
	private Button mParallelEdgeButton;
	private Button mParallelGrayButton;
	private ImageView mImage;
	private Bitmap mBitmapOrig;
	private Bitmap mBitmapNew;
	private String mCurrentPhotoPath;


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
		mCameraButton = (Button) findViewById(R.id.camera_button);
		mParallelEdgeButton = (Button) findViewById(R.id.parallel_edge_button);
		mParallelGrayButton = (Button) findViewById(R.id.parallel_gray_button);
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
				parallelPrint();
				//				Drawable originalImage = sContext.getDrawable(R.drawable.campanile);
				mImage.setImageBitmap(mBitmapOrig);
			}

		});

		mGrayButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				//				getString();
				mBitmapNew = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				long startTime = System.nanoTime();
				convertToGray(mBitmapOrig, mBitmapNew);
				long endTime = System.nanoTime();
				long duration = (endTime - startTime)/ 1000000;
				Toast.makeText(sContext, "Operation took " + duration +" milliseconds", Toast.LENGTH_SHORT).show();
				mImage.setImageBitmap(mBitmapNew);
			}

		});

		mEdgeButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				Bitmap greyBitmap = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				convertToGray(mBitmapOrig, greyBitmap);
				mBitmapNew = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				long startTime = System.nanoTime();
				findEdges(greyBitmap, mBitmapNew);
				long endTime = System.nanoTime();
				long duration = (endTime - startTime)/ 1000000;
				Toast.makeText(sContext, "Operation took " + duration +" milliseconds", Toast.LENGTH_SHORT).show();
				mImage.setImageBitmap(mBitmapNew);
			}

		});

		mCameraButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
				if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
					File photoFile = null;
					try {
						photoFile = createImageFile();
					} catch (IOException ex) {

					}
					if (photoFile != null) {
						takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT,
								Uri.fromFile(photoFile));
						startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
					}
				}
			}

		});

		mParallelGrayButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				mBitmapNew = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				long startTime = System.nanoTime();
				parallelConvertToGray(mBitmapOrig, mBitmapNew);
				long endTime = System.nanoTime();
				long duration = (endTime - startTime)/ 1000000;
				Toast.makeText(sContext, "Operation took " + duration +" milliseconds", Toast.LENGTH_SHORT).show();
				
				mImage.setImageBitmap(mBitmapNew);
			}

		});

		mParallelEdgeButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				Bitmap greyBitmap = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				convertToGray(mBitmapOrig, greyBitmap);
				mBitmapNew = Bitmap.createBitmap(mBitmapOrig.getWidth(), mBitmapOrig.getHeight(),
						Config.ALPHA_8);
				
				long startTime = System.nanoTime();
				parallelFindEdges(greyBitmap, mBitmapNew);
				long endTime = System.nanoTime();
				long duration = (endTime - startTime)/ 1000000;
				Toast.makeText(sContext, "Operation took " + duration +" milliseconds", Toast.LENGTH_SHORT).show();
				
				mImage.setImageBitmap(mBitmapNew);
			}

		});
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		if (requestCode == REQUEST_IMAGE_CAPTURE) {
			if (resultCode == RESULT_OK) {
				// Image captured and saved to fileUri specified in the Intent
				Toast.makeText(this, "Image captured", Toast.LENGTH_SHORT).show();
				mBitmapOrig = BitmapFactory.decodeFile(mCurrentPhotoPath);
				//                mBitmapOrig = (Bitmap) data.getExtras().get("data");
				ExifInterface exif = null;
				try {
					exif = new ExifInterface(mCurrentPhotoPath);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
				System.out.println("orientation: " + orientation);
				switch (orientation) {
				case ExifInterface.ORIENTATION_ROTATE_90:
					Matrix matrix = new Matrix();
					matrix.postRotate(90);
					mBitmapOrig = Bitmap.createBitmap(mBitmapOrig, 0, 0, mBitmapOrig.getWidth(), mBitmapOrig.getHeight(), matrix, true); ; 
					break;
				}
				mImage.setImageBitmap(mBitmapOrig);
			} else if (resultCode == RESULT_CANCELED) {
				// User cancelled the image capture
			} else {
				// Image capture failed, advise user
			}
		}
	}

	private File createImageFile() throws IOException {
		// Create an image file name
		String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
		String imageFileName = "JPEG_" + timeStamp + "_";
		File storageDir = Environment.getExternalStoragePublicDirectory(
				Environment.DIRECTORY_PICTURES);
		File image = File.createTempFile(
				imageFileName,  /* prefix */
				".jpg",         /* suffix */
				storageDir      /* directory */
				);

		// Save a file: path for use with ACTION_VIEW intents
		mCurrentPhotoPath = image.getAbsolutePath();
		return image;
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
	public native void  parallelPrint();
	public native void convertToGray(Bitmap bitmapIn,Bitmap bitmapOut);
	public native void parallelConvertToGray(Bitmap bitmapIn,Bitmap bitmapOut);
	public native void findEdges(Bitmap bitmapIn, Bitmap bitmapOut);
	public native void parallelFindEdges(Bitmap bitmapIn, Bitmap bitmapOut);
}
