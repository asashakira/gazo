/****************************************************************
 *
 *   Quantize DCT result with quantization table
 *
 *   date   : 2019/11/13
 *
 *   compile : gcc -O3 -std=gnu99 quantize-sample.c -o quantize -lm
 *   execute : ./quantize
 *   result  :
 *		 23  -3   3  -1   2   0  -1   1 
 *		  4  -6   4  -1  -1   0   0   0 
 *		 -1  -3   0   1  -1   0  -1   0 
 *		 -1  -1   1  -1   0   0   0   0 
 *		  0   1   0   0   0   0   0   0 
 *		  0   0  -1   0   0   0   0   0 
 *		  0   0   0   0   0   0   0   0 
 *		  0   0   0   0   0   0   0   0 
 *
 ****************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/**** Defines ****/
#define N	8

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int quantize(int dct_amp[][N ], int quantize_tbl[][N ], int quantize_result[][N ]);

/****************************************************************/
int main(int argc, char *argv[]) {
  // DCTで得られた値（量子化の入力）
  int	dct_amp[N][N] = {
    {186 ,   -18  ,  15  ,   -9  ,    23  ,    -9  ,    -14  ,   19 },
    {21  ,   -34  ,  26  ,   -9  ,    -11 ,    11  ,    14   ,   7  },
    {-10 ,   -24  ,  -2  ,   6   ,    -18 ,    3   ,    -20  ,   -1 },
    {-8  ,   -5   ,  14  ,   -15 ,    -8  ,    -3  ,    -3   ,   8  },
    {-3  ,   10   ,  8   ,   1   ,    -11 ,    18  ,    18   ,   15 },
    {4   ,   -2   ,  -18 ,   8   ,    8   ,    -4  ,    1    ,   -7 },
    {9   ,   1    ,  -3  ,   4   ,    -1  ,    -7  ,    -1   ,   -2 },
    {0   ,   -8   ,  -2  ,   2   ,    1   ,    4   ,    -6   ,   0  }
  };

  // 量子化テーブル
  int	quantize_tbl[N][N] = {
    {8   ,   6    ,  5   ,   8   ,    12  ,    20  ,    26   ,   30},
    {6   ,   6    ,  7   ,   10  ,    13  ,    29  ,    30   ,   28},
    {7   ,   7    ,  8   ,   12  ,    20  ,    29  ,    35   ,   28},
    {7   ,   9    ,  11  ,   15  ,    26  ,    44  ,    40   ,   31},
    {9   ,   11   ,  19  ,   28  ,    34  ,    55  ,    52   ,   39},
    {12  ,   18   ,  28  ,   32  ,    41  ,    52  ,    57   ,   46},
    {25  ,   32   ,  39  ,   44  ,    52  ,    61  ,    60   ,   51},
    {36  ,   46   ,  48  ,   49  ,    56  ,    50  ,    52   ,   50},
  };

  // 量子化結果
  int quantize_result[N][N];

  // 量子化処理
  quantize(dct_amp, quantize_tbl, quantize_result);

  // 結果の表示
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      printf("%3d ", quantize_result[i][j]);
    }
    printf("\n");
  }

  return 0;
}

int quantize(int dct_amp[][N], int quantize_tbl[][N], int quantize_result[][N]) {
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      quantize_result[i][j] = round((float)dct_amp[i][j] / quantize_tbl[i][j]);
    }
  }
  return 0;
}
