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
#include <stdlib.h>
#include <stdint.h>
#include "game.h"

extern const GAMEDRIVER *GAME_TS1;

static const GAMEDRIVER **GAMELIST[] =
{
	&GAME_TS1
};

static const GAMEDRIVER *CURRENT_GAME = NULL;
static const uint8_t upper = (sizeof(GAMELIST) / sizeof(GAMELIST[0]));

uint8_t GAME_Status(void);
void GAME_Inject(void);
const char *GAME_Name(void);

//==========================================================================
// Purpose: check all game interfaces for game
//==========================================================================
uint8_t GAME_Status(void)
{
	const GAMEDRIVER *THIS_GAME;
	CURRENT_GAME = NULL;
	for(uint8_t i = 0; (i < upper) && (CURRENT_GAME == NULL); i++)
	{
		THIS_GAME = *(GAMELIST[i]);
		if(THIS_GAME != NULL && THIS_GAME->Status())
			CURRENT_GAME = THIS_GAME;
	}
	return (CURRENT_GAME != NULL);
}
//==========================================================================
// Purpose: inject via game driver
//==========================================================================
void GAME_Inject(void)
{
	if(CURRENT_GAME != NULL)
		CURRENT_GAME->Inject();
}
//==========================================================================
// Purpose: return game driver name
//==========================================================================
const char *GAME_Name(void)
{
	if(CURRENT_GAME != NULL)
		return CURRENT_GAME->Name;
	return NULL;
}