//==========================================================================
// Mouse Injector for PCSX2
//==========================================================================
// Copyright (C) 2020 Carnivorous
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
#include <stdint.h>
#include <windows.h>
#include "hook.h"

#define DLLEXPORT __declspec(dllexport)
#define CALLBACK __stdcall

static HINSTANCE lilyhandle = NULL; // handle for lilypad dll
static FARPROC lilyfunc[24] = {NULL}; // stores lilypad's calls addresses

uint8_t memready = 0; // trying to read memory when pcsx2 has no game running will crash - use this as a cheap way to check if we can safely access memory

uint8_t HOOK_Init(void);
void HOOK_Quit(void);
uint8_t HOOK_LoadLilyCalls(void);
DLLEXPORT void CALLBACK PADupdate(int32_t pad);
DLLEXPORT uint32_t CALLBACK PS2EgetLibType(void);
DLLEXPORT uint32_t CALLBACK PS2EgetLibVersion2(uint32_t type);
DLLEXPORT char * CALLBACK PSEgetLibName();
DLLEXPORT char * CALLBACK PS2EgetLibName(void);
DLLEXPORT void CALLBACK PADshutdown();
DLLEXPORT int32_t CALLBACK PADinit(uint32_t flags);
DLLEXPORT int32_t CALLBACK PADopen(void *pDsp);
DLLEXPORT void CALLBACK PADclose();
DLLEXPORT uint8_t CALLBACK PADstartPoll(int32_t pad);
DLLEXPORT uint8_t CALLBACK PADpoll(uint8_t value);
DLLEXPORT uint32_t CALLBACK PADquery();
DLLEXPORT void CALLBACK PADabout();
DLLEXPORT int32_t CALLBACK PADtest();
DLLEXPORT void * CALLBACK PADkeyEvent();
DLLEXPORT uint32_t CALLBACK PADreadPort1(void *pads);
DLLEXPORT uint32_t CALLBACK PADreadPort2(void *pads);
DLLEXPORT uint32_t CALLBACK PSEgetLibType();
DLLEXPORT uint32_t CALLBACK PSEgetLibVersion();
DLLEXPORT void CALLBACK PADconfigure();
DLLEXPORT int32_t CALLBACK PADfreeze(int32_t mode, void *data);
DLLEXPORT int32_t CALLBACK PADsetSlot(uint8_t port, uint8_t slot);
DLLEXPORT int32_t CALLBACK PADqueryMtap(uint8_t port);
DLLEXPORT void CALLBACK PADsetSettingsDir(const char *dir);

