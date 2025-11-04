/*
   The Nvidia NV1 Multimedia Accelerator Simulator 
   
   Application: QUADTEX
   
   Original software © 1995 Eugene Lapidous (NVidia Corporation; March 7, 1995)
   NV1Sim version    © 2025 Connor Hyde (starfrost013)

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
      Main functions of Windows message loop

   History: 
      Eugene Lapidous      03/07-1995
      Modified for NV1sim  4 November 2025
*/

#include "sample.hpp"
#include "resource.hpp"

static CHAR szMainTitle[] = "NVidia SDK Sample QUADTEX";

extern NVCANVAS NvCanvas;
extern NVACCESS NvAccess;

HANDLE hInstance;
char fileName[256];

extern int _argc;
extern char ** _argv;

long FAR PASCAL 
MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK 
About (HWND, unsigned, WORD, LONG);

/****************************************************************************
*
*    WinMain(HANDLE, HANDLE, LPSTR, int)
*
*    Calls initialization function, processes message loop
*
****************************************************************************/

INT PASCAL WinMain ( HANDLE hInst, HANDLE hPrevInst, LPSTR  lpszCmd, INT nShow)
{
    HWND     hwnd;
    MSG      msg;
    WNDCLASS wc;
    CHAR     szMainClass[] = "NVidia SDK";

     hInstance = hInst;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC) MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst, IDI_APPLICATION );
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = IDR_MENU1;
    wc.lpszClassName = szMainClass;
    if ( !RegisterClass(&wc) ) return (FALSE);

    hwnd = CreateWindow(szMainClass, /* window class name    */
              szMainTitle,            /* window caption       */
              WS_OVERLAPPEDWINDOW,    /* window style         */
              CW_USEDEFAULT,          /* x position           */
              CW_USEDEFAULT,          /* y position           */
              CW_USEDEFAULT,          /* x size               */
              CW_USEDEFAULT,          /* y size               */
              NULL,                   /* parent window handle */
              NULL,                   /* window menu handle   */
              hInst,                  /* app instance handle  */
              NULL);                  /* creation parameters  */

    if (!hwnd)  {
        MessageBox(NULL, "Can't create main window!", szMainTitle, MB_OK);
        return (FALSE);
    }

  /* Display window according to initial view */
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    NvCanvas.drawNext = 2;

    do  {
       while ( NvCanvas.drawNext &&
           (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) ) {
           NvcDraw ();
       }

       if ( GetMessage(&msg, NULL, 0, 0) ) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
       }

    }  while ( msg.message != WM_QUIT );

    return(msg.wParam);
}



/****************************************************************************
*    MainWndProc(HWND, UINT, WPARAM, LPARAM) 
*
*    Processes Windows messages
****************************************************************************/

long FAR PASCAL 
MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (message) {
          case WM_CREATE:
        /* Initialise NvCanvas */
            NvCanvas.isActive = TRUE;
            NvCanvas.hiddenBuffer = 1;
            NvCanvas.eraseBackground = TRUE;
            NvCanvas.hWnd = hwnd;

            if ( NvcCreate() == FALSE ) {;
               MessageBox(NULL, "Patch creation error", 
                   szMainTitle, MB_OK);           
               PostQuitMessage(0);           
               return(0);           
            }

            NvcGetCanvas ();

            if ( NvcInit() == FALSE ) {;
               MessageBox(NULL, "Change color resolution to 16 or 24 bit.", 
                   szMainTitle, MB_OK);           
               PostQuitMessage(0);           
               return(0);           
            }

            NvcSetHiddenBuffer ();

            NvcModelInit ();

    /* Store file directory; initialize "test.bmp" */
            strnset ( fileName, '\0', 255 );
            strncpy ( fileName, _argv[0], 
                      strlen ( _argv[0] ) - strlen ( "quadtex.exe" ) );
            strcat ( fileName, "test.bmp");          
            if ( NvcInitImage ( fileName ) == FALSE )
               NvcCloseImage ();
            break;


      case WM_COMMAND:  
            switch ( wParam ) {
                case IDM_OPEN:    
                    if ( NvcInitImage (NULL) == FALSE ) 
                       NvcCloseImage ();
                    break;        
 
                case IDM_ABOUT: 
                    DialogBox(hInstance, IDD_DIALOG1, hwnd, 
                           (DLGPROC) About);   
                    break;                 
            }        
            break;    
            
            
      case WM_ACTIVATE:
 /* In this example application doesn't render when in inactive state */
            if ( wParam == WA_INACTIVE ) {
               NvCanvas.isActive = FALSE;
               UpdateWindow (hwnd);
            } else {
               NvcSetHiddenBuffer ();
               NvCanvas.isActive = TRUE;
            }

            break;


      case WM_SIZE:
            NvcGetCanvas ();
            NvCanvas.eraseBackground = TRUE;
            return (DefWindowProc(hwnd, message, wParam, lParam));


      case WM_KEYDOWN:
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MOUSEMOVE:
            if (NvcProcessUserInput (message, wParam, lParam) == TRUE) {
                NvCanvas.drawNext = 2;
                InvalidateRect (hwnd, NULL, FALSE);
            }    
            break;

    /* Everything that Windows draws using calls to a video driver goes to
        both buffers. Application has to use direct rendering to clean up
        single buffer when needed.
    */       
      case WM_ERASEBKGND:
            if ( NvCanvas.eraseBackground )    {
                NvCanvas.eraseBackground = FALSE; 
                return(DefWindowProc(hwnd, message, wParam, lParam));  
            }
            break;
 
      case WM_PAINT:
     /* BeginPaint and EndPaint messages are necessary even if GDI rendering
        procedures are not used.
     */     
            BeginPaint(hwnd, &ps);

            if ( NvcDraw() == FALSE ) {;
              MessageBox(NULL, "Direct access draw error", 
                  szMainTitle, MB_OK);           
            }

            if ( NvCanvas.isActive == FALSE ) {
     /* Draw the same picture in the other buffer */
                if ( NvcDraw() == FALSE ) {;
                   MessageBox(NULL, "Direct access draw error", 
                       szMainTitle, MB_OK);           
               }
            }

            EndPaint (hwnd, &ps);
            break;

     case WM_DESTROY:           
            NvcCloseImage ();
            NvcDestroy (); 
            PostQuitMessage(0);           
            break;

      default:
            return (DefWindowProc(hwnd, message, wParam, lParam));
   }    

   return (0);
}


/**************************************
* Get dimensions of the client area 
***************************************/
void
NvcGetCanvas (void)
{
    RECT Rect;

    GetClientRect ( NvCanvas.hWnd, &Rect );

    NvCanvas.width = Rect.right - Rect.left + 1;
    NvCanvas.height = Rect.bottom - Rect.top + 1;

   /* Get distance from the viewport's center to the 
      closest side */
    if ( NvCanvas.width > NvCanvas.height )
         NvCanvas.halfSide = NvCanvas.height >> 1;
      else    
         NvCanvas.halfSide = NvCanvas.width >> 1;

    return;
}

/********************************************
 About Dialog Box
*********************************************/ 
BOOL CALLBACK 
About (HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
   switch (message)  {
       case WM_INITDIALOG: 
          return (TRUE);

       case WM_COMMAND:    
          if (wParam == IDC_OK ) {
             EndDialog(hDlg, TRUE); 
             return (TRUE);
          }
          break;
   }
   return (FALSE);              
}
