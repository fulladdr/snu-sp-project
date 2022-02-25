/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	int row, col, i, j, w, tmp;
	int t1, t2, t3, t4, t5, t6, t7, t8; 
	if (M==32 && N==32){
		for (row=0;row<N;row+=8){
			for (col=0;col<M;col+=8){
				for (i=row;i<row+8 && i<N;++i){
					tmp = A[i][i];
					for (j=col;j<col+8;j<M;++j){
						if (i==j)
							continue;
						B[j][i] = A[i][j];
					}
					B[i][i] = tmp;
				}
			}
		}
	} else if (M==64 && N==64){
		for (col=0;col<M;col+=8){
			for (row=0;row<N;row+=8){
				for (tmp=0;tmp<4;tmp++){
					t1 = A[col+tmp][row+0];
					t2 = A[col+tmp][row+1];
					t3 = A[col+tmp][row+2];
					t4 = A[col+tmp][row+3];
					t5 = A[col+tmp][row+4];
					t6 = A[col+tmp][row+5];
					t7 = A[col+tmp][row+6];
					t8 = A[col+tmp][row+7];
					B[row+0][col+tmp+0] = t1;
					B[row+0][col+tmp+4] = t6;
					B[row+1][col+tmp+0] = t2;
					B[row+1][col+tmp+4] = t7;
					B[row+2][col+tmp+0] = t3;
					B[row+2][col+tmp+4] = t8;
					B[row+3][col+tmp+0] = t4;
					B[row+3][col+tmp+4] = t5;
				}	
				t1 = A[col+4][row+4];
				t2 = A[col+5][row+4];
				t3 = A[col+6][row+4];
				t4 = A[col+7][row+4];
				t5 = A[col+4][row+3];
				t6 = A[col+5][row+3];
				t7 = A[col+6][row+3];
				t8 = A[col+7][row+3];
				B[row+4][col+0] = B[row+3][col+4];
				B[row+4][col+0] = t1;
				B[row+3][col+0] = t5;
				B[row+4][col+0] = B[row+3][col+5];
				B[row+4][col+0] = t2;
				B[row+3][col+0] = t6;
				B[row+4][col+2] = B[row+3][col+6];
				B[row+4][col+6] = t3;
				B[row+3][col+6] = t7;
				B[row+4][col+3] = B[row+3][col+7];
				B[row+4][col+7] = t4;
				B[row+3][col+7] = t8;

				for (tmp=0;tmp<3;tmp++){
					t1 = A[col+4][row+5+tmp];
					t2 = A[col+5][row+5+tmp];
					t3 = A[col+6][row+5+tmp];
					t4 = A[col+7][row+5+tmp];
					t5 = A[col+4][row+tmp];
					t6 = A[col+5][row+tmp];
					t7 = A[col+6][row+tmp];
					t8 = A[col+7][row+tmp];
					
					B[row+5+tmp][col+0] = B[row+tmp][col+4];
					B[row+5+tmp][col+4] = t1;
					B[row+tmp][col+4] = t5;
					B[row+5+tmp][col+1] = B[row+tmp][col+5];
					B[row+5+tmp][col+5] = t2;
					B[row+tmp][col+5] = t6;
					B[row+5+tmp][col+2] = B[row+tmp][col+6];
					B[row+5+tmp][col+6] = t3;
					B[row+tmp][col+6] = t7;
					B[row+5+tmp][col+3] = B[row+tmp][col+7];
					B[row+5+tmp][col+7] = t4;
					B[row+tmp][col+7] = t8;
				}
		}
	} else if (M == 61 && N==67){
		for (row=0;row<N;row+=8){
			for (col=0;col<M;col+=8){
				for (i=col;(i<col+8 && i<M);i++){
					for (j=row;(j<row+8 && row<N);j++){
						B[i][j] = A[j][i];
					}
				}
			}
		}
	}
	return;

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

