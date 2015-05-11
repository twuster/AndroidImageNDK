#include <omp.h>

int main() {
  #pragma omp parallel
  {
    printf("Hello from thread %d!\n", omp_get_thread_num());
  }
  return 0;
}

void Java_com_example_hellojni_HelloJni_digitRecognition(int* image, double* all_features)
{
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
  int num_boxes;
  int boxes_idx;
  int box_height;
  int box_width;

  double* block_height;
  double* block_width;

  int by;
  int bx;

  int explored;
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

  int box_height;
  int box_width;

  double* feature;

  // Get edge detected version
  // Either copy and paste or call the fn?

  // Get bounding boxes
  points_buf = calloc(height*width*2, sizeof(int));
  points_check = calloc(height*width, sizeof(int));

  points_buf_idx = 0;
  for (y = 0; y < height; y++) {
    for (x = 0; x < height; x++) {
      if (image[y*width + x] > 128) {
        points_buf[points_buf_idx] = y;
        points_buf[points_buf_idx+1] = x;
        points_buf_idx++;
        num_points++;
        points_check[y*width + x] = 1;
      }
    }
  }

  explored = calloc(height*width, sizeof(int));
  num_explored = 0;

  current_idx = 0;
  boxes = calloc(height*width*4, sizeof(int));
  boxes_idx = 0;
  num_boxes = 0;
  while (num_explored < num_points) {
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
    fringe_idx = 0;
    fringe[fringe_idx] = pt_y;
    fringe[fringe_idx+1] = pt_x;
    fringe_idx += 2;
    num_fringe = 1;

    explored[pt_y*width + pt_x] = 1;
    while (num_fringe > 0) {
      fringe_idx -= 2;
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

          if (explored[check_pt_y*width + check_pt_x] != 1 &&
              points_check[check_pt_y*width + check_pt_x] == 1) {
            fringe[fringe_idx] = check_pt_y;
            fringe[fringe_idx+1] = check_pt_x;
            fringe_idx += 2;

            explored[check_pt_y*width + check_pt_x] = 0;
          }
        }
      }
    }

    boxes[boxes_idx] = top;
    boxes[boxes_idx+1] = right;
    boxes[boxes_idx+2] = bottom;
    boxes[boxes_idx+3] = left;

    boxes_idx += 4;
    num_boxes++;
    current_idx += 2;
  }
  //return boxes;

  // Get features from boxes
  all_features = calloc(28*28*num_boxes, sizeof(double));
  for (current_idx = 0; current_idx < num_boxes; current_idx += 4) {
    top = boxes[current_idx];
    right = boxes[current_idx+1];
    bottom = boxes[current_idx+2];
    left = boxes[current_idx+3];

    box_height = bottom - top;
    box_width = right - left;

    if (box_height < 28 || box_width < 28) {
      continue;
    }

    block_height = box_height / 28.0;
    block_width = box_width / 28.0;

    feature = all_features + (current_idx*28*28);
    for (y = 0; y < 28; y++) {
      for (x = 0; x < 28; x++) {
        // Get average of area
        for (by = 0; by < (int)ceil(block_height); by++) {
          for (bx = 0; bx < (int)ceil(block_width); bx++) {
            if (by < (int)block_height && by < (int)block_width) {
              feature[y*28 + x] = image[(top+y)*width + (left+x) + by*width + bx]/block_height/block_width;
            }
            else if (by >= (int)block_heighti && by+y < block_height) {
              feature[y*28 + x] += image[(top+y)*width+(left+x)+by*width+bx]*
                                   (block_height-(int)block_height)/block_height/block_width;
            }
            else if (bx >= (int)block_width && bx+x < block_width) {
              feature[y*28 + x] += image[(top+y)*width+(left+x)+by*width+bx]*
                                   (block_width - (int)block_width)/block_height/block_width;
                                   block_height/block_width;
            }
          }
        }
        // end get average
      }
    }
  }
}   
