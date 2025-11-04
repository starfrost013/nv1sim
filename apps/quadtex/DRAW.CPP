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
       Interaction with user, main drawing procedure,
       rendering pipeline and drawing buffer control.
                          

     History: 
       Eugene Lapidous    03/07-1995

****************************************************************************/

#include "sample.hpp"
#include <math.hpp>

extern NVCANVAS NvCanvas;
extern NVACCESS NvAccess;
extern NVOBJNAMES NvObjName;
extern NVGEOM NvGeom;
extern long nBitsPerPixel;

BOOL doSort = TRUE;
BOOL selectPoint = TRUE;

/****************************************************
* Set subchannels to use objects in the pipeline 
* that is connected to the hidden buffer.
****************************************************/
void 
NvcSetRenderingPipeline ()
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;

    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 2 * 4;

    if ( NvCanvas.hiddenBuffer == 0 ) {            
        npDev->subchannel[1].control.object = 
            NvObjName.RenderSolidRectangle1;

        npDev->subchannel[2].control.object = 
            NvObjName.RenderTextureQuadratic1;
    } else {
        npDev->subchannel[1].control.object = 
            NvObjName.RenderSolidRectangle2;

        npDev->subchannel[2].control.object = 
            NvObjName.RenderTextureQuadratic2;
    }

    NvAccess.free = freeCount;

    return;
}    


/******************************************************************
 Interaction with user: 
      HOME: reset initial size and position
      PAGE UP, PAGE DOWN - zoom up, down,
      INSERT - enable z-order rendering, 
      DELETE - disable z-order rendering.
                   
      Mouse: 
        LEFT BUTTON DOWN:
           move [001] model normal in the world space
                       
           SHIFT pressed:
               shift in the viewport with 2x cursor speed
               
        RIGHT BUTTON DOWN:
           move closest control point in the XY plane
               
 Returns:
        TRUE - image redraw is needed;
        FALSE - image redraw isn't needed.                        
*******************************************************************/                         
BOOL
NvcProcessUserInput ( UINT message, WPARAM wParam, LPARAM lParam )
{
   POINT cursorPos;
   uint32_t inputMode;
   float fX, fY;

#define INPUT_DEFORM  0
#define INPUT_SHIFT   1
#define INPUT_ROTATE  2
#define INPUT_AXIS    3

   if ( NvCanvas.width < 2 || NvCanvas.height < 2 )
       return (FALSE);

   switch ( message ) {
       case WM_KEYDOWN: {
            switch ( wParam ) {
        /* Reset coordinates */
               case VK_HOME:
                   NvcModelInit ();
                   return (TRUE);
                   
        /* Zoom up */
               case VK_PRIOR:
                   NvGeom.Scale *= 1.03;
                   break;
                   
        /* Zoom down */
               case VK_NEXT:
                   NvGeom.Scale /= 1.03;
                   break;
                   
        /* Enable z-order rendering */
               case VK_INSERT:
                   doSort = TRUE;
                   return (FALSE);
                   
        /* Disable z-order rendering */
               case VK_DELETE:
                   doSort = FALSE;
                   NvGeom.startCorner = 0;
                   NvGeom.minorCorner = 3;
                   return (FALSE);
                   
               default:
                   return (FALSE);
            }      
            NvcModelTransform (); 
            return (TRUE);
       }           

       case WM_RBUTTONDOWN:
            inputMode = INPUT_DEFORM;
            selectPoint = TRUE;
            break;
       
       case WM_LBUTTONDOWN:
            if ( wParam & MK_SHIFT ) 
               inputMode = INPUT_SHIFT;
             else if ( NvGeom.dataMode == MODEL_DEFORM ){
   /* Deformation resets Axis001 to [001] - reset
      cursor to the center */
               cursorPos.x = NvCanvas.width >> 1;
               cursorPos.y = NvCanvas.height >> 1;
               ClientToScreen ( NvCanvas.hWnd, &cursorPos );
               SetCursorPos ( cursorPos.x, cursorPos.y );
               return (FALSE);
            } else   
               inputMode = INPUT_AXIS;
            break;   
       
       case WM_MOUSEMOVE:  
            selectPoint = FALSE;
            if ( wParam & MK_RBUTTON )
               inputMode = INPUT_DEFORM;
             else if ( wParam & MK_LBUTTON ) {
               if ( wParam & MK_SHIFT ) 
                  inputMode = INPUT_SHIFT;
                else
                  inputMode = INPUT_AXIS;
            } else
               return (FALSE);
            break;
            
       default:
            return (FALSE);
   }
   
 /* Get cursor position in a uniform way - meaning of data in 
    lParam differs for different mouse messages 
 */
   GetCursorPos ( &cursorPos );
   ScreenToClient ( NvCanvas.hWnd, &cursorPos );
   
 /* Set geometry inputs and call model processing procedure */
   switch ( inputMode ) {

      case INPUT_DEFORM: 
          fX = (cursorPos.x - (NvCanvas.width / 2) - 
                 NvGeom.viewShift.ix) / (float) (NvCanvas.halfSide);
          fY = (cursorPos.y - (NvCanvas.height / 2) - 
                 NvGeom.viewShift.iy) / (float) (NvCanvas.halfSide);
               
          NvcModelDeform (fX, fY);
          break;
       
      case INPUT_SHIFT:
   /* Make shift 2 times larger than motion to test clip */   
          NvGeom.viewShift.ix = (cursorPos.x << 1) - NvCanvas.width;
          NvGeom.viewShift.iy = (cursorPos.y << 1) - NvCanvas.height;
          break;
          

      case INPUT_AXIS: {
          float fCos;
   /* X - asimuth xz, Y - elevation y */

          fX = (cursorPos.x - (NvCanvas.width / 2)) / 
               (float) (NvCanvas.width / 2);
          fY = (cursorPos.y - (NvCanvas.height / 2)) / 
               (float) (NvCanvas.height / 2);
 
          NvGeom.Axle001.c.x = sin ( 3.141592 * fX );
          NvGeom.Axle001.c.z = cos ( 3.141592 * fX );
   
          fCos = cos ( 3.141592 * fY );
          NvGeom.Axle001.c.x *= fCos;
          NvGeom.Axle001.c.z *= fCos;
          
          NvGeom.Axle001.c.y = sin ( 3.141592 * fY );

          NvcModelTransform (); 
          break;
      }   
    }

    return (TRUE);
}   
               
               
                  
   

