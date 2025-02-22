/* Hey EMACS -*- linux-c -*- */
/* $Id: cabl_err.h 651 2004-04-25 15:22:07Z roms $ */

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

#ifndef __TI68K_ERRCODES__
#define __TI68K_ERRCODES__

/* Error codes ERR_...  */
/* Error codes must begin at 768 up to 1023 */

// New codes
#define ERR_NONE				0
#define ERR_CANT_OPEN			768
#define ERR_INVALID_STATE		769
#define ERR_INVALID_IMAGE		770
#define ERR_INVALID_UPGRADE		771
#define ERR_NO_IMAGE			772
#define ERR_HID_FAILED			773
#define ERR_INVALID_ROM_SIZE	774
#define ERR_NOT_TI_FILE			775
//#define ERR_MALLOC				776
#define ERR_CANT_OPEN_DIR		777
#define ERR_CANT_UPGRADE		778
#define	ERR_INVALID_ROM			779

#endif
