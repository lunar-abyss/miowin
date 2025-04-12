////////////////////////////////// ABOUT.TXT ///////////////////////////////////
/******************************************************************************\
  ABOUT:    Minimal IO library for Windows
  PLATFORM: Windows 10 x64, other windows versions probably also (didn't test)
  AUTHOR:   Lunaryss, 2025
  LICENSE:  Public Domain, no warranty given, use at your own risk
  VERSION:  Beta 0.1
\******************************************************************************/



/////////////////////////////////// MIOWIN.H ///////////////////////////////////
#ifndef MIOWIN_H
#define MIOWIN_H

// libs to include
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// flags for mio window
#define MIO_SCALE1     0x01
#define MIO_SCALE2     0x02
#define MIO_SCALE4     0x04
#define MIO_SCALE8     0x08
#define MIO_SCALE16    0x10
#define MIO_FPS30      0x20
#define MIO_FPS60      0x40

// macro to put or get pixels
#define mio_pixel(x, y) \
  mio_pixels[(y) * mio_width + (x)]

// variables
extern char          mio_quit;
extern unsigned int* mio_pixels;
extern char          mio_keys[256];
extern int           mio_width;
extern int           mio_height;

// functions
void mio_window(char* title, int w, int h, unsigned char flags);
void mio_update(void);
void mio_die(void);

// miowin.h
#endif



/////////////////////////////////// MIOWIN.C ///////////////////////////////////
#ifdef MIOWIN_C

// including windows api
#include <windows.h>
#include <stdio.h>
#include <time.h>


//// TYPES SECTION
// structure with all hidden data for the library
typedef struct {
  HWND    hwnd;
  HBITMAP hbitmap;
  HDC     hdcmem;
  int     swidth;
  int     sheight;
  char    scale;
  char*   title;
  char    fps;
} mio_data_t;


//// VARIABLES SECTION
// non-hidden variables
char          mio_quit;
unsigned int* mio_pixels;
char          mio_keys[256];
int           mio_width;
int           mio_height;

// hidden variables
mio_data_t miod;

// hidden functions
LRESULT mioh_winproc(HWND, UINT, WPARAM, LPARAM);
void    mioh_paint(void);
void    mioh_init_win(void);
void    mioh_init_buf(void);
void    mioh_sync(void);

//// FUNCTIONS SECTION
// create a window
void mio_window(char* title, int w, int h, unsigned char flags)
{
  // set properties of miod
  miod.scale   = flags & 0b11111 ? flags & 0b11111 : 1;
  mio_width    = w;
  mio_height   = h;
  miod.swidth  = w * miod.scale;
  miod.sheight = h * miod.scale;
  miod.title   = title;
  
  // setting fps from flags
  miod.fps =
    flags & MIO_FPS60 ? 60 :
    flags & MIO_FPS30 ? 30 : 20;

  // init buffer
  mio_pixels = malloc(w * h * 4);
  memset(mio_pixels, 0, w * h * 4);

  // init window and graphics stuff
  mioh_init_win();
  mioh_init_buf();
}

// updater function
void mio_update(void)
{
  // vars
  MSG msg;

  // redrawing the window
  RedrawWindow(
    miod.hwnd, 0, 0,
    RDW_UPDATENOW | RDW_INVALIDATE);

  // sleep for a frame
  mioh_sync();

  // must have message loop
  while (PeekMessageA(&msg, miod.hwnd, 0, 0, PM_REMOVE))
    TranslateMessage(&msg),
    DispatchMessage(&msg);
}

// function that destroys everything
void mio_die(void)
{
  // clear weird windows data
  PostQuitMessage(0);
  DestroyWindow(miod.hwnd);
  DeleteDC(miod.hdcmem);
  DeleteObject(miod.hbitmap);

  // free buffer
  free(mio_pixels);

  // exiting
  exit(0);
}


//// INTERNAL FUNCTION SECTION
// message handler
LRESULT mioh_winproc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
  // switch messages
  switch (umsg)
  {
    // user desided to exit
    case WM_QUIT: case WM_CLOSE:
      mio_quit = 1;
      return 0;

    // painter function
    case WM_PAINT:
      mioh_paint();
      return 0;

    // keypress event
    case WM_KEYDOWN: case WM_KEYUP:
      mio_keys[wparam & 0xff] = umsg == WM_KEYDOWN;
      return 0;
  }

  // default window procedure
  return DefWindowProc(hwnd, umsg, wparam, lparam);
}

// initializing window stuff
void mioh_init_win(void)
{
  // creating the window of the application
  char regres =
    RegisterClassA(&(WNDCLASS) {
      .style = CS_OWNDC,
      .lpfnWndProc = mioh_winproc,
      .lpszClassName = miod.title,
      .hCursor = LoadCursor(0, IDC_ARROW),
    });

  // window style
  DWORD style =
       WS_OVERLAPPEDWINDOW
    & ~WS_THICKFRAME
    & ~WS_MAXIMIZEBOX;

  // size the window
  RECT rect = { 0, 0, miod.swidth, miod.sheight };
  AdjustWindowRect(&rect, style, 0);

  // Create the window
  miod.hwnd =
    CreateWindowExA(
      0, miod.title, miod.title, style,
      CW_USEDEFAULT, CW_USEDEFAULT,
      rect.right - rect.left,
      rect.bottom - rect.top,
      0, 0, 0, 0);

  // showing the window
  ShowWindow(miod.hwnd, SW_SHOW);
}

// initializing bitmap stuff
void mioh_init_buf(void)
{
  // creating bitmap info
  BITMAPINFO bmi = {
    .bmiHeader.biSize        = sizeof(BITMAPINFOHEADER),
    .bmiHeader.biWidth       = mio_width,
    .bmiHeader.biHeight      = -mio_height,
    .bmiHeader.biPlanes      = 1,
    .bmiHeader.biBitCount    = 32,
    .bmiHeader.biCompression = BI_RGB,
  };

  // create hdc
  HDC hdc = GetDC(miod.hwnd);

  // how do you even able to understand to do this
  miod.hbitmap =
    CreateDIBSection(
      hdc, &bmi,
      DIB_RGB_COLORS,
      (void**)&mio_pixels,
      0, 0);

  // create compatible dc I guess
  miod.hdcmem =
    CreateCompatibleDC(hdc);

  // release and select
  ReleaseDC(miod.hwnd, hdc);
  SelectObject(miod.hdcmem, miod.hbitmap);
}

// hidden paint function
void mioh_paint(void)
{
  // start painting the window
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(miod.hwnd, &ps);
  
  // copy the buffer to the screen
  StretchBlt(
    hdc, 0, 0, miod.swidth, miod.sheight,
    miod.hdcmem, 0, 0, mio_width, mio_height,
    SRCCOPY);
  
  // end of painting
  EndPaint(miod.hwnd, &ps);
}

// syncing with the fps
void mioh_sync(void)
{
  // calculating all the stuff
  static int past = 0;
  int wait = (past + 1000 / miod.fps) - clock();
  
  // using api to sleep for the time
  if (wait > 0)
    Sleep(wait),
    past += 1000 / miod.fps;
  else
    past = clock();
}

// miowin.c
#endif