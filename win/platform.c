/****************************************************************

 platform.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

#include "../gui/gui.h"
#include "../shared.h"
#include "platform.h"
#include <windows.h>
#include <time.h>
#include <stdio.h>

/** Stolen from winuser.h since VS 6.0 does not declare any MIIM_STRING. **/
#ifndef MIIM_STRING
#define MIIM_STRING      0x00000040
#endif
/** Stolen from winbase.h since VS 6.0 does not declare any INVALID_SET_FILE_POINTER. **/
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif
/** Globals **/
#ifndef NUMBER_WINDOWS
#define	NUMBER_WINDOWS				3

#define WINDOW_ONE					0
#define WINDOW_TWO					1
#define WINDOW_THREE				2

#endif
#undef EARLIER_VERSION_STUDIO

static n_int          practical_number_windows;
static n_int          practical_window_dimension_x;
static n_int          practical_window_dimension_y;

static HWND           global_hwnd[NUMBER_WINDOWS];
static HBITMAP        offscreen[NUMBER_WINDOWS];
static BITMAPINFO	 *bmp_info[NUMBER_WINDOWS];

static n_byte		  window_definition[NUMBER_WINDOWS] = {NUM_VIEW, NUM_TERRAIN, NUM_CONTROL};

static n_int            firedown = -1;
static unsigned char    firecontrol = 0;

static unsigned char	dialog_up = 0;
static HMENU  hMenu, hMenuPopup[4];
static HANDLE current_file = NULL;
static n_string_block current_file_name[MAX_PATH];


