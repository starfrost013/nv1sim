/***************************************************************************

     NVidia SDK Samples ver. 1.0. 
     Copyright (C) NVidia Corporation 1995.    All Rights Reserved.

     Sample title: QUADTEX

     Sample description: 
       Interactive quadratic texture map renderer.
       
       Renders 1 quadratic texture patch ( 9 control points )
       using DIB test.bmp. Interaction with user: rotation,
       shift, deformation.
       
       Demonstrates: 
       3-pipeline patch ( 2 buffers separately and together );
       direct quadratic texture rendering;  
       interaction with user in double-buffered application;
       Z-order rendering of quadratic texture patch;
       clipping of quadratic texture patch.
       
       Interactivity included to observe variety of forms of
       a single patch and their subsets most suitable for z-order 
       rendering, margin clipping.


     File description:
       Structure and macro definitions 

     History: 
       Eugene Lapidous    03/07-1995

****************************************************************************/

#ifndef NVSAMPLE_INCLUDED
#define NVSAMPLE_INCLUDED


#include <windows.h>

/* macros from "windowsx.h adjusted to 32-bit API */
#define GlobalUnlockPtr32(lp)     \
    GlobalUnlock(GlobalHandle((LPCVOID)lp))

#define GlobalAllocPtr32(flags, cb)    \
    (GlobalLock(GlobalAlloc((flags), (cb))))

#define GlobalFreePtr32(lp)        \
    (GlobalUnlockPtr32(lp), (BOOL)GlobalFree(GlobalHandle((LPCVOID)lp)))


#ifndef NV_INCLUDED
#define NV_INCLUDED
#include <nvwin32.h>
#endif /* NV_INCLUDED */


#include "patch.h"

#define GET_FREE_COUNT(Ch, Sub) \
         (Ch)->subchannel[Sub].control.free

#define IN_PROGRESS 0xff

#define NV_TEXTURE_SIZE_MASK  0xFFFFFF00

typedef struct tag_NVCANVAS {
  HWND    hWnd;         /* Current window   */
  uint16_t    width;         /* Width and height of client area */
  uint16_t    height;
  uint32_t    halfSide;      /* Distance from viewport's center to the
                             closest side */
  uint32_t    hiddenBuffer; /* Current hidden buffer */
  BOOL    isActive;     /* Application's state */
  uint32_t    drawNext;     /* Next draw request counter */
  BOOL    eraseBackground; /* Permission to use Windows background
                              erase procedure
                            */
}NVCANVAS;


/***************************************************
  NVIMAGE structure stores image information:
  pointers to bitmaps and bitmap data,
  static parameters for image drawing
***************************************************/
typedef struct tag_NVIMAGE {
    BITMAPINFOHEADER * npBmInfoPacked;
    char * npBitmap;
    
    uint32_t bytesPerPix;
    uint32_t scanBytes;  /* Bytes per scanline */

    uint32_t nSubdiv;
} NVIMAGE;
    

/************************************************** 
 NVGEOM structure and associated structures store
 information related to the geometry database :
 coordinates in the model, world and view space,
 transformation parameters.
***************************************************/
#define MODEL_VIEW      0
#define MODEL_DEFORM      1

typedef struct tagNVCOORD3D {
   float x;
   float y;
   float z;
} NVCOORD3D;   

typedef struct tagNVINDEX3D {
   float xyz[3];
} NVINDEX3D;   

typedef union tagNVPOINT3D {
   NVCOORD3D c;
   NVINDEX3D n;
} NVPOINT3D;

typedef struct tagNVPOINT2D {
   short ix;
   short iy;
} NVPOINT2D;

typedef struct tagNVGEOM {
   uint32_t       dataMode;  /* View or deform */
   
   float      Scale;     /* Zoom factor */ 
   
   NVPOINT3D  Axle001;   /* [001] model axis in the world coordinates */
   
   NVPOINT3D  initPnt[9]; /* Initial database in the model coordinates */ 
   NVPOINT3D  trnsPnt[9]; /* Database in the world coordinates
                ( after transformation and scaling from initial model */

   uint32_t       outCode[9]; /* Cohen-Sutherland outcodes for clipping
                             sets of points against viewport */

   NVPOINT2D  viewShift;      
   NVPOINT2D  viewPnt[9]; /* Coordinates in the view space
                              high 16 bits - Y, low 16 bits - X */

/* Corners of the patch are enumerated as 0 3 , 
                                          1 2
   where correspondence corner -> quadratic control point is                           
   0 -> 0; 1 -> 2; 2 -> 8; 3 -> 6
*/
   uint32_t       startCorner;  /* Corner corresponding to minimal z -
                               start rendeing point  */
   uint32_t       minorCorner;  /* Corner corresponding to the end of the
                               first minor strip */ 
} NVGEOM;




/***************************************************
 NvAccess structure stores channel ID, free count,

 pointers to objects that are accessed at rendering
 time;
 
 pointers to DMA notification structures;

 pointers to all other resources that need to be 
 released at the end 
****************************************************/
typedef struct tag_NVACCESS  {
    NvChannel *npChannel;
    uint32_t free;             /* Storage of free count */

    NvNotification *npNotifier; /* Default notifier */

    NvRenderSolidRectangle *Rectangle;

    NvRenderTextureQuadratic *Texture;

    NvNotification *npNotifierVideoSwitch;
	 NvVideoSwitch *VideoSwitch;

    NvContextOrdinal *ContextOrdinal;

    char *npImageToVideoOSstring;

    char *npVideoSinkOSstring;

}NVACCESS;


void NvcSetRenderingPipeline (void);
void NvcSetHiddenBuffer (void);
void NvcGetCanvas (void);
BOOL NvcCreate (void);
BOOL NvcInit (void);
BOOL NvcDraw (void);
void NvcDestroy (void); 

BOOL NvcProcessUserInput ( UINT message, WPARAM wParam, LPARAM lParam );
BOOL NvcInitImage ( char * szName );
void NvcDrawImage (void); 
void NvcCloseImage (void); 
 
void NvcModelInit (void);
void NvcModelTransform (void);
void NvcModelDeform ( float fX, float fY );
void NvcViewTransform (void);
void NvcSort (void);

BOOL NvcReadDIB ( char * szName );

#endif /* NVSAMPLE_INCLUDED */

