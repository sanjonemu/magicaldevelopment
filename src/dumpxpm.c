/*
  dumpxpm.c

  >mingw32-make -f makefile.tdmgcc64

  MIME image/x-xpixmap

  (needless now) Linux X11
  ftp://export.lcs.mit.edu/pub/xorg/X11R7.7/src/
    everything/libXpm-3.5.10.tar.bz2
    everything/libXpm-3.5.10.tar.gz
    lib/libXpm-3.5.10.tar.bz2
    lib/libXpm-3.5.10.tar.gz

  (needless now) Windows (xpm.h depends on simx.h in libgw32c-src ? etc.)
  http://gnuwin32.sourceforge.net/packages/xpm.htm
    xpm-3.5.1-1-bin.zip
    xpm-3.5.1-1-lib.zip -> libXpm.dll(.a) libXpm.lib xpm.h (libgw32c.a)
    xpm-3.5.1-1-src.zip
    xpm-3.5.1-1-doc.zip
    xpm-3.5.1-1-dep.zip
    (Requirements libgw32c) (needless now)
    http://gnuwin32.sourceforge.net/packages/libgw32c.htm
      libgw32c-0.4-lib.zip
      libgw32c-0.4-src.zip
  (skip GetGnuWin32-0.6.3.exe) (needless now)
*/

#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <conio.h>
#include <time.h>

typedef enum {
  black, dblue, dgreen, dcyan, dred, dmagenta, dyellow, gray,
  dgray, blue, green, cyan, red, magenta, yellow, white,
} color;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define BUFSIZE 4096
#define PROMPT "hit ENTER key"

#define IASPECTY 6
#define IASPECTX 4

#define CON_BG dblue
#define CON_FG cyan
#define HEIGHT 50
#define WIDTH 80

static HANDLE handle = 0;
static COORD curs = {0, 0};
static ushort attr = 0;
static CHAR_INFO consb[HEIGHT * WIDTH];
static COORD conssize = {WIDTH, HEIGHT};
static COORD conspos = {0, 0};
static SMALL_RECT consarea = {0, 0, WIDTH - 1, HEIGHT - 1};

void cursor(ushort y, ushort x, int flag)
{
  curs.Y = y, curs.X = x;
  // SetConsoleCursorPosition(handle, (curs.Y << 16) | (curs.X & 0x0000FFFF));
  if(flag) SetConsoleCursorPosition(handle, curs);
}

void conattr(char bg, char fg)
{
  SetConsoleTextAttribute(handle, attr = (bg << 4) | (fg & 0x0F));
}

void cls(int t, int b, int l, int r, char c)
{
  int i, j;
  for(j = t; j <= b; ++j){
    for(i = l; i <= r; ++i){
      consb[j * WIDTH + i].Char.AsciiChar = c;
      consb[j * WIDTH + i].Attributes = attr;
    }
  }
}

void bputs(ushort y, ushort x, uchar *s)
{
  uchar *p;
  cursor(y, x, 0);
  for(p = s; *p; ++p, ++curs.X){
    consb[curs.Y * WIDTH + curs.X].Char.AsciiChar = *p;
    consb[curs.Y * WIDTH + curs.X].Attributes = attr;
  }
}

void beep(void)
{
  putch('\a');
}

