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
 *
 */
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <omp.h>

#define  LOG_TAG    "AndroidImageNDK"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef struct
{
	uint8_t alpha;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} argb;

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */
jstring
Java_com_example_hellojni_HelloJni_stringFromJNI( JNIEnv* env,
		jobject thiz )
{
#if defined(__arm__)
#if defined(__ARM_ARCH_7A__)
#if defined(__ARM_NEON__)
#if defined(__ARM_PCS_VFP)
#define ABI "armeabi-v7a/NEON (hard-float)"
#else
#define ABI "armeabi-v7a/NEON"
#endif
#else
#if defined(__ARM_PCS_VFP)
#define ABI "armeabi-v7a (hard-float)"
#else
#define ABI "armeabi-v7a"
#endif
#endif
#else
#define ABI "armeabi"
#endif
#elif defined(__i386__)
#define ABI "x86"
#elif defined(__x86_64__)
#define ABI "x86_64"
#elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */
#define ABI "mips64"
#elif defined(__mips__)
#define ABI "mips"
#elif defined(__aarch64__)
#define ABI "arm64-v8a"
#else
#define ABI "unknown"
#endif

	return (*env)->NewStringUTF(env, "Hello from JNI !  Compiled with ABI " ABI ".");
}

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */
void
Java_com_example_hellojni_HelloJni_parallelPrint( JNIEnv* env,
		jobject obj)
{
	int length = 20;
	int i = 0;
#pragma omp parallel for
	for (i = 0; i < length; i++) {
		/* Do work... */
		LOGI("printing out: %d", i);
	}
}

/*
convertToGray
Pixel operation
 */
void Java_com_example_hellojni_HelloJni_convertToGray(JNIEnv* env,
		jobject  obj,
		jobject bitmapcolor,
		jobject bitmapgray)
{
	AndroidBitmapInfo  infocolor;
	void*              pixelscolor;
	AndroidBitmapInfo  infogray;
	void*              pixelsgray;
	int                ret;
	int             y;
	int             x;

	LOGI("convertToGray");
	if ((ret = AndroidBitmap_getInfo(env, bitmapcolor, &infocolor)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmapgray, &infogray)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infocolor.width,infocolor.height,infocolor.stride,infocolor.format,infocolor.flags);
	if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return;
	}

	LOGI("gray image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infogray.width,infogray.height,infogray.stride,infogray.format,infogray.flags);
	if (infogray.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapcolor, &pixelscolor)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapgray, &pixelsgray)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	// modify pixels with image processing algorithm
	for (y=0;y<infocolor.height;y++) {
		argb * line = (argb *) pixelscolor;
		uint8_t * grayline = (uint8_t *) pixelsgray;
		for (x=0;x<infocolor.width;x++) {
			grayline[x] = 0.3 * line[x].red + 0.59 * line[x].green + 0.11*line[x].blue;
		}

		pixelscolor = (char *)pixelscolor + infocolor.stride;
		pixelsgray = (char *) pixelsgray + infogray.stride;
	}

	LOGI("unlocking pixels");
	AndroidBitmap_unlockPixels(env, bitmapcolor);
	AndroidBitmap_unlockPixels(env, bitmapgray);
}

/*
convertToGray
Pixel operation
 */
void Java_com_example_hellojni_HelloJni_parallelConvertToGray(JNIEnv* env,
		jobject  obj,
		jobject bitmapcolor,
		jobject bitmapgray)
{
	AndroidBitmapInfo  infocolor;
	void*              pixelscolor;
	AndroidBitmapInfo  infogray;
	void*              pixelsgray;
	int                ret;
	int             y;
	int             x;

	LOGI("convertToGray");
	if ((ret = AndroidBitmap_getInfo(env, bitmapcolor, &infocolor)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmapgray, &infogray)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infocolor.width,infocolor.height,infocolor.stride,infocolor.format,infocolor.flags);
	if (infocolor.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return;
	}

	LOGI("gray image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infogray.width,infogray.height,infogray.stride,infogray.format,infogray.flags);
	if (infogray.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapcolor, &pixelscolor)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapgray, &pixelsgray)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	// modify pixels with image processing algorithm
	#pragma omp parallel for private(y)
	for (y=0;y<infocolor.height;y++) {
		argb * line = (argb *) pixelscolor;
		uint8_t * grayline = (uint8_t *) pixelsgray;
		#pragma omp parallel for private(x)
		for (x=0;x<infocolor.width;x++) {
			grayline[x] = 0.3 * line[x].red + 0.59 * line[x].green + 0.11*line[x].blue;
		}

		pixelscolor = (char *)pixelscolor + infocolor.stride;
		pixelsgray = (char *) pixelsgray + infogray.stride;
	}

	LOGI("unlocking pixels");
	AndroidBitmap_unlockPixels(env, bitmapcolor);
	AndroidBitmap_unlockPixels(env, bitmapgray);
}



/*
findEdges
Matrix operation
 */
