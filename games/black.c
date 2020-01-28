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
#include "../maindll.h"
#include "../mouse.h"
#include "game.h"
#include "memory.h"

#define CROSSHAIRY 70.f // 0x428C0000
// BLACK ADDRESSES - OFFSET ADDRESSES BELOW (REQUIRES PLAYERBASE TO USE)
#define BLACK_camx 0x005A8FA8 - 0x0058E780
#define BLACK_camy 0x005A8FAC - 0x0058E780
#define BLACK_fov 0x005A8D5C - 0x0058E780
// STATIC ADDRESSES BELOW
#define BLACK_playerdata 0x0040F4BC
#define BLACK_gameid 0x003F23C8

static uint8_t BLACK_Status(void);
static void BLACK_DetectMap(void);
static void BLACK_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Black",
	BLACK_Status,
	BLACK_Inject
};

const GAMEDRIVER *GAME_BLACK = &GAMEDRIVER_INTERFACE;

static uint32_t playerbase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t BLACK_Status(void)
{
	return (MEM_ReadUInt(BLACK_gameid) == 0x63616C42U);
}
//==========================================================================
// Purpose: detects player pointer
// Changes Globals: playerbase
//==========================================================================
static void BLACK_DetectMap(void)
{
	const uint32_t tempptr = MEM_ReadUInt(BLACK_playerdata);
	const float camy = MEM_ReadFloat(tempptr + BLACK_camy);
	const uint32_t fov = MEM_ReadUInt(tempptr + BLACK_fov);
	if(playerbase != tempptr && camy >= -CROSSHAIRY && camy <= CROSSHAIRY && fov >= 0x3F800000U && fov <= 0x3F800002U)
		playerbase = tempptr;
}
//==========================================================================
// Purpose: calculate mouse movement and inject
//==========================================================================
static void BLACK_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
	BLACK_DetectMap();
	if(!playerbase) // if playerbase is invalid
		return;
	const float fov = MEM_ReadFloat(playerbase + BLACK_fov);
	const float looksensitivity = (float)sensitivity / 40.0f;
	float camx = MEM_ReadFloat(playerbase + BLACK_camx);
	float camy = MEM_ReadFloat(playerbase + BLACK_camy);
	if(camy >= -180 && camy <= 180 && camy >= -CROSSHAIRY && camy <= CROSSHAIRY)
	{
		camx -= (float)xmouse / 10.f * looksensitivity * (1.f / fov);
		camy += (float)(!invertpitch ? ymouse : -ymouse) / 10.f * looksensitivity * (1.f / fov);
		while(camx < -180)
			camx += 360;
		while(camx > 180)
			camx -= 360;
		MEM_WriteFloat(playerbase + BLACK_camx, camx);
		MEM_WriteFloat(playerbase + BLACK_camy, ClampFloat(camy, -CROSSHAIRY, CROSSHAIRY));
	}
}