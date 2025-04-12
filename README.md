# MIOWIN: Minimal IO library for Windows

## About
MIOWIN is a minimal IO library for Windows.
The goal of this project is to provide a simple way to use input/output functions in Windows.
The main advantage of this library is it's small size.
Smallest `.exe` I've been able to make is 17KB (using `upx` tool: 11KB). 

The whole documentation is in this readme file.

Current version: `Beta 0.1`

License: Public Domain, no warranty given, use at your own risk.

## Usage
To use this library:
1. Download the library (`miowin.h`).
2. Choose one file where you will paste the implementation of the library.
3. Include the library in this file.
4. Right before the `#include` line, add `#define MIOWIN_C`.
5. In other files just include the library.


## Documentation
The library is that small, so it's possible to fit documentation there.

```c
// flags for window scaling, use it in mio_window
// when using them together, they add up
#define MIO_SCALE1  0x01
#define MIO_SCALE2  0x02
#define MIO_SCALE4  0x04
#define MIO_SCALE8  0x08
#define MIO_SCALE16 0x10

  // example
  mio_window("MIOWIN", 48, 32, MIO_SCALE16 | MIO_SCALE8); // scaled by 24

// flags for fps, use it in mio_window
#define MIO_FPS30 0x20
#define MIO_FPS60 0x40

// macro for getting and setting pixels
#define mio_pixel(x, y)

  // example
  mio_pixel(1, 1)            // returns the pixel at (1, 1)
  mio_pixel(2, 2) = 0x00ff00 // sets the pixel at (2, 2) to green 

// does user want to quit?
char mio_quit;

// buffer with all pixels, format is 0x00RRGGBB
unsigned int* mio_pixels;

// keyboard current state, the index is VK from windows api,
// look it up in the internet. 1 for pressed, 0 for not pressed
char mio_keys[256];

// width and height passed to mio_window
int mio_width, mio_height;

// the initialization function, that creates a window
// title: the title of the window
// w: width of the window
// h: height of the window
// flags: flags for window's scaling and fps
//   the default scaling is 1, and fps is 20
void mio_window(char* title, int w, int h, unsigned char flags);

  // example
  mio_window(
    "MIOWIN", 64, 64,
    MIO_SCALE16 | MIO_FPS30);

// the update function, that updates the window
// you should call it first in your infinite loop
void mio_update(void);

// the die function, that destroys the window
// it's preferably to end program with calling this function
void mio_die(void);
```
And yes, this is the whole library, for now.
Nothing unnecessary is included.


## Sample Program
```c
#define MIOWIN_C
#include <miowin.h>

unsigned char pos = 0;

int main() {
  mio_window(
    "MIOWIN",
    48, 32,
    MIO_SCALE16);

  while (!mio_quit) {
    mio_update();
    mio_pixel(pos % mio_width, 0) = 0x000000;
    if (mio_keys['D'])
      pos++;
    mio_pixel(pos % mio_width, 0) = 0x00ff00;
  }

  mio_die();
}
```