/**********************************************************
*							  *
*      Kyoto University Multi Images Handling System	  *
*							  *
*      file I/O routines				  *
*							  *
*   Written by Izuru Takaya, Thurs, June 29th, 1989	  *
*   Modified by Michihiko Minoh, Fri, Sept. 1st, 1989     *
*   Modified by Masayuki Mukunoki, Thu, Jun. 21, 1990	  *
*   Modified by Masayuki Mukunoki, 2007/08/16        	  *
*     TODO: テストが不十分                                *
*							  *
*   Copy right by Kyoto University, 1989		  *
*							  *
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kumi3.h"

typedef unsigned char uchar;

// 外部コマンドと拡張子との対応(最後は NULL)
struct {
    char	*ext;        // 拡張子
    char	*inpCmd;     // 入力用外部コマンド
    char	*outCmd;     // 出力用外部コマンド
} extcmd[] = {{".jpg", "djpeg", "cjpeg"},
	      {".png", "pngtopnm", "pnmtopng"},
	      {".gif", "giftopnm", "pnmtogif"},
	      {".gz",  "zcat", "gzip -c"},
	      {NULL, NULL, NULL}};

// pnm 形式を使うファイルの拡張子(最後は NULL)
char *pnmext[] = {".jpg", ".png", ".gif", ".pnm", ".ppm", ".pgm", ".pbm", NULL};

int _k_debug  = 1;   // debug mode

#define	MAXLEN	2050  // popen に渡すコマンドの最大長

/****************************************************************/
/**** Prototype Declaration ****/
K_IMAGE *fio_allocImage(void);
int fio_freeImage(K_IMAGE *image);
int fio_allocBody(K_IMAGE *image);
int fio_freeBody(K_IMAGE *image);
int fio_setupParam(K_IMAGE *image);
int fio_kumiReadHeader(FILE *fp, K_IMAGE *image, char *magic);
int fio_kumiWriteHeader(FILE *fp, K_IMAGE *image);
int fio_pnmReadHeader(FILE *fp, K_IMAGE *image, char *magic);
int fio_pnmWriteHeader(FILE *fp, K_IMAGE *image);
static void _skipSP(FILE *fp, int skip_comment);
static int _getVal(FILE *fp);

/*********************************************************
*   k_open
*
*    read image from file to buffer
*
**********************************************************/
K_IMAGE *k_open( char *filename )
{
    FILE	*fp;
    K_IMAGE	*image;
    char	*cmd;
    char	pCommand[MAXLEN];
    int		i;
    int		x, y, m;

    char	magic[10];

    // 構造体の領域を確保
    if ((image = fio_allocImage())==NULL) return(NULL);

    // 拡張子に応じたコマンド選択
    cmd = NULL;
    for(i = 0; extcmd[i].ext != NULL; i++)
    {
	if ( strstr(filename, extcmd[i].ext) != NULL)
	{
	    cmd = extcmd[i].inpCmd;
	    break;
	}
    }

    //
    // ファイルの open
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	fp = stdin;
    }else if (cmd != NULL){
	snprintf( pCommand, MAXLEN, "%s < %s", cmd, filename);
	fp = popen(pCommand, "r");
    }else{
	fp = fopen(filename, "r");
    }
    if (fp == NULL)
    {
	k_perror("Error: Can't open file for read(%s)!!\n", filename);
	return(NULL);
    }

    //
    // ヘッダの読み込み
    //  magic の読み込みと判定
    //
    int dummy = fread(magic, 2, 1, fp);
    if (magic[0] == 'P' && magic[1] >= '4' && magic[1] <= '6')
    {
	if (fio_pnmReadHeader(fp, image, magic)) return(NULL);
    }else if (magic[0] == K_MAGIC[0] && magic[1] == K_MAGIC[1]){
	if (fio_kumiReadHeader(fp, image, magic)) return(NULL);
    }else{
	k_perror("Error: Unknown Magic(Not supported image / Can't find file)\n");
	return( NULL );
    }

    // 各種パラメータを事前に計算
    if (fio_setupParam(image)) return(NULL);
    
    // 画像本体のデータ領域を確保
    if (fio_allocBody(image)) return(NULL);

    //
    // 本体の読み込み
    //
    if (k_pixeltype(image) == K_BIT)
    {
	// 8画素が 1byte にパックされている
	// 8の倍数になるように領域確保が必要 (fio_allocBody)
	// 左から順に詰め込む
	int		v, val;

	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x+= 8)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    val = fgetc(fp);
		    for(i = 0; i < 8; i++)
		    {
			v = ((val & 0x80) != 0)? 1: 0;
			((uchar ***)k_data(image))[m][y][x+i] = v;
			val <<= 1;
		    }
		}
	    }
	}
    }else if (k_pixeltype(image) == K_UCHAR){
	// K_UCHAR はよく使うので，特別に場合分け
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    int dummy = fread(buff, lineBytes, 1, fp);
	    for(m = 0; m < k_multi(image); m++)
	    {
		dataPtr = (image->data)[m][y];
		boffs = m * k_pixelbyte(image);
		for(x = 0; x < k_xsize(image); x++)
		{
		    *(uchar *)(dataPtr + x * k_pixelbyte(image)) = buff[boffs];
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	}
	free(buff);
    }else{
	// ファイル上は multi 優先で並んでいるが
	// 読み込み後は m, y, x の順
#if 0
	// 素直な実装だけど遅い
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x++)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    int dummy = fread((image->data)[m][y]+x*k_pixelbyte(image), 
			  k_pixelbyte(image), 1, fp);
		}
	    }
	}
