/****************************************************************

 platform.c

 =============================================================

 Copyright 1996-2013 Tom Barbalet. All rights reserved.

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

 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.

 ****************************************************************/
#include "../noble/noble.h"
#include "../universe/universe.h"
#include "../gui/gui.h"
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
#define	NUMBER_WINDOWS				2

#define WINDOW_ONE					0
#define WINDOW_TWO					1

#endif
#undef EARLIER_VERSION_STUDIO

static HWND           global_hwnd[NUMBER_WINDOWS];
static HBITMAP        offscreen[NUMBER_WINDOWS];
static BITMAPINFO	* bmp_info[NUMBER_WINDOWS];

static unsigned char	* local_buffer;

static unsigned char    firedown = 0, firecontrol = 0;
static int				fire_x, fire_y;
static unsigned char	dialog_up = 0;
static HMENU  hMenu, hMenuPopup[4];
static HANDLE current_file = NULL;
static TCHAR current_file_name[MAX_PATH];


LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
/** Nobleape platform functions **/

static void plat_update();
static unsigned char plat_ourwind(HWND hwnd);
static unsigned char plat_file_open();
static unsigned char plat_file_save();
static unsigned char plat_file_save_as();

#define	WINDOW_OFFSET_X		6
#define	WINDOW_OFFSET_Y		(25) //(10+22)
#define WINDOW_MENU_OFFSET	(19)

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
    /** Create a window with 0 x, or y co-ord, and with the menu attached. **/

    int     loop = 0;
    static TCHAR szAppName[] = TEXT ( "NobleApe" ) ;
    unsigned short i = 0;
    unsigned short	fit[256*3];

#define	Y_DELTA	36
#define	X_DELTA	20
    int	window_value[4] = {0, -300, MAP_DIMENSION, MAP_DIMENSION};

    /** Locals specific to Windows... **/
    MSG msg ;
    WNDCLASS wndclass ;
    HDC hdc[NUMBER_WINDOWS];
    BOOL bReturn;
    int	winOffset = 7;
    int	winOffsetTop = 18;

    FILETIME ft;
    unsigned long tmpres = 0;

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres ^= ft.dwLowDateTime;

    draw_fit(land_points, fit);

    wndclass.style = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc = WndProc ;
    wndclass.cbClsExtra = 0 ;
    wndclass.cbWndExtra = 0 ;
    wndclass.hInstance = hInstance ;
    wndclass.hIcon = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
    wndclass.lpszMenuName = NULL ;
    wndclass.lpszClassName = szAppName ;

    if (!RegisterClass (&wndclass))
    {
        MessageBox (NULL, TEXT ("Program requires Windows NT!"),
                    szAppName, MB_ICONERROR) ;
        return 0 ;
    }

    hMenu = CreateMenu();
    hMenuPopup[0] = CreateMenu();
    AppendMenu(hMenuPopup[0], MF_STRING, FILE_NEW_HANDLE, TEXT("&New"));
    AppendMenu(hMenuPopup[0], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[0], MF_STRING, FILE_OPEN_HANDLE, TEXT("&Open..."));
    AppendMenu(hMenuPopup[0], MF_STRING, FILE_OPEN_SCRIPT_HANDLE, TEXT("Open Script..."));
    AppendMenu(hMenuPopup[0], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[0], MF_STRING, FILE_CLOSE_HANDLE, TEXT("&Close"));
    AppendMenu(hMenuPopup[0], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[0], MF_STRING, FILE_SAVE_AS_HANDLE, TEXT("&Save As..."));
    AppendMenu(hMenuPopup[0], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[0], MF_STRING, FILE_EXIT_HANDLE, TEXT("E&xit"));
    AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT)hMenuPopup[0], TEXT("&File"));

    hMenuPopup[1] = CreateMenu();
    AppendMenu(hMenuPopup[1], MF_STRING, EDIT_UNDO_HANDLE, TEXT("&Undo"));
    AppendMenu(hMenuPopup[1], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[1], MF_STRING, EDIT_CUT_HANDLE, TEXT("Cu&t"));
    AppendMenu(hMenuPopup[1], MF_STRING, EDIT_COPY_HANDLE, TEXT("&Copy"));
    AppendMenu(hMenuPopup[1], MF_STRING, EDIT_PASTE_HANDLE, TEXT("&Paste"));
    AppendMenu(hMenuPopup[1], MF_STRING, EDIT_CLEAR_HANDLE, TEXT("C&lear"));
    AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT)hMenuPopup[1], TEXT("&Edit"));

    hMenuPopup[2] = CreateMenu();
    AppendMenu(hMenuPopup[2], MF_STRING, CONTROL_PAUSE_HANDLE, TEXT("&Pause"));
    
    
    AppendMenu(hMenuPopup[2], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[2], MF_STRING, CONTROL_PREV_HANDLE, TEXT("P&revious Ape"));
    AppendMenu(hMenuPopup[2], MF_STRING, CONTROL_NEXT_HANDLE, TEXT ("&Next Ape"));
    AppendMenu(hMenuPopup[2], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[2], MF_STRING, CONTROL_CREATE_AUTOLOAD, TEXT("Create Autoload File"));
    AppendMenu(hMenuPopup[2], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[2], MF_STRING, CONTROL_CLEAR_ERRORS, TEXT ("Clear Errors"));
    AppendMenu(hMenuPopup[2], MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPopup[2], MF_STRING, CONTROL_NO_WEATHER_HANDLE, TEXT("No &Weather"));

    AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT)hMenuPopup[2], TEXT("&Control"));

    hMenuPopup[3] = CreateMenu();
    AppendMenu(hMenuPopup[3], MF_STRING, HELP_ABOUT_HANDLE, TEXT("&About Noble Ape..."));
    AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT)hMenuPopup[3], TEXT("&Help"));

    window_value[1] += window_value[3] + Y_DELTA + 19 + 10;
    window_value[3] = MAP_DIMENSION;

    global_hwnd[WINDOW_ONE] = CreateWindow(szAppName, TEXT ( "Noble Ape: Map" ),
                                           WS_OVERLAPPED + WS_SYSMENU,
                                           window_value[0], window_value[1],
                                           window_value[2] + WINDOW_OFFSET_X,
                                           window_value[3] + WINDOW_OFFSET_Y + WINDOW_MENU_OFFSET,
                                           NULL, hMenu, hInstance, NULL) ;

    window_value[0] += MAP_DIMENSION + X_DELTA;

    window_value[2] = TERRAIN_WINDOW_WIDTH;
    window_value[3] = TERRAIN_WINDOW_HEIGHT;

    global_hwnd[WINDOW_TWO] = CreateWindow(szAppName, TEXT ( "Noble Ape: Terrain" ),
                                           WS_OVERLAPPED,
                                           window_value[0], window_value[1],
                                           window_value[2] + WINDOW_OFFSET_X,
                                           window_value[3] + WINDOW_OFFSET_Y,
                                           NULL, NULL, hInstance, NULL) ;

    /* can't close the meters window, it's also the menu window */
    EnableMenuItem(hMenuPopup[0], FILE_CLOSE_HANDLE, MF_DISABLED | MF_GRAYED);


    /* this is called here to allow for the gworld memory allocation first */
    local_buffer = (unsigned char *) control_init(2, tmpres);

    if (io_disk_check((unsigned char *)"NobleApeAutoload.txt") == 1)
    {
        n_file	  tester;
        tester.size = 4096;
        tester.data = io_new(4096);
        tester.location = 0;
        (void)io_disk_read(&tester,(unsigned char *)"NobleApeAutoload.txt");
        if(sim_filein(tester.data, tester.location) == 0)
        {
            control_init(0, 0);
        }
        io_free(tester.data);
    }
    if (io_disk_check((unsigned char *)"ApeScriptAutoload.txt") == 1)
    {
        n_file	  tester;
        tester.size = 4096;
        tester.data = io_new(4096);
        tester.location = 0;
        (void)io_disk_read(&tester,"ApeScriptAutoload.txt");
        (void)sim_interpret(tester.data, tester.location);
        io_free(tester.data);
    }

    loop = 0;

    while (loop < NUMBER_WINDOWS)
    {
        bmp_info[loop] = (LPBITMAPINFO) malloc(sizeof(BYTE) * (sizeof(BITMAPINFOHEADER)
                                               + (256 * sizeof(RGBQUAD))));

        bmp_info[loop]->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmp_info[loop]->bmiHeader.biWidth = 512;
        bmp_info[loop]->bmiHeader.biHeight = -512 + loop;
        bmp_info[loop]->bmiHeader.biPlanes = 1;
        bmp_info[loop]->bmiHeader.biBitCount = 8;
        bmp_info[loop]->bmiHeader.biCompression = BI_RGB;
        bmp_info[loop]->bmiHeader.biSizeImage = 0;
        bmp_info[loop]->bmiHeader.biXPelsPerMeter = 0;
        bmp_info[loop]->bmiHeader.biYPelsPerMeter = 0;
        bmp_info[loop]->bmiHeader.biClrUsed = 0;
        bmp_info[loop]->bmiHeader.biClrImportant = 0;

        i = 0;
        while (i < 256)
        {
            bmp_info[loop]->bmiColors[i].rgbRed =   (unsigned char)(fit[i * 3 + 0] >> 8);
            bmp_info[loop]->bmiColors[i].rgbGreen = (unsigned char)(fit[i * 3 + 1] >> 8);
            bmp_info[loop]->bmiColors[i].rgbBlue =  (unsigned char)(fit[i * 3 + 2] >> 8);
            bmp_info[loop]->bmiColors[i].rgbReserved = 0;
            ++i;
        }

        hdc[loop] = GetDC(global_hwnd[loop]);

        offscreen[loop] = CreateDIBitmap(hdc[loop], (const struct tagBITMAPINFOHEADER *)bmp_info[loop], 0, NULL, NULL, DIB_RGB_COLORS);

        SetBitmapDimensionEx(offscreen[loop], 512, 512-loop, NULL);

        ShowWindow (global_hwnd[loop], iCmdShow) ;
        UpdateWindow (global_hwnd[loop]) ;
        InvalidateRect(global_hwnd[loop], NULL, TRUE);
        loop++;
    }

    while ( 1 )
    {
        if ( (bReturn = PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) != 0)
        {
            if (msg.message == WM_QUIT)
                return 0;
            TranslateMessage (&msg) ;
            DispatchMessage (&msg) ;
        }
    }
    return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM
                          lParam)
{
    int i = 0;
    WORD menu_handle;

    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        if (firedown != 0)
        {
            control_mouse((n_byte)(firedown - 1), fire_x, fire_y, firecontrol);
        }

        control_simulate(((60*clock())/(CLK_TCK)));
            
        plat_update();

        InvalidateRect(global_hwnd[0], NULL, TRUE);
        InvalidateRect(global_hwnd[1], NULL, TRUE);

        return 0;

    case WM_LBUTTONDOWN:
        firedown = plat_ourwind(hwnd);
        return 0;

    case WM_LBUTTONUP:
        firedown = 0;
        return 0;

    case WM_MOUSEMOVE:
        fire_x = LOWORD(lParam);
        fire_y = HIWORD(lParam);
        return 0;

    case WM_KEYDOWN:
    {
        int				windownum = plat_ourwind(hwnd);
        unsigned short	response = 0;

        switch(wParam)
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
        if(response != 0)
        {
            if(firecontrol)
                response |= 2048;
            control_key((n_byte)(windownum - 1), response);
        }
    }
    return 0;
    case WM_KEYUP:
        if(wParam == VK_CONTROL)
        {
            firecontrol = 0;
        }
        return 0;
    case WM_CLOSE:

        return 0;

    case WM_COMMAND:
        menu_handle = LOWORD(wParam);

        switch (menu_handle)
        {
            /** Help Menu... **/
        case HELP_ABOUT_HANDLE:
            CheckMenuItem(hMenuPopup[2], CONTROL_PAUSE_HANDLE, MF_CHECKED);

            control_about((unsigned char *)"Windows");
            return 0;

            /** Control Menu... **/
        case CONTROL_PAUSE_HANDLE:
            if (control_toggle_pause(1))
                CheckMenuItem(hMenuPopup[2], CONTROL_PAUSE_HANDLE, MF_CHECKED);
            else
                CheckMenuItem(hMenuPopup[2], CONTROL_PAUSE_HANDLE, MF_UNCHECKED);
            return 0;

        case CONTROL_PREV_HANDLE:
            control_key(0, 2079);
            return 0;
        case CONTROL_NEXT_HANDLE:
            control_key(0, 2078);
            return 0;

        case CONTROL_CREATE_AUTOLOAD:
            if (io_disk_check((unsigned char *)"NobleApeAutoload.txt") == 0)
            {
                unsigned long		buff_len;
                unsigned char * buff = sim_fileout(&buff_len);
                FILE          * outputfile = fopen("NobleApeAutoload.txt","w");
                fwrite(buff, buff_len, 1, outputfile);
                fclose(outputfile);
                io_free(buff);
            }
            return 0;

        case CONTROL_CLEAR_ERRORS:
            (void)draw_error(0L);
            return 0;


        case CONTROL_NO_WEATHER_HANDLE:
            if (draw_toggle_weather())
                CheckMenuItem(hMenuPopup[2], CONTROL_NO_WEATHER_HANDLE, MF_UNCHECKED);
            else
                CheckMenuItem(hMenuPopup[2], CONTROL_NO_WEATHER_HANDLE, MF_CHECKED);

            return 0;

            /** File Menu... **/
        case FILE_NEW_HANDLE:
            control_init(1, clock());
            return 0;

        case FILE_OPEN_HANDLE:

            dialog_up = 1;
            if (plat_file_open(&sim_filein))
            {
                control_init(0, 0);
            }
            dialog_up = 0;
            return 0;

        case FILE_OPEN_SCRIPT_HANDLE:
            dialog_up = 1;
            (void)plat_file_open(&sim_interpret);

            dialog_up = 0;
            return 0;

        case FILE_CLOSE_HANDLE:
            return 0;

        case FILE_SAVE_AS_HANDLE:
            dialog_up = 1;
            plat_file_save_as(&sim_fileout);
            dialog_up = 0;
            return 0;
        case FILE_EXIT_HANDLE:
            PostMessage(hwnd, WM_DESTROY, 0, 0);
            return 0;
        }
        return 0;

    case WM_DESTROY:
        if (io_disk_check((unsigned char *)"NobleApeAutoload.txt") == 1)
        {
            unsigned long		buff_len;
            unsigned char * buff = sim_fileout(&buff_len);
            FILE          * outputfile = fopen("NobleApeAutoload.txt","w");
            fwrite(buff, buff_len, 1, outputfile);
            fclose(outputfile);
            io_free(buff);
        }
        DeleteObject(offscreen[0]);
        DeleteObject(offscreen[1]);
        sim_close();

        PostQuitMessage (0) ;
        return 0 ;
    }

    return DefWindowProc (hwnd, message, wParam, lParam) ;
}

static void plat_update()
{
    PAINTSTRUCT ps[NUMBER_WINDOWS];
    HDC         hdc[NUMBER_WINDOWS];
    unsigned char lp = 0;

    while (lp < NUMBER_WINDOWS)
    {
        SIZE            sz;
        HDC             hdcMem;
        unsigned char * value;
        hdc[lp] = BeginPaint(global_hwnd[lp], &ps[lp]);
        GetBitmapDimensionEx(offscreen[lp], &sz);
        hdcMem = CreateCompatibleDC(hdc[lp]);
        if (lp == 0)
        {
            value = VIEWWINDOW(local_buffer);
        }
        else
        {
            value = TERRAINWINDOW(local_buffer);
        }

        SetDIBits(hdcMem, offscreen[lp], 0, 512-lp, value, bmp_info[lp], DIB_RGB_COLORS);

        SelectObject(hdcMem, offscreen[lp]);
        BitBlt(hdc[lp], 0, 0, sz.cx, sz.cy, hdcMem, 0, 0, SRCCOPY);
        DeleteDC(hdcMem);
        EndPaint(global_hwnd[lp], &ps[lp]);
        lp++;
    }
}

static unsigned char plat_ourwind(HWND hwnd)
{
    unsigned char	lp = 0;
    while (lp < NUMBER_WINDOWS)
    {
        if (hwnd == global_hwnd[lp])
            return (unsigned char)(lp + 1);
        lp++;
    }

    return 0;
}

