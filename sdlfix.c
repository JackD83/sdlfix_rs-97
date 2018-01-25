#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int force = 1;

typedef struct SDLPixelFormat {
  void *palette;
  uint8_t  BitsPerPixel;
  uint8_t  BytesPerPixel;
  uint8_t  Rloss, Gloss, Bloss, Aloss;
  uint8_t  Rshift, Gshift, Bshift, Ashift;
  uint32_t Rmask, Gmask, Bmask, Amask;
  uint32_t colorkey;
  uint8_t  alpha;
} SDLPixelFormat;

typedef struct{
    uint32_t hw_available:1;
    uint32_t wm_available:1;
    uint32_t blit_hw:1;
    uint32_t blit_hw_CC:1;
    uint32_t blit_hw_A:1;
    uint32_t blit_sw:1;
    uint32_t blit_sw_CC:1;
    uint32_t blit_sw_A:1;
    uint32_t blit_fill:1;
    uint32_t video_mem;
    void *vfmt;
    int current_w;
    int current_h;
} SDLVideoInfo;


typedef struct SDL_Surface
{
    uint32_t flags;              
    SDLPixelFormat *format;   
    int w, h;                   
    int pitch;                  
    void *pixels;    
    void *userdata;   
    int locked;                 
    void *lock_data;      
         
} SDLSurface;


SDLSurface* screen = NULL;
SDLSurface* realScreen = NULL;

void updateScreen(void)
{    

  //printf("size of pixel array %lu %d\n", (unsigned long)sizeof(realScreen->pixels), screen->pitch);  
  //printf("update screen %dx %d %d %ld\n", screen->w, screen->h, screen->pitch, sizeof(screen->pixels)/sizeof(screen->pixels[0]));
  //printf("update real screen %dx %d %d %ld\n", realScreen->w, realScreen->h, realScreen->pitch, sizeof(realScreen->pixels) );
  
  int pitch = screen->w * screen->format->BytesPerPixel;
  int dataWith = sizeof(screen->pixels[0]);

   
  

   int i;
    for(i=0; i < 240; i++) {
        int offset = 2 * i;      
        memcpy(((uint8_t *)realScreen->pixels) + (offset * pitch * dataWith ), ((uint8_t *)screen->pixels) + i * pitch * dataWith, pitch);
        memcpy(((uint8_t *)realScreen->pixels) + (offset + 1) * pitch * dataWith , ((uint8_t *)screen->pixels) + i *  pitch * dataWith, pitch);   
     
       //  memcpy(((int *)realScreen->pixels) + (offset + 1) * screen->pitch , ((int *)screen->pixels) + i *  screen->pitch, screen->pitch);   
    }  
   // printf("update screen finish\n");
   
}


void* SDL_GetVideoInfo() { 
  static void*(*info)() = NULL;
   printf("Call to SDL_GetVideoInfo intercepted\n");

  if (info == NULL) {   
    info = dlsym(RTLD_NEXT, "SDL_GetVideoInfo");
    assert(info != NULL);
  }

  SDLVideoInfo* inf = (SDLVideoInfo*)info();   //<-- calls SDL_GetVideoInfo();   
  int screenWidth = inf->current_w;
  int screenHeight = inf->current_h;

  if(screenWidth == 320 && screenHeight == 480) {
    printf("Call to SDL_GetVideoInfo %d x %d\n", screenWidth, screenHeight );
    printf("correcting to 320x240\n");
    inf->current_h = 240;
  } else {
     printf("Call to SDL_GetVideoInfo intercepted\n");
  }
  
  return inf;
}



void* SDL_SetVideoMode(int width, int height, int bitsperpixel, uint32_t flags) {
  static void*(*real_func)(int,int,int,uint32_t) = NULL;
  static void*(*info)() = NULL;
  static void*(*create)(uint32_t , int, int, int,uint32_t , uint32_t ,  uint32_t , uint32_t ) =NULL;

   printf("Call to SDL_SetVideoMode intercepted\n");
    

  if (real_func == NULL) {
    real_func = dlsym(RTLD_NEXT, "SDL_SetVideoMode");    
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

  const SDLVideoInfo* inf = (SDLVideoInfo*)info();   //<-- calls SDL_GetVideoInfo();   
  int screenWidth = inf->current_w;
  int screenHeight = inf->current_h;



  if( (screenWidth == 320 && screenHeight == 480 && width == 320 && height == 240) || (width == 320 && height ==480) || force )  {
    printf("Call to SDL_SetVideoMode %d x %d, corrected screen to 320x240\n", width, height );
    screen = (SDLSurface*)create(0, 320, 240, bitsperpixel, 0, 0, 0, 0);
    realScreen = (SDLSurface*)real_func(320, 480, bitsperpixel, flags);
    return screen;
  } else {
     printf("Call to SDL_SetVideoMode intercepted, real screen was used\n");
     realScreen = (SDLSurface*)real_func(width, height, bitsperpixel, flags);
  }

  
  return realScreen;
}

static void*(*flip)() = NULL;

void* SDL_Flip(void* surface) { 
 //  printf("Call to SDL_Flip intercepted\n");

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