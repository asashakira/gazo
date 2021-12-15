/****************************************************************
 *
 *   Run-Length Compression
 *
 *   date   : 2019/11/20
 *
 *   compile : gcc -O3 -funroll-loops -I. runlength-sample.c kumi3.c -o runlength -lm
 *   execute : ./runlength sample03.pbm
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <kumi3.h>

/**** Defines ****/

/**** Macros ****/
#define out_len(flg, len) printf("%s(%d) ", (flg==0)?"白":"黒", len)

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
void runlength(K_IMAGE *img);

/****************************************************************/
int main(int argc, char *argv[]) {
  K_IMAGE	*inp_img;
  char	*inp_fname	= K_STDFILE;

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "h")) != EOF) {
    switch(c) {
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
  if (optind != argc) {
    usage(argv[0]);
    exit(1);
  }

  if ((inp_img = k_open(inp_fname)) == NULL) return(-1);
  if (k_pixeltype(inp_img) != K_BIT) {
    fprintf(stderr, "Input image must be BINARY image!!\n");
    exit(1);
  }

  runlength(inp_img);

  k_close(inp_img);

  return 0;
}


int usage(char	*command) {
  fprintf(stderr, "RunLength Compression\n");
  fprintf(stderr, "Usage: %s [-h] ", command);
  fprintf(stderr, "<input_file>\n");
  fprintf(stderr, "\t -h : help\n");

  return(0);
}

void runlength(K_IMAGE *img) {
  int	xsize = k_xsize(img);
  int	ysize = k_ysize(img);

  uchar **iptr = (uchar **)k_data(img)[0];

  for(int y = 0; y < ysize; y++) {
    int flg = iptr[y][0];
    int len = 1;
    for(int x = 1; x < xsize; x++) {
      if (iptr[y][x] != flg) {
        // 画素値が変化した場合
        out_len(flg, len);
        flg = iptr[y][x];
        len = 1;
      } else {
        // 同じ値が続く場合
        len++;
      }
    }
    // 行の区切りを出力
    out_len(flg, len); printf("\n"); 
  }
}
