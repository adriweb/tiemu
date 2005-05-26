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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include "support.h"

extern struct progress_window {
	GtkWidget *window;
	GtkWidget *pbar1;
	GtkWidget *pbar2;
	GtkWidget *label;
	GtkWidget *label_rate;
} p_win;

void create_pbar_type1(const gchar * title);
void create_pbar_type2(const gchar * title, gchar * text);
void create_pbar_type3(const gchar * title);
void create_pbar_type4(const gchar * title, gchar * text);
void create_pbar_type5(const gchar * title, gchar * text);
void destroy_pbar(void);

GLADE_CB void on_pbar_okbutton1_pressed(GtkButton * button,
					gpointer user_data);
