/*==============================================================================================
    
    snapshot.cpp
    Copyright 2019-2021 Buster Schrader
    
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
    
==============================================================================================*/

#include "snapshot.hpp"

snapshot::snapshot(int prog_size)
{
    sys_val[ERR_NR] = 0xFF;
    sys_val[FLAGS] =  0x01;
    sys_val[ERR_SP] = 0xFC; sys_val[ERR_SP+1] = 0x47;
    sys_val[RAMTOP] = 0x00; sys_val[RAMTOP+1] = 0x48;
    sys_val[MODE] =   0x00;
    sys_val[PPC] =    0x00; sys_val[PPC+1] =    0x00;
    sys_val[VERSN] =  0x00;
    sys_val[E_PPC] =  0x00; sys_val[E_PPC+1] =  0x00;
    sys_val[D_FILE] = 0x7D; sys_val[D_FILE+1] = 0x40;
    sys_val[DF_CC] =  0x97; sys_val[DF_CC+1] =  0x40;
    sys_val[VARS] =   0x97; sys_val[VARS+1] =   0x40;
    sys_val[DEST] =   0x00; sys_val[DEST+1] =   0x00;
    sys_val[E_LINE] = 0x98; sys_val[E_LINE+1] = 0x40;
    sys_val[CH_ADD] = 0x9A; sys_val[CH_ADD+1] = 0x40;
    sys_val[X_PTR] =  0x00; sys_val[X_PTR+1] =  0x00;
    sys_val[STKBOT] = 0x9A; sys_val[STKBOT+1] = 0x40;
    sys_val[STKEND] = 0x9A; sys_val[STKEND+1] = 0x40;
    sys_val[BREG] =   0xFF;
    sys_val[MEM] =    0x00; sys_val[MEM+1] =    0x00;
    sys_val[DF_SZ] =  0x02;
    sys_val[S_TOP] =  0x00; sys_val[S_TOP+1] =  0x00;
    sys_val[LAST_K] = 0xFF; sys_val[LAST_K+1] = 0xFF;
    sys_val[LK_DB] =  0x00;
    sys_val[MARGIN] = 0x1F;
    sys_val[NXTLIN] = 0x00; sys_val[NXTLIN+1] = 0x00;
    sys_val[OLDPPC] = 0x00; sys_val[OLDPPC+1] = 0x00;
    sys_val[FLAGX] =  0x00;
    sys_val[STRLEN] = 0x00; sys_val[STRLEN+1] = 0x00;
    sys_val[T_ADDR] = 0x00; sys_val[T_ADDR+1] = 0x00;
    sys_val[SEED] =   0x00; sys_val[SEED+1] =   0x00;
    sys_val[FRAMES] = 0x00; sys_val[FRAMES+1] = 0x00; //CHECK THIS PAIR
    sys_val[COORDS] = 0x00; sys_val[COORDS+1] = 0x00;
    sys_val[PR_CC] =  0x00;
    sys_val[S_POSN] = 0x21; sys_val[S_POSN+1] = 0x00;
    sys_val[CDFLAG] = 0xC0;
    
    for (int i = 0; i < 33; i++)
    {
        sys_val[PRBUFF+i] = 0x00;
    }
}
