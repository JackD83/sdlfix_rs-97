#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>



#define SDL_FULLSCREEN 0x80000000u
#define SDL_NOFRAME 0x00000020u

#define Uint8 uint8_t
#define Uint32 uint32_t
#define Uint16 uint16_t
#define Sint16 short int

int force = 0;


typedef struct{
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 unused;
} SDL_Color;

typedef struct{
  Sint16 x, y;
  Uint16 w, h;
} SDL_Rect;

typedef struct{
  int ncolors;
  SDL_Color *colors;
} SDL_Palette;

typedef struct SDL_PixelFormat {
  SDL_Palette *palette;
  Uint8  BitsPerPixel;
  Uint8  BytesPerPixel;
  Uint8  Rloss, Gloss, Bloss, Aloss;
  Uint8  Rshift, Gshift, Bshift, Ashift;
  Uint32 Rmask, Gmask, Bmask, Amask;
  Uint32 colorkey;
  Uint8  alpha;
} SDL_PixelFormat;



typedef struct{
    Uint32 hw_available:1;
    Uint32 wm_available:1;
    Uint32 blit_hw:1;
    Uint32 blit_hw_CC:1;
    Uint32 blit_hw_A:1;
    Uint32 blit_sw:1;
    Uint32 blit_sw_CC:1;
    Uint32 blit_sw_A:1;
    Uint32 blit_fill:1;
    Uint32 video_mem;
    SDL_PixelFormat *vfmt;
    int current_w;
    int current_h;
} SDL_VideoInfo;


typedef struct SDL_Surface
{
    Uint32 flags;              
    SDL_PixelFormat *format;   
    int w, h;                   
    int pitch;                  
    void *pixels;    
    void *userdata;   
    int locked;                 
    void *lock_data;       
    SDL_Rect clip_rect;      
    struct SDL_BlitMap *map;        
    int refcount;               
} SDL_Surface;


SDL_Surface* screen = NULL;
SDL_Surface* realScreen = NULL;

void updateScreen(void)
{      
  //printf("update screen %dx %d %d %ld\n", screen->w, screen->h, screen->pitch, sizeof(screen->pixels)/sizeof(screen->pixels[0]));
  //printf("update real screen %dx %d %d %ld\n", realScreen->w, realScreen->h, realScreen->pitch, sizeof(realScreen->pixels) );
   int i;
    for(i=0; i < 240; i++) {
        int offset = 2 * i;
        memcpy(((Uint8 *)realScreen->pixels) + (offset * screen->pitch ), ((Uint8 *)screen->pixels) + i * screen->pitch, screen->pitch);
        memcpy(((Uint8 *)realScreen->pixels) + (offset + 1) * screen->pitch , ((Uint8 *)screen->pixels) + i *  screen->pitch, screen->pitch);           
    }  
    //printf("update screen finish\n");
}


void* SDL_GetVideoInfo() { 
  static void*(*info)() = NULL;

  if (info == NULL) {   
    info = dlsym(RTLD_NEXT, "SDL_GetVideoInfo");
    assert(info != NULL);
  }

  SDL_VideoInfo* inf = (SDL_VideoInfo*)info();   //<-- calls SDL_GetVideoInfo();   
  int screenWidth = inf->current_w;
  int screenHeight = inf->current_h;

  if(screenWidth == 320 && screenHeight == 480) {
    printf("Call to SDL_GetVideoInfo %d x %d\n", screenWidth, screenHeight );
    printf("correcting to 320x240\n");
    inf->current_h = 240;
  } 
  
  return inf;
}



void* SDL_SetVideoMode(int width, int height, int bitsperpixel, uint32_t flags) {
  static void*(*real_func)(int,int,int,uint32_t) = NULL;
  static void*(*info)() = NULL;
  static void*(*create)(Uint32 , int, int, int,Uint32 , Uint32 ,  Uint32 , Uint32 ) =NULL;

  if (real_func == NULL) {
    real_func = dlsym(RTLD_NEXT, "SDL_SetVideoMode");
    info = dlsym(RTLD_NEXT, "SDL_GetVideoInfo");
    create = dlsym(RTLD_NEXT, "SDL_CreateRGBSurface");
    assert(real_func != NULL);
  }

  if (info == NULL) {    
    info = dlsym(RTLD_NEXT, "SDL_GetVideoInfo");   
    assert(info != NULL);
  }

   if (create == NULL) { 
     create = dlsym(RTLD_NEXT, "SDL_CreateRGBSurface");
     assert(create != NULL);
  }


  const SDL_VideoInfo* inf = (SDL_VideoInfo*)info();   //<-- calls SDL_GetVideoInfo();   
  int screenWidth = inf->current_w;
  int screenHeight = inf->current_h;

  if( (screenWidth == 320 && screenHeight == 480 && width == 320 && height == 240) || (width == 320 && height ==480) || force )  {
    printf("Call to SDL_SetVideoMode %d x %d, corrected screen to 320x240\n", width, height );
    screen = (SDL_Surface*)create(0, 320, 240, bitsperpixel, 0, 0, 0, 0);
    realScreen = (SDL_Surface*)real_func(320, 480, bitsperpixel, flags);
    return screen;
  } else {
     realScreen = (SDL_Surface*)real_func(width, height, bitsperpixel, flags);
  }

  
  return realScreen;
}

void* SDL_Flip(void* surface) { 
  static void*(*flip)() = NULL;
 
  if (flip == NULL) {   
    flip = dlsym(RTLD_NEXT, "SDL_Flip");
    assert(flip != NULL);
  }    

  if(surface == realScreen) {
        //printf("flipping real screen\n");
        return flip(surface);
  } else if(surface == screen) {
      //printf("flipping adapted screen\n");
      updateScreen();
      return flip(realScreen);
  } else {
    return flip(surface);
  } 
 
}