#endif
	// 多少高速化
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    int dummy = fread(buff, lineBytes, 1, fp);
	    for(m = 0; m < k_multi(image); m++)
	    {
		boffs = m * k_pixelbyte(image);
		dataPtr = (image->data)[m][y];
		for(x = 0; x < k_xsize(image); x++)
		{
		    memcpy(dataPtr + x * k_pixelbyte(image), 
			   buff + boffs, k_pixelbyte(image));
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	}
	free(buff);
    }
      
    //
    // ファイルの close
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	; // Do Nothing 
    }else if (cmd != NULL){
	pclose(fp);
    }else{
	fclose(fp);
    }

    return( image );
}
/*********************************************************
*
*   k_create
*
*    allocate new image header area and copy it.
*    Note: This routine does not copy actual image bodies.
*
**********************************************************/
K_IMAGE *k_create(K_HEAD *header)
{
    K_IMAGE *image;

    // 構造体の領域を確保
    if ((image = fio_allocImage()) == NULL) return( NULL );

    // ヘッダの内容をコピー
    *k_header(image) = *header;

    // 各種パラメータを事前に計算
    if (fio_setupParam(image)) return(NULL);

    // 画像本体のデータ領域を確保
    if (fio_allocBody(image)) return(NULL);

    return( image );
}
/*********************************************************
*
*   k_write
*
*   save an image in the buffer to a file  
*
**********************************************************/
int k_write(K_IMAGE *image, char *filename)
{
    FILE	*fp;
    char	*cmd;
    char	pCommand[MAXLEN];
    int		pnmFlag;
    int		i;
    int		x, y, m;

    // 拡張子に応じたコマンド選択
    cmd = NULL;
    for(i = 0; extcmd[i].ext != NULL; i++)
    {
	if ( strstr(filename, extcmd[i].ext) != NULL)
	{
	    cmd = extcmd[i].outCmd;
	    break;
	}
    }

    //
    // ファイルの open
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	fp = stdout;
    }else if (cmd != NULL){
	snprintf( pCommand, MAXLEN, "%s > %s", cmd, filename);
	fp = popen(pCommand, "w");
    }else{
	fp = fopen(filename, "w");
    }
    if (fp == NULL)
    {
	k_perror("Error: Can't create file for write (%s)!!\n", filename);
	return(-1);
    }
    
    //
    // ヘッダの書き込み
    //

    // PNM形式か判定
    pnmFlag = 0;
    for(i = 0; pnmext[i] != NULL; i++)
    {
	if (strstr(filename, pnmext[i]) != NULL)
	{
	    pnmFlag = 1;
	    break;
	}
    }

    // それぞれの形式でヘッダを書き込み
    if (pnmFlag == 1)
    {
	if (fio_pnmWriteHeader(fp, image)) return(-1);
    }else{
	if (fio_kumiWriteHeader(fp, image)) return(-1);
    }

    //
    // 本体の出力
    //
    if (k_pixeltype(image) == K_BIT)
    {   // K_BIT の場合は8画素毎に 1byte にパックする
	int	val, v;

	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x+=8)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    val = 0;
		    for(i = 0; i < 8; i++)
		    {
			val <<= 1;
			v = ((uchar ***)k_data(image))[m][y][x+i];
			val |= (v & 0x01);
		    }
		    fputc(val, fp);
		}
	    }
	}
    }else if (k_pixeltype(image) == K_UCHAR){
	// K_UCHAR はよく使うので，特別に場合分け
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(m = 0; m < k_multi(image); m++)
	    {
		dataPtr = (image->data)[m][y];
		boffs = m * k_pixelbyte(image);
		for(x = 0; x < k_xsize(image); x++)
		{
		    buff[boffs] = *(uchar *)(dataPtr + x * k_pixelbyte(image));
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	    fwrite(buff, lineBytes, 1, fp);
	}
	free(buff);

    }else{
#if 0
	// multi 毎に分かれているメモリ上の画像データを
	// multi 優先でファイルに出力
	// 素直な実装だけど遅い
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x++)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    fwrite((image->data)[m][y]+x*k_pixelbyte(image), 
			   k_pixelbyte(image), 1, fp);
		}
	    }
	}
