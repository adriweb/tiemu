/* Hey EMACS -*- linux-c -*- */
/* $Id: intl.h 366 2004-03-22 18:04:07Z roms $ */

/*  skinedit - a skin editor for the TiEmu emulator
 *  Copyright (C) 1999-2004  Romain Lievin
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
  Contains the right headers for the internationalization library
*/

#ifndef SKIN_INTL_H
#define SKIN_INTL_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <locale.h>

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#if defined(__WIN32__) && !defined(__MINGW32__)
# undef PACKAGE
# define PACKAGE "skinedit"	// name of package
# define LOCALEDIR ""		// place of the translated file
# define VERSION "1.28"	// version
#endif

#endif
