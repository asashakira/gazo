/****************************************************************
 *
 *   Affine Transform
 *
 *   writer  :	M.Mukunoki
 *   date    :	2016/12/08
 *   compile :
 *    gcc -O3 -std=gnu99 -I. affine.c kumi3.c -o affine -lm
 *   execute :
 *    ./affine -a -2 0 0 2 512 0 -s 512 512 sample01.pgm out01.pgm
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <getopt.h>

#include <kumi3.h>

/**** Defines ****/

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

// affineパラメータを保持する構造体
typedef struct _affine_param
{
  // x' = a x + b y + e
  // y' = c x + d y + f
  double      a;
  double      b;
  double      c;
  double      d;
  double      e;
  double      f;
} affine_param;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int affine(K_IMAGE *inp_img, K_IMAGE *out_img, affine_param *param);

/**** Routines ****/
int main(int argc, char *argv[]) {
  K_IMAGE	*inp_img, *out_img;
  K_HEAD	header;
  char	*inp_fname	= K_STDFILE;
  char	*out_fname	= K_STDFILE;
  affine_param	param = {1., 0., 0., 1., 0., 0.};
  int		out_xsize = -1;
  int		out_ysize = -1;

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  // 引数の処理
  while((c = getopt(argc, argv, "ashi:o:P")) != EOF) {
    switch(c) {
      case 'i':
        inp_fname = optarg;
        break;
      case 'o':
        out_fname = optarg;
        break;
      case 'a':
        param.a = atof(argv[optind++]);
        param.b = atof(argv[optind++]);
        param.c = atof(argv[optind++]);
        param.d = atof(argv[optind++]);
        param.e = atof(argv[optind++]);
        param.f = atof(argv[optind++]);
        break;
      case 's':
        out_xsize = atoi(argv[optind++]);
        out_ysize = atoi(argv[optind++]);
      case 'P':
        out_fname = K_STDPNM;
        break;
      case 'h':
        errflg++;
        break;
      default:
        errflg++;
        break;
    }
    if (errflg) {
      usage(argv[0]);
      exit(1);
    }
  }/* end of while */
  if (optind < argc) {
    inp_fname = argv[optind++];
  }
  if (optind < argc) {
    out_fname = argv[optind++];
  }
  if (optind != argc) {
    usage(argv[0]);
    exit(1);
  }

  // 入力画像の読み込み
  if ((inp_img = k_open(inp_fname)) == NULL) {
    return(-1);
  }
  if (k_pixeltype(inp_img) != K_UCHAR) {
    fprintf(stderr, "Pixeltype of input image must be K_UCHAR!!\n");
    exit(1);
  }

  // 出力画像の確保
  header = *k_header(inp_img);
  if (out_xsize > 0) {
    header.xsize = out_xsize;
  }
  if (out_ysize > 0) {
    header.ysize = out_ysize;
  }

  if ((out_img = k_create(&header)) == NULL) {
    return(-1);
  }

  // アフィン変換
  affine(inp_img, out_img, &param);

  // 出力画像の保存
  k_write(out_img, out_fname);

  k_close(inp_img);
  k_close(out_img);

  return 0;
}

int usage(char	*command) {
  fprintf(stderr, "Copy Image.\n");
  fprintf(stderr, "Usage: %s [-hP] [-a <affine parameters(6)>] ", command);
  fprintf(stderr, "[-s out_xsize out_ysize] ");
  fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -P : output with NETPBM format\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
  fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

  return(0);
}

int affine(K_IMAGE *inp_img, K_IMAGE *out_img, affine_param *param) {
  // 行列式
  double	det = param->a * param->d - param->b * param->c;
  double a = param->a, b = param->b, c = param->c, d = param->d, e = param->e, f = param->f;

  for(int m = 0; m < k_multi(inp_img); m++) {
    uchar	**iptr = (uchar **)k_data(inp_img)[m];
    uchar	**optr = (uchar **)k_data(out_img)[m];

    for(int oy = 0; oy < k_ysize(out_img); oy++) {
      for(int ox = 0; ox < k_xsize(out_img); ox++) {
        // 逆変換
        double x = (d*(ox-e) - b*(oy-f)) / det;
        double y = (-c*(ox-e) + a*(oy-f)) / det;
        int ix = (int)floor(x); // 対応する入力画像のx座標
        int iy = (int)floor(y); // 対応する入力画像のy座標
        double	s = x - ix; // x方向の内分比
        double	t = y - iy; // y方向の内分比

        if (ix < 0 || ix >= k_xsize(inp_img)-1 || iy < 0 || iy >= k_ysize(inp_img)-1) continue;

        // 共一次補間
        int val = (1-t)*(1-s)*iptr[iy][ix] + t*(1-s)*iptr[iy+1][ix] + (1-t)*s*iptr[iy][ix+1] + t*s*iptr[iy+1][ix+1];
        //int val = iptr[iy][ix];

        if (val < 0) {
          val = 0;
        }
        if (val > 255) {
          val = 255;
        }

        optr[oy][ox] = val;
      }
    }
  }

  return 0;
}