#endif
	// 多少高速化
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(m = 0; m < k_multi(image); m++)
	    {
		boffs = m * k_pixelbyte(image);
		dataPtr = (image->data)[m][y];
		for(x = 0; x < k_xsize(image); x++)
		{
		    memcpy(buff + boffs,
			   dataPtr + x * k_pixelbyte(image), 
			   k_pixelbyte(image));
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	    fwrite(buff, lineBytes, 1, fp);
	}
	free(buff);
    }

    //
    // ファイルの close
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	; // Do Nothing
    }else if (cmd != NULL){
	pclose(fp);
    }else{
	fclose(fp);
    }

    return( 0 );
}
/**********************************************************
*
*   k_close
*
*    free image buffer
*    Note: This routine does not save image buffer.
*
***********************************************************/
int k_close( K_IMAGE *image )
{
    if (fio_freeBody( image )) return(-1);
    if (fio_freeImage( image )) return(-1);

    return( 0 );
}
/**********************************************************
*
*   k_open_frame
*
*    read image with the given frame number
*    fnhead is the printf format
*      ex. "image%06d.ppm"
*
***********************************************************/
K_IMAGE *k_open_frame(char *fnhead, int num)
{
    char fname[MAXLEN];

    snprintf(fname, MAXLEN, fnhead, num);
    return(k_open(fname));
}
/**********************************************************
*
*   k_write_frame
*
*    write image with the given frame number
*    fnhead is the printf format
*      ex. "image%06d.ppm"
*
***********************************************************/
int k_write_frame(K_IMAGE *image, char *fnhead, int num)
{
    char fname[MAXLEN];

    snprintf(fname, MAXLEN, fnhead, num);
    return(k_write(image, fname));
}
/****************************************************************/
// 構造体の領域を確保
K_IMAGE *fio_allocImage()
{
    K_IMAGE *image;

    image = (K_IMAGE *)malloc(sizeof(K_IMAGE));
    if (image == NULL)
    {
	k_perror("Error: Can't malloc K_IMAGE!!\n");
	return( NULL );
    }

    image->header = (K_HEAD *)malloc(sizeof(K_HEAD));
    if (image->header == NULL)
    {
	k_perror("Error: Can't malloc K_HEAD!!\n");
	return( NULL );
    }
    return( image );
}

// 構造体の領域を開放
int fio_freeImage( K_IMAGE *image )
{
    free(image->header);
    free(image);
    return(0);
}


// 画像本体のデータ領域を確保
int fio_allocBody(K_IMAGE *image)
{
    int		m, y;
    int		lineBytes;

    // width 方向は 8の倍数切り上げの画素分だけ領域を確保( for K_BIT )
    lineBytes = (((k_xsize(image)+7)/8)*8)*k_pixelbyte(image);

    image->data = (void ***)malloc(k_multi(image) * sizeof(void **));
    if (image->data == NULL)
    {
	k_perror("Error: Can't malloc image body(1)!!\n");
	return(-1);
    }
    for(m = 0; m < k_multi(image); m++)
    {
	image->data[m] = (void **)malloc(k_ysize(image) * sizeof(void *));
	if (image->data[m] == NULL)
	{
	    k_perror("Error: Can't malloc image body(2)!!\n");
	    return(-1);
	}
	for(y = 0; y < k_ysize(image); y++)
	{
	    image->data[m][y] = (void *)malloc(lineBytes);
	    if (image->data[m][y] == NULL)
	    {
		k_perror("Error: Can't malloc image body(3)!!\n");
		return(-1);
	    }
	}
    }

    return( 0 );
}

