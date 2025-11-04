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
        Initialisation of NVidia client and configuring it for
        rendering. File "patch.h" contains patch description and
        format for storage of object names.

     History: 
       Eugene Lapidous    03/07-1995

****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "sample.hpp"


/* Memory for global NV structures is allocated here. 
   All other files use external references.
*/

NVCANVAS NvCanvas;
NVACCESS NvAccess;
NVOBJNAMES NvObjName;
NVIMAGE NvImage;                                             
NVGEOM NvGeom;

uint32_t nColorBgnd;
float fSelectPointDistance = 0.5;
int32_t  nClipMargin = 0;

long nBitsPerPixel;


/************************************************************* 
*  Generate unique names for all objects in the patch
**************************************************************/
static void
NvcMakeUniqueNames (void)
{
     long  nObjNames, i;
     uint32_t *npName = (uint32_t *) &NvObjName;
    
     
     nObjNames = sizeof ( NVOBJNAMES ) >> 2; /* Number of all objects */


     for ( i = 1; i <= nObjNames; i++ ) {
         *npName = NV_NVIDIA_NAME_LIMIT + i;
         npName++;
     }

     return;
}



/*********************************************************************
*  Create instances of all objects in the patch,
*  connect patchcords to outputs of their source
*  objects.
*
*  Order of entry:
*    objects common for two pipelines;
*    first pipeline;
*    second pipeline;
*
*  For every source object: 
*    allocate FIFO space;
*    create all DMA objects associated with source object;
*    create patchcord(s) connected to the output(s) of source object;
*    create source object, select it, set associations with DMA objects;
*    connect patchcord(s) to the output(s).
*
***********************************************************************/

