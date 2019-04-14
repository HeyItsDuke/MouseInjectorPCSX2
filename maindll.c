//==========================================================================
// Mouse Injector for PCSX2
//==========================================================================
// Copyright (C) 2019 Carnivorous
// All rights reserved.
//
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "maindll.h"
#include "hook.h"
#include "mouse.h"
#include "./games/game.h"

enum EDITINGCURRENT {EDITINGSENSITIVITY = 0, EDITINGCROSSHAIR};

static HANDLE threadhandle = 0; // thread identifier
static uint8_t mousetoggle = 0;
static uint8_t selectedoption = EDITINGSENSITIVITY;
static uint8_t locksettings = 0;
static uint8_t welcomed = 0;
static uint8_t stopthread = 1;

uint8_t sensitivity = 20;
uint8_t crosshair = 3;
uint8_t invertpitch = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
static DWORD WINAPI InjectThread();
static void GUI_Init(void);
static void GUI_Quit(void);
static void GUI_Welcome(void);
static void GUI_Interact(void);
static void GUI_Update(void);
static void GUI_Clear(void);
static void INI_Load(void);
static void INI_Save(void);

//==========================================================================
// Purpose: first called upon launch
// Changed Globals: stopthread, threadhandle
//==========================================================================
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		GUI_Init(); // create console window for interface
		if(!HOOK_Init()) // if dll failed to load (not found/unable to read)
		{
			printf("\n Mouse Injector for %s %s\n%s\n\n   LilyPad.dll was not found. Closing...", PCSX2VERSION, BUILDINFO, LINE);
			Sleep(3000);
			return FALSE;
		}
		if(!HOOK_LoadLilyCalls()) // if lilypad calls didn't hook successfully (missing function/incompatible API)
		{
			printf("\n Mouse Injector for %s %s\n%s\n\n   LilyPad could not be hooked (unsupported API). Closing...", PCSX2VERSION, BUILDINFO, LINE);
			Sleep(3000);
			return FALSE;
		}
		if(!MOUSE_Init()) // if mouse was not detect
		{
			printf("\n Mouse Injector for %s %s\n%s\n\n   Mouse not detected. Closing...", PCSX2VERSION, BUILDINFO, LINE);
			Sleep(3000);
			return FALSE;
		}
		if(stopthread) // check if thread isn't running already
		{
			stopthread = 0;
			threadhandle = CreateThread(NULL, 0, InjectThread, NULL, 0, NULL); // start thread, return thread identifier
		}
		INI_Load(); // load settings
		if(!welcomed) // if not welcomed before
			GUI_Welcome(); // show welcome message
		else
			GUI_Update(); // update screen with options
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		GUI_Quit();
		INI_Save();
		HOOK_Quit();
		MOUSE_Quit();
		if(!stopthread) // check if thread is running
		{
			stopthread = 1;
			CloseHandle(threadhandle);
			Sleep(50); // wait for thread to exit, 50ms is plenty of time (pcsx2 crashes if you don't wait long enough... so pls keep this here)
		}
	}
	return TRUE;
}
//==========================================================================
// Purpose: Polls ManyMouse for input and injects into game
//==========================================================================
static DWORD WINAPI InjectThread()
{
	while(!stopthread)
	{
		GUI_Interact(); // check hotkey input
		if(mousetoggle && memready)
		{
			if(GAME_Status()) // if supported game has been detected
			{
				MOUSE_Update(); // update xmouse and ymouse vars so injection use latest mouse input
				GAME_Inject(); // inject mouselook to game
			}
			else // pcsx2 has no game loaded or game not found, wait 100 ms and try again
				Sleep(100);
		}
		Sleep(TICKRATE);
	}
	return 0;
}
//==========================================================================
// Purpose: init console window for gui
//==========================================================================
static void GUI_Init(void)
{
	if(AllocConsole() != 0) // if created console
	{
		AttachConsole(GetCurrentProcessId()); // attach console to pcsx2 process
		SetConsoleTitle("Mouse Injector");
		system("mode 80, 25"); // set window height and width
		freopen("CON", "w", stdout);
	}
	GUI_Clear();
}
//==========================================================================
// Purpose: free and close console window
//==========================================================================
static void GUI_Quit(void)
{
	FreeConsole();
}
//==========================================================================
// Purpose: prints the welcome message
//==========================================================================
static void GUI_Welcome(void)
{
	GUI_Clear();
	printf("\n    Mouse Injector for %s %s\n%s\n\n   Addendum - Please Read before Use\n\n\n", PCSX2VERSION, BUILDINFO, LINE);
	printf("    1)  This is a unfinished test, expect issues and crashes\n\n");
	printf("    2)  Made for PCSX2 1.5.0, newer versions may not work with plugin\n\n");
	printf("    3)  Made for the NTSC release of TimeSplitters 1: SLUS-20090 CRC: B4A004F2\n\n");
	printf("    4)  You must use Windows Messaging for mouse input or injection won't work\n\n");
	printf("    5)  Dynamic controller remapping is unsupported - use arrow keys for menu\n\n");
	printf("    6)  Split-screen multiplayer and MapMaker are unsupported\n\n");
	printf("    7)  Read readme.txt for a quick start guide - thank you and enjoy\n\n\n");
	printf("   Press CTRL+1 to confirm you've read this message...\n%s\n", LINE);
}
//==========================================================================
// Purpose: checks keyboard and will update internal values
// Changed Globals: welcomed, mousetoggle, selectedoption, invertpitch, sensitivity, crosshair, locksettings
//==========================================================================
static void GUI_Interact(void)
{
	if(!welcomed) // if not welcomed before
	{
		if(K_CTRL1) // accept welcome message (CTRL+0)
		{
			welcomed = 1;
			GUI_Update();
		}
		return;
	}
	uint8_t updateinterface = 0, updatequick = 0;
	if(K_4) // mouse toggle (4)
	{
		MOUSE_Lock();
		MOUSE_Update();
		mousetoggle = !mousetoggle;
		updateinterface = 1;
	}
	if(K_5 && !locksettings && !updateinterface) // mouse sensitivity (5)
	{
		selectedoption = EDITINGSENSITIVITY;
		updateinterface = 1;
	}
	if(K_6 && !locksettings && !updateinterface) // crosshair sensitivity (6)
	{
		selectedoption = EDITINGCROSSHAIR;
		updateinterface = 1;
	}
	if(K_7 && !locksettings && !updateinterface) // invert pitch toggle (7)
	{
		invertpitch = !invertpitch;
		updateinterface = 1;
	}
	if(K_PLUS && !locksettings && !updateinterface) // numpad plus (+)
	{
		if(selectedoption == EDITINGSENSITIVITY && sensitivity < 100)
			sensitivity++, updateinterface = 1;
		if(selectedoption == EDITINGCROSSHAIR && crosshair < 18)
			crosshair++, updateinterface = 1;
		updatequick = 1;
	}
	if(K_MINUS && !locksettings && !updateinterface) // numpad minus (-)
	{
		if(selectedoption == EDITINGSENSITIVITY && sensitivity > 1)
			sensitivity--, updateinterface = 1;
		if(selectedoption == EDITINGCROSSHAIR && crosshair > 0)
			crosshair--, updateinterface = 1;
		updatequick = 1;
	}
	if(K_CTRL0 && !updateinterface) // hide/show settings (CTRL+0)
	{
		locksettings = !locksettings;
		updateinterface = 1;
	}
	if(updateinterface)
	{
		GUI_Update();
		Sleep(updatequick ? 100 : 200);
	}
}
//==========================================================================
// Purpose: update interface
//==========================================================================
static void GUI_Update(void)
{
	GUI_Clear();
	if(memready) // memory is safe to read, check for detected game
		printf("\n Mouse Injector for %s %s\n", GAME_Status() ? GAME_Name() : PCSX2VERSION, BUILDINFO); // title
	else
		printf("\n Mouse Injector for %s %s\n", PCSX2VERSION, BUILDINFO); // title
	printf("%s\n\n   Main Menu - Press [#] to Use Menu\n\n\n", LINE);
	printf(mousetoggle ? "   [4] - [ON] Mouse Injection\n\n" : "   [4] - [OFF] Mouse Injection\n\n");
	if(!locksettings)
	{
		printf("   [5] - Mouse Sensitivity: %d%%", sensitivity * 5);
		printf(selectedoption == EDITINGSENSITIVITY ? " [+/-]\n\n" : "\n\n");
		printf("   [6] - Crosshair Sway: ");
		printf(crosshair ? "%d%%" : "Locked", crosshair * 100 / 6);
		printf(selectedoption == EDITINGCROSSHAIR ? " [+/-]\n\n" : "\n\n");
		printf(invertpitch ? "   [7] - [ON] Invert Pitch\n\n" : "   [7] - [OFF] Invert Pitch\n\n");
		printf("\n\n\n\n\n");
		printf("   [CTRL+0] - Lock Settings\n\n");
	}
	else
	{
		printf("\n\n\n\n\n\n\n\n\n\n\n");
		printf("   [CTRL+0] - Unlock Settings\n\n");
	}
	printf(" Note: [+/-] to Change Values\n%s\n", LINE);
}
//==========================================================================
// Purpose: clear screen without using system("cls")
//==========================================================================
static void GUI_Clear(void)
{
	DWORD n; // number of characters written
	DWORD size; // number of visible characters
	COORD coord = {0}; // top left screen position
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE consolehandle = GetStdHandle(STD_OUTPUT_HANDLE); // get a handle to the console
	GetConsoleScreenBufferInfo(consolehandle, &csbi);
	size = csbi.dwSize.X * csbi.dwSize.Y; // find the number of characters to overwrite
	FillConsoleOutputCharacter(consolehandle, TEXT(' '), size, coord, &n); // overwrite the screen buffer with whitespace
	GetConsoleScreenBufferInfo(consolehandle, &csbi);
	FillConsoleOutputAttribute(consolehandle, csbi.wAttributes, size, coord, &n);
	SetConsoleCursorPosition(consolehandle, coord); // reset the cursor to the top left position
}
//==========================================================================
// Purpose: loads settings stored in mouseinjector.ini
// Changes Globals: sensitivity, crosshair, invertpitch, locksettings, welcomed
//==========================================================================
static void INI_Load(void)
{
	FILE *fileptr; // create a file pointer and open mouseinjector.ini from same dir as our program
	if((fileptr = fopen("mouseinjector.ini", "r")) == NULL) // if the INI doesn't exist
	{
		INI_Save(); // create mouseinjector.ini
		return;
	}
	char line[128][128]; // char array used for file to write to
	char lines[128]; // maximum lines read size
	uint8_t counter = 0; // used to assign each line to a array
	while(fgets(lines, sizeof(lines), fileptr) != NULL && counter < 11) // read the first 10 lines
	{
		strcpy(line[counter], lines); // read file lines and assign value to line array
		counter++; // add 1 to counter, so the next line can be read
	}
	fclose(fileptr); // close the file stream
	if(counter == 5) // check if mouseinjector.ini length is valid
	{
		sensitivity = ClampInt(atoi(line[0]), 1, 100);
		crosshair = ClampInt(atoi(line[1]), 0, 18);
		invertpitch = !(!atoi(line[2]));
		locksettings = !(!atoi(line[3]));
		welcomed = !(!atoi(line[4]));
	}
	else
	{
		GUI_Clear();
		printf("\n Mouse Injector for %s %s\n%s\n\n   Loading mouseinjector.ini failed, using default settings...", PCSX2VERSION, BUILDINFO, LINE);
		Sleep(3000);
		INI_Save(); // overwrite mouseinjector.ini with valid settings
	}
}
//==========================================================================
// Purpose: saves current settings to mouseinjector.ini
//==========================================================================
static void INI_Save(void)
{
	FILE *fileptr; // create a file pointer and open mouseinjector.ini from same dir as our program
	if((fileptr = fopen("mouseinjector.ini", "w")) == NULL) // if the INI doesn't exist
		return;
	fprintf(fileptr, "%u\n%u\n%u\n%u\n%u", sensitivity, crosshair, invertpitch, locksettings, welcomed); // write current settings to mouseinjector.ini
	fclose(fileptr); // close the file stream
}