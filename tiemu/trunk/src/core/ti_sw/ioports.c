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

/*
    IO ports definitions loader/parser.
*/

#include <assert.h>
#include <stdio.h>

#include "ti68k_def.h"
#include "ioports.h"

GNode*	tree = NULL;

const char* ioports_get_filename()
{
	tihw.calc_type = TI92;
	switch(tihw.calc_type)
	{
	case TI89:  return "ioports_89.txt";
	case TI89t: return "ioports_89t.txt";
	case TI92:  return "ioports_92.txt";
	case TI92p: return "ioports_92p.txt";
	case V200:  return "ioports_v200.txt";
	}

	return "";
}

// get section name [section]
static char* get_section(char *s)
{
	char *b, *e;

	b = strchr(s, '[');
	e = strrchr(s, ']');

	if(!b ||!e)
	{
		fprintf(stdout, "Missing '[' or ']' token in section name!\n");
		return NULL;
	}

	b++;
	*e = '\0';

	return b;
}

static char* get_name(char *s)
{
	while(*s == ' ') s++;
	return s;
}

// convert "ro", "wo", "rw" into value
static int get_type(const char* s)
{
	while(*s == ' ') s++;

	if(s[0] == 'r' && s[1] == 'o')
		return IO_RO;
	else if(s[0] == 'w' && s[1] == 'o')
		return IO_WO;
	else if(s[0] == 'r' && s[1] == 'w')
		return IO_RW;

	return 0;
}

// parse <..5.....> entry and returns number of available bits
static int get_bits(const char *s, int size, int *bits)
{
	char *b, *e;
	int i, j, nbits = 8 * size;
	int all;

	while(*s == ' ') s++;

	b = strchr(s, '<');
	e = strrchr(s, '>');

	if(!b ||!e)
	{
		fprintf(stdout, "Missing '<' or '>' token !\n");
		return -1;
	}

	b++;
	e--;
	all = !strncmp(b, "all", 3);

	if((e - b + 1) != nbits && !all)
	{
		fprintf(stdout, "Number of bits does not match size !\n");
		return -1;
	}

	memset(bits, 0, nbits);
	for(i = 0, j = 0; i < nbits; i++)
	{
		if(b[i] == '.')
		{
		}
		else if(isdigit(b[i]) || all)
		{
			bits[j++] = nbits - i - 1;
		}
		else
		{
			fprintf(stdout, "Wrong character in bit sequence (digit or '.') !\n");
			return -1;
		}
	}

	return j;
}

/*
	Load information on I/O ports.
	Return 0 if success, -1 otherwise.

	File naming scheme : "ioports_model.txt" => ioports_89.txt
*/
int ioports_load(const char* path)
{
	FILE *f;
	gchar *filename;
	int n;
	char line[1024];

	GNode* parent = NULL;
	GNode* node;
	
	filename = g_strconcat(path, ioports_get_filename(), NULL);
	fprintf(stdout, "Parsing I/O port definitions (%s)... ", filename);


	f = fopen(filename, "rb");
	if(f == NULL)
	{
		g_free(filename);
		return -1;
	}

	tree = g_node_new(NULL);

	for(n = 0; !feof(f);)
	{
		gchar **split;
		IOPORT *s;

		fgets(line, sizeof(line), f);
		line[strlen(line) - 2] = '\0';

		if(line[0] == ';')
			continue;
		else if(line[0] == '[')
		{
			char *name = get_section(line);
			if(name == NULL) return -1;

			s = (IOPORT*)calloc(1, sizeof(IOPORT));
			s->name = strdup(name);

			parent = g_node_new(s);
			g_node_append(tree, parent);

			continue;
		}
		else if(line[0] != '$')
			continue;

		split = g_strsplit(line, "|", 5);
		if(!split[0] || !split[1] || !split[2] || !split[3] || !split[4] )
		{
			fprintf(stderr, "Error at line %i: malformed line !\n", n);
			return -1;
		}

		s = (IOPORT*)calloc(1, sizeof(IOPORT));

		sscanf(split[0], "$%06x", &s->addr);
		sscanf(split[1], "%i", &s->size);
		s->type = get_type(split[2]);
		s->bit_str = strdup(split[3]);
		if((s->nbits = get_bits(split[3], s->size, s->bits)) == -1)
			return -1;
		s->all_bits = (s->nbits == (8 * s->size));
		s->name = strdup(get_name(split[4]));

		if(parent == NULL)
		{
			fprintf(stderr, "Error at line %i: no section defined !\n", n);
			return -1;
		}

		node = g_node_new(s);
		g_node_append(parent, node);

		n++;
	}

	g_free(filename);
	fclose(f);

	fprintf(stdout, "%i entries\n", n);

    return 0;
}

static gboolean free_node(GNode *node, gpointer data)
{
	if (node)
		if(node->data)
			free(node->data);

	return FALSE;
}

int ioports_unload(void)
{
  if(tree != NULL) 
  {
		g_node_traverse(tree, G_IN_ORDER, G_TRAVERSE_ALL, -1, free_node, NULL);
		g_node_destroy(tree);
		tree = NULL;
  }
  return 0;
}

GNode* ioports_tree(void)
{
	return tree;
}