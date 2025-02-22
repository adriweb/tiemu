/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  TiEmu - an TI emulator
 *
 *  Copyright (c) 2000, Thomas Corvazier, Romain Lievin
 *  Copyright (c) 2001-2002, Romain Lievin, Julien Blache
 *  Copyright (c) 2003-2004, Romain Li�vin
 *  Copyright (c) 2005, Romain Li�vin, Kevin Kofler
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

#ifndef __TI68K_STATE__
#define __TI68K_STATE__

// Used to locate sections
typedef struct
{
	long	revision;		// structure revision
	long	size;			// and size (backwards compatibility)
	long    regs_offset;    // offset to M68K area
    long    io_offset;      // offset to IO area
    long    ram_offset;     // offset to RAM area
	long	misc_offset;	// offset to extra informations
    long    bkpts_offset;   // offset to bkpts area
	long	rom_offset;		// offset to FLASH changes
} SAV_INFO;

int ti68k_state_load(char *filename);
int ti68k_state_save(char *filename);

#endif