void Java_com_example_hellojni_HelloJni_findEdges(JNIEnv
		* env, jobject  obj, jobject bitmapgray,jobject bitmapedges)
{
	AndroidBitmapInfo  infogray;
	void*              pixelsgray;
	AndroidBitmapInfo  infoedges;
	void*              pixelsedge;
	int                ret;
	int             y;
	int             x;
	int             sumX,sumY,sum;
	int             i,j;
	int                Gx[3][3];
	int                Gy[3][3];
	uint8_t            *graydata;
	uint8_t            *edgedata;

	LOGI("findEdges running");

	Gx[0][0] = -1;Gx[0][1] = 0;Gx[0][2] = 1;
	Gx[1][0] = -2;Gx[1][1] = 0;Gx[1][2] = 2;
	Gx[2][0] = -1;Gx[2][1] = 0;Gx[2][2] = 1;

	Gy[0][0] = 1;Gy[0][1] = 2;Gy[0][2] = 1;
	Gy[1][0] = 0;Gy[1][1] = 0;Gy[1][2] = 0;
	Gy[2][0] = -1;Gy[2][1] = -2;Gy[2][2] = -1;

	if ((ret = AndroidBitmap_getInfo(env, bitmapgray, &infogray)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmapedges, &infoedges)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	LOGI("gray image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infogray.width,infogray.height,infogray.stride,infogray.format,infogray.flags);
	if (infogray.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infoedges.width,infoedges.height,infoedges.stride,infoedges.format,infoedges.flags);
	if (infoedges.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapgray, &pixelsgray)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapedges, &pixelsedge)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	// modify pixels with image processing algorithm

	LOGI("time to modify pixels....");

	graydata = (uint8_t *) pixelsgray;
	edgedata = (uint8_t *) pixelsedge;

	for (y=0;y<=infogray.height - 1;y++) {
		for (x=0;x<infogray.width -1;x++) {
			sumX = 0;
			sumY = 0;
			// check boundaries
			if (y==0 || y == infogray.height-1) {
				sum = 0;
			} else if (x == 0 || x == infogray.width -1) {
				sum = 0;
			} else {
				// calc X gradient
				for (i=-1;i<=1;i++) {
					for (j=-1;j<=1;j++) {
						sumX += (int) ( (*(graydata + x + i + (y + j)
								* infogray.stride)) * Gx[i+1][j+1]);
					}
				}

				// calc Y gradient
				for (i=-1;i<=1;i++) {
					for (j=-1;j<=1;j++) {
						sumY += (int) ( (*(graydata + x + i + (y + j)
								* infogray.stride)) * Gy[i+1][j+1]);
					}
				}
				sum = abs(sumX) + abs(sumY);
			}

			if (sum>255) sum = 255;
			if (sum<0) sum = 0;

			*(edgedata + x + y*infogray.width) = 255 - (uint8_t) sum;
		}
	}

	AndroidBitmap_unlockPixels(env, bitmapgray);
	AndroidBitmap_unlockPixels(env, bitmapedges);
}


/*
findEdges
Matrix operation
 */
void Java_com_example_hellojni_HelloJni_parallelFindEdges(JNIEnv
		* env, jobject  obj, jobject bitmapgray,jobject bitmapedges)
{
	AndroidBitmapInfo  infogray;
	void*              pixelsgray;
	AndroidBitmapInfo  infoedges;
	void*              pixelsedge;
	int                ret;
	int             y;
	int             x;
	int             sumX,sumY,sum;
	int             i,j;
	int                Gx[3][3];
	int                Gy[3][3];
	uint8_t            *graydata;
	uint8_t            *edgedata;

	LOGI("findEdges running");

	Gx[0][0] = -1;Gx[0][1] = 0;Gx[0][2] = 1;
	Gx[1][0] = -2;Gx[1][1] = 0;Gx[1][2] = 2;
	Gx[2][0] = -1;Gx[2][1] = 0;Gx[2][2] = 1;

	Gy[0][0] = 1;Gy[0][1] = 2;Gy[0][2] = 1;
	Gy[1][0] = 0;Gy[1][1] = 0;Gy[1][2] = 0;
	Gy[2][0] = -1;Gy[2][1] = -2;Gy[2][2] = -1;

	if ((ret = AndroidBitmap_getInfo(env, bitmapgray, &infogray)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if ((ret = AndroidBitmap_getInfo(env, bitmapedges, &infoedges)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	LOGI("gray image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infogray.width,infogray.height,infogray.stride,infogray.format,infogray.flags);
	if (infogray.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	LOGI("color image :: width is %d; height is %d; stride is %d; format is %d;flags is %d",infoedges.width,infoedges.height,infoedges.stride,infoedges.format,infoedges.flags);
	if (infoedges.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapgray, &pixelsgray)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmapedges, &pixelsedge)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	// modify pixels with image processing algorithm

	LOGI("time to modify pixels....");

	graydata = (uint8_t *) pixelsgray;
	edgedata = (uint8_t *) pixelsedge;
	#pragma omp parallel for private(x)
	for (y=0;y<=infogray.height - 1;y++) {

		for (x=0;x<infogray.width -1;x++) {
//		#pragma omp parallel for private(y)
			sumX = 0;
			sumY = 0;
			// check boundaries
			if (y==0 || y == infogray.height-1) {
				sum = 0;
			} else if (x == 0 || x == infogray.width -1) {
				sum = 0;
			} else {
				// calc X gradient
				for (i=-1;i<=1;i++) {
					for (j=-1;j<=1;j++) {
						sumX += (int) ( (*(graydata + x + i + (y + j)
								* infogray.stride)) * Gx[i+1][j+1]);
					}
				}

				// calc Y gradient
				for (i=-1;i<=1;i++) {
					for (j=-1;j<=1;j++) {
						sumY += (int) ( (*(graydata + x + i + (y + j)
								* infogray.stride)) * Gy[i+1][j+1]);
					}
				}
				sum = abs(sumX) + abs(sumY);
			}

			if (sum>255) sum = 255;
			if (sum<0) sum = 0;

			*(edgedata + x + y*infogray.width) = 255 - (uint8_t) sum;
		}
	}


	AndroidBitmap_unlockPixels(env, bitmapgray);
	AndroidBitmap_unlockPixels(env, bitmapedges);
}
