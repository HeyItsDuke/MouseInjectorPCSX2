//==========================================================================
// Mouse Injector for PCSX2
//==========================================================================
// Copyright (C) 2019 Carnivorous
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL CARNIVOROUS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//==========================================================================
#include <stdint.h>
#include "../maindll.h"
#include "../mouse.h"
#include "game.h"
#include "memory.h"

#define CROSSHAIRLIMIT 0.9999997616f // 0x3F7FFFFC
// TIMESPLITTERS ADDRESSES - OFFSET ADDRESSES BELOW (REQUIRES PLAYERBASE TO USE)
#define TS1_camx 0x0123E394 - 0x0123E280
#define TS1_camy 0x0123E39C - 0x0123E280
#define TS1_fov 0x0123E384 - 0x0123E280
#define TS1_crosshairx 0x0123E5AC - 0x0123E280
#define TS1_crosshairy 0x0123E5B0 - 0x0123E280
#define TS1_aimingflag 0x0123E294 - 0x0123E280
// STATIC ADDRESSES BELOW
#define TS1_playerdata 0x003A40A0
#define TS1_pause 0x003A4374
#define TS1_menu 0x00347BAC
#define TS1_gameid 0x00395608

static uint8_t TS1_Status(void);
static void TS1_DetectMap(void);
static void TS1_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"TimeSplitters",
	TS1_Status,
	TS1_Inject
};

const GAMEDRIVER *GAME_TS1 = &GAMEDRIVER_INTERFACE;

static uint32_t playerbase = 0;
static const uint32_t invalidptrs[7] = {0x00000000, 0x00C7F6C0, 0x01DD5E10, 0x01DD5E20, 0x01DD5E40, 0x00FFF6C0, 0x0104AD20};

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t TS1_Status(void)
{
	return ((uint32_t)MEM_ReadInt(TS1_gameid) == 0x53554C53U);
}
//==========================================================================
// Purpose: detects player pointer
// Changes Globals: playerbase
//==========================================================================
static void TS1_DetectMap(void)
{
	const uint32_t tempptr = (uint32_t)MEM_ReadInt(TS1_playerdata);
	for(int index = 0; index < 7; index++)
	{
		if(tempptr == invalidptrs[index]) // if player pointer is invalid (menu, credits, map maker)
		{
			playerbase = 0;
			return;
		}
	}
	const float camy = MEM_ReadFloat(tempptr + TS1_camy);
	const uint32_t fov = MEM_ReadInt(tempptr + TS1_fov);
	if(playerbase != tempptr && camy >= -50 && camy <= 50 && fov == 0x42700000U)
		playerbase = tempptr;
}
//==========================================================================
// Purpose: calculate mouse movement and inject
//==========================================================================
static void TS1_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	TS1_DetectMap();
	if(!playerbase) // if playerbase is invalid
		return;
	const int pause = MEM_ReadInt(TS1_pause);
	const int menu = MEM_ReadInt(TS1_menu);
	const int aimingflag = MEM_ReadInt(playerbase + TS1_aimingflag);
	const float fov = MEM_ReadFloat(playerbase + TS1_fov);
	const float looksensitivity = (float)sensitivity / 40.0f;
	const float crosshairsensitivity = ((float)crosshair / 80.f) * looksensitivity;
	float camx = MEM_ReadFloat(playerbase + TS1_camx);
	float camy = MEM_ReadFloat(playerbase + TS1_camy);
	float crosshairx = MEM_ReadFloat(playerbase + TS1_crosshairx);
	float crosshairy = MEM_ReadFloat(playerbase + TS1_crosshairy);
	if(camx >= 0 && camx < 360 && camy >= -50 && camy <= 50 && fov >= 5 && fov <= 60 && pause == 0 && menu == 0 && aimingflag >= 0 && aimingflag <= 3)
	{
		camx -= (float)xmouse / 10.f * looksensitivity * (fov / 60.f);
		camy -= (float)(!invertpitch ? ymouse : -ymouse) / 10.f * looksensitivity * (fov / 60.f);
		if(camx < 0)
			camx += 360;
		else if(camx >= 360)
			camx -= 360;
		MEM_WriteFloat(playerbase + TS1_camx, camx);
		MEM_WriteFloat(playerbase + TS1_camy, ClampFloat(camy, -50, 50));
 		if(crosshair)
		{
			crosshairx += (float)xmouse / 40.f * crosshairsensitivity * (fov / 60);
			crosshairy += (float)(!invertpitch ? ymouse : -ymouse) / 40.f * crosshairsensitivity * (fov / 60);
			MEM_WriteFloat(playerbase + TS1_crosshairx, ClampFloat(crosshairx, -CROSSHAIRLIMIT, CROSSHAIRLIMIT));
			MEM_WriteFloat(playerbase + TS1_crosshairy, ClampFloat(crosshairy, -CROSSHAIRLIMIT, CROSSHAIRLIMIT));
		}
	}
}