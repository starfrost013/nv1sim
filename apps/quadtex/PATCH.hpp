/***************************************************************************

     NVidia SDK Samples ver. 1.0. 
     Copyright (C) NVidia Corporation 1995.   All Rights Reserved.

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
       Patch descriprion and format of structure to hold
       names of all objects in the patch ( unique for
       every patch )

     History: 
       Eugene Lapidous    03/07-1995

****************************************************************************/

#ifndef PATCH_INCLUDED
#define PATCH_INCLUDED

/******************************************************
  Patch description:

  Simple example of patch that uses branches 
  connected through Video Switch to draw in each of 
  the two buffers independently.
  
  Double-buffering requires at least 2 separate
  rendering pipelines, each connected to it's own
  Image To Video object. Video Switch, using 
  Context Ordinal data, controls which pipeline
  is connected to display ( Video Sink ).

  Objects which  states change during rendering time 
  are shown in dash-frames, objects which states remain 
  unchanged are shown in dot-frames. Each
  connection of Patchcord Image to the input of
  an object is marked as PI, each Patchcord Video
  is marked as PV, Patchcord Rop is marked as PR. 

  Render rectangle objects are used to erase background.

        
  .----------------.    .----------------.      
  | Render Texture |    | Render Texture |      
  |   Quadratic 1  |    |   Quadratic 2  |      
  '----------------'    '----------------'      
     |                      |                   
     |                      |                   
     |                      |                   
     |  .-----------.       |     .-----------. 
     |  |  Render   |       |     |  Render   | 
     |  |Rectangle 1|       |     |Rectangle 2|    
     |  '-----------'       |     '-----------'    
     |           |          |          |        
    P|I         P|I        P|I        P|I       
     V           V          V          V           
  ..................    ..................              
  .Image To Video 1.    .Image To Video 2.              
  ..................    ..................              
           |                    |               
           |                    |               
           P|V                 P|V              
           V                    V               
  ..........................................    
  .             Video Switch               .    
  ..........................................    
               ^                
               .(No Patchcord)  
               .                
         .---------------.      
         |Context Ordinal|      
         '---------------'      
               
******************************************************************/

#ifndef NV_INCLUDED
#define NV_INCLUDED
#include <nvwin32.h>
#endif /* NV_INCLUDED */


/******************************************************* 
  Storage of unique names for object's instances
  in the patch. Each Patchcord is named by object
  which output is connected to it.

  Order of entries: 
      objects common for two pipelines;
      first pipeline;
      second pipeline;

  For every source object: 
      all new DMA objects associated with it;
      patchcord(s) connected to it's output;
      source object's name.
********************************************************/

typedef struct tag_NVOBJNAMES {
 /****************************** 
    Objects common for two
    pipelines 
 *******************************/
    uint32_t VideoSinkOSstring;
    uint32_t VideoSink;

    uint32_t PV_VideoSwitch;
 /* 
    Video Switch input change notification
    to check buffer flip status
 */
    uint32_t NotifierVideoSwitch;
    uint32_t VideoSwitch;

    uint32_t ContextOrdinal;

 /******************** 
   First pipeline    
 *********************/
  /* The same OS name for all pipelines */
    uint32_t ImageToVideoOSstring; 
    uint32_t PV_ImageToVideo1;
    uint32_t ImageToVideo1;

    uint32_t PI_RenderTextureQuadratic1;
    uint32_t RenderTextureQuadratic1;

    uint32_t PI_RenderSolidRectangle1;
    uint32_t RenderSolidRectangle1;

 /******************** 
   Second pipeline    
 *********************/
    uint32_t PV_ImageToVideo2;
    uint32_t ImageToVideo2;

    uint32_t PI_RenderTextureQuadratic2;
    uint32_t RenderTextureQuadratic2;

    uint32_t PI_RenderSolidRectangle2;
    uint32_t RenderSolidRectangle2;

} NVOBJNAMES;

#endif /* PATCH_INCLUDED */