void getstr(int y, int x, char a, char z, char *name, int len)
{
  int i;
  for(i = 0; i < len; i++) name[i] = '_', cursor(y, x + i, 0), putch('-');
  i = 0;
  while(i < len){
    int c;
    cursor(y, x + i, 0), putch(name[i]), cursor(y, x + i, 0);
    c = getch();
    if(c == 0x0D || c == 0x0A){ (name[i] == '_' ? i : ++i); break; }
    if(c == 0x08){ name[i] = '_', putch('-'), (i ? --i : i); continue; }
    if(c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if((c >= a && c <= z) || c == '.') name[i] = c;
    else if(c == 0x20 && name[i] != '_') ++i;
    else beep();
  }
  name[i] = '\0';
}

void raw_input(int y, int x, char *prompt)
{
  char buf[BUFSIZE];
  bputs(y, x, prompt); cursor(y, x + strlen(prompt), 1);
  WriteConsoleOutputA(handle, consb, conssize, conspos, &consarea);
  fgets(buf, sizeof(buf) / sizeof(buf[0]), stdin);
}

void bprintf(int y, int x, char *fmt, ...)
{
  char buf[BUFSIZE];
  va_list va;
  va_start(va, fmt);
  vsprintf(buf, fmt, va);
  va_end(va);
  bputs(y, x, buf);
}

int berror(int e, char *fmt, ...)
{
  char buf[BUFSIZE];
  va_list va;
  va_start(va, fmt);
  vsprintf(buf, fmt, va);
  va_end(va);
  bputs(HEIGHT - 2, 0, buf);
  return e;
}

#define OFFBITS (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))

int writeBMP(char *fn, HBITMAP hbmp, int c, int r, HDC hdc)
{
  size_t wlen, sz;
  LPBYTE p;
  FILE *fp;
  if(!(fp = fopen(fn, "wb"))) return berror(1, "cannot create: %s", fn);
  wlen = c * 3;
  if(wlen % 4) wlen += 4 - wlen % 4;
  sz = OFFBITS + r * wlen;
  if(p = (LPBYTE)malloc(sz)){
    LPBITMAPFILEHEADER bh = (LPBITMAPFILEHEADER)p;
    LPBITMAPINFOHEADER bi = (LPBITMAPINFOHEADER)(p + sizeof(BITMAPFILEHEADER));
    LPBYTE pixels = p + OFFBITS;
    memset(bh, 0, sizeof(BITMAPFILEHEADER));
    memset(bi, 0, sizeof(BITMAPINFOHEADER));
    bh->bfType = ('M' << 8) | 'B';
    bh->bfSize = sz;
    bh->bfOffBits = OFFBITS;
    bi->biSize = sizeof(BITMAPINFOHEADER);
    bi->biWidth = c;
    bi->biHeight = r;
    bi->biPlanes = 1; // not be 3
    bi->biBitCount = 24; // not be 8
    bi->biCompression = BI_RGB;
    // rows may be reversed top <-> bottom copying them from bitmap to buffer
    GetDIBits(hdc, hbmp, 0, r, pixels, (LPBITMAPINFO)bi, DIB_RGB_COLORS);
    fwrite(p, sz, 1, fp);
    free(p);
  }
  fclose(fp);
  return 0;
}

// cols rows planes depth-bits ary
int dispBMP(int c, int r, int p, int d, DWORD *a, char *fn)
{
  // should call CreateCompatibleBitmap(hdc, c, r) when use unknown device
  HBITMAP hbmp = CreateBitmap(c, r, p, d, (BYTE *)a); // (A)RGB L.E. -> B,G,R,A
  HWND hwnd = GetDesktopWindow();
  HDC hdc = GetDC(hwnd);
  HDC hmdc = CreateCompatibleDC(hdc);
  HBITMAP obmp = (HBITMAP)SelectObject(hmdc, hbmp);
  StretchBlt(hdc, 0, 0, c * IASPECTX, r * IASPECTY, hmdc, 0, 0, c, r, SRCCOPY);
  SelectObject(hmdc, obmp);
  writeBMP(fn, hbmp, c, r, hmdc);
  DeleteObject(hbmp);
  DeleteDC(hmdc);
  ReleaseDC(hwnd, hdc);
  return 0;
}

#define XPML 2
#define XPM_FMT_FIRST "/* XPM"
#define XPM_FMT_SECOND "static char *"
#define XPM_FMT_XPMEXT "XPMEXT"
#define XPM_PNONE "          "
#define XPM_CPP_MAX 10 // length of XPM_PNONE (set <= 10)
#define XPM_COLOR_NONE ((((CON_BG << 4) | (CON_FG & 0x0F)) << 24) | 0x00EEAA22)
#define XPM_COLOR_BUF 16 // string length of a descriptor
#define XPM_PALETTE_MAX 16

#define XPMNCPY(D, S) do{ \
  if(S){ strncpy(D, S, sizeof(D) - 1); D[sizeof(D) - 1] = '\0'; }\
  else D[0] = '\0'; \
}while(0)

typedef struct _XPMCOLORMAP {
  DWORD argb;
  char *c;
} XPMCOLORMAP;

typedef struct _XPMCOLOR {
  char s[XPM_COLOR_BUF]; // str
  char c[XPM_COLOR_BUF]; // color
  char m[XPM_COLOR_BUF]; // mono
  char g[XPM_COLOR_BUF]; // gray
  DWORD argb; // (DWORD *) (A)RGB L.E. -> (BYTE *) B,G,R,A (PNONE-A is BGFG)
  char p[XPM_CPP_MAX + 1]; // characters per pixel and space for '\0'
  char reserved[1]; // alignment
} XPMCOLOR;

