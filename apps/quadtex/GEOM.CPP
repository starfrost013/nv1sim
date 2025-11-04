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
       Operations on geometry database: transformations, sorting.
       Global coordinate system : X from left to right of the screen
                                  Y from top to bottom of the screen
                                  Z from the viewer behind the screen
       ( positive direction defined by right-hand rule )                           

     History: 
        Eugene Lapidous    03/07-1995

****************************************************************************/

#include "sample.hpp"
#include <cmath>

extern NVCANVAS NvCanvas;
extern NVIMAGE NvImage;
extern NVGEOM NvGeom;

extern float fSelectPointDistance;
extern int32_t  nClipMargin;

/*********************************************
 Simple geometry initialisation: flat 
 square boundary with elevation of the center
 point
**********************************************/ 
void 
NvcModelInit ()
{
   NvGeom.Scale = 1.;
   
   NvGeom.Axle001.c.x = 0;
   NvGeom.Axle001.c.y = 0;
   NvGeom.Axle001.c.z = 1;
       
   NvGeom.viewShift.ix = 0;
   NvGeom.viewShift.iy = 0;
   
  /* X, Y - square centered at 0, 0;
     Z = 0 for all points except center (4) */
   NvGeom.initPnt[0].c.x = -0.8;
   NvGeom.initPnt[0].c.y = -0.8;
   NvGeom.initPnt[0].c.z = 0;

   NvGeom.initPnt[1].c.x = -0.8;
   NvGeom.initPnt[1].c.y = 0;
   NvGeom.initPnt[1].c.z = 0;

   NvGeom.initPnt[2].c.x = -0.8;
   NvGeom.initPnt[2].c.y = 0.8;
   NvGeom.initPnt[2].c.z = 0;

   NvGeom.initPnt[3].c.x = 0;
   NvGeom.initPnt[3].c.y = -0.8;
   NvGeom.initPnt[3].c.z = 0;

   NvGeom.initPnt[4].c.x = 0;
   NvGeom.initPnt[4].c.y = 0;
   NvGeom.initPnt[4].c.z = -0.8;

   NvGeom.initPnt[5].c.x = 0;
   NvGeom.initPnt[5].c.y = 0.8;
   NvGeom.initPnt[5].c.z = 0;

   NvGeom.initPnt[6].c.x = 0.8;
   NvGeom.initPnt[6].c.y = -0.8;
   NvGeom.initPnt[6].c.z = 0;

   NvGeom.initPnt[7].c.x = 0.8;
   NvGeom.initPnt[7].c.y = 0;
   NvGeom.initPnt[7].c.z = 0;

   NvGeom.initPnt[8].c.x = 0.8;
   NvGeom.initPnt[8].c.y = 0.8;
   NvGeom.initPnt[8].c.z = 0;
   
  /* Set data in the world space */
   NvcModelTransform ();
   
   return;
}   
   

/******************************************************
   Transform all points of the object in the 3D space:
   
   1) Rotate object around vector normal to
      the Axle001 and to [001] coordinate axis of initial
      model space, in order to align [001] model
      space coordinate axis and Axle001.
      
   2) Apply scale factor   

   Calculate 3 * 3 matrix for parallel 
   transformation without shift and apply it to all points 
   in the  initial database (initPnt); store results in 
   (irnsPnt). 
*******************************************************/
void 
NvcModelTransform ( )
{

  int32_t iRow, iCol, iPnt;
  float Matrix[3][3];         /* [Row][Column] */
  float x, y, z, x2, y2, xy, sX, sY;
  float fCos, f1_Cos, fSin, fLngth;
  

  NvGeom.dataMode = MODEL_VIEW; 

 /* Normal to Axle001 and 001 as normalized vector product*/
  x = NvGeom.Axle001.c.y;
  y = - NvGeom.Axle001.c.x;
  z = 0;

  fLngth = sqrt ( x * x + y * y );
  if ( fLngth ) {
          x /= fLngth;
        y /= fLngth;
  } else {
 // direction doesn't matter - no rotation
        x = 1;
        y = 0;
  }


 /* Rotation angle around [xyz] */     
  fCos = NvGeom.Axle001.c.z; // projection of normalised vector
  f1_Cos = 1 - fCos;
  fSin = sqrt ( 1. - fCos * fCos );


 /* Rotation around vector - see Foley & vanDam, 2nd ed. p.227 */
  xy = x * y * f1_Cos;

  x2 = x * x;
  y2 = y * y;

  x *= fSin;
  y *= fSin;

  sX = x2 + fCos * ( 1. - x2 );
  sY = y2 + fCos * ( 1. - y2 );

  Matrix[0][0] = sX;
  Matrix[0][1] = xy; 
  Matrix[0][2] = y; 

  Matrix[1][0] = xy;
  Matrix[1][1] = sY;  
  Matrix[1][2] = -x;

  Matrix[2][0] = -y;
  Matrix[2][1] = x;   
  Matrix[2][2] = fCos;


 /* Scaling */
  if ( NvGeom.Scale != 1. ) {
       for ( iRow = 0; iRow < 3; iRow++ ) {
              for ( iCol = 0; iCol < 3; iCol++ ) {
                    Matrix[iRow][iCol] *= NvGeom.Scale;
              }                  
       }
  }

 /* Transform coordinates */
  for ( iPnt = 0; iPnt < 9; iPnt++ ) {
       long iRow = 2;

       while ( iRow >= 0 ) {
         NvGeom.trnsPnt[iPnt].n.xyz[iRow] = 
            Matrix[iRow][0] * NvGeom.initPnt[iPnt].n.xyz[0] +
            Matrix[iRow][1] * NvGeom.initPnt[iPnt].n.xyz[1] +
            Matrix[iRow][2] * NvGeom.initPnt[iPnt].n.xyz[2];

         iRow--;
       }
  }

  return;
}