LRESULT CALLBACK WndProc ( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
/** Nobleape platform functions **/

static void plat_update();
static n_int plat_ourwind( HWND hwnd );
static void plat_file_open( n_byte script );
static unsigned char plat_file_save();
static unsigned char plat_file_save_as();

static unsigned char plat_initialized = 0;

#define	WINDOW_OFFSET_X		16 //6
#define	WINDOW_OFFSET_Y		(38) //(10+22)
#define WINDOW_MENU_OFFSET	(21)

static void platform_menus( void )
{
    hMenu = CreateMenu();
    hMenuPopup[0] = CreateMenu();
    AppendMenu( hMenuPopup[0], MF_STRING, FILE_NEW_HANDLE, TEXT( "&New" ) );
    AppendMenu( hMenuPopup[0], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[0], MF_STRING, FILE_OPEN_HANDLE, TEXT( "&Open..." ) );
    AppendMenu( hMenuPopup[0], MF_STRING, FILE_OPEN_SCRIPT_HANDLE, TEXT( "Open Script..." ) );
    AppendMenu( hMenuPopup[0], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[0], MF_STRING, FILE_CLOSE_HANDLE, TEXT( "&Close" ) );
    AppendMenu( hMenuPopup[0], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[0], MF_STRING, FILE_SAVE_AS_HANDLE, TEXT( "&Save As..." ) );
    AppendMenu( hMenuPopup[0], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[0], MF_STRING, FILE_EXIT_HANDLE, TEXT( "E&xit" ) );
    AppendMenu( hMenu, MF_POPUP | MF_STRING, ( UINT )hMenuPopup[0], TEXT( "&File" ) );

    hMenuPopup[1] = CreateMenu();
    AppendMenu( hMenuPopup[1], MF_STRING, EDIT_UNDO_HANDLE, TEXT( "&Undo" ) );
    AppendMenu( hMenuPopup[1], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[1], MF_STRING, EDIT_CUT_HANDLE, TEXT( "Cu&t" ) );
    AppendMenu( hMenuPopup[1], MF_STRING, EDIT_COPY_HANDLE, TEXT( "&Copy" ) );
    AppendMenu( hMenuPopup[1], MF_STRING, EDIT_PASTE_HANDLE, TEXT( "&Paste" ) );
    AppendMenu( hMenuPopup[1], MF_STRING, EDIT_CLEAR_HANDLE, TEXT( "C&lear" ) );
    AppendMenu( hMenu, MF_POPUP | MF_STRING, ( UINT )hMenuPopup[1], TEXT( "&Edit" ) );

    hMenuPopup[2] = CreateMenu();
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_PAUSE_HANDLE, TEXT( "&Pause" ) );
    AppendMenu( hMenuPopup[2], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_PREV_HANDLE, TEXT( "P&revious Ape" ) );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_NEXT_HANDLE, TEXT ( "&Next Ape" ) );
    AppendMenu( hMenuPopup[2], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_CLEAR_ERRORS, TEXT ( "Clear Errors" ) );
    AppendMenu( hMenuPopup[2], MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_WEATHER_HANDLE, TEXT( "Draw &Weather" ) );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_BRAIN_HANDLE, TEXT( "Draw Brain" ) );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_BRAINCODE_HANDLE, TEXT( "Draw Brain Code" ) );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_TERRITORY_HANDLE, TEXT( "Draw Territory" ) );
    AppendMenu( hMenuPopup[2], MF_STRING, CONTROL_DAYLIGHT_TIDES_HANDLE, TEXT( "Draw Daylight Tides" ) );

    AppendMenu( hMenu, MF_POPUP | MF_STRING, ( UINT )hMenuPopup[2], TEXT( "&Control" ) );

    hMenuPopup[3] = CreateMenu();
    AppendMenu( hMenuPopup[3], MF_STRING, HELP_ABOUT_HANDLE, TEXT( "&About Noble Ape..." ) );
    AppendMenu( hMenu, MF_POPUP | MF_STRING, ( UINT )hMenuPopup[3], TEXT( "&Help" ) );

    /* can't close the meters window, it's also the menu window */
    EnableMenuItem( hMenuPopup[0], FILE_CLOSE_HANDLE, MF_DISABLED | MF_GRAYED );

    CheckMenuItem( hMenuPopup[2], CONTROL_WEATHER_HANDLE, MF_CHECKED );
    CheckMenuItem( hMenuPopup[2], CONTROL_BRAIN_HANDLE, MF_CHECKED );
}


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     PSTR szCmdLine, int iCmdShow )
{
    /** Create a window with 0 x, or y co-ord, and with the menu attached. **/

#ifdef COMMON_DEBUG_ON
    // Allocate a console for this application
    AllocConsole();
    // Redirect the standard output to the console
    freopen("CONOUT$", "w", stdout);
#endif

    int     loop = 0;
    static TCHAR szAppName[] = TEXT ( "NobleApe" ) ;
    unsigned short i = 0;
    unsigned short	fit[256 * 3];

#define	Y_DELTA	36
#define	X_DELTA	20
    int	window_value[4] = {0, -300, 512, 512};

    n_int dimensions[4];

    shared_dimensions( dimensions );

    practical_number_windows = dimensions[0];

    practical_window_dimension_x = dimensions[1];
    practical_window_dimension_y = dimensions[2];

    window_value[2] = practical_window_dimension_x;
    window_value[3] = practical_window_dimension_y;

    /** Locals specific to Windows... **/
    MSG msg ;
    WNDCLASS wndclass ;
    HDC hdc[NUMBER_WINDOWS];
    BOOL bReturn;
    int	winOffset = 7;
    int	winOffsetTop = 18;

    FILETIME ft;
    unsigned long tmpres = 0;

    GetSystemTimeAsFileTime( &ft );

    tmpres = ft.dwHighDateTime;
    tmpres ^= ft.dwLowDateTime;

    shared_color_8_bit_to_48_bit( fit );

    wndclass.style = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc = WndProc ;
    wndclass.cbClsExtra = 0 ;
    wndclass.cbWndExtra = 0 ;
    wndclass.hInstance = hInstance ;
    wndclass.hIcon = LoadIcon ( NULL, IDI_APPLICATION ) ;
    wndclass.hCursor = LoadCursor ( NULL, IDC_ARROW ) ;
    wndclass.hbrBackground = ( HBRUSH ) GetStockObject ( WHITE_BRUSH ) ;
    wndclass.lpszMenuName = NULL ;
    wndclass.lpszClassName = szAppName ;

    if ( !RegisterClass ( &wndclass ) )
    {
        MessageBox ( NULL, TEXT ( "Program requires Windows NT!" ),
                     szAppName, MB_ICONERROR ) ;
        return 0 ;
    }

    if ( dimensions[3] )
    {
        platform_menus();
    }
    else
    {
        hMenu = NULL;
    }

    window_value[1] += window_value[3] + Y_DELTA + 19 + 10;
    window_value[3] = practical_window_dimension_y;

    global_hwnd[WINDOW_ONE] = CreateWindow( szAppName, dimensions[3] ? TEXT ( "Noble Ape: Map" ) : TEXT ( "Mushroom Boy" ),
                                            WS_OVERLAPPED + WS_SYSMENU,
                                            window_value[0], window_value[1],
                                            window_value[2] + WINDOW_OFFSET_X,
                                            window_value[3] + WINDOW_OFFSET_Y + WINDOW_MENU_OFFSET,
                                            NULL, hMenu, hInstance, NULL ) ;

    if ( practical_number_windows > 1 )
    {
        window_value[0] += practical_window_dimension_x + X_DELTA;

        window_value[2] = practical_window_dimension_x;
        window_value[3] = practical_window_dimension_y;

        global_hwnd[WINDOW_TWO] = CreateWindow( szAppName, TEXT ( "Noble Ape: Terrain" ),
                                                WS_OVERLAPPED,
                                                window_value[0], window_value[1],
                                                window_value[2] + WINDOW_OFFSET_X,
                                                window_value[3] + WINDOW_OFFSET_Y,
                                                NULL, NULL, hInstance, NULL ) ;

        global_hwnd[WINDOW_THREE] = CreateWindow( szAppName, TEXT ( "Noble Ape: Control" ),
                                                WS_OVERLAPPED,
                                                window_value[0], window_value[1],
                                                window_value[2] + WINDOW_OFFSET_X,
                                                window_value[3] + WINDOW_OFFSET_Y,
                                                NULL, NULL, hInstance, NULL ) ;
    }

    loop = 0;

    while ( loop < practical_number_windows )
    {
        window_definition[loop] = ( n_byte )shared_init( window_definition[loop], tmpres );


        bmp_info[loop] = ( LPBITMAPINFO ) malloc( sizeof( BYTE ) * ( sizeof( BITMAPINFOHEADER )
                         + ( 256 * sizeof( RGBQUAD ) ) ) );

        bmp_info[loop]->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
        bmp_info[loop]->bmiHeader.biWidth = practical_window_dimension_x;
        bmp_info[loop]->bmiHeader.biHeight = ( 0 - practical_window_dimension_y ) + loop;
        bmp_info[loop]->bmiHeader.biPlanes = 1;
        bmp_info[loop]->bmiHeader.biBitCount = 8;
        bmp_info[loop]->bmiHeader.biCompression = BI_RGB;
        bmp_info[loop]->bmiHeader.biSizeImage = 0;
        bmp_info[loop]->bmiHeader.biXPelsPerMeter = 0;
        bmp_info[loop]->bmiHeader.biYPelsPerMeter = 0;
        bmp_info[loop]->bmiHeader.biClrUsed = 0;
        bmp_info[loop]->bmiHeader.biClrImportant = 0;

        i = 0;
        while ( i < 256 )
        {
            bmp_info[loop]->bmiColors[i].rgbRed =   ( unsigned char )( fit[i * 3 + 0] >> 8 );
            bmp_info[loop]->bmiColors[i].rgbGreen = ( unsigned char )( fit[i * 3 + 1] >> 8 );
            bmp_info[loop]->bmiColors[i].rgbBlue =  ( unsigned char )( fit[i * 3 + 2] >> 8 );
            bmp_info[loop]->bmiColors[i].rgbReserved = 0;
            ++i;
        }

        hdc[loop] = GetDC( global_hwnd[loop] );

        offscreen[loop] = CreateDIBitmap( hdc[loop], ( const struct tagBITMAPINFOHEADER * )bmp_info[loop], 0, NULL, NULL, DIB_RGB_COLORS );

        SetBitmapDimensionEx( offscreen[loop], practical_window_dimension_x, practical_window_dimension_y /*-loop*/, NULL );

        ShowWindow ( global_hwnd[loop], iCmdShow ) ;
        UpdateWindow ( global_hwnd[loop] ) ;
        InvalidateRect( global_hwnd[loop], NULL, TRUE );
        loop++;
    }
    plat_initialized = 1;
    while ( 1 )
    {
        if ( ( bReturn = PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) ) != 0 )
        {
            if ( msg.message == WM_QUIT )
            {
                return 0;
            }
            TranslateMessage ( &msg ) ;
            DispatchMessage ( &msg ) ;
        }
    }
    return msg.wParam ;
}

static int platform_check( int na_menu, int win_menu )
{
    if ( shared_menu( na_menu ) )
    {
        CheckMenuItem( hMenuPopup[2], win_menu, MF_CHECKED );
    }
    else
    {
        CheckMenuItem( hMenuPopup[2], win_menu, MF_UNCHECKED );
    }
    return 0;
}

LRESULT CALLBACK WndProc ( HWND hwnd, UINT message, WPARAM wParam, LPARAM
                           lParam )
{
    int i = 0;
    WORD menu_handle;

    switch ( message )
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        n_uint  local_time = ( n_uint )time( 0L );
        shared_cycle( local_time, NUM_VIEW );
        if ( practical_number_windows > 1 )
        {
            shared_cycle( local_time, NUM_TERRAIN );
        }
        shared_cycle( local_time, WINDOW_PROCESSING );
    }
    plat_update();

    InvalidateRect( global_hwnd[0], NULL, TRUE );
    if ( practical_number_windows > 1 )
    {
        InvalidateRect( global_hwnd[1], NULL, TRUE );
        InvalidateRect( global_hwnd[2], NULL, TRUE );
    }
    return 0;

    case WM_LBUTTONUP:
        firedown = -1;
        shared_mouseUp();
        return 0;


    case WM_LBUTTONDOWN:
        firedown = plat_ourwind( hwnd );
    case WM_MOUSEMOVE:
        if ( firedown != -1 )
        {
            shared_mouseOption( firecontrol );
            shared_mouseReceived( LOWORD( lParam ), HIWORD( lParam ), ( n_byte )firedown );
        }
        return 0;

    case WM_KEYDOWN:
    {
        int				windownum = plat_ourwind( hwnd );
        unsigned short	response = 0;

        switch ( wParam )
        {
        case VK_LEFT:
            response = 28;
            break;
        case VK_RIGHT:
            response = 29;
            break;
        case VK_UP:
            response = 30;
            break;
        case VK_DOWN:
            response = 31;
            break;
        case VK_CONTROL:
            firecontrol = 1;
            break;
        }
        if ( response != 0 )
        {
            if ( firecontrol )
            {
                response |= 2048;
            }
            if ( windownum != -1 )
            {
                shared_keyReceived( response, windownum );
            }
        }
    }
    return 0;
    case WM_KEYUP:
        firecontrol = 0;

        /*
            if(wParam == VK_CONTROL)
        {
            firecontrol = 0;
        }
         */
        shared_keyUp();
        return 0;
    case WM_CLOSE:
        PostMessage( hwnd, WM_DESTROY, 0, 0 );
        return 0;

    case WM_COMMAND:
        menu_handle = LOWORD( wParam );

        switch ( menu_handle )
        {
        /** Help Menu... **/
        case HELP_ABOUT_HANDLE:
            CheckMenuItem( hMenuPopup[2], CONTROL_PAUSE_HANDLE, MF_CHECKED );

            shared_about( ( unsigned char * )"Windows" );
            return 0;

        /** Control Menu... **/
        case CONTROL_PAUSE_HANDLE:
            return platform_check( NA_MENU_PAUSE, CONTROL_PAUSE_HANDLE );

        case CONTROL_PREV_HANDLE:
            ( void ) shared_menu( NA_MENU_PREVIOUS_APE );
            return 0;
        case CONTROL_NEXT_HANDLE:
            ( void ) shared_menu( NA_MENU_NEXT_APE );
            return 0;

        case CONTROL_CLEAR_ERRORS:
            ( void ) shared_menu( NA_MENU_CLEAR_ERRORS );
            return 0;

        case CONTROL_WEATHER_HANDLE:
            return platform_check( NA_MENU_WEATHER, CONTROL_WEATHER_HANDLE );

        case CONTROL_BRAIN_HANDLE:
            return platform_check( NA_MENU_BRAIN, CONTROL_BRAIN_HANDLE );

        case CONTROL_BRAINCODE_HANDLE:
            return platform_check( NA_MENU_BRAINCODE, CONTROL_BRAINCODE_HANDLE );

        case CONTROL_TERRITORY_HANDLE:
            return platform_check( NA_MENU_TERRITORY, CONTROL_TERRITORY_HANDLE );

        case CONTROL_DAYLIGHT_TIDES_HANDLE:
            return platform_check( NA_MENU_TIDEDAYLIGHT, CONTROL_DAYLIGHT_TIDES_HANDLE );

        /** File Menu... **/
        case FILE_NEW_HANDLE:
        {
            FILETIME ft;
            unsigned long tmpres = 0;

            GetSystemTimeAsFileTime( &ft );

            tmpres = ft.dwHighDateTime;
            tmpres ^= ft.dwLowDateTime;
            shared_new( tmpres );
        }
        return 0;

        case FILE_OPEN_HANDLE:

            dialog_up = 1;
            plat_file_open( 0 );
            dialog_up = 0;
            return 0;

        case FILE_OPEN_SCRIPT_HANDLE:
            dialog_up = 1;
            plat_file_open( 1 );

            dialog_up = 0;
            return 0;

        case FILE_CLOSE_HANDLE:
            return 0;

        case FILE_SAVE_AS_HANDLE:
            dialog_up = 1;
            plat_file_save_as();
            dialog_up = 0;
            return 0;
        case FILE_EXIT_HANDLE:
            PostMessage( hwnd, WM_DESTROY, 0, 0 );
            return 0;
        }
        return 0;

    case WM_DESTROY:
        DeleteObject( offscreen[0] );
        if ( practical_number_windows > 1 )
        {
            DeleteObject( offscreen[1] );
        }
        shared_close();

        PostQuitMessage ( 0 ) ;
        return 0 ;
    }

    return DefWindowProc ( hwnd, message, wParam, lParam ) ;
}

