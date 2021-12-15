/****************************************************************
 *
 *   Convert Fourier Image to Power Image
 *
 *   date   : 2016/12/1
 *
 *   compile : gcc -I. -std=gnu11 ftpower.c kumi3.c -o ftpower -lm
 *   execute : ./ftpower dft01.kumi power01.kumi
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <kumi3.h>

/**** Defines ****/
#ifndef M_PI
#define	M_PI		3.1415927
#endif

#define	DEF_SCALE	(-1.0)

/**** Macros ****/
#define sqr(x)	((x)*(x))

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
void ftpower(K_IMAGE *iimg, K_IMAGE *oimg, float scale);

/****************************************************************/
int main(int argc, char *argv[])
{
  K_IMAGE	*iimg, *oimg;
  K_HEAD	header;
  char	*iimg_fname	= K_STDFILE;
  char	*oimg_fname	= K_STDFILE;
  float	scale = DEF_SCALE;

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "ri:o:s:Ph")) != EOF)
  {
    switch(c)
    {
      case 'i':
        iimg_fname = optarg;
        break;
      case 'o':
        oimg_fname = optarg;
        break;
      case 's':
        scale = atof(optarg);
        break;
      case 'P':
        oimg_fname = K_STDPNM;
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
  if (optind < argc)
  {
    iimg_fname = argv[optind++];
  }
  if (optind < argc)
  {
    oimg_fname = argv[optind++];
  }
  if (optind != argc)
  {
    usage(argv[0]);
    exit(1);
  }

  if ((iimg = k_open(iimg_fname)) == NULL) return(-1);
  if (k_pixeltype(iimg) != K_FLOAT || k_multi(iimg) != 2)
  {
    fprintf(stderr, "Input image must be COMPLEX image!!\n");
    exit(1);
  }
  header = *k_header(iimg);
  header.multi = 1;
  header.pixeltype = K_UCHAR;
  header.ebit  = 8;
  if ((oimg = k_create(&header)) == NULL) return(-1);

  ftpower(iimg, oimg, scale);

  if (k_write(oimg, oimg_fname)) return(-1);
  k_close(oimg);
  k_close(iimg);

  return 0;
}


int usage(char	*command)
{
  fprintf(stderr, "Convert Fourier Image to Power Image.\n");
  fprintf(stderr, "Usage: %s [-hP] [-s scale] ", command);
  fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
  fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

  return(0);
}

void ftpower(K_IMAGE *iimg, K_IMAGE *oimg, float scale)
{
  int	M = k_xsize(iimg);
  int	N = k_ysize(iimg);

  float ***iptr = (float ***)k_data(iimg);
  uchar **optr  = (uchar **)k_data(oimg)[0];

  if (scale < 0.0)
  {
    float max_val = 0.0;
    for(int y = 0; y < N; y++)
    {
      for(int x = 0; x < M; x++)
      {
        float d = sqr(iptr[0][y][x])+sqr(iptr[1][y][x]);
        d = log(d+1.0);
        if (max_val < d)
        {
          max_val = d;
        }
      }
    }
    scale = 255.0 / max_val;
  }

  for(int y = 0; y < N; y++)
  {
    for(int x = 0; x < M; x++)
    {
      float d = sqr(iptr[0][y][x])+sqr(iptr[1][y][x]);
      d = log(d+1.0) * scale;

      // shuffle
      optr[(y+N/2)%N][(x+M/2)%M] = ku_crop_uchar((int)d);
    }
  }

}