//==========================================================================
// Purpose: loads lilypad dll
// Changed Globals: lilyhandle
//==========================================================================
uint8_t HOOK_Init(void)
{
	lilyhandle = LoadLibraryA(".\\plugins\\LilyPad.dll"); // load lilypad.dll
	return (lilyhandle != NULL);
}
//==========================================================================
// Purpose: safely unload lilypad dll
// Changed Globals: lilyhandle
//==========================================================================
void HOOK_Quit(void)
{
	if(lilyhandle != NULL)
	{
		FreeLibrary(lilyhandle); // unload lilypad.dll
		lilyhandle = NULL;
	}
}
//==========================================================================
// Purpose: map function calls from lilypad library
// Changed Globals: lilyfunc
//==========================================================================
uint8_t HOOK_LoadLilyCalls(void)
{
	const char *lilyfuncnames[24] = {"PADupdate", "PS2EgetLibType", "PS2EgetLibVersion2", "PSEgetLibName", "PS2EgetLibName", "PADshutdown", "PADinit", "PADopen", "PADclose", "PADstartPoll", "PADpoll", "PADquery", "PADabout", "PADtest", "PADkeyEvent", "PADreadPort1", "PADreadPort2", "PSEgetLibType", "PSEgetLibVersion", "PADconfigure", "PADfreeze", "PADsetSlot", "PADqueryMtap", "PADsetSettingsDir"};
	for(uint8_t index = 0; index < 24; index++)
	{
		lilyfunc[index] = GetProcAddress(lilyhandle, lilyfuncnames[index]);
		if(lilyfunc[index] == NULL) // if call was not found, abort
			return 0;
	}
	return 1; // all lilypad functions found
}
//==========================================================================
// Purpose: pass through PADupdate call to lilypad
//==========================================================================
DLLEXPORT void CALLBACK PADupdate(int32_t pad)
{
	lilyfunc[0](pad);
}
//==========================================================================
// Purpose: pass through PS2EgetLibType call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PS2EgetLibType(void)
{
	return (uint32_t)lilyfunc[1]();
}
//==========================================================================
// Purpose: pass through PS2EgetLibVersion2 call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PS2EgetLibVersion2(uint32_t type)
{
	return (uint32_t)lilyfunc[2](type);
}
//==========================================================================
// Purpose: pass through PSEgetLibName call to lilypad
//==========================================================================
DLLEXPORT char * CALLBACK PSEgetLibName()
{
	return (char *)lilyfunc[3]();
}
//==========================================================================
// Purpose: pass through PS2EgetLibName call to lilypad
//==========================================================================
DLLEXPORT char * CALLBACK PS2EgetLibName(void)
{
	return (char *)lilyfunc[4]();
}
//==========================================================================
// Purpose: pass through PADshutdown call to lilypad  (when PCSX2 is shutting down DLL)
//==========================================================================
DLLEXPORT void CALLBACK PADshutdown()
{
	memready = 0;
	lilyfunc[5]();
}
//==========================================================================
// Purpose: pass through PADinit call to lilypad
//==========================================================================
DLLEXPORT int32_t CALLBACK PADinit(uint32_t flags)
{
	return (int32_t)lilyfunc[6](flags);
}
//==========================================================================
// Purpose: pass through PADopen call to lilypad (when game has started/unpaused)
// Changed Globals: memready
//==========================================================================
DLLEXPORT int32_t CALLBACK PADopen(void *pDsp)
{
	memready = 1;
	return (int32_t)lilyfunc[7](pDsp);
}
//==========================================================================
// Purpose: pass through PADclose call to lilypad (when game has stopped/paused)
// Changed Globals: memready
//==========================================================================
DLLEXPORT void CALLBACK PADclose()
{
	memready = 0;
	lilyfunc[8]();
}
//==========================================================================
// Purpose: pass through PADstartPoll call to lilypad
//==========================================================================
DLLEXPORT uint8_t CALLBACK PADstartPoll(int32_t pad)
{
	return (uint8_t)lilyfunc[9](pad);
}
//==========================================================================
// Purpose: pass through PADpoll call to lilypad
//==========================================================================
DLLEXPORT uint8_t CALLBACK PADpoll(uint8_t value)
{
	return (uint8_t)lilyfunc[10](value);
}
//==========================================================================
// Purpose: pass through PADquery call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PADquery()
{
	return (uint32_t)lilyfunc[11]();
}
//==========================================================================
// Purpose: pass through PADabout call to lilypad
//==========================================================================
DLLEXPORT void CALLBACK PADabout()
{
	lilyfunc[12]();
}
//==========================================================================
// Purpose: pass through PADtest call to lilypad
//==========================================================================
DLLEXPORT int32_t CALLBACK PADtest()
{
	return (int32_t)lilyfunc[13]();
}
//==========================================================================
// Purpose: pass through PADkeyEvent call to lilypad
//==========================================================================
DLLEXPORT void * CALLBACK PADkeyEvent()
{
	return (void *)lilyfunc[14]();
}
//==========================================================================
// Purpose: pass through PADreadPort1 call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PADreadPort1(void *pads)
{
	return (uint32_t)lilyfunc[15](pads);
}
//==========================================================================
// Purpose: pass through PADreadPort2 call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PADreadPort2(void *pads)
{
	return (uint32_t)lilyfunc[16](pads);
}
//==========================================================================
// Purpose: pass through PSEgetLibType call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PSEgetLibType()
{
	return (uint32_t)lilyfunc[17]();
}
//==========================================================================
// Purpose: pass through PSEgetLibVersion call to lilypad
//==========================================================================
DLLEXPORT uint32_t CALLBACK PSEgetLibVersion()
{
	return (uint32_t)lilyfunc[18]();
}
//==========================================================================
// Purpose: pass through PADconfigure call to lilypad
//==========================================================================
DLLEXPORT void CALLBACK PADconfigure()
{
	lilyfunc[19]();
}
//==========================================================================
// Purpose: pass through PADfreeze call to lilypad
//==========================================================================
DLLEXPORT int32_t CALLBACK PADfreeze(int32_t mode, void *data)
{
	return (int32_t)lilyfunc[20](mode, data);
}
//==========================================================================
// Purpose: pass through PADsetSlot call to lilypad
//==========================================================================
DLLEXPORT int32_t CALLBACK PADsetSlot(uint8_t port, uint8_t slot)
{
	return (int32_t)lilyfunc[21](port, slot);
}
//==========================================================================
// Purpose: pass through PADqueryMtap call to lilypad
//==========================================================================
DLLEXPORT int32_t CALLBACK PADqueryMtap(uint8_t port)
{
	return (int32_t)lilyfunc[22](port);
}
//==========================================================================
// Purpose: pass through PADsetSettingsDir call to lilypad
//==========================================================================
DLLEXPORT void CALLBACK PADsetSettingsDir(const char *dir)
{
	lilyfunc[23](dir);
}