static unsigned char plat_file_open(control_file_handle cfh)
{
    unsigned char * buff;
    unsigned long	buff_len;
    n_int			format_ok = -1;

    char actual_file_name[MAX_PATH] = { 0 };
    long file_return ;
    DWORD seek_result;
    BOOL read_result;
    DWORD read_len;

    OPENFILENAME opf;
    ZeroMemory(&opf, sizeof(opf));

    opf.lStructSize = sizeof(opf);
    opf.hwndOwner = global_hwnd[WINDOW_ONE];
    opf.lpstrFilter = TEXT(NOBLE_APE_FILE_OPEN);
    opf.nFilterIndex = 1;
    opf.lpstrFile = (LPCWSTR)(current_file_name);
    opf.nMaxFile = sizeof(current_file_name);
    opf.lpstrFileTitle = (LPCWSTR)actual_file_name;
    opf.nMaxFileTitle = sizeof(actual_file_name);
    opf.lpstrInitialDir = NULL;
    opf.lpstrTitle = TEXT("Noble Ape File Open...");
    opf.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    file_return = GetOpenFileName(&opf);

    if (file_return )
    {
        if ((current_file = CreateFile((LPCWSTR)current_file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        {
            MessageBox(global_hwnd[WINDOW_ONE], TEXT("Invalid file handle"), TEXT("Noble Ape File Error"), MB_OK);
            return 0;
        }

        if ((buff_len = GetFileSize(current_file, NULL)) == INVALID_FILE_SIZE)
        {
            MessageBox(global_hwnd[0], TEXT("Unable to get file size"), TEXT("Noble Ape File Error"), MB_OK);
            CloseHandle(current_file);
            return 0;
        }

        buff = (unsigned char*) io_new ( buff_len * sizeof(unsigned char));

        if ((seek_result = SetFilePointer(current_file, 0, NULL, FILE_BEGIN)) == INVALID_SET_FILE_POINTER)
        {
            MessageBox(global_hwnd[0], TEXT("Unable to set file pointer"), TEXT("Noble Ape File Error"), MB_OK);
            io_free(buff);
            CloseHandle(current_file);
            return 0;
        }

        read_result = ReadFile(current_file, buff, buff_len, &read_len, NULL);

        /* I think there is still a problem read_len should be tested against buff_len surely - TSB 050620 */
        if ((read_result != 1) || (read_len != buff_len))
        {
            MessageBox(global_hwnd[0], TEXT("Unable to read from file"), TEXT("Noble Ape File Error"), MB_OK);
            io_free(buff);
            CloseHandle(current_file);
            return 0;
        }

        format_ok = (*cfh)(buff, buff_len);

        if(format_ok == -1)
        {
            MessageBox(global_hwnd[0], TEXT("File processing failed"), TEXT("Noble Ape File Error"), MB_OK);
            io_free(buff);
            CloseHandle(current_file);
        }

        io_free(buff);
        CloseHandle(current_file);
    }
    return (unsigned char)(format_ok == 0);
}

static unsigned char plat_file_save(n_file_out cfo)
{
    unsigned long	buff_len;
    unsigned char * buff = (*cfo)(&buff_len);
    DWORD seek_result;
    BOOL write_result;
    DWORD write_len;

    if ((current_file = CreateFile((LPCWSTR)&current_file_name, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        MessageBox(global_hwnd[0], TEXT("Invalid file handle"), TEXT("Noble Ape File Error"), MB_OK);
        return 0;
    }

    if ((seek_result = SetFilePointer(current_file, 0, NULL, FILE_BEGIN)) == INVALID_SET_FILE_POINTER)
    {
        MessageBox(global_hwnd[0], TEXT("Unable to set file pointer"), TEXT("Noble Ape File Error"), MB_OK);
        CloseHandle(current_file);
        return 0;
    }

    if ((write_result = WriteFile(current_file, buff, buff_len, &write_len, NULL)) == 0)
    {
        MessageBox(global_hwnd[0], TEXT("Unable to write to file"), TEXT("Noble Ape File Error"), MB_OK);
        CloseHandle(current_file);
        return 0;
    }

    if ((write_result = SetEndOfFile(current_file)) == 0)
    {
        MessageBox(global_hwnd[0], TEXT("Unable to set EOF"), TEXT("Noble Ape File Error"), MB_OK);
        CloseHandle(current_file);
        return 0;
    }
    io_free(buff);
    CloseHandle(current_file);
    return 1;
}

static unsigned char plat_file_save_as(n_file_out cfo)
{
    BOOL save_result;
    char actual_file_name[MAX_PATH] = { 0 };

    OPENFILENAME opf;
    ZeroMemory(&opf, sizeof(opf));

    opf.lStructSize = sizeof(opf);
    opf.hwndOwner = global_hwnd[WINDOW_ONE];
    opf.lpstrFilter = TEXT(NOBLE_APE_FILE_SAVE);
    opf.nFilterIndex = 1;
    opf.lpstrFile = (LPCWSTR)current_file_name;
    opf.nMaxFile = sizeof(current_file_name);

    opf.lpstrFileTitle = (LPCWSTR)actual_file_name;
    opf.nMaxFileTitle = sizeof(actual_file_name);

    opf.lpstrInitialDir = NULL;
    opf.lpstrTitle = TEXT("Noble Ape File Save...");
    opf.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    save_result = GetSaveFileName(&opf);

    if (!save_result)
    {
        DWORD error = CommDlgExtendedError();
        /** Cancel button pressed **/
        if (error == 0)
        {
            return 0;
        }
        MessageBox(global_hwnd[WINDOW_ONE], TEXT("Unable to save file"), TEXT("Noble Ape File Error"), MB_OK);
        return 0;
    }
    plat_file_save(cfo);

    return 1;
}
