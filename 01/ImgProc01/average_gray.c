/****************************************************************
 *
 *   Calculate Average Pixel Value for Gray Image
 *
 *   filename	:	average_gray.c
 *   writer	:	M.Mukunoki
 *   date	:	2016/08/25
 *
 *   compile     :
 *    gcc -I. average_gray.c kumi3.c -o average_gray
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <kumi3.h>

/**** Defines ****/

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int average_gray(K_IMAGE *img, float *ave);

/**** Routines ****/
int main(int argc, char *argv[]) {
  K_IMAGE	*inp_img;
  char	*inp_fname	= K_STDFILE;

  int		c;
  int		errflg = 0;
  float	ave;

  extern int	optind;
  extern char	*optarg;
  
  printf("%d\n", optind);
  while((c = getopt(argc, argv, "i:h")) != EOF) {
    switch(c)
    {
      case 'i':
        inp_fname = optarg;
        break;
      case 'h':
        errflg++;
        break;
      default:
        errflg++;
        break;
    }
    if (errflg)
    {
      usage(argv[0]);
      exit(1);
    }
  }/* end of while */

  if (optind < argc) {
    inp_fname = argv[optind++];
  }
  if (optind != argc)
  {
    usage(argv[0]);
    exit(1);
  }

  if ((inp_img = k_open(inp_fname)) == NULL)
  {
    return(-1);
  }

  if (k_pixeltype(inp_img) != K_UCHAR)
  {
    fprintf(stderr, "Pixeltype of input image must be K_UCHAR!!\n");
    exit(1);
  }

  average_gray(inp_img, &ave);
  printf("%f\n", ave);

  k_close(inp_img);

  exit(0);
}

int usage(char	*command)
{
  fprintf(stderr, "Calculate Average Pixel Value for Gray Image.\n");
  fprintf(stderr, "Usage: %s [-h] ", command);
  fprintf(stderr, "[[-i] input_file\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");

  return(0);
}

int average_gray(K_IMAGE *img, float *ave) {
  int	sum = 0;
  uchar	**iptr = (uchar **)(k_data(img)[0]);

  for(int y = 0; y < k_ysize(img); y++) {
    for(int x = 0; x < k_xsize(img); x++) {
      sum += iptr[y][x];
    }
    printf("\n");
  }
  *ave = (float)sum/(k_xsize(img) * k_ysize(img));

  return 0;
}
