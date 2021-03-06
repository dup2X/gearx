/*  emacs: -*- mode: c++; coding: gbk -*- TAB SIZE: 4 -*-  */

/*
    Copyright (C) 2007 GearX Team

    This source code is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This source code is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    $Id$
    ChenZaichun@gmail.com
*/

/**
 * @file   GXMain.cpp
 * @author ChenZaichun <ChenZaichun@gmail.com>
 * @date   Mon Nov 12 16:24:13 2007
 * 
 * @brief  
 * 
 * 
 */

#include <windows.h>
#include "GXLua.hpp"

#if defined DEBUG || defined _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif

/** 
 * window procedure callback function
 * 
 * @param HWND  	window handle
 * @param UINT 		message
 * @param WPARAM 	parameter
 * @param LPARAM 	parameter
 * 
 * @return 
 */
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

///////////////////////////////////////////////////////////////////////////////
/** 
 * quit application, called in Lua
 * 
 * @param L pointer to lua state
 * 
 * @return 0
 */
static int Quit(lua_State* L)
{
	PostQuitMessage(0);
	return 0;
}

/** 
 * initialize libs for lua
 * 
 * @param L pointer to lua state
 * 
 * @return 0
 */
static int pmain (lua_State* L)
{
	luaL_openlibs(L);				/* open standard libraries */

	// regist _DEBUG for lua script
#if defined DEBUG || defined _DEBUG
	lua_pushboolean(L, 1);
	lua_setglobal(L, "_DEBUG");
#endif

	lua_pushcfunction(L, Quit);		// regist Quit
	lua_setglobal(L, "Quit");

	return 0;
}

/// main lua script file path
#define MAIN_SCRIPT		"lua/GXMain.lua"

/** 
 * create main window
 * 
 * @param hInstance application instance
 * @param lpCmdLine command line
 * @param nCmdShow  the window show status
 * 
 * @return window handle
 */
HWND CreateMainWindow(HINSTANCE hInstance, char *lpCmdLine,int nCmdShow)
{
	static TCHAR szAppName[] = TEXT ("GearX") ;
	HWND         hwnd ;
	WNDCLASS     wndclass ;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = szAppName ;

	if (!RegisterClass (&wndclass)) {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
			szAppName, MB_ICONERROR) ;
		return 0 ;
	}
	
	DWORD style = WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX) & (~WS_SIZEBOX);
	hwnd = CreateWindow (szAppName,                  // window class name
		TEXT ("GearX"),				// window caption
		style,						// window style
		CW_USEDEFAULT,              // initial x position
		CW_USEDEFAULT,              // initial y position
		800,				        // initial x size
		600+GetSystemMetrics(SM_CYCAPTION),	// initial y size
		NULL,                       // parent window handle
		NULL,                       // window menu handle
		hInstance,                  // program instance handle
		NULL) ;                     // creation parameters	
	
	return hwnd;
}

lua_State *L;					// lua state pointer -- global

/** 
 * Windows Main entry
 * 
 * @param hInstance 		process instance
 * @param hPrevInstance 	NULL
 * @param lpCmdLine 		command line
 * @param nCmdShow 			window show status
 * 
 * @return 
 */
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
				   char *lpCmdLine,int nCmdShow)
{
//	hinst = hInstance;				// save instance to global
	MSG      msg ;
	
	/* Open libraries */
	L = lua_open();  /* create state */
	if (L == NULL) {
		printf("cannot create Lua state\n");
		return 1;
	}
	
	lua_pushcfunction(L, pmain);
	if (lua_pcall(L, 0, 0, 0)) {
		printf("%s", lua_tostring(L, -1));
		return 1;
	}	/* Execute Lua code */
	if (luaL_loadfile(L, MAIN_SCRIPT)) {
		printf("%s", lua_tostring(L, -1));
		return 1;
	}
	if (lua_pcall(L, 0, 0, 0)) {
		printf( "%s", lua_tostring(L, -1));
		return 1;
	}
	
	HWND hwnd = CreateMainWindow(hInstance, lpCmdLine, nCmdShow);

	lua_getglobal(L, "GXInit");
	if (!lua_isfunction(L, -1)) {
		printf("GXInit not funciton\n");
		return 1;
	}
	lua_pushinteger(L, (int)(hwnd));
	if (lua_pcall(L, 1, 0, 0) != 0) {
		printf("%s", lua_tostring(L, -1));
		return 1;
	}

	ShowWindow (hwnd, nCmdShow) ;
	UpdateWindow (hwnd);
	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if(!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			lua_getglobal(L, "GXMain");
			if (lua_pcall(L, 0, 0, 0) != 0) {
				printf("%s", lua_tostring(L, -1));
				return 1;
			}
		}
	}
	
	lua_getglobal(L, "GXExit");
	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("%s", lua_tostring(L, -1));
		return 1;
	}
	lua_close(L);
	return msg.wParam;
}

#if defined DEBUG || _DEBUG
int main()
{
	return WinMain(GetModuleHandle(NULL),NULL,
		NULL, SW_SHOW);
}
#endif

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int ret;
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0) ;
		return 0;
		
	default:
		lua_getglobal(L, "WndProc");
		lua_pushinteger(L, message);
		lua_pushinteger(L, wParam);
		lua_pushinteger(L, lParam);
		if (lua_pcall(L, 3, 1, 0) != 0) {
			printf("%s", lua_tostring(L, -1));
			return 1;
		}
		ret = lua_toboolean(L, -1);
		if (ret) {
			return 0;
		} else {
			// not handle, so use default window handler
			break;
		}
	}	
	return DefWindowProc (hwnd, message, wParam, lParam);
}
