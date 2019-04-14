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
#define PCSX2MEMOFFSET 0x20000000
#define MEMSHORT(X) *(int16_t *)(X + PCSX2MEMOFFSET)
#define MEMINT(X) *(int32_t *)(X + PCSX2MEMOFFSET)
#define MEMFLOAT(X) *(float *)(X + PCSX2MEMOFFSET)

#define WITHINRANGE(X) (X < 0x02000000U)
#define GARBAGESHORT (int16_t)0xABAD
#define GARBAGEINT (int32_t)0xABADC0DE

//==========================================================================
// Purpose: read short from memory
// Parameter: address
//==========================================================================
static inline int16_t MEM_ReadShort(const uint32_t addr)
{
	return WITHINRANGE(addr) ? MEMSHORT(addr) : GARBAGESHORT; // return garbage if request invalid location
}
//==========================================================================
// Purpose: write short to memory location
// Parameter: address, value
//==========================================================================
static inline void MEM_WriteShort(const uint32_t addr, const int16_t value)
{
	if(WITHINRANGE(addr))
		MEMSHORT(addr) = value;
}
//==========================================================================
// Purpose: read int from memory
// Parameter: address
//==========================================================================
static inline int32_t MEM_ReadInt(const uint32_t addr)
{
	return WITHINRANGE(addr) ? MEMINT(addr) : GARBAGEINT;
}
//==========================================================================
// Purpose: write int to memory location
// Parameter: address, value
//==========================================================================
static inline void MEM_WriteInt(const uint32_t addr, const int value)
{
	if(WITHINRANGE(addr))
		MEMINT(addr) = value;
}
//==========================================================================
// Purpose: read float from memory
// Parameter: address
//==========================================================================
static inline float MEM_ReadFloat(const uint32_t addr)
{
	return WITHINRANGE(addr) ? MEMFLOAT(addr) : GARBAGEINT;
}
//==========================================================================
// Purpose: write float to memory location
// Parameter: address, value
//==========================================================================
static inline void MEM_WriteFloat(const uint32_t addr, const float value)
{
	if(WITHINRANGE(addr))
		MEMFLOAT(addr) = value;
}