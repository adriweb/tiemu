/* Hey EMACS -*- linux-c -*- */
/* $Id: main.c 245 2004-05-23 20:45:43Z roms $ */

/*  TiEmu - an TI emulator
 *
 *  Copyright (c) 2000-2001, Thomas Corvazier, Romain Lievin
 *  Copyright (c) 2001-2003, Romain Lievin
 *  Copyright (c) 2003, Julien Blache
 *  Copyright (c) 2004, Romain Li�vin
 *  Copyright (c) 2005, Romain Li�vin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __TI68K_MEM92p__
#define __TI68K_MEM92p__

#include "stdint.h"

/* Functions */

int ti92p_mem_init(void);

uint8_t  ti92p_get_byte(uint32_t addr);
uint16_t ti92p_get_word(uint32_t addr);
uint32_t ti92p_get_long(uint32_t addr);

void ti92p_put_long(uint32_t addr, uint32_t arg);
void ti92p_put_word(uint32_t addr, uint16_t arg);
void ti92p_put_byte(uint32_t addr, uint8_t  arg);

uint8_t* ti92p_get_real_addr(uint32_t addr);

#endif