typedef struct _XPMINFO {
  int c, r, p, d; // cols rows planes depth-bits
  int bpp, wlen, sz, xpmext, ncolors, cpp, x_hot, y_hot;
  XPMCOLOR pal[XPM_PALETTE_MAX]; // color table (none = pal[0])
  char fn[MAX_PATH]; // filename
  DWORD *a; // pixel buffer
} XPMINFO;

XPMCOLORMAP xpmcolors[] = {
  {XPM_COLOR_NONE, "none"},
  {0x07000000, "black"},
  {0x1E00007F, "dblue"},
  {0x2D007F00, "dgreen"},
  {0x3C007F7F, "dcyan"},
  {0x4B7F0000, "dred"},
  {0x5A7F007F, "dmagenta"},
  {0x697F7F00, "dyellow"},
  {0x707F7F7F, "gray"},
  {0x8F3F3F3F, "dgray"},
  {0x960000FF, "blue"},
  {0xA500FF00, "green"},
  {0xB400FFFF, "cyan"},
  {0xC3FF0000, "red"},
  {0xD2FF00FF, "magenta"},
  {0xE1FFFF00, "yellow"},
  {0xF8FFFFFF, "white"},
  {XPM_COLOR_NONE, "None"}, // dummy
  {0x293F7F00, "darkolivegreen"}, // test
  {0xB100FFFF, "lightblue"}, // cyan
  {0x877F7F7F, "gray50"}, // test
  {0x70B2B2B2, "gray70"}, // test
  {0x7FD8D8D8, "gray85"}, // test
  {0x7EBFBFBF, "lightgray"}}; // dummy

DWORD getXPMcolor(char *c)
{
  int i;
  if(!c) return XPM_COLOR_NONE;
  for(i = 0; i < sizeof(xpmcolors) / sizeof(xpmcolors[0]); ++i){
    if(!strncmp(xpmcolors[i].c, c, XPM_COLOR_BUF-1)) return xpmcolors[i].argb;
  }
  if(c[0] == '#'){
    DWORD argb;
    int n = sscanf(c, "#%x", &argb);
    if(n){
      if(argb & 0xFF000000) return argb;
      else{
        uchar r = (argb >> 16) & 0x0FF;
        uchar g = (argb >> 8) & 0x0FF;
        uchar b = argb & 0x0FF;
#if 0
        uchar rt = 223, gt = 183, bt = 191;
#else
        uchar rt = 223, gt = 187, bt = 191;
#endif
        uchar bg = 8 + ((r>rt ? 4 : 0) | (g>gt ? 2 : 0) | (b>bt ? 1 : 0));
        uchar fg = 0x0F - bg;
        return (((bg << 4) | (fg & 0x0F)) << 24) | argb;
      }
    }
  }
  berror(1, "unknown color: [%s]", c);
  return XPM_COLOR_NONE;
}

int setXPMpal(XPMINFO *xi, int n, char *s, char *c, char *m, char *g,
  DWORD a, char *p)
{
  XPMCOLOR *t = &xi->pal[n];
  XPMNCPY(t->s, s); XPMNCPY(t->c, c); XPMNCPY(t->m, m); XPMNCPY(t->g, g);
  t->argb = a; XPMNCPY(t->p, p);
  return 0;
}

DWORD pickXPMpal(XPMINFO *xi, char *p)
{
  int i;
  for(i = 0; i < XPM_PALETTE_MAX; ++i){
    if(!strncmp(xi->pal[i].p, p, xi->cpp)) return xi->pal[i].argb;
  }
  berror(1, "unknown color palette: [%s]", p);
  raw_input(HEIGHT - 1, 0, PROMPT);
  return XPM_COLOR_NONE;
}

int buildBMP(XPMINFO *xi)
{
  int x, y;
  xi->bpp = xi->p * xi->d / 8; // bytes per pixel
  xi->wlen = xi->c * xi->bpp; // bytes per line
  xi->sz = xi->r * xi->wlen; // data bytes
  xi->a = (DWORD *)malloc(xi->sz); // (DWORD *) (A)RGB L.E. -> (BYTE *) B,G,R,A
  if(!xi->a) return berror(1, "cannot allocate pixel buffer");
  // initialize pixel buffer
  for(y = 0; y < xi->r; ++y){
#if 0
    DWORD t = y & 1 ? 0x000000FF : 0x0000FF00;
    for(x = 0; x < xi->c; ++x) xi->a[xi->c * y + x] = x & 1 ? t : t << 8;
#else
    for(x = 0; x < xi->c; ++x) xi->a[xi->c * y + x] = xi->pal[0].argb;
#endif
  }
  return 0;
}

