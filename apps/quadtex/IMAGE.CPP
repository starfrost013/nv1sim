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
       Operations with image: set parameters, draw.

     History: 
       Eugene Lapidous    03/07-1995


****************************************************************************/

#include "sample.hpp"

extern NVACCESS NvAccess;
extern NVIMAGE NvImage;
extern NVGEOM NvGeom;


/***************************************************
 Verify and initialise image data for rendering.

 This sample expects 24-bit uncompressed DIB with
 binary sizes 4,8,16,32,64,128 or 256.
****************************************************/
BOOL
NvcInitImage ( char * szName )
{
    if ( NvcReadDIB ( szName ) == FALSE ) 
        return (FALSE);

    if (NvImage.npBmInfoPacked->biBitCount != 24 ||
        NvImage.npBmInfoPacked->biCompression != BI_RGB )
        return (FALSE);    


    NvImage.bytesPerPix = 3;
 /* DIB scanlines are always DWORD-aligned */
    NvImage.scanBytes = 
       ((NvImage.bytesPerPix * 
         NvImage.npBmInfoPacked->biWidth ) + 3) & 0xFFFFFFFC;


 /* Make sure that bitmap sizes are acceptable */
    switch ( NvImage.npBmInfoPacked->biWidth ) {

        case 4:
        case 8:
        case 16:
        case 32:
        case 64:
        case 128:
        case 256:
            break;

        default:
            return (FALSE);
     }


     switch ( NvImage.npBmInfoPacked->biHeight )    {

        case 4:
        case 8:
        case 16:
        case 32:
        case 64:
        case 128:
        case 256:
            break;

        default:
            return (FALSE);
    }

     return (TRUE);
}


/*********************************************
 Control point enumeration arrays for
 different combinations of startCorner (first)
 and minorCorner (last in the _NN suffix )
**********************************************/
uint32_t PntEnum_03[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
uint32_t PntEnum_01[] = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };
uint32_t PntEnum_10[] = { 2, 5, 8, 1, 4, 7, 0, 3, 6 };
uint32_t PntEnum_12[] = { 2, 1, 0, 5, 4, 3, 8, 7, 6 };
uint32_t PntEnum_21[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0 };
uint32_t PntEnum_23[] = { 8, 5, 2, 7, 4, 1, 6, 3, 0 };
uint32_t PntEnum_32[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };
uint32_t PntEnum_30[] = { 6, 7, 8, 3, 4, 5, 0, 1, 2 };