/***************************************************************
* Set hidden buffer without releasing control 
* ( used when allpication becomes active )
****************************************************************/
void 
NvcSetHiddenBuffer ()
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;
    int32_t i;

  /* May be called after PostQuitMessage */
    if ( nBitsPerPixel < 16 ) return;

    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 2 * 4;

    NvAccess.npNotifierVideoSwitch->status = NV_STATUS_IN_PROGRESS;
    NvAccess.VideoSwitch->SetSwitchNotify[1 - NvCanvas.hiddenBuffer] = 
        NV_NOTIFY_WONT_SLEEP;

    NvAccess.ContextOrdinal->SetOrdinal = 1 - NvCanvas.hiddenBuffer;

  /* Wait for notification */
    i = 0;
    while ( NvAccess.npNotifierVideoSwitch->status == 
        NV_STATUS_IN_PROGRESS ) {
  /* Protect from infinite loop */
        if ( i++ > 1000000 ) 
            return;
    }

    NvAccess.free = freeCount;

    return;
}



/***************************************************************
* Sample draw procedure.
*
* Erase hidden buffer ( rectangle );
* Sort data for rendering, transform to the view space
* and draw image.
****************************************************************/
BOOL
NvcDraw (void)
{
    int32_t  freeCount;
    NvChannel * npDev = NvAccess.npChannel;

    NvRenderSolidRectangle * nvRect = NvAccess.Rectangle;
    extern uint32_t nColorBgnd;

  /* May be called after PostQuitMessage */
    if ( nBitsPerPixel < 16 ) 
       return (FALSE);

  /* Temporarily release control while waiting for buffer flip */
    if ( NvAccess.npNotifierVideoSwitch->status == NV_STATUS_IN_PROGRESS ) 
        return ( TRUE );
    if ( NvAccess.npNotifierVideoSwitch->status != NV_STATUS_DONE_OK )
        return ( FALSE );

  /* Decrease draw counter - maximum 2 rendering requests for
     a user input event */
    if ( NvCanvas.drawNext > 0 )  NvCanvas.drawNext--;    

  /* Reset rendering pipeline for a new hidden buffer */
    NvcSetRenderingPipeline ();

    freeCount = NvAccess.free;

  /* Draw rectangle to erase hidden buffer */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 4 * 4;

    nvRect->Color = nColorBgnd;
    nvRect->Rectangle[0].y_x = 0;
    nvRect->Rectangle[0].height_width = * ((uint32_t*) &(NvCanvas.width));

  /* Transform, sort and render */
    if ( doSort == TRUE ) NvcSort ();
    NvcViewTransform ();
    NvcDrawImage ();

    if ( NvCanvas.isActive == TRUE ) {
  /* Ask for a buffer flip with notification */
       while ( freeCount < 2 * 4 ) 
            freeCount = GET_FREE_COUNT (npDev, 0);
       freeCount -= 2 * 4;

       NvAccess.npNotifierVideoSwitch->status = NV_STATUS_IN_PROGRESS;
       NvAccess.VideoSwitch->SetSwitchNotify[1 - NvCanvas.hiddenBuffer] = 
           NV_NOTIFY_WONT_SLEEP;

       NvCanvas.hiddenBuffer = 1 - NvCanvas.hiddenBuffer;
       NvAccess.ContextOrdinal->SetOrdinal = 1 - NvCanvas.hiddenBuffer; 
    }    
 
    NvAccess.free = freeCount;

    return (TRUE);
}

