/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

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
 *  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __TI68K_INTF__
#define __TI68K_INTF__

#include "ti68k_def.h"
#include "tilibs.h"
 
/*************/
/* Functions */
/*************/

//Note: [ti68k]_[group]_[short_or_long_name]

// Initialization
int ti68k_config_load_default(void);

int ti68k_init(void);
int ti68k_reset(void);
int ti68k_exit(void);

unsigned int ti68k_get_cycle_count(int reset, unsigned int *diff);

// Link
int ti68k_linkport_send_file(const char *filename);
int ti68k_linkport_reconfigure(void);

// Keyboard
void ti68k_kbd_set_key(int key, int active);

// Misc
int ti68k_get_rom_size(int calc_type);
int ti68k_get_ram_size(int calc_type);
int ti68k_get_io_size(int calc_type);
int ti68k_get_io2_size(int calc_type);
int ti68k_get_io3_size(int calc_type);

uint8_t* ti68k_get_real_address(uint32_t addr);

// Others
#include "bkpts.h"
#include "images.h"
#include "registers.h"
#include "state.h"
#include "type2str.h"
#include "debug.h"
#include "timem.h"
#include "vat.h"

// Errors
int ti68k_error_get(int err_num, char *error_msg);

#endif