static void plat_update()
{
    PAINTSTRUCT ps[NUMBER_WINDOWS];
    HDC         hdc[NUMBER_WINDOWS];
    unsigned char lp = 0;
    if ( plat_initialized == 0 )
    {
        return;
    }
    while ( lp < practical_number_windows )
    {
        SIZE            sz;
        HDC             hdcMem;
        unsigned char    *value = shared_legacy_draw( lp, practical_window_dimension_x, practical_window_dimension_y );

        hdc[lp] = BeginPaint( global_hwnd[lp], &ps[lp] );
        GetBitmapDimensionEx( offscreen[lp], &sz );
        hdcMem = CreateCompatibleDC( hdc[lp] );

        SetDIBits( hdcMem, offscreen[lp], 0, practical_window_dimension_x/*-lp*/, value, bmp_info[lp], DIB_RGB_COLORS );

        SelectObject( hdcMem, offscreen[lp] );
        BitBlt( hdc[lp], 0, 0, sz.cx, sz.cy, hdcMem, 0, 0, SRCCOPY );
        DeleteDC( hdcMem );
        EndPaint( global_hwnd[lp], &ps[lp] );
        lp++;
    }
}

static n_int plat_ourwind( HWND hwnd )
{
    unsigned char	lp = 0;
    while ( lp < practical_number_windows )
    {
        if ( hwnd == global_hwnd[lp] )
        {
            return window_definition[lp];
        }
        lp++;
    }

    return -1;
}

