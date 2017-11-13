/*==============================================================================================
	
	snapshot.hpp
	Copyright 2019 Buster Schrader
	
	This file is part of SIASM.
	
	SIASM is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	any later version.
	
	SIASM is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with SIASM.  If not, see <https://www.gnu.org/licenses/>.
	
==============================================================================================

	Z81 Snapshot
	1/26/17 - B.D.S.
	Purpose: Provides typeless interfaces to emulator.
	
==============================================================================================*/

#ifndef _SNAPSHOT_HPP
#define _SNAPSHOT_HPP

#include "bs_util.hpp"

#define ERR_NR	0x00
#define FLAGS	0x01
#define ERR_SP	0x02
#define RAMTOP	0x04
#define MODE	0x06
#define PPC		0x07
#define VERSN	0x09
#define E_PPC	0x0A
#define D_FILE	0x0C
#define DF_CC	0x0E
#define VARS	0x10
#define DEST	0x12
#define E_LINE	0x14
#define CH_ADD	0x16
#define X_PTR	0x18
#define STKBOT	0x1A
#define STKEND	0x1C
#define BREG	0x1E
#define MEM		0x1F
#define DF_SZ	0x22
#define S_TOP	0x23
#define LAST_K	0x25
#define LK_DB	0x27
#define MARGIN	0x28
#define NXTLIN	0x29
#define OLDPPC	0x2B
#define FLAGX	0x2D
#define STRLEN	0x2E
#define T_ADDR	0x30
#define SEED	0x32
#define FRAMES	0x34
#define COORDS	0x36
#define PR_CC	0x38
#define S_POSN	0x39
#define CDFLAG	0x3B
#define PRBUFF	0x3C
#define MEMBOT	0x5D

class snapshot
{
	snapshot(int prog_size);
	uchar sys_val[2048];
};

#endif
