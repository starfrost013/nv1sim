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
        Operations with files containing input data.

     History: 
       Eugene Lapidous    03/07-1995


****************************************************************************/

#include "sample.hpp"

extern NVCANVAS NvCanvas;
extern NVIMAGE NvImage;
extern char fileName[];

/***************************************************
  Read DIB from the file.
****************************************************/
BOOL
NvcReadDIB ( char * szName )
{
   BITMAPFILEHEADER bmfh;
   HFILE  hFile;
   OFSTRUCT OpenBuf = {sizeof(OFSTRUCT)};
   DWORD  dwByteCount;
   DWORD  dwDibSize;
   OPENFILENAME ofn;

   if ( szName == NULL ) {
  	   memset ( &ofn, 0, sizeof (OPENFILENAME) );
      ofn.lStructSize = sizeof(OPENFILENAME);			
      ofn.hwndOwner = NvCanvas.hWnd;			
	   ofn.lpstrFilter = "24-bit DIBs (*.bmp)\0*.bmp\0";			
	   ofn.nFilterIndex = 1;			
      fileName[0] = 0;    /* pass in NULL */			
	   ofn.lpstrFile = (LPSTR)fileName;			
	   ofn.nMaxFile = 256;			
      ofn.Flags = OFN_FILEMUSTEXIST;			

	   if (GetOpenFileName((LPOPENFILENAME)&ofn) == FALSE)
        return (FALSE);
   }

   if ( ( hFile = OpenFile (fileName, &OpenBuf, OF_READ) ) == HFILE_ERROR )  
       return (FALSE);

   if ( ReadFile ((HANDLE)hFile, (LPVOID) &bmfh, sizeof (BITMAPFILEHEADER), 
         &dwByteCount, NULL) == FALSE ) {
       _lclose (hFile);
       return (FALSE);
   }

   if ( bmfh.bfType != *(WORD *) "BM" ) {
       _lclose (hFile);
       return (FALSE);
   }

   dwDibSize = bmfh.bfSize - sizeof (BITMAPFILEHEADER);
       
   if ( NvImage.npBmInfoPacked != NULL ) {
       GlobalFreePtr32 ( NvImage.npBmInfoPacked );
       NvImage.npBmInfoPacked = NULL;
       NvImage.npBitmap = NULL;
   }

   if ( (NvImage.npBmInfoPacked = ( BITMAPINFOHEADER * ) 
       GlobalAllocPtr32 ( GMEM_FIXED | GMEM_ZEROINIT, dwDibSize )) == NULL ) {
       _lclose (hFile);
       return (FALSE);
   }


   if ( ReadFile ((HANDLE)hFile, (LPVOID) NvImage.npBmInfoPacked, dwDibSize, 
         &dwByteCount, NULL) == FALSE ) {
       _lclose (hFile);
       return (FALSE);
   }

   NvImage.npBitmap = (char *) NvImage.npBmInfoPacked + 
       sizeof ( BITMAPINFOHEADER );

   _lclose (hFile);

   return (TRUE);
}

