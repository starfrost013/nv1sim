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
       Clean-up routines: invalidate object names,
                            free resources

     History: 
       Eugene Lapidous    03/07-1995

****************************************************************************/

#include "sample.hpp"


extern NVCANVAS NvCanvas;
extern NVACCESS NvAccess;
extern NVOBJNAMES NvObjName;

/***********************************************************
*  Destroy instances of all objects in the patch
*
*  Order of entry:
*    objects common for two pipelines;
*    first pipeline;
*    second pipeline;
*
*  For every source object: 
*    allocate FIFO space;
*    destroy patchcord(s) connected to the output(s) of source object;
*    destroy source object
*
*  Last - destroy all objects that need deallocation of 
*  buffers in the global memory, making sure that no data
*  remain to be sent to these buffers.
*************************************************************/

static void
NvcDestroyObjectInstances (void)
{
    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;
    int32_t i;

 /************************************************* 
     Destroy objects common for both pipelines : 
 **************************************************/
 /* Video Sink */ 
    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);

    if ( NvObjName.VideoSink ) {
        npDev->subchannel[0].control.object  = NV_VIDEO_SINK;
        npDev->subchannel[0].nvClass.Destroy = NvObjName.VideoSink;
        freeCount -= 2 * 4;

        NvObjName.VideoSink = 0;
    }


 /* Video Switch */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);

    if ( NvObjName.PV_VideoSwitch ) {
        npDev->subchannel[1].control.object  = NV_PATCHCORD_VIDEO;
        npDev->subchannel[1].nvClass.Destroy = NvObjName.PV_VideoSwitch;
        freeCount -= 2 * 4;

        NvObjName.PV_VideoSwitch = 0;
    }

    if ( NvObjName.VideoSwitch ) {
        npDev->subchannel[2].control.object  = NV_VIDEO_SWITCH;
        npDev->subchannel[2].nvClass.Destroy = NvObjName.VideoSwitch;
        freeCount -= 2 * 4;

        NvObjName.VideoSwitch = 0;
    }


 /* Context ordinal */
    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);

    if ( NvObjName.ContextOrdinal ) { 
        npDev->subchannel[3].control.object  = NV_CONTEXT_ORDINAL;
        npDev->subchannel[3].nvClass.Destroy = NvObjName.ContextOrdinal;
        freeCount -= 2 * 4;

        NvObjName.ContextOrdinal = 0;
    }



 /************************************************* 
     Destroy objects in the first pipeline  
 **************************************************/
 /* Image To Video 1 */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);

    if ( NvObjName.PV_ImageToVideo1 ) {
        npDev->subchannel[0].control.object  = NV_PATCHCORD_VIDEO;
        npDev->subchannel[0].nvClass.Destroy = NvObjName.PV_ImageToVideo1;
        freeCount -= 2 * 4;

        NvObjName.PV_ImageToVideo1 = 0;
    }


    if ( NvObjName.ImageToVideo1 ) {
        npDev->subchannel[2].control.object  = NV_IMAGE_TO_VIDEO;
        npDev->subchannel[2].nvClass.Destroy = NvObjName.ImageToVideo1;
        freeCount -= 2 * 4;

        NvObjName.ImageToVideo1 = 0;
    }


 /* Render Solid Rectangle 1 */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);

    if ( NvObjName.PI_RenderSolidRectangle1 ) {
        npDev->subchannel[3].control.object  = NV_PATCHCORD_IMAGE;
        npDev->subchannel[3].nvClass.Destroy = NvObjName.PI_RenderSolidRectangle1;
        freeCount -= 2 * 4;

        NvObjName.PI_RenderSolidRectangle1 = 0;
    }

    if ( NvObjName.RenderSolidRectangle1 ) {
        npDev->subchannel[4].control.object  = NV_RENDER_SOLID_RECTANGLE;
        npDev->subchannel[4].nvClass.Destroy = NvObjName.RenderSolidRectangle1;
        freeCount -= 2 * 4;

        NvObjName.RenderSolidRectangle1 = 0;
    }


 /* Render Texture Quadratic 1 */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 5);

    if ( NvObjName.PI_RenderTextureQuadratic1 ) {
        npDev->subchannel[5].control.object  = NV_PATCHCORD_IMAGE;
        npDev->subchannel[5].nvClass.Destroy = NvObjName.PI_RenderTextureQuadratic1;
        freeCount -= 2 * 4;

        NvObjName.PI_RenderTextureQuadratic1 = 0;
    }

    if ( NvObjName.RenderTextureQuadratic1 ) {
        npDev->subchannel[6].control.object  = NV_RENDER_TEXTURE_QUADRATIC;
        npDev->subchannel[6].nvClass.Destroy = NvObjName.RenderTextureQuadratic1;
        freeCount -= 2 * 4;

        NvObjName.RenderTextureQuadratic1 = 0;
    }


 /************************************************* 
     Destroy objects in the second pipeline  
 **************************************************/
 /* Image To Video 2 */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);

    if ( NvObjName.PV_ImageToVideo2 ) {
        npDev->subchannel[0].control.object  = NV_PATCHCORD_VIDEO;
        npDev->subchannel[0].nvClass.Destroy = NvObjName.PV_ImageToVideo2;
        freeCount -= 2 * 4;

        NvObjName.PV_ImageToVideo2 = 0;
    }

    if ( NvObjName.ImageToVideo2 ) {
        npDev->subchannel[2].control.object  = NV_IMAGE_TO_VIDEO;
        npDev->subchannel[2].nvClass.Destroy = NvObjName.ImageToVideo2;
        freeCount -= 2 * 4;

        NvObjName.ImageToVideo2 = 0;
    }


 /* Render Solid Rectangle 2 */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 3);

    if ( NvObjName.PI_RenderSolidRectangle2 ) {
        npDev->subchannel[3].control.object  = NV_PATCHCORD_IMAGE;
        npDev->subchannel[3].nvClass.Destroy = NvObjName.PI_RenderSolidRectangle2;
        freeCount -= 2 * 4;

        NvObjName.PI_RenderSolidRectangle2 = 0;
    }

    if ( NvObjName.RenderSolidRectangle2 ) {
        npDev->subchannel[4].control.object  = NV_RENDER_SOLID_RECTANGLE;
        npDev->subchannel[4].nvClass.Destroy = NvObjName.RenderSolidRectangle2;
        freeCount -= 2 * 4;

        NvObjName.RenderSolidRectangle2 = 0;
    }


 /* Render Texture Quadratic 2 */
    while ( freeCount < 4 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 5);

    if ( NvObjName.PI_RenderTextureQuadratic2 ) {
        npDev->subchannel[5].control.object  = NV_PATCHCORD_IMAGE;
        npDev->subchannel[5].nvClass.Destroy = NvObjName.PI_RenderTextureQuadratic2;
        freeCount -= 2 * 4;

        NvObjName.PI_RenderTextureQuadratic2 = 0;
    }

    if ( NvObjName.RenderTextureQuadratic2 ) {
        npDev->subchannel[6].control.object  = NV_RENDER_TEXTURE_QUADRATIC;
        npDev->subchannel[6].nvClass.Destroy = NvObjName.RenderTextureQuadratic2;
        freeCount -= 2 * 4;

        NvObjName.RenderTextureQuadratic2 = 0;
    }


 /*********************************************************************
  Destroy all objects that need global memory buffer deallocation -
  notifiers last
 **********************************************************************/
    while ( freeCount < 14 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 0);

 /* Video Sink OS string */
    if ( NvObjName.VideoSinkOSstring ) {
        npDev->subchannel[0].control.object = NV_CONTEXT_DMA_FROM_MEMORY;
        npDev->subchannel[0].nvClass.Destroy = NvObjName.VideoSinkOSstring;
        freeCount -= 2 * 4;

        GlobalFreePtr32 (NvAccess.npVideoSinkOSstring);
        NvAccess.npVideoSinkOSstring = 0;

        NvObjName.VideoSinkOSstring = 0;
    }


  /* Image To Video OS string, common to all Image To Video objects, 
     destroyed last before notifiers. Notification on destruction  
     assures that all previous operations in the FIFO are processed 
     and it's possible to destroy notification buffers (no objects 
     with pending operations are in the zomby state)
  */
    npDev->subchannel[1].control.object = NV_CONTEXT_DMA_FROM_MEMORY;

    npDev->subchannel[1].nvClass.SetNotifyCtxDma = 
        NV_ERROR_NOTIFIER;
    NvAccess.npNotifier->status = NV_STATUS_IN_PROGRESS;
    npDev->subchannel[1].nvClass.SetNotify = NV_NOTIFY_WONT_SLEEP;

    npDev->subchannel[1].nvClass.Destroy = NvObjName.ImageToVideoOSstring;
    freeCount -= 4 * 4;

  /* Wait for notification */
    i = 0;
    while ( NvAccess.npNotifier->status == NV_STATUS_IN_PROGRESS ) {
  /* Protect from infinite loop */
        if ( i++ > 100000000 ) break;
    }

    GlobalFreePtr32 (NvAccess.npImageToVideoOSstring);
    NvAccess.npImageToVideoOSstring = 0;
    NvObjName.ImageToVideoOSstring = 0;


  /* Notifier Video Switch  ( wait for destruction notification ) */
    if ( NvObjName.NotifierVideoSwitch ) {
        npDev->subchannel[2].control.object = NV_CONTEXT_DMA_TO_MEMORY;

        npDev->subchannel[2].nvClass.SetNotifyCtxDma = 
            NV_ERROR_NOTIFIER;
        NvAccess.npNotifier->status = NV_STATUS_IN_PROGRESS;
        npDev->subchannel[2].nvClass.SetNotify = NV_NOTIFY_WONT_SLEEP;

        npDev->subchannel[2].nvClass.Destroy = NvObjName.NotifierVideoSwitch;
        freeCount -= 4 * 4;

       /* Wait for notification */
        i = 0;
        while ( NvAccess.npNotifier->status == NV_STATUS_IN_PROGRESS ) {
       /* Protect from infinite loop */
            if ( i++ > 100000000 ) break;
        }

        GlobalFreePtr32 (NvAccess.npNotifierVideoSwitch);
        NvAccess.npNotifierVideoSwitch = 0;

        NvObjName.NotifierVideoSwitch = 0;
    }


 /************************************************** 
   Destroy NV_NOTIFIER_ERROR object last, when 
    execution of methods on all previous objects is 
    confirmed and no more error messages are
    expected.
 ***************************************************/
    while ( freeCount < 2 * 4 ) 
        freeCount = GET_FREE_COUNT (npDev, 1);

 /* Notifier doesn't have a unique name - check pointer */
    if ( NvAccess.npNotifier != 0 ) {
        npDev->subchannel[1].control.object = NV_CONTEXT_DMA_TO_MEMORY;
        npDev->subchannel[1].nvClass.Destroy =    NV_ERROR_NOTIFIER;

        GlobalFreePtr32 (NvAccess.npNotifier);
        NvAccess.npNotifier = 0;
        freeCount -= 2 * 4;
    }

    NvAccess.free = freeCount;

    return;
}



/**************************************************
*  Destroy objects, close channel and free resources
***************************************************/
void           
NvcDestroy (void) 
{
    NvcDestroyObjectInstances();
                                                               
    if ( NvAccess.npChannel != NULL ) {
         NvClose(NvAccess.npChannel);        
         NvAccess.npChannel = NULL;
    }       

    return;
}