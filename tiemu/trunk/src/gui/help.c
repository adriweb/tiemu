/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  tilp - Ti Linking Program
 *  Copyright (C) 1999-2004  Romain Lievin
 *
 *  This program is free software you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif				/*  */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "intl.h"
#include "help.h"
#include "paths.h"
#include "engine.h"

gint display_help_dbox()
{
	GladeXML *xml;
	GtkWidget *dbox;
	GtkTextBuffer *txtbuf;
	GtkWidget *text;
	FILE *fd;
	gchar *filename;
	gchar buffer[32768];
	gint len = 0;
	struct stat stbuf;
	gint result;
#if 0
	GtkTextIter start, end;
#endif

	filename =
	    g_strconcat(inst_paths.help_dir, "help_en_us", NULL);
	if (access(filename, F_OK) == 0) {
		if (stat(filename, &stbuf) != -1) {
			len = stbuf.st_size;
			len -= 2;
		}
		if ((fd = fopen(filename, "r")) != NULL) {
			memset(buffer, 0, sizeof(buffer));
			len = fread(buffer, 1, len, fd);
			fclose(fd);
		}
	}

	xml = glade_xml_new
	    (tilp_paths_build_glade("manpage-2.glade"), "manpage_dbox",
	     PACKAGE);
	if (!xml)
		g_error("GUI loading failed !\n");
	glade_xml_signal_autoconnect(xml);

	dbox = glade_xml_get_widget(xml, "manpage_dbox");
	text = glade_xml_get_widget(xml, "textview1");
	txtbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
#if 0
	gtk_text_buffer_get_bounds(txtbuf, &start, &end);
	gtk_text_buffer_delete(txtbuf, &start, &end);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 15);
#endif
	gtk_text_buffer_set_text(txtbuf, buffer, len);
	gtk_widget_realize(dbox);
	gtk_widget_show(dbox);
	//gtk_window_resize(GTK_WINDOW(dbox), 640, 480);

	while (gtk_events_pending())
		gtk_main_iteration();

	result = gtk_dialog_run(GTK_DIALOG(dbox));
	switch (result) {
	case GTK_RESPONSE_OK:
		ti68k_engine_unhalt();
		break;
	default:
		break;
	}

	gtk_widget_destroy(dbox);

	return 0;
}
