/*  gtktiemu - a TI89/92/92+ emulator
 *  (c) Copyright 2000-2001, Romain Lievin and Thomas Corvazier
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

/*
  Transcoding of error codes into message strings
*/

#include <stdio.h>
#include <string.h>

#include "intl.h"
#include "errcodes.h"

/* 
   This function put in err_msg the error message corresponding to the 
   error code.
   If the error code has been handled, the function returns 0 else it 
   propagates the error code by returning it.
*/
int ti68k_getError(int err_num, char *error_msg)
{
  switch(err_num)
    {
    case ERR_NONE:
      strcpy(error_msg, _("No error."));
      break;
    case ERR_INVALID_ROM: 
      strcpy(error_msg, _("Invalid ROM."));
      break;
    case ERR_INVALID_SIZE: 
      strcpy(error_msg, _("Invalid ROM size."));
      break;
    case ERR_CANT_OPEN:
      strcpy(error_msg, _("Can not open ROM."));
      break;
    case ERR_CANT_CLOSE:
      strcpy(error_msg, _("Unable to close file."));
      break;
    case ERR_INTERNAL:
      strcpy(error_msg, _("Internal error."));
      break;
    case ERR_TI_FILE:
      strcpy(error_msg, _("Invalid TI file."));
      break;
    case ERR_INVALID_FLASH:
      strcpy(error_msg, _("Invalid FLASH file."));
      break;
    case ERR_ROM_NOT_LOADED:
      strcpy(error_msg, _("A ROM should have been loaded before."));
      break;
    case ERR_CANT_OPEN_DIR:
      strcpy(error_msg, _("Unable to open directory."));
      break;
    case ERR_CANT_CLOSE_DIR:
      strcpy(error_msg, ("Unable to close directory."));
      break;
    case ERR_INVALID_FILE:
      strcpy(error_msg, ("Invalid file."));
      break;
    case ERR_INVALID_STATE:
      strcpy(error_msg, ("Invalid state file."));
      break;
    case ERR_INVALID_MODE:
      strcpy(error_msg, ("Breakpoint, invalid mode."));
      break;
    default:
      strcpy(error_msg, _("Error code not found in the list.\nThis is a bug. Please report it.\n."));
      return err_num;
      //      break;
    }
  
  return 0;
}