static BOOL
NvcCreateObjectInstances (void)
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;

    uint32_t dataSelector;

    dataSelector = NvGetFlatDataSelector ();

 /*********************************************************** 
     Set a default error notification buffer using
     already existing object NV_ERROR_NOTIFIER, to get error 
     code for every executed method. 
 ************************************************************/

    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 4 * 4;

    npDev->subchannel[0].control.object = NV_ERROR_NOTIFIER;

    if ( (NvAccess.npNotifier = GlobalAllocPtr32 ( GMEM_FIXED | GMEM_ZEROINIT, 
            sizeof ( NvNotification ) )) == NULL ) 
            return (FALSE);

    npDev->subchannel[0].contextDmaToMemory.SetDmaSpecifier.address[0] =
        (uint32_t) NvAccess.npNotifier;
    npDev->subchannel[0].contextDmaToMemory.SetDmaSpecifier.address[1] =
        dataSelector;
    npDev->subchannel[0].contextDmaToMemory.SetDmaSpecifier.limit =
        (sizeof(NvNotification) - 1);   


 /************************************************* 
     Create objects common to two pipelines  
 **************************************************/
  /* Video Sink  (Windows OS name for NV_VIDEO_SINK : "CON:") */
    while ( freeCount < 11 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);
    freeCount -= 11 * 4;

    npDev->subchannel[1].control.object = NV_CONTEXT_DMA_FROM_MEMORY;
    npDev->subchannel[1].nvClass.Create = NvObjName.VideoSinkOSstring;
    npDev->subchannel[1].control.object = NvObjName.VideoSinkOSstring;

    if ( (NvAccess.npVideoSinkOSstring = 
        GlobalAllocPtr32 (GMEM_FIXED | GMEM_ZEROINIT, 80)) == NULL ) 
            return (FALSE);
    sprintf((LPSTR)NvAccess.npVideoSinkOSstring, "CON:");

    npDev->subchannel[1].contextDmaFromMemory.SetDmaSpecifier.address[0] = 
        (uint32_t) NvAccess.npVideoSinkOSstring;
    npDev->subchannel[1].contextDmaFromMemory.SetDmaSpecifier.address[1] = 
        dataSelector; 
    npDev->subchannel[1].contextDmaFromMemory.SetDmaSpecifier.limit = 
        strlen ((LPSTR)NvAccess.npVideoSinkOSstring); 
    
    npDev->subchannel[2].control.object = NV_VIDEO_SINK;
 /* Name association has to be done before object is created */
    npDev->subchannel[2].nvClass.SetNameCtxDma = NvObjName.VideoSinkOSstring;
    npDev->subchannel[2].nvClass.SetNameStart = 0;        
    npDev->subchannel[2].nvClass.SetNameLength = 
        strlen ((LPSTR)NvAccess.npVideoSinkOSstring); 
    npDev->subchannel[2].nvClass.Create = NvObjName.VideoSink;


 /* Video Switch, input switch notification */
    while ( freeCount < 14 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);
    freeCount -= 14 * 4;

    npDev->subchannel[3].control.object = NV_CONTEXT_DMA_TO_MEMORY;
    npDev->subchannel[3].nvClass.Create = NvObjName.NotifierVideoSwitch;
    npDev->subchannel[3].control.object = NvObjName.NotifierVideoSwitch;

    if ( (NvAccess.npNotifierVideoSwitch = 
        GlobalAllocPtr32 (GMEM_FIXED | GMEM_ZEROINIT, 
            sizeof (NvNotification) ) ) == NULL ) 
            return (FALSE);

    npDev->subchannel[3].contextDmaToMemory.SetDmaSpecifier.address[0] =
        (uint32_t) NvAccess.npNotifierVideoSwitch;
    npDev->subchannel[3].contextDmaToMemory.SetDmaSpecifier.address[1] =
        dataSelector;
    npDev->subchannel[3].contextDmaToMemory.SetDmaSpecifier.limit =
        (sizeof(NvNotification) - 1);   

    npDev->subchannel[4].control.object = NV_PATCHCORD_VIDEO;
    npDev->subchannel[4].nvClass.Create = NvObjName.PV_VideoSwitch;

    npDev->subchannel[5].control.object = NV_VIDEO_SWITCH;
    npDev->subchannel[5].nvClass.Create = NvObjName.VideoSwitch;
    npDev->subchannel[5].control.object = NvObjName.VideoSwitch;

    npDev->subchannel[5].videoSwitch.SetVideoOutput = 
           NvObjName.PV_VideoSwitch;

  /* Use the same notifier for both inputs */
    npDev->subchannel[5].videoSwitch.SetSwitchNotifyCtxDma[0] = 
           NvObjName.NotifierVideoSwitch;

    npDev->subchannel[5].videoSwitch.SetSwitchNotifyCtxDma[1] = 
           NvObjName.NotifierVideoSwitch;


 /* Context Ordinal */
    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 6);
    freeCount -= 2 * 4;

    npDev->subchannel[6].control.object = NV_CONTEXT_ORDINAL;
    npDev->subchannel[6].nvClass.Create = NvObjName.ContextOrdinal;


 /************************************************* 
     Create objects in the first pipeline : 
 **************************************************/
 /* Image To Video1  */
    while ( freeCount < 15 * 4 )  
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 15 * 4;

    npDev->subchannel[0].control.object = NV_CONTEXT_DMA_FROM_MEMORY;
    npDev->subchannel[0].nvClass.Create = NvObjName.ImageToVideoOSstring;
    npDev->subchannel[0].control.object = NvObjName.ImageToVideoOSstring;

 /* Windows OS name for NV_IMAGE_TO_VIDEO : "WNDxxx:", where
     xxx is a unique windows handle - the same for all ImageToVideo objects
 */
    if ((NvAccess.npImageToVideoOSstring = 
        GlobalAllocPtr32 (GMEM_FIXED | GMEM_ZEROINIT, 80 )) == NULL ) 
            return (FALSE);
    sprintf((LPSTR)NvAccess.npImageToVideoOSstring, "WND%d:", NvCanvas.hWnd);

    npDev->subchannel[0].contextDmaFromMemory.SetDmaSpecifier.address[0] = 
        (uint32_t) NvAccess.npImageToVideoOSstring;
    npDev->subchannel[0].contextDmaFromMemory.SetDmaSpecifier.address[1] = 
        dataSelector; 
    npDev->subchannel[0].contextDmaFromMemory.SetDmaSpecifier.limit = 
        strlen ((LPSTR)NvAccess.npImageToVideoOSstring); 

    npDev->subchannel[1].control.object = NV_PATCHCORD_VIDEO;
    npDev->subchannel[1].nvClass.Create = NvObjName.PV_ImageToVideo1;

    npDev->subchannel[2].control.object = NV_IMAGE_TO_VIDEO;
    npDev->subchannel[2].nvClass.SetNameCtxDma = 
        NvObjName.ImageToVideoOSstring;
    npDev->subchannel[2].nvClass.SetNameStart = 0;        
    npDev->subchannel[2].nvClass.SetNameLength = 
        strlen ((LPSTR)NvAccess.npImageToVideoOSstring); 
    npDev->subchannel[2].nvClass.Create = NvObjName.ImageToVideo1;
    npDev->subchannel[2].control.object = NvObjName.ImageToVideo1;

    npDev->subchannel[2].imageToVideo.SetVideoOutput = 
           NvObjName.PV_ImageToVideo1;



 /* Render Solid Rectangle 1 */
    while ( freeCount < 6 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);
    freeCount -= 6 * 4;

    npDev->subchannel[3].control.object = NV_PATCHCORD_IMAGE;
    npDev->subchannel[3].nvClass.Create = NvObjName.PI_RenderSolidRectangle1;

    npDev->subchannel[4].control.object = NV_RENDER_SOLID_RECTANGLE;
    npDev->subchannel[4].nvClass.Create = NvObjName.RenderSolidRectangle1;
    npDev->subchannel[4].control.object = NvObjName.RenderSolidRectangle1;

    npDev->subchannel[4].renderSolidRectangle.SetImageOutput = 
           NvObjName.PI_RenderSolidRectangle1;

 /* Render Texture Quadratic 1 */
    while ( freeCount < 6 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 5);
    freeCount -= 6 * 4;

    npDev->subchannel[5].control.object = NV_PATCHCORD_IMAGE;
    npDev->subchannel[5].nvClass.Create = NvObjName.PI_RenderTextureQuadratic1;

    npDev->subchannel[6].control.object = NV_RENDER_TEXTURE_QUADRATIC;
    npDev->subchannel[6].nvClass.Create = NvObjName.RenderTextureQuadratic1;
    npDev->subchannel[6].control.object = NvObjName.RenderTextureQuadratic1;

    npDev->subchannel[6].renderTextureQuadratic.SetImageOutput = 
           NvObjName.PI_RenderTextureQuadratic1;


 /************************************************* 
     Create objects in the second pipeline : 
 **************************************************/
 /* Image To Video2  */
    while ( freeCount < 9 * 4 )  
        freeCount = GET_FREE_COUNT (npDev, 1);
    freeCount -= 9 * 4;

    npDev->subchannel[1].control.object = NV_PATCHCORD_VIDEO;
    npDev->subchannel[1].nvClass.Create = NvObjName.PV_ImageToVideo2;

    npDev->subchannel[2].control.object = NV_IMAGE_TO_VIDEO;
    npDev->subchannel[2].nvClass.SetNameCtxDma = 
        NvObjName.ImageToVideoOSstring;
    npDev->subchannel[2].nvClass.SetNameStart = 0;        
    npDev->subchannel[2].nvClass.SetNameLength = 
        strlen ((LPSTR)NvAccess.npImageToVideoOSstring); 
    npDev->subchannel[2].nvClass.Create = NvObjName.ImageToVideo2;
    npDev->subchannel[2].control.object = NvObjName.ImageToVideo2;

    npDev->subchannel[2].imageToVideo.SetVideoOutput = 
           NvObjName.PV_ImageToVideo2;


 /* Render Solid Rectangle 2 */
    while ( freeCount < 6 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);
    freeCount -= 6 * 4;

    npDev->subchannel[3].control.object = NV_PATCHCORD_IMAGE;
    npDev->subchannel[3].nvClass.Create = NvObjName.PI_RenderSolidRectangle2;

    npDev->subchannel[4].control.object = NV_RENDER_SOLID_RECTANGLE;
    npDev->subchannel[4].nvClass.Create = NvObjName.RenderSolidRectangle2;
    npDev->subchannel[4].control.object = NvObjName.RenderSolidRectangle2;

    npDev->subchannel[4].renderSolidRectangle.SetImageOutput = 
           NvObjName.PI_RenderSolidRectangle2;

 /* Render Texture Quadratic  2*/
    while ( freeCount < 6 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 5);
    freeCount -= 6 * 4;

    npDev->subchannel[5].control.object = NV_PATCHCORD_IMAGE;
    npDev->subchannel[5].nvClass.Create = NvObjName.PI_RenderTextureQuadratic2;

    npDev->subchannel[6].control.object = NV_RENDER_TEXTURE_QUADRATIC;
    npDev->subchannel[6].nvClass.Create = NvObjName.RenderTextureQuadratic2;
    npDev->subchannel[6].control.object = NvObjName.RenderTextureQuadratic2;

    npDev->subchannel[6].renderTextureQuadratic.SetImageOutput = 
           NvObjName.PI_RenderTextureQuadratic2;


    NvAccess.free = freeCount;

    return (TRUE);
}



