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
#include <math.h>
#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <omp.h>

#define  LOG_TAG    "AndroidImageNDK"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

void predict(int, int, int, int, double*, double*, double*, double*);
void predict_parallel(int, int, int, int, double*, double*, double*, double*);
void matmul(int, int, int, double*, double*, double*);
void matmul_parallel(int, int, int, double*, double*, double*);
void vtanh(int, double*, double*);
void vtanh_parallel(int, double*, double*);
void sigmoid(int, double*, double*);
void sigmoid_parallel(int, double*, double*);

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


jstring Java_com_example_hellojni_HelloJni_predict(JNIEnv* env, jobject obj,
		jobject bitmap, jdoubleArray weights1, jdoubleArray weights2, jint in_parallel) {
	if (in_parallel == 0) {
		LOGI("Call to non parallel predict");
	}
	else {
		LOGI("Call to parallel predict");
	}
	AndroidBitmapInfo infobitmap;
	void* pixels;
	uint8_t* data;
	int ret;

	if ((ret = AndroidBitmap_getInfo(env, bitmap, &infobitmap)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return;
	}

	if (infobitmap.format != ANDROID_BITMAP_FORMAT_A_8) {
		LOGE("Bitmap format is not A_8 !");
		return;
	}

	if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
	}

	data = (uint8_t*)pixels;

	int y;
	int x;

	int height;
	int width;

	int* points_buf;
	int* points_check;
	int points_buf_idx;
	int num_points;

	int current_idx;
	int* boxes;
	int overlaps_box;
	int num_boxes;
	int boxes_idx;
	int box_height;
	int box_width;

	double block_height;
	double block_width;

	int by;
	int bx;

	double max_value;
	int max_pred;
	int i;
	int j;
	int* final_predictions;

	int* explored;
	int num_explored;

	int pt_y;
	int pt_x;
	int check_pt_y;
	int check_pt_x;
	int top;
	int bottom;
	int left;
	int right;

	int* fringe;
	int fringe_idx;
	int num_fringe;

	double* all_features;
	uint8_t* work_area;
	uint8_t* work_area2;
	double pixel_value;

	double* wt1;
	double* wt2;
	double* pred_results;
	double temp;

	double* feature;

	height = infobitmap.height;
	width = infobitmap.width;
	LOGI("height %d width %d area %d", height, width, height*width);
	// Get bounding boxes
	points_buf = calloc(height*width*2, sizeof(int));
	points_check = calloc(height*width, sizeof(int));
	num_points = 0;
	points_buf_idx = 0;

	if (height < 50 || width < 50) {
		return (*env)->NewStringUTF(env, "image too small");
	}

	for (x = 25; x < width - 25; x++) {
		for (y = 25; y < height - 25; y++) {
			if (data[y*width + x] > 250) {
				points_buf[points_buf_idx] = y;
				points_buf[points_buf_idx+1] = x;
				points_buf_idx+=2;
				num_points++;
				points_check[y*width + x] = 1;
			}
		}
	}
	LOGI("num_points %d out of %d", num_points, height*width);

	explored = calloc(height*width, sizeof(int));
	num_explored = 0;

	current_idx = 0;
	boxes = calloc(height*width*4, sizeof(int));
	boxes_idx = 0;
	num_boxes = 0;

	for (i = 0; i < num_points; i++) {
	//while (num_explored < num_points) {
		pt_y = points_buf[current_idx];
		pt_x = points_buf[current_idx+1];

		if (explored[pt_y*width + pt_x] == 1) {
			current_idx += 2;
			continue;
		}

		// Get a bounding box
		top = pt_y;
		bottom = pt_y;
		left = pt_x;
		right = pt_x;
		fringe = calloc(height*width*2, sizeof(int));
		if (fringe == NULL) {
			LOGE("ran out of memory");
			return;
		}
		fringe_idx = 0;
		fringe[fringe_idx] = pt_y;
		fringe[fringe_idx+1] = pt_x;
		fringe_idx += 2;
		num_fringe = 1;

		explored[pt_y*width + pt_x] = 1;
		num_explored += 1;
		while (num_fringe > 0) {
			fringe_idx -= 2;
			num_fringe -= 1;
			pt_y = fringe[fringe_idx];
			pt_x = fringe[fringe_idx+1];

			if (pt_y < top) {
				top = pt_y;
			}
			else if (pt_y > bottom) {
				bottom = pt_y;
			}

			if (pt_x < left) {
				left = pt_x;
			}
			else if (pt_x > right) {
				right = pt_x;
			}

			for (y = -1; y < 2; y++) {
				for (x = -1; x < 2; x++) {
					check_pt_y = pt_y + y;
					check_pt_x = pt_x + x;

					if (check_pt_y >= height || check_pt_y < 0) {
						continue;
					}
					if (check_pt_x >= width || check_pt_x < 0) {
						continue;
					}
					if (check_pt_y*width + check_pt_x >= height*width) {
						LOGI("check_pt_y %d and check_pt_x %d", check_pt_y, check_pt_x);
					}
					if (fringe_idx >= height*width-4) {
						LOGI("fringe_idx %d", fringe_idx);
					}
					if (explored[check_pt_y*width + check_pt_x] != 1 &&
							points_check[check_pt_y*width + check_pt_x] == 1) {
						fringe[fringe_idx] = check_pt_y;
						fringe[fringe_idx+1] = check_pt_x;
						fringe_idx += 2;
						num_fringe += 1;

						explored[check_pt_y*width + check_pt_x] = 1;
						num_explored += 1;
					}
				}
			}
		}

		overlaps_box = 0;
		for (j = 0; j < num_boxes; j++) {
			if ((top > boxes[j*4] - 15 && top < boxes[j*4] + 15) &&
					(left > boxes[j*4 + 3] - 15 && left < boxes[j*4 + 3] + 15)) {
				overlaps_box = 1;
			}
		}

		if (((bottom - top) > 3*(right - left)) ||
				((right - left) > 3*(bottom - top)) ||
				((bottom-top) < 5) ||
				((right-left) < 5) ||
				((bottom - top) > (height - 60)) ||
				overlaps_box == 1) {
			current_idx += 2;
			free(fringe);
			continue;
		}

		boxes[boxes_idx] = top;
		boxes[boxes_idx+1] = right;
		boxes[boxes_idx+2] = bottom;
		boxes[boxes_idx+3] = left;

		LOGI("box %d is %d %d %d %d with height %d and width %d", num_boxes, top, right, bottom, left,
				bottom-top, right - left);

		boxes_idx += 4;
		num_boxes++;
		current_idx += 2;
		free(fringe);
	}


	// Get features from boxes
	all_features = calloc(28*28*num_boxes, sizeof(double));
	for (current_idx = 0; current_idx < num_boxes; current_idx ++) {
		top = boxes[current_idx*4];
		right = boxes[current_idx*4+1];
		bottom = boxes[current_idx*4+2];
		left = boxes[current_idx*4+3];

		box_height = bottom - top;
		box_width = right - left;

		if (box_height < 28 || box_width < 28) {
			continue;
		}

		block_height = box_height / 28.0;
		block_width = box_width / 28.0;

		feature = all_features + (current_idx*28*28);
		work_area = data + top*width + left;
		for (y = 0; y < 28; y++) {
			for (x = 0; x < 28; x++) {
				work_area2 = work_area + y*(int)block_height*width + x*(int)block_width;
				pixel_value = 0;
				// Get average of area
				for (by = 0; by < (int)ceil(block_height); by++) {
					for (bx = 0; bx < (int)ceil(block_width); bx++) {
						if (by < (int)block_height && bx < (int)block_width) {
							pixel_value += work_area2[by*width + bx];
						}
						else if (by >= (int)block_height && by+y*(int)block_height+top < height &&
								bx < (int)block_width) {
							pixel_value += work_area2[by*width + bx]*(block_height - floor(block_height));
						}
						else if (bx >= (int)block_width && bx+x*(int)block_width+left < width &&
								by < (int)block_height) {
							pixel_value += work_area2[by*width + bx]*(block_width - floor(block_width));
						}
						else if (bx >= (int)block_width && bx+x*(int)block_width+left < width &&
								by >= (int)block_height && by+y*(int)block_height+top < height) {
							pixel_value += work_area2[by*width + bx]*(block_width - floor(block_width))*
									(block_height - floor(block_height));
						}
					}
				}
				pixel_value = pixel_value/(int)ceil(block_height)/(int)ceil(block_width);
				feature[y*28+x] = pixel_value;
				// end get average
			}
		}
	}


	wt1 = (*env)->GetDoubleArrayElements(env, weights1, NULL);
	wt2 = (*env)->GetDoubleArrayElements(env, weights2, NULL);

	pred_results = calloc(num_boxes*10, sizeof(double));
	if (in_parallel == 0) {
		predict(784, 200, 10, num_boxes, all_features, wt1, wt2, pred_results);
	}
	else {
		predict_parallel(784, 200, 10, num_boxes, all_features, wt1, wt2, pred_results);
	}

	final_predictions = calloc(num_boxes, sizeof(int));

	for (current_idx = 0; current_idx < num_boxes; current_idx++) {
		max_value = 0;
		max_pred = 0;
		for (i = 0; i < 10; i++) {
			if (pred_results[current_idx*10 + i] > max_value) {
				max_pred = i;
				max_value = pred_results[current_idx*10 + i];
			}
		}
		final_predictions[current_idx] = max_pred;
		LOGI("final prediction is %d for box at (l: %d, t: %d)", max_pred, boxes[current_idx*4+3],
				boxes[current_idx*4]);
	}

	AndroidBitmap_unlockPixels(env, bitmap);
	return (*env)->NewStringUTF(env, "what");
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

void predict(int input_size, int hidden_size, int output_size, int num_ex,
		double* value, double* wt_1, double* wt_2, double* results)
{
	// X * wt_1
	double *z2 = calloc(num_ex*hidden_size, sizeof(double));

	if (z2 == NULL) {
		return;
	}
	matmul(num_ex, input_size, hidden_size, value, wt_1, z2);

	// tanh(z2)
	double *a2 = calloc(num_ex*hidden_size, sizeof(double));
	vtanh(num_ex*hidden_size, z2, a2);

	// a2 * wt_2
	double *z3 = calloc(num_ex*output_size, sizeof(double));
	matmul(num_ex, hidden_size, output_size, a2, wt_2, z3);

	// sigmoid(z3)
	sigmoid(num_ex*output_size, z3, results);
}

void predict_parallel(int input_size, int hidden_size, int output_size, int num_ex,
		double* value, double* wt_1, double* wt_2, double* results)
{
	// X * wt_1
	double *z2 = calloc(num_ex*hidden_size, sizeof(double));

	if (z2 == NULL) {
		return;
	}
	matmul_parallel(num_ex, input_size, hidden_size, value, wt_1, z2);

	// tanh(z2)
	double *a2 = calloc(num_ex*hidden_size, sizeof(double));
	vtanh_parallel(num_ex*hidden_size, z2, a2);

	// a2 * wt_2
	double *z3 = calloc(num_ex*output_size, sizeof(double));
	matmul_parallel(num_ex, hidden_size, output_size, a2, wt_2, z3);

	// sigmoid(z3)
	sigmoid_parallel(num_ex*output_size, z3, results);
}


void matmul(int Ar, int Ac, int Bc, double* A, double* B, double* C)
{
	int i;
	int j;
	int k;
	double cij;

	for (i = 0; i < Ar; i++) {
		for (j = 0; j < Bc; j++) {
			cij = C[i+j*Ar];
			for(k = 0; k < Ac; k++) {
				cij += A[i+k*Ar] * B[k+j*Ac];
				C[i+j*Ar] = cij;
			}
		}
	}
}

void matmul_parallel(int Ar, int Ac, int Bc, double* A, double* B, double* C)
{
	int i;
	int j;
	int k;
	double cij;

	#pragma omp parallel for
	for (i = 0; i < Ar; i++) {
		for (j = 0; j < Bc; j++) {
			cij = C[i+j*Ar];
			for(k = 0; k < Ac; k++) {
				cij += A[i+k*Ar] * B[k+j*Ac];
				C[i+j*Ar] = cij;
			}
		}
	}
}

void vtanh(int c, double* A, double* B)
{
	int i;
	for (i = 0; i < c; i++)
	{
		B[i] = tanh(A[i]);
	}
}

void vtanh_parallel(int c, double* A, double* B)
{
	int i;

	#pragma omp parallel for
	for (i = 0; i < c; i++)
	{
		B[i] = tanh(A[i]);
	}
}

void sigmoid(int c, double* A, double* B)
{
	int i;
	for (i = 0; i < c; i++)
	{
		B[i] = 1.0/(1.0 + exp(-1*A[i]));
	}
}

void sigmoid_parallel(int c, double* A, double* B)
{
	int i;

	#pragma omp parallel for
	for (i = 0; i < c; i++)
	{
		B[i] = 1.0/(1.0 + exp(-1*A[i]));
	}
}