static void plat_file_open( n_byte script )
{
    char actual_file_name[MAX_PATH] = { 0 };
    long file_return ;

    OPENFILENAME opf;
    ZeroMemory( &opf, sizeof( opf ) );

    opf.lStructSize = sizeof( opf );
    opf.hwndOwner = global_hwnd[WINDOW_ONE];
    opf.lpstrFilter = TEXT( NOBLE_APE_FILE_OPEN );
    opf.nFilterIndex = 1;
    opf.lpstrFile = ( LPWSTR )( current_file_name );
    opf.nMaxFile = sizeof( current_file_name );
    opf.lpstrFileTitle = ( LPWSTR )actual_file_name;
    opf.nMaxFileTitle = sizeof( actual_file_name );
    opf.lpstrInitialDir = NULL;
    opf.lpstrTitle = TEXT( "Noble Ape File Open..." );
    opf.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    file_return = GetOpenFileName( &opf );

    if ( file_return )
    {

        if ( shared_openFileName( ( n_string )current_file_name, script ) == 0 )
        {
            MessageBox( global_hwnd[0], TEXT( "File processing failed" ), TEXT( "Noble Ape File Error" ), MB_OK );
            CloseHandle( current_file );
        }
        CloseHandle( current_file );
    }
}

static unsigned char plat_file_save( n_file_out cfo )
{
    unsigned long	buff_len;
    unsigned char *buff = ( *cfo )( &buff_len );
    DWORD seek_result;
    BOOL write_result;
    DWORD write_len;

    if ( ( current_file = CreateFile( ( LPCWSTR )&current_file_name, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ) == INVALID_HANDLE_VALUE )
    {
        MessageBox( global_hwnd[0], TEXT( "Invalid file handle" ), TEXT( "Noble Ape File Error" ), MB_OK );
        return 0;
    }

    if ( ( seek_result = SetFilePointer( current_file, 0, NULL, FILE_BEGIN ) ) == INVALID_SET_FILE_POINTER )
    {
        MessageBox( global_hwnd[0], TEXT( "Unable to set file pointer" ), TEXT( "Noble Ape File Error" ), MB_OK );
        CloseHandle( current_file );
        return 0;
    }

    if ( ( write_result = WriteFile( current_file, buff, buff_len, &write_len, NULL ) ) == 0 )
    {
        MessageBox( global_hwnd[0], TEXT( "Unable to write to file" ), TEXT( "Noble Ape File Error" ), MB_OK );
        CloseHandle( current_file );
        return 0;
    }

    if ( ( write_result = SetEndOfFile( current_file ) ) == 0 )
    {
        MessageBox( global_hwnd[0], TEXT( "Unable to set EOF" ), TEXT( "Noble Ape File Error" ), MB_OK );
        CloseHandle( current_file );
        return 0;
    }
    memory_free( &buff );
    CloseHandle( current_file );
    return 1;
}

static unsigned char plat_file_save_as( void )
{
    BOOL save_result;
    char actual_file_name[MAX_PATH];

    OPENFILENAME opf;
    ZeroMemory( &opf, sizeof( opf ) );

    opf.lStructSize = sizeof( opf );
    opf.hwndOwner = global_hwnd[WINDOW_ONE];
    opf.lpstrFilter = TEXT( NOBLE_APE_FILE_SAVE );
    opf.nFilterIndex = 1;
    opf.lpstrFile = ( LPWSTR )current_file_name;
    opf.nMaxFile = sizeof( current_file_name );

    opf.lpstrFileTitle = ( LPWSTR )actual_file_name;
    opf.nMaxFileTitle = sizeof( actual_file_name );

    opf.lpstrInitialDir = NULL;
    opf.lpstrTitle = TEXT( "Noble Ape File Save..." );
    opf.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    save_result = GetSaveFileName( &opf );

    if ( !save_result )
    {
        DWORD error = CommDlgExtendedError();
        /** Cancel button pressed **/
        if ( error == 0 )
        {
            return 0;
        }
        MessageBox( global_hwnd[WINDOW_ONE], TEXT( "Unable to save file" ), TEXT( "Noble Ape File Error" ), MB_OK );
        return 0;
    }
    shared_saveFileName( actual_file_name );
    return 1;
}