/*******************************************************************
* Connect inputs of source objects to patchcords, 
* associate static objects    that don't use patchcords
*
* Entry order:
*      objects common to all pipelines, bottom to top
*    objects in the first pipeline, bottom to top
*      objects in the second pipeline, bottom to top
********************************************************************/
static void 
NvcConnectInputs (void)
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;


  /***********************************
     Objects common to two pipelines
  ************************************/
 /* Video Sink*/
    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 2 * 4;

    npDev->subchannel[0].control.object = 
           NvObjName.VideoSink;
    npDev->subchannel[0].videoSink.SetVideoInput[0] = 
           NvObjName.PV_VideoSwitch;


 /* Video Switch */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);
    freeCount -= 4 * 4;

    npDev->subchannel[1].control.object = 
           NvObjName.VideoSwitch;
    npDev->subchannel[1].videoSwitch.SetVideoInput[0] = 
           NvObjName.PV_ImageToVideo1;
    npDev->subchannel[1].videoSwitch.SetVideoInput[1] = 
           NvObjName.PV_ImageToVideo2;
    npDev->subchannel[1].videoSwitch.SetCtxOrdinal = 
           NvObjName.ContextOrdinal;


  /*************************************
     First pipeline
  **************************************/
 /* Image To Video 1 */
    while ( freeCount < 3 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 2);
    freeCount -= 3 * 4;

    npDev->subchannel[2].control.object = 
           NvObjName.ImageToVideo1;
    npDev->subchannel[2].imageToVideo.SetImageInput[0] = 
           NvObjName.PI_RenderSolidRectangle1;
    npDev->subchannel[2].imageToVideo.SetImageInput[1] = 
           NvObjName.PI_RenderTextureQuadratic1;


  /*************************************
     Second pipeline
  **************************************/
 /* Image To Video 2 */
    while ( freeCount < 3 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);
    freeCount -= 3 * 4;

    npDev->subchannel[3].control.object = 
           NvObjName.ImageToVideo2;
    npDev->subchannel[3].imageToVideo.SetImageInput[0] = 
           NvObjName.PI_RenderSolidRectangle2;
    npDev->subchannel[3].imageToVideo.SetImageInput[1] = 
           NvObjName.PI_RenderTextureQuadratic2;

    return;
}