/************************************************
 Reset initial data base to a transformed one
 ( point Axle001 in [001] direction ), find 
 point with projection closest to selected fX, fY
 coordinates and move it in XY plane to align
 it's projection with selected coordinates
*************************************************/ 
void NvcModelDeform ( float fX, float fY )
{
  int32_t i;
  static float fSelectedLength = 1.e9;
  static uint32_t nSelectedPoint;
  extern BOOL selectPoint;

 /* When switching to deformation mode -
    reset initial data to a data after
    last transformation.
 */   

  if ( NvGeom.dataMode == MODEL_VIEW ) {
      int32_t j;
  
      NvGeom.dataMode = MODEL_DEFORM;

      for ( i = 0; i < 9; i++ ) {
           for ( j = 0; j < 3; j++ )
               NvGeom.initPnt[i].n.xyz[j] = 
                  NvGeom.trnsPnt[i].n.xyz[j];
      }           
                  
      NvGeom.Axle001.c.x = 0;
      NvGeom.Axle001.c.y = 0;
      NvGeom.Axle001.c.z = 1;
       
      NvGeom.Scale = 1.;
      
   }    
  

 /* Get number of the point which projection
    on the XY plane is closest to a 
    selected coordinates
 */    
  if ( selectPoint == TRUE ) {

     nSelectedPoint = 0;
     fSelectedLength = 1.e9;
     for ( i = 0; i < 9; i++ ) {
         float fLength;
         float dX, dY;
      
         dX =  NvGeom.initPnt[i].c.x - fX;
         dY =  NvGeom.initPnt[i].c.y - fY;
  
         fLength = sqrt ( dX * dX + dY * dY );
                        
         if ( fLength < fSelectedLength ) {
             fSelectedLength = fLength;
             nSelectedPoint = i; 
         }
     }
  }   
  
 /* If selected coordinates are close enough - snap
    X, Y coordinates of the closest point to the 
    selected ones 
 */
  if ( fSelectedLength < fSelectPointDistance ) {
      NvGeom.initPnt[nSelectedPoint].c.x = fX;
      NvGeom.initPnt[nSelectedPoint].c.y = fY;  
      
      NvGeom.trnsPnt[nSelectedPoint].c.x = fX;
      NvGeom.trnsPnt[nSelectedPoint].c.y = fY;   
  }
  
  return;
}                             
        

/********************************************************** 
 Parallel projection of an object data to the viewport
 and outcode calculation
***********************************************************/
void 
NvcViewTransform ( )
{
  int32_t iPnt;
  int32_t maxClipX, maxClipY;

 /* Visibility of the texture in the viewport may be determined
    using Cohen-Sutherland outcodes: patch that has non-zero
    intersection (AND) of it's outcodes is invisible. If all
    borders of the patch are straight lines, outcodes may be
    calculated against viewport rectangle; for curved borders
    some margin of error has to be added ( nClipMargin ), which
    depends on the constraints of the patch geometry.
    
    This sample allows to interactively change shape of the patch 
    and experiment with the relationship of patch geometry and
    clip check conditions.
 */
  maxClipX = (NvCanvas.width >> 1) + nClipMargin;
  maxClipY = (NvCanvas.height >> 1) + nClipMargin;

  for ( iPnt = 0; iPnt < 9; iPnt++ ) {
        int32_t nX, nY;

        NvGeom.outCode[iPnt] = 0;

        nX = NvGeom.trnsPnt[iPnt].c.x * NvCanvas.halfSide;
               
        if ( nX > maxClipX )                 
                NvGeom.outCode[iPnt] += 2;
         else if ( nX < - maxClipX )            
                NvGeom.outCode[iPnt] += 1;

      /* Position in the center of the viewport */
        nX += NvCanvas.width >> 1;

      /* Clamp to 16-bit range to avoid sign change
         when sending pairs od 16-bit coordinates to
         hardware
      */   
       if ( nX < -32767 ) nX = -32767;         
        else if ( nX > 32767 ) nX = 32767;
      
 
       nY = NvGeom.trnsPnt[iPnt].c.y * NvCanvas.halfSide;
               
       if ( nY > maxClipY ) 
               NvGeom.outCode[iPnt] += 4;
        else if ( nY < - maxClipY )            
                NvGeom.outCode[iPnt] += 8;

       nY += NvCanvas.height >> 1;

       if ( nY < -32767 ) nY = -32767; 
        else if ( nY > 32767 ) nY = 32767;
        
       NvGeom.viewPnt[iPnt].ix = nX + NvGeom.viewShift.ix;
       NvGeom.viewPnt[iPnt].iy = nY + NvGeom.viewShift.iy;
  }
  
  return;
}  