// 画像本体のデータ領域を開放
int fio_freeBody(K_IMAGE *image)
{
    int		m, y;

    for(m = 0; m < k_multi(image); m++)
    {
	for(y = 0; y < k_ysize(image); y++)
	{
	    free(image->data[m][y]);
	}
	free(image->data[m]);
    }
    free(image->data);
    return(0);	    
}

// 各種パラメータを事前に計算
int fio_setupParam(K_IMAGE *image)
{
    switch(k_pixeltype(image))
    {
    case K_UCHAR:  image->pixelbyte = 1; break;
    case K_USHORT: image->pixelbyte = 2; break;
    case K_INT:    image->pixelbyte = 4; break;
    case K_FLOAT:  image->pixelbyte = 4; break;
    case K_DOUBLE: image->pixelbyte = 8; break;
    case K_BIT:    image->pixelbyte = 1; break;
    default: k_perror("Error: Unknown Pixel Type (%d)!!\n", k_pixeltype(image));
	return(-1);
    }
    return(0);
}

// KUMI画像のヘッダ読み込み
int fio_kumiReadHeader(FILE *fp, K_IMAGE *image, char *magic)
{
    // Magic の残りの2文字を読み込む
    int dummy = fread(&magic[2], 2, 1, fp);
    if (magic[2] != K_MAGIC[2] || magic[3] != K_MAGIC[3])
    {
	magic[4] = '\0';
	k_perror("Error: Unknown Magic(%s)!!\n", magic);
	return(-1);
    }

    // ヘッダの残りを読み込む
    int dummy2 = fread(image->header, K_INT_SIZE, K_HEADER_ITEMS, fp);
    return(0);
}

// KUMI画像のヘッダ出力
int fio_kumiWriteHeader(FILE *fp, K_IMAGE *image)
{
    // Magic の出力
    fwrite(K_MAGIC, 4, 1, fp);

    // 旧バージョンとの互換性のために設定
    image->header->stype     = K_FILTER;
    image->header->imgtype   = 0;
    image->header->order     = 1;

    // ヘッダの残りを出力
    fwrite(image->header, K_INT_SIZE, K_HEADER_ITEMS, fp);
    return(0);
}

// PNM画像のヘッダ読み込み(先頭 2bytes は読み込み済み)
int fio_pnmReadHeader(FILE *fp, K_IMAGE *image, char *magic)
{
    int		maxval;

    if (magic[1] == '4')
    { // pbm
	_skipSP(fp, 1);
	image->header->xsize = _getVal(fp);
	_skipSP(fp, 1);
	image->header->ysize = _getVal(fp);
	_skipSP(fp, 0);

	image->header->multi = 1;
	image->header->pixeltype = K_BIT;
	image->header->ebit = 8;
    } else if (magic[1] == '5' || magic[1] == '6') {
	// pgm, ppm
	_skipSP(fp, 1);
	image->header->xsize = _getVal(fp);
	_skipSP(fp, 1);
	image->header->ysize = _getVal(fp);
	_skipSP(fp, 1);
	maxval = _getVal(fp);
	_skipSP(fp, 0);

	image->header->pixeltype = K_UCHAR;
	if (magic[1] == '5')
	{      // pgm
	    image->header->multi = 1;
	}else{ // ppm
	    image->header->multi = 3;
	}
	image->header->ebit = ku_get_ebit(maxval);
    }else{
	magic[2] = '\0';
	k_perror("Error: Unkwon Magic netpbm-(%s)\n", magic);
	return(-1);
    }
    return(0);
}

// PNM画像のヘッダを出力
int fio_pnmWriteHeader(FILE *fp, K_IMAGE *image)
{
    int		maxval;

    maxval = (1 << k_ebit(image))-1;
    if (k_pixeltype(image) == K_BIT && k_multi(image) == 1)
    {
        // PBM
	fprintf(fp, "P4\n");
	fprintf(fp, "%d %d\n", k_xsize(image), k_ysize(image));
    }else if (k_pixeltype(image) == K_UCHAR && k_multi(image) == 1){
	// PGM
	fprintf(fp, "P5\n");
	fprintf(fp, "%d %d\n", k_xsize(image), k_ysize(image));
	fprintf(fp, "%d\n", maxval);
    }else if (k_pixeltype(image) == K_UCHAR && k_multi(image) == 3){
        // PPM
	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n", k_xsize(image), k_ysize(image));
	fprintf(fp, "%d\n", maxval);
    }else{
	k_perror("Error: Can't write in netpbm format\n\t pixeltype=%d, multi=%d\n",
		 k_pixeltype(image), k_multi(image));
	return(-1);
    }
    return(0);
}

// 空白，タブ，改行，コメントを読み飛ばす
//   skip_comment == 0 ならばコメント('#')は飛ばさない
//   (raster data の最初が '#' の場合に対処
static void _skipSP(FILE *fp, int skip_comment)
{
    int		ch;

    while ((ch = fgetc(fp))!=EOF)
    {
	// 空白は無視
	if (ch == ' ' || ch == '\t' || ch == '\n') continue;

	if (skip_comment == 1 && ch == '#')
	{
	    // コメントは行末まで読み飛ばす
	    while ((ch = fgetc(fp))!=EOF)
	    {
		if (ch == '\n')
		{
		    break;
		}
	    }
	    continue;
	}

	// 必要なデータを読んでしまったので戻す
	ungetc(ch, fp);
	return;
    }
}

// 数値を読み込む (0以上の整数のみ, int で表現できない数もダメ)
static int _getVal(FILE *fp)
{
    int		ch;
    int		val;

    val = 0;
    while ((ch = fgetc(fp))!=EOF)
    {
	// 数値以外を見つけたら抜ける
	if (ch < '0' || ch > '9') break;

	// 一桁増やして数値を追加
	val = val * 10 + ch - '0';
    }
    return(val);
}

/****************************************************************
 *
 * access 関数
 *   データをパックしたまとまりとして扱う
 *   普通は使わない ( image.data をキャストして使う方が速い )
 *
 ****************************************************************/
// k_pixelbyte 分のデータを buf に格納
int k_getpix(K_IMAGE *image, int x, int y, int m, void *buf)
{
    if (m < 0 || m >= k_multi(image) || x < 0 || x >= k_xsize(image) ||
	y < 0 || y >= k_ysize(image)) 
    {
	k_perror("Error: Out of Range in getpix %d %d %d\n", x, y, m);
	return(-1);
    }
	
    memcpy(buf,
	   (image->data)[m][y] + x * k_pixelbyte(image), 
	   k_pixelbyte(image));
    return(0);
}

// k_pixelbyte 分のデータを buf からコピー
int k_putpix(K_IMAGE *image, int x, int y, int m, void *buf)
{
    if (m < 0 || m >= k_multi(image) || x < 0 || x >= k_xsize(image) ||
	y < 0 || y >= k_ysize(image)) 
    {
	k_perror("Error: Out of Range in putpix %d %d %d\n", x, y, m);
	return(-1);
    }
	
    memcpy((image->data)[m][y] + x * k_pixelbyte(image), 
	   buf,
	   k_pixelbyte(image));
    return(0);
}
/****************************************************************
*
*	print error message
*
****************************************************************/
#include <stdarg.h>
void k_perror(const char *format, ...)
{
    if ( _k_debug )
    {
	va_list	ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
    }
}
/****************************************************************
 *
 *  Utility routines (ku_XXX)
 *
 ****************************************************************/
// val を [0..255] の範囲に丸める
int ku_crop_uchar(int val)
{
    if (val < 0)
    {
	val = 0;
    }else if (val > 255){
	val = 255;
    }
    return(val);
}

// maxval を表現できる2進数での桁数を求める
int ku_get_ebit(int maxval)
{
    int		c;

    c = 1;
    while( (maxval >>= 1) > 0)
    {
	c++;
    }
    return(c);
}

// ebit で表現できる最大数を求める
int ku_get_maxval(int ebit)
{
    return((1 << ebit)-1);
}