/***********************************************
 Function assisting patchcord validation.
 Takes 4 * 2 FIFO bytes.
************************************************/
static BOOL 
NvcCheckValidation (V032 * npValidate)
{
    int32_t i = 0;

    *npValidate = 0;

  /* Wait for notification */
    while ( NvAccess.npNotifier->status == NV_STATUS_IN_PROGRESS ) {
  /* Protect from infinite loop */
        if ( i++ > 100000000 ) 
            return (FALSE);
    }

    if ( NvAccess.npNotifier->status != NV_STATUS_DONE_OK ) return (FALSE);
      else return (TRUE);
}

/**************************************
* Check validity of patchcords
*
* Full version: start from the 
* end of the pipeline and go back to
* find out where error happens;
*
* Short version (as shown): test ends of 
* branches of the rendering patch - it 
* checks validity of remaining branch
*
* Entry order:
*    objects in the first pipeline
*    objects in the second pipeline
**************************************/
static BOOL  
NvcValidatePatch (void) 
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;


 /******************** 
   First pipeline    
 *********************/
 /* Render Solid Rectangle 1 */
    while ( freeCount < 5 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 5 * 4;

    npDev->subchannel[0].control.object = 
           NvObjName.PI_RenderSolidRectangle1;
    npDev->subchannel[0].patchcordImage.SetNotifyCtxDma = 
        NV_ERROR_NOTIFIER;
    NvAccess.npNotifier->status = NV_STATUS_IN_PROGRESS;

    npDev->subchannel[0].patchcordImage.SetNotify = NV_NOTIFY_WONT_SLEEP;
    if ( NvcCheckValidation ( (V032 *)
         &(npDev->subchannel[0].patchcordImage.Validate)) == FALSE )
        return (FALSE);

 /* Render Texture Quadratic 1 */
    while ( freeCount < 5 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);
    freeCount -= 5 * 4;

    npDev->subchannel[1].control.object = 
           NvObjName.PI_RenderTextureQuadratic1;
    npDev->subchannel[1].patchcordImage.SetNotifyCtxDma = 
        NV_ERROR_NOTIFIER;
    NvAccess.npNotifier->status = NV_STATUS_IN_PROGRESS;

    npDev->subchannel[1].patchcordImage.SetNotify = NV_NOTIFY_WONT_SLEEP;
    if ( NvcCheckValidation ( (V032 *)
         &(npDev->subchannel[1].patchcordImage.Validate)) == FALSE )
        return (FALSE);


 /******************** 
   Second pipeline    
 *********************/
 /* Render Solid Rectangle 2 */
    while ( freeCount < 5 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 5 * 4;

    npDev->subchannel[0].control.object = 
           NvObjName.PI_RenderSolidRectangle2;
    npDev->subchannel[0].patchcordImage.SetNotifyCtxDma = 
        NV_ERROR_NOTIFIER;
    NvAccess.npNotifier->status = NV_STATUS_IN_PROGRESS;

    npDev->subchannel[0].patchcordImage.SetNotify = NV_NOTIFY_WONT_SLEEP;
    if ( NvcCheckValidation ( (V032 *)
         &(npDev->subchannel[0].patchcordImage.Validate)) == FALSE )
        return (FALSE);

 /* Render Texture Quadratic 2 */
    while ( freeCount < 5 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);
    freeCount -= 5 * 4;

    npDev->subchannel[1].control.object = 
           NvObjName.PI_RenderTextureQuadratic2;
    npDev->subchannel[1].patchcordImage.SetNotifyCtxDma = 
        NV_ERROR_NOTIFIER;
    NvAccess.npNotifier->status = NV_STATUS_IN_PROGRESS;

    npDev->subchannel[1].patchcordImage.SetNotify = NV_NOTIFY_WONT_SLEEP;
    if ( NvcCheckValidation ( (V032 *)
         &(npDev->subchannel[1].patchcordImage.Validate)) == FALSE )
        return (FALSE);

    NvAccess.free = freeCount;

    return (TRUE);
}




/********************************************************
*  Open NVidia direct access channel.
* 
*  Create, configure and validate patch for
*  NVidia direct access application
********************************************************/

BOOL 
NvcCreate(void)
{
    int32_t  freeCount = NvAccess.free;

    if ( (NvAccess.npChannel = (NvChannel FAR * ) NvOpen(NULL)) == NULL ) 
        return (FALSE);

    NvcMakeUniqueNames ();

    if ( NvcCreateObjectInstances () == FALSE )
        return (FALSE);

    NvcConnectInputs ();

#ifdef VALIDATE
    if ( NvcValidatePatch () == FALSE )
        return ( FALSE );
#endif /* Validate */

    NvAccess.free = freeCount;
    
    NvcModelInit ();

    return ( TRUE );
}



/*************************************************************
*    Initialize non-volatile states of source objects,
*  store copies of some volatile parameters for multiple reuse,
**************************************************************/

BOOL 
NvcInit (void)
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;

    HDC hDC;

 /* 
    If output frame buffer has a resolution of 8 bits per pixel,
    only 8-bit indexed color formats are acceptable, and the
    patch can't use blend operations. Generally,
    8-bit color resolution is not recommended
    for NVidia direct access applications.

    In this sample 24-bit input DIB is used for 16-bit and 
    32-bit output formats; 8-bit output    format is rejected 
    ( in real application it may need additional 8-bit input DIB, 
      or conversion of 24-bit DIB to the current palette ).
 */

    hDC = GetDC ( NvCanvas.hWnd );
    nBitsPerPixel = GetDeviceCaps ( hDC, BITSPIXEL );
    if ( nBitsPerPixel < 16 )
         return (FALSE);
    ReleaseDC ( NvCanvas.hWnd, hDC );

          
 /* Context Ordinal */
    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 2 * 4;

    npDev->subchannel[0].control.object = NvObjName.ContextOrdinal;
    npDev->subchannel[0].contextOrdinal.SetOrdinal = 1 - NvCanvas.hiddenBuffer;

 /* Rectangle */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);
    freeCount -= 4 * 4;

     /* White color ( used as a background ) */
    nColorBgnd = 0x0000ffff; /* White */

    npDev->subchannel[1].control.object =  NvObjName.RenderSolidRectangle1;
        /* 16-bit color, alpha disabled */
    npDev->subchannel[1].renderSolidRectangle.SetColorFormat =  
        NV_COLOR_FORMAT_LE_X17R5G5B5;
    npDev->subchannel[1].control.object = NvObjName.RenderSolidRectangle2;
    npDev->subchannel[1].renderSolidRectangle.SetColorFormat =  
        NV_COLOR_FORMAT_LE_X17R5G5B5;

 /* Texture */
    /*  
      Set maximal numbers of subdivisions of surrounding edges
      to supress gap filling
    */
    NvImage.nSubdiv = NV_QTM_SUBDIVIDE_HEIGHT_02_68_256 |
                        NV_QTM_SUBDIVIDE_WIDTH_06_28_256|
                        NV_QTM_SUBDIVIDE_EDGE_02_256    |
                        NV_QTM_SUBDIVIDE_EDGE_68_256    |
                        NV_QTM_SUBDIVIDE_EDGE_06_256    |
                        NV_QTM_SUBDIVIDE_EDGE_28_256;

    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 2);
    freeCount -= 4 * 4;

    npDev->subchannel[2].control.object =  NvObjName.RenderTextureQuadratic1;
        /* 24-bit color, alpha disabled */
    npDev->subchannel[2].renderTextureQuadratic.SetColorFormat =  
        NV_COLOR_FORMAT_LE_X8R8G8B8;
    npDev->subchannel[2].control.object = NvObjName.RenderTextureQuadratic2;
    npDev->subchannel[2].renderTextureQuadratic.SetColorFormat =  
        NV_COLOR_FORMAT_LE_X8R8G8B8;


 /***************************************************
  Assign source objects that need to be accessed at 
  rendering time to subchannels; set pointers
  in the NvAccess structure.
 ****************************************************/   
  /* Object common to two pipelines */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);
    freeCount -= 4 * 4;

    npDev->subchannel[0].control.object = NvObjName.ContextOrdinal;
    NvAccess.ContextOrdinal = &(npDev->subchannel[0].contextOrdinal);

    npDev->subchannel[3].control.object = NvObjName.VideoSwitch;
    NvAccess.VideoSwitch = &(npDev->subchannel[3].videoSwitch);

    NvAccess.free = freeCount;

 /* Assign pointers for objects in the pipeline to subchannels */
    NvAccess.Rectangle = &(npDev->subchannel[1].renderSolidRectangle);
    NvAccess.Texture = &(npDev->subchannel[2].renderTextureQuadratic);

 /* Assign subchannels to objects in the current pipeline */
    NvcSetRenderingPipeline ();

    NvAccess.npNotifierVideoSwitch->status = NV_STATUS_DONE_OK;


    return (TRUE);
}