int loadXPMINFO(XPMINFO *xi, char *buf)
{
  int r;
  char remain[BUFSIZE];
  if(buf[0] != '"' || buf[strlen(buf) - 2] != '"'
  || buf[strlen(buf) - 1] != ',') return berror(1, "no XPMINFO: %s", buf);
  r = sscanf(buf, "\"%d %d %d %d %d %d %[^\"]s",
    &xi->c, &xi->r, &xi->ncolors, &xi->cpp, &xi->x_hot, &xi->y_hot, remain);
  if(r < 4) return berror(1, "XPMINFO requires 4 integers: %s", buf);
  if(r >= 7 && !strncmp(remain, XPM_FMT_XPMEXT, strlen(XPM_FMT_XPMEXT)))
    xi->xpmext = 1;
#if 0
  bprintf(XPML, 40, "%d[%d][%d][%d][%d][%d][%d][%d]",
    r, xi->c, xi->r, xi->ncolors, xi->cpp, xi->x_hot, xi->y_hot, xi->xpmext);
#endif
  if(xi->ncolors > XPM_PALETTE_MAX)
    return berror(1, "too much palettes: %d", xi->ncolors);
  if(xi->cpp > XPM_CPP_MAX)
    return berror(1, "too large cpp: %d", xi->cpp);
  return 0;
}

int loadXPMpal(XPMINFO *xi, int n, char *buf)
{
  char p[XPM_CPP_MAX + 1] = {0};
  char *s = NULL, *c = NULL, *m = NULL, *g = NULL, *q, *r;
  buf[strlen(buf) - 2] = '\0';
  strncpy(p, buf + 1, xi->cpp); if(p[1] == '\t') p[1] = ' ';
  for(q = buf + 1 + (xi->cpp <= 1 ? 2 : xi->cpp); *q; q = r + 1){
    switch(*q){
    case 's': s = q + 2; break;
    case 'c': c = q + 2; break;
    case 'm': m = q + 2; break;
    case 'g': g = q + 2; break;
    default:
      berror(1, "unknown color descriptor: [%s] %s", p, q);
      raw_input(HEIGHT - 1, 0, PROMPT);
    }
    if(r = strchr(q, '\t')) *r = '\0';
    else break;
  }
  return setXPMpal(xi, n, s, c, m, g, getXPMcolor(c), p);
}

void bputsXPMattr(XPMINFO *xi, ushort y, ushort x, uchar *s)
{
  uchar *p;
  int q;
  cursor(y, x, 0);
  for(p = s, q = 0; *p; ++p, ++q, ++curs.X){
    int o = q / xi->cpp; // round
    DWORD argb = pickXPMpal(xi, s + o * xi->cpp);
    consb[curs.Y * WIDTH + curs.X].Char.AsciiChar = *p;
    consb[curs.Y * WIDTH + curs.X].Attributes = (argb >> 24) & 0x0FF;
    xi->a[xi->c * (y - XPML) + x + o] = argb;
  }
}

int loadXPMpixel(XPMINFO *xi, int n, char *buf)
{
  int h = XPML + n;
  buf[1 + xi->c * xi->cpp] = '\0';
  if(h < HEIGHT) bputsXPMattr(xi, h, 0, buf + 1); // never use bprintf for buf
  return 0;
}

char *basename(char *p)
{
  char *q = strrchr(p, '/');
  return q ? q+1 : p;
}

char *splitext(char *p, int n)
{
  static char keepit[MAX_PATH];
  char *q;
  strncpy(keepit, p, sizeof(keepit));
  if(q = strrchr(keepit, '.')) *q = '\0';
  return n ? (q ? q+1 : keepit+strlen(keepit)) : keepit;
}

int rstrip(char *buf)
{
  int len = strlen(buf);
  if(!len) return len;
  if(buf[len - 1] == 0x0A) buf[len - 1] = '\0';
  len = strlen(buf);
  if(!len) return len;
  if(buf[len - 1] == 0x0D) buf[len - 1] = '\0';
  return strlen(buf);
}

