#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void matmul(int, int, int, double*, double*, double*);
void vtanh(int, double*, double*);
void sigmoid(int, double*, double*);
void predict(int, int, int, int, double*, double*, double*, double*);
/*
void matmul_parallel(int, int, int, double*, double*, double*);
void vtanh_parallel(int, double*, double*);
void sigmoid_parallel(int, double*, double*);
void predict_parallel(int, int, int, int, double*, double*, double*, double*);
*/
int main()
{
  int ar;
  int ac;
  int bc;
  int i;
  int j;

  double A[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
  double B[9] = {.1, .2, .3, .4, .5, .6, .7, .8, .9};
  double *C = calloc(12, sizeof(double));

  double D[12] = {1.2, 3.2, .52, .45, -.52, -.16, .57, .38, -.29, .16, .22, .43};
  double *D_tanh = calloc(12, sizeof(double));
  double *D_sig = calloc(12, sizeof(double));
  ar = 3;
  ac = 3;
  bc = 4;

  matmul(ar, ac, bc, A, D, C);
  vtanh(12, D, D_tanh);
  sigmoid(12, D, D_sig);

  for (i = 0; i < ar; i++) {
    for (j = 0; j < bc; j++) {
      printf("%f ", C[i + ar * j]);
    }
    printf("\n");
  }
  for (i = 0; i < ar; i++) {
    for (j = 0; j < bc; j++) {
      printf("%f ", D_tanh[i + ar * j]);
    }
    printf("\n");
  }
  for (i = 0; i < ar; i++) {
    for (j = 0; j < bc; j++) {
      printf("%f ", D_sig[i + ar * j]);
    }
    printf("\n");
  }

  double *R = calloc(9, sizeof(double));
  predict(3, 3, 3, 3, A, B, B, R);


  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      printf("%f ", R[i + ar * j]);
    }
    printf("\n");
  }

  return 0;
}
/*
void predict_parallel(int input_size, int hidden_size, int output_size, int num_ex,
             double* value, double* wt_1, double* wt_2, double* results)
{
  // X * wt_1
  double *z2 = calloc(num_ex*hidden_size, sizeof(double));
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
*/
void predict(int input_size, int hidden_size, int output_size, int num_ex,
             double* value, double* wt_1, double* wt_2, double* results)
{
  // X * wt_1
  double *z2 = calloc(num_ex*hidden_size, sizeof(double));
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