/****************************************
 Draw texture
*****************************************/
void 
NvcDrawImage () 
{

    int32_t  freeCount = NvAccess.free;
    NvChannel * npDev = NvAccess.npChannel;
    NvRenderTextureQuadratic * Tx = NvAccess.Texture;
    int32_t nMinor, nMajor;

    int32_t nMinStep[4], nMajStep, i, nStartShift;
    uint32_t * npPntEnum;
    char *npStartLine;

    uint32_t nOutcode = 0xffffffff;
    
    if ( NvImage.npBmInfoPacked == NULL )
       return;

    for ( i = 0; i < 9; i++ ) 
       nOutcode &= NvGeom.outCode[i];
       
  /* If all control points are in the semi-spaces
     outside the viewport - nothing to draw
  */   
    if ( nOutcode )
       return;   

  /* Calculate offset of the first pixel to draw
     and increments in minor and major directions
  */   
     switch ( NvGeom.startCorner ) {
        case 0:
            nStartShift = NvImage.scanBytes * 
                (NvImage.npBmInfoPacked->biHeight - 1);

            if ( NvGeom.minorCorner == 1 ) {
                nMinStep[0] = -NvImage.scanBytes;
                nMajStep = NvImage.bytesPerPix;
               npPntEnum = PntEnum_01;
            } else {
                 nMinStep[0] = NvImage.bytesPerPix;
                nMajStep = -NvImage.scanBytes;
               npPntEnum = PntEnum_03;
            }
        
        
            break;

        case 1:
            nStartShift = 0;  

            if ( NvGeom.minorCorner == 2 ) {
                 nMinStep[0] = NvImage.bytesPerPix;
                nMajStep = NvImage.scanBytes;
               npPntEnum = PntEnum_12;
            } else {
                nMinStep[0] = NvImage.scanBytes;
                nMajStep = NvImage.bytesPerPix;
               npPntEnum = PntEnum_10;
            }
            break;

        case 2:
            nStartShift = (NvImage.npBmInfoPacked->biWidth - 1) * 
                NvImage.bytesPerPix;

            if ( NvGeom.minorCorner == 3 ) {
                nMinStep[0] = NvImage.scanBytes;
                nMajStep = -NvImage.bytesPerPix;
               npPntEnum = PntEnum_23;
            } else {
                 nMinStep[0] = -NvImage.bytesPerPix;
                nMajStep = NvImage.scanBytes;
               npPntEnum = PntEnum_21;
            }                 
            break;
            
        case 3:
            nStartShift = NvImage.scanBytes * 
                NvImage.npBmInfoPacked->biHeight - 
                    NvImage.bytesPerPix;

            if ( NvGeom.minorCorner == 0 ) {
                 nMinStep[0] = -NvImage.bytesPerPix;
                nMajStep = -NvImage.scanBytes;
               npPntEnum = PntEnum_30;
            } else {
                nMinStep[0] = -NvImage.scanBytes;
                nMajStep = - NvImage.bytesPerPix;
               npPntEnum = PntEnum_32;
            }
            break;
    }
                                       
    nMinStep[1] = nMinStep[0] << 1;
    nMinStep[2] = nMinStep[1] + nMinStep[0];
    nMinStep[3] = nMinStep[1] << 1;

    if ( nMajStep == NvImage.scanBytes ||
       nMajStep == -NvImage.scanBytes ) {
       nMinor = NvImage.npBmInfoPacked->biWidth;
       nMajor = NvImage.npBmInfoPacked->biHeight;
    } else {   
       nMajor = NvImage.npBmInfoPacked->biWidth;
       nMinor = NvImage.npBmInfoPacked->biHeight;
    }   
 
  /* Set minor ( width ) and major ( height ) subdivision bits,
     corresponding to texture's width and height
  */
    NvImage.nSubdiv &= NV_TEXTURE_SIZE_MASK;

    switch ( nMinor ) {

        case 4:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_4;
            break;

        case 8:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_8;
            break;

        case 16:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_16;
            break;

        case 32:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_32;
            break;

        case 64:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_64;
            break;

        case 128:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_128;
            break;

        case 256:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_WIDTH_06_28_256;
            break;

        default:
            return;
     }


     switch ( nMajor )    {

        case 4:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_4;
            break;

        case 8:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_8;
            break;

        case 16:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_16;
            break;

        case 32:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_32;
            break;

        case 64:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_64;
            break;

        case 128:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_128;
            break;

        case 256:
            NvImage.nSubdiv |= NV_QTM_SUBDIVIDE_HEIGHT_02_68_256;
            break;

        default:
            return;
    }

    while ( freeCount < 10 * 4 )              
      freeCount = GET_FREE_COUNT (npDev, 2);              
    freeCount -= 10 * 4;    

  /* Set texture size and coordinates */
    Tx->SubdivideIn = NvImage.nSubdiv;

    Tx->ControlPointOut.y0_x0 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[0]]); 
    Tx->ControlPointOut.y1_x1 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[1]]); 
    Tx->ControlPointOut.y2_x2 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[2]]); 
    Tx->ControlPointOut.y3_x3 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[3]]); 
    Tx->ControlPointOut.y4_x4 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[4]]); 
    Tx->ControlPointOut.y5_x5 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[5]]); 
    Tx->ControlPointOut.y6_x6 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[6]]); 
    Tx->ControlPointOut.y7_x7 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[7]]); 
    Tx->ControlPointOut.y8_x8 = *(uint32_t *) &(NvGeom.viewPnt[npPntEnum[8]]); 

    npStartLine = NvImage.npBitmap + nStartShift;

  /* Render texture */
    while ( nMajor > 0 ) {              
       long iMinor = nMinor; 
       char * npColor;              

       npColor = npStartLine;

       while ( iMinor >= 4 ) {              
            while ( freeCount < 4 * 4 )              
               freeCount = GET_FREE_COUNT (npDev, 2);              
            freeCount -= 4 * 4;    
            
           /* Use consecutive addresses for Color to increase
              speed of bus writes; unroll loop to amortize
              flow control and counter checks
            */
            Tx->Color[0] = *(uint32_t *) npColor;           
            Tx->Color[1] = *(uint32_t *) (npColor + nMinStep[0]);           
            Tx->Color[2] = *(uint32_t *) (npColor + nMinStep[1]);           
            Tx->Color[3] = *(uint32_t *) (npColor + nMinStep[2]);          
            
            iMinor -= 4; 

            npColor +=  nMinStep[3];
       }

       npStartLine += nMajStep;              
       nMajor--;               
    } 
   
    NvAccess.free = freeCount;

    return;
}
                


/****************************************
 Free resources associated with an image
*****************************************/
void
NvcCloseImage () 
{

   if ( NvImage.npBmInfoPacked != NULL ) {
       GlobalFreePtr32 ( NvImage.npBmInfoPacked );
       NvImage.npBmInfoPacked = NULL;
       NvImage.npBitmap = NULL;
   }

   return;
}