int skipcomment(char *buf, int flag) // fake parser
{
  if(!flag){
    char *p = strstr(buf, "//");
    if(p){ *p = '\0'; return 0; }
    p = strstr(buf, "/*");
    if(p){
      char *q = strstr(p, "*/");
      *p = '\0';
      return q ? 0 : 1;
    }
    return 0;
  }else{
    char *q = strstr(buf, "*/");
    *buf = '\0';
    return q ? 0 : 1;
  }
}

#define XPM_OUTBMP "\\tmp\\__________%s.bmp"
#define XPM_CHECKER "../res/checker.xpm"
#define XPM_ROUNDB "../res/roundb.xpm"
#define XPM_NEKO "../res/neko.xpm"
#define XPM_XTERM_LINUX "../res/xterm-linux.xpm"
#define XPM_NEMU "../res/nemu_48x48_c16.xpm"

char *xpmfiles[] = {XPM_CHECKER, XPM_ROUNDB,
  XPM_NEKO, XPM_XTERM_LINUX, XPM_NEMU};

int dumpxpm(char *xpmfile)
{
  FILE *fp;
  char buf[BUFSIZE];
  int stat = 0, flag = 0, line = 0;
  XPMINFO xi;
  memset(&xi, 0, sizeof(XPMINFO));
  // test (c=8, r=4, p=4, d=8) 128B=32W(=r4xc8)=32B/line=4B/pixel(=d8p4)
  xi.c = 64, xi.r = 38, xi.p = 4, xi.d = 8;
  setXPMpal(&xi, 0, "none", "none", "none", "none", XPM_COLOR_NONE, XPM_PNONE);
  sprintf(xi.fn, XPM_OUTBMP, splitext(basename(xpmfile), 0));
  SetConsoleTitleA(xpmfile);
  bprintf(0, 0, "loading: %s", xpmfile);
  if(!(fp = fopen(xpmfile, "rb"))) return berror(1, "not found: %s", xpmfile);
  while(fgets(buf, sizeof(buf) / sizeof(buf[0]), fp)){
    if(!rstrip(buf)) continue;
    if(!stat){
      if(!strncmp(buf, XPM_FMT_FIRST, strlen(XPM_FMT_FIRST))) ++stat;
      else{ berror(1, "line 0 expected: %s", XPM_FMT_FIRST); break; }
    }else if(stat == 1){
      if(flag = skipcomment(buf, flag)) continue; // fake parser
      if(!strlen(buf)) continue;
      if(!line){
        if(!strncmp(buf, XPM_FMT_SECOND, strlen(XPM_FMT_SECOND))
        && buf[strlen(buf) - 1] == '{') ++line;
        else{ berror(1, "unknown statement: %s", buf); break; }
      }else if(line == 1){
        if(loadXPMINFO(&xi, buf)) break;
        if(!xi.a){ if(buildBMP(&xi)) break; }
        ++line;
      }else if(line >= XPML && line < XPML + xi.ncolors){
        if(loadXPMpal(&xi, line++ - XPML, buf)) break;
      }else{
        if(loadXPMpixel(&xi, line++ - xi.ncolors - XPML, buf)) break;
      }
    }else{
      berror(1, "unknown stat: %d", stat);
    }
  }
  fclose(fp);
  if(xi.a){
    dispBMP(xi.c, xi.r, xi.p, xi.d, xi.a, xi.fn);
    free(xi.a); xi.a = NULL;
    bprintf(1, 0, "done. written to: %s", xi.fn);
  }
  return 0;
}

int main(int ac, char **av)
{
  int i;
  // AllocConsole();
  handle = GetStdHandle(-11);
  SetConsoleScreenBufferSize(handle, conssize);
  SetConsoleWindowInfo(handle, TRUE, &consarea);
  conattr(CON_BG, CON_FG);
  for(i = 0; i < sizeof(xpmfiles) / sizeof(xpmfiles[0]); ++i){
    cls(0, HEIGHT - 1, 0, WIDTH - 1, ' ');
    ScrollConsoleScreenBuffer(handle, &consarea, NULL, conspos, consb);
    dumpxpm(xpmfiles[i]);
    raw_input(HEIGHT - 1, 0, PROMPT);
  }
  // FreeConsole();
  return 0;
}