/****************************************
   Sort corners of the texture map in 
   z-order to set start corner ( maximal z )
   and minor corner ( maximal z of 2 corners 
   adjacent to the start corner ). 
   
   First minor strip of texture map is 
   rendered from the start corner ( control point 
   re-enumerated to 0 ) to the minor corner 
   ( control point re-enumerated to 6 ).
   
   Drawing in z-order assures that in cases
   when image of texture map is folded, texels
   representing surface points closer to the
   viewer ( with smaller z ) are drawn over 
   texels representing surface points further
   from the viewer ( with larger z ), 
   therefore providing automatic implementation 
   of painter algorithm for each texel.
   
   However, this approach works only for
   a subset of quadratic texture maps with 
   some restrictions on the mutual positions
   of control points.
   
   Free-form manual deformation feature
   of this sample allows to experiment with
   different configurations of control points
   to choose which restriction user wants
   to impose on the model to assure correct
   drawing order.  
************************************************/ 
void 
NvcSort ()
{
    int32_t nLeft, nRight, nNext, j;
   float zMaxCorner, zCorner[4];

/*
   Store z - coordinates for 4 corners (circular enumeration):
                                                           0  3
                                                           1  2
    ( correspondence corner -> vertice of quadratic texture: 
      0 -> 0; 1 -> 2; 2 -> 8; 3 -> 6 )
*/
      zCorner[0] = NvGeom.trnsPnt[0].c.z;
      zCorner[1] = NvGeom.trnsPnt[2].c.z;
      zCorner[2] = NvGeom.trnsPnt[8].c.z;
      zCorner[3] = NvGeom.trnsPnt[6].c.z;

/* Find corner with minimal z */
      zMaxCorner = zCorner[0];
      NvGeom.startCorner = 0;

      for ( j = 1; j < 4; j++ ) {
        if ( zCorner[j] > zMaxCorner ) {
            zMaxCorner = zCorner[j];
            NvGeom.startCorner = j;
        }
      }

/* 
   Find neighbour vertice for minor stripes
    (with second highest z)
*/
      if ( (nRight = NvGeom.startCorner - 1) < 0 )
        nRight = 3;

      if ( (nLeft = NvGeom.startCorner + 1) > 3 )
        nLeft = 0;

      if ( zCorner[nLeft] > zCorner[nRight] ) {
         if ( zCorner[NvGeom.startCorner] > zCorner[nLeft] ) {
              NvGeom.minorCorner = nLeft;
         } else {   
 /* 
    if min z is the same for the left vertice - compare it's
     left neighbour with nRight and    choose one with lowest z
     for major direction    
 */     
              if ( (nNext = nLeft + 1) > 3 ) 
                nNext = 0;

              if ( zCorner[nNext] > zCorner[nRight] ) {
                NvGeom.minorCorner = NvGeom.startCorner;
                NvGeom.startCorner = nLeft;
              } else {
                NvGeom.minorCorner = nLeft;
              }
         }
      } else if ( zCorner[nLeft] < zCorner[nRight] ) {
         if ( zCorner[NvGeom.startCorner] > zCorner[nRight] ) {
              NvGeom.minorCorner = nRight;
         } else {   
 /* 
    if min z is the same for the right vertice - compare it's
     right neighbour with nLeft and choose one with lowest z
     for major direction    
 */     
              if ( (nNext = nRight - 1) < 0 ) 
                nNext = 3;

              if ( zCorner[nNext] > zCorner[nLeft] ) {
                NvGeom.minorCorner = NvGeom.startCorner;
                NvGeom.startCorner = nRight;
              } else {
                NvGeom.minorCorner = nRight;
              }
         }
  /* In case left z == right z - any choice is acceptable */
      } else {
         NvGeom.minorCorner = nRight;
      }

      return;
}
    


                    