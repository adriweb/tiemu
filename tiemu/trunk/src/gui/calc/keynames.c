/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  TiEmu - Tiemu Is an EMUlator
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "keydefs.h"
#include "pckeys.h"

typedef struct {
    int         value;
    const char  *name;
} KeyTuple;

const KeyTuple tikeys[] = {
    { TIKEY_DOWN,       "TIKEY_DOWN" },
    { TIKEY_RIGHT,      "TIKEY_RIGHT" },
    { TIKEY_UP,         "TIKEY_UP" },
    { TIKEY_LEFT,       "TIKEY_LEFT" },  
    { TIKEY_HAND,       "TIKEY_HAND" },
    { TIKEY_SHIFT,      "TIKEY_SHIFT" },
    { TIKEY_DIAMOND,    "TIKEY_DIAMOND" },
    { TIKEY_2ND,        "TIKEY_2ND" },
    { TIKEY_3,          "TIKEY_3" },
    { TIKEY_2,          "TIKEY_2" },
    { TIKEY_1,          "TIKEY_1" },
    { TIKEY_F8,         "TIKEY_F8" },
    { TIKEY_W,          "TIKEY_W" },
    { TIKEY_S,          "TIKEY_S" },
    { TIKEY_Z,          "TIKEY_Z" },
    { TIKEY_6,          "TIKEY_6" },
    { TIKEY_5,          "TIKEY_5" },
    { TIKEY_4,          "TIKEY_4" },
    { TIKEY_F3,         "TIKEY_F3" },
    { TIKEY_E,          "TIKEY_E" },
    { TIKEY_D,          "TIKEY_D" },
    { TIKEY_X,          "TIKEY_X" },
    { TIKEY_9,          "TIKEY_9" },
    { TIKEY_8,          "TIKEY_8" },
    { TIKEY_7,          "TIKEY_7" },
    { TIKEY_F7,         "TIKEY_F7" },
    { TIKEY_R,          "TIKEY_R" },
    { TIKEY_F,          "TIKEY_F" },
    { TIKEY_C,          "TIKEY_C" },
    { TIKEY_STORE,      "TIKEY_STORE" },
    { TIKEY_COMMA,      "TIKEY_COMMA" },
    { TIKEY_PARIGHT,    "TIKEY_PARIGHT" },
    { TIKEY_PALEFT,     "TIKEY_PALEFT" },
    { TIKEY_F2,         "TIKEY_F2" },
    { TIKEY_T,          "TIKEY_T" },
    { TIKEY_G,          "TIKEY_G"  },
    { TIKEY_V,          "TIKEY_V" },
    { TIKEY_SPACE,      "TIKEY_SPACE"},
    { TIKEY_TAN,        "TIKEY_TAN"},
    { TIKEY_COS,        "TIKEY_COS" },
    { TIKEY_SIN,        "TIKEY_SIN" },
    { TIKEY_F6,         "TIKEY_F6" },
    { TIKEY_Y,          "TIKEY_Y" },
    { TIKEY_H,          "TIKEY_H" },
    { TIKEY_B,          "TIKEY_B" },
    { TIKEY_DIVIDE,     "TIKEY_DIVIDE"},
    { TIKEY_P,          "TIKEY_P"}, 
    { TIKEY_ENTER2,     "TIKEY_ENTER2"}, 
    { TIKEY_LN,         "TIKEY_LN" },
    { TIKEY_F1,         "TIKEY_F1" },
    { TIKEY_U,          "TIKEY_U" },
    { TIKEY_J,          "TIKEY_J" },
    { TIKEY_N,          "TIKEY_N" },
    { TIKEY_POWER,      "TIKEY_POWER"},
    { TIKEY_MULTIPLY,   "TIKEY_MULTIPLY" },
    { TIKEY_APPS,       "TIKEY_APPS" },
    { TIKEY_CLEAR,      "TIKEY_CLEAR" },
    { TIKEY_F5,         "TIKEY_F5"  },
    { TIKEY_I,          "TIKEY_I" },
    { TIKEY_K,          "TIKEY_K" },
    { TIKEY_M,          "TIKEY_M" },
    { TIKEY_EQUALS,     "TIKEY_EQUALS"},
    { TIKEY_NU,         "TIKEY_NU"}, 
    { TIKEY_ESCAPE,     "TIKEY_ESCAPE"}, 
    { TIKEY_MODE,       "TIKEY_MODE"}, 
    { TIKEY_PLUS,       "TIKEY_PLUS"}, 
    { TIKEY_O,          "TIKEY_O" },
    { TIKEY_L,          "TIKEY_L" },
    { TIKEY_THETA,      "TIKEY_THETA" },
    { TIKEY_BACKSPACE,  "TIKEY_BACKSPACE" },
    { TIKEY_NEGATE,     "TIKEY_NEGATE"}, 
    { TIKEY_PERIOD,     "TIKEY_PERIOD" },
    { TIKEY_0,          "TIKEY_0" },
    { TIKEY_F4,         "TIKEY_F4" },
    { TIKEY_Q,          "TIKEY_Q" },
    { TIKEY_A,          "TIKEY_A" },
    { TIKEY_ENTER1,     "TIKEY_ENTER1"}, 
    { TIKEY_MINUS,      "TIKEY_MINUS"},
    { TIKEY_ON,         "TIKEY_ON"},
    { TIKEY_ALPHA,      "TIKEY_ALPHA" },
    { TIKEY_EE,         "TIKEY_EE"  },
    { TIKEY_CATALOG,    "TIKEY_CATALOG" },
    { TIKEY_HOME,       "TIKEY_HOME" },
    { TIKEY_PIPE,       "TIKEY_PIPE" },
    { TIKEY_VOID,       "TIKEY_VOID"},
    { -1, "" },
};

const KeyTuple pckeys[] = {
	{ PCKEY_LBUTTON, "PCKEY_LBUTTON" },
	{ PCKEY_RBUTTON, "PCKEY_RBUTTON" },
	{ PCKEY_CANCEL, "PCKEY_CANCEL" },
	{ PCKEY_MBUTTON, "PCKEY_MBUTTON" },
	{ PCKEY_BACK, "PCKEY_BACK" },
	{ PCKEY_TAB, "PCKEY_TAB" },
	{ PCKEY_CLEAR, "PCKEY_CLEAR" },
	{ PCKEY_RETURN, "PCKEY_RETURN" },
	{ PCKEY_SHIFT_L, "PCKEY_SHIFT_L" },
	{ PCKEY_CONTROL_L, "PCKEY_CONTROL_L" },
#ifndef __MACOSX__
/* FIXME: We need the key codes for these on OS X. */
	{ PCKEY_SHIFT_R, "PCKEY_SHIFT_R" },
	{ PCKEY_CONTROL_R, "PCKEY_CONTROL_R" },
#endif
	{ PCKEY_MENU, "PCKEY_MENU" },
	{ PCKEY_PAUSE, "PCKEY_PAUSE" },
	{ PCKEY_CAPITAL, "PCKEY_CAPITAL" },
	{ PCKEY_ESCAPE, "PCKEY_ESCAPE" },
	{ PCKEY_SPACE, "PCKEY_SPACE" },
	{ PCKEY_PRIOR, "PCKEY_PRIOR" },
	{ PCKEY_NEXT, "PCKEY_NEXT" },
	{ PCKEY_END, "PCKEY_END" },
	{ PCKEY_HOME, "PCKEY_HOME" },
	{ PCKEY_LEFT, "PCKEY_LEFT" },
	{ PCKEY_UP, "PCKEY_UP" },
	{ PCKEY_RIGHT, "PCKEY_RIGHT" },
	{ PCKEY_DOWN, "PCKEY_DOWN" },
	{ PCKEY_SELECT, "PCKEY_SELECT" },
	{ PCKEY_PRINT, "PCKEY_PRINT" },
	{ PCKEY_EXECUTE, "PCKEY_EXECUTE" },
	{ PCKEY_SNAPSHOT, "PCKEY_SNAPSHOT" },
	{ PCKEY_INSERT, "PCKEY_INSERT" },
	{ PCKEY_DELETE, "PCKEY_DELETE" },
	{ PCKEY_HELP, "PCKEY_HELP" },
	{ PCKEY_0, "PCKEY_0" },
	{ PCKEY_1, "PCKEY_1" },
	{ PCKEY_2, "PCKEY_2" },
	{ PCKEY_3, "PCKEY_3" },
	{ PCKEY_4, "PCKEY_4" },
	{ PCKEY_5, "PCKEY_5" },
	{ PCKEY_6, "PCKEY_6" },
	{ PCKEY_7, "PCKEY_7" },
	{ PCKEY_8, "PCKEY_8" },
	{ PCKEY_9, "PCKEY_9" },
	{ PCKEY_A, "PCKEY_A" },
	{ PCKEY_B, "PCKEY_B" },
	{ PCKEY_C, "PCKEY_C" },
	{ PCKEY_D, "PCKEY_D" },
	{ PCKEY_E, "PCKEY_E" },
	{ PCKEY_F, "PCKEY_F" },
	{ PCKEY_G, "PCKEY_G" },
	{ PCKEY_H, "PCKEY_H" },
	{ PCKEY_I, "PCKEY_I" },
	{ PCKEY_J, "PCKEY_J" },
	{ PCKEY_K, "PCKEY_K" },
	{ PCKEY_L, "PCKEY_L" },
	{ PCKEY_M, "PCKEY_M" },
	{ PCKEY_N, "PCKEY_N" },
	{ PCKEY_O, "PCKEY_O" },
	{ PCKEY_P, "PCKEY_P" },
	{ PCKEY_Q, "PCKEY_Q" },
	{ PCKEY_R, "PCKEY_R" },
	{ PCKEY_S, "PCKEY_S" },
	{ PCKEY_T, "PCKEY_T" },
	{ PCKEY_U, "PCKEY_U" },
	{ PCKEY_V, "PCKEY_V" },
	{ PCKEY_W, "PCKEY_W" },
	{ PCKEY_X, "PCKEY_X" },
	{ PCKEY_Y, "PCKEY_Y" },
	{ PCKEY_Z, "PCKEY_Z" },
	{ PCKEY_STARTKEY, "PCKEY_STARTKEY" },
	{ PCKEY_CONTEXTKEY, "PCKEY_CONTEXTKEY" },
	{ PCKEY_NUMPAD0, "PCKEY_NUMPAD0" },
	{ PCKEY_NUMPAD1, "PCKEY_NUMPAD1" },
	{ PCKEY_NUMPAD2, "PCKEY_NUMPAD2" },
	{ PCKEY_NUMPAD3, "PCKEY_NUMPAD3" },
	{ PCKEY_NUMPAD4, "PCKEY_NUMPAD4" },
	{ PCKEY_NUMPAD5, "PCKEY_NUMPAD5" },
	{ PCKEY_NUMPAD6, "PCKEY_NUMPAD6" },
	{ PCKEY_NUMPAD7, "PCKEY_NUMPAD7" },
	{ PCKEY_NUMPAD8, "PCKEY_NUMPAD8" },
	{ PCKEY_NUMPAD9, "PCKEY_NUMPAD9" },
	{ PCKEY_MULTIPLY, "PCKEY_MULTIPLY" },
	{ PCKEY_ADD, "PCKEY_ADD" },
	{ PCKEY_SEPARATOR, "PCKEY_SEPARATOR" },
	{ PCKEY_SUBTRACT, "PCKEY_SUBTRACT" },
	{ PCKEY_DECIMAL, "PCKEY_DECIMAL" },
	{ PCKEY_DIVIDE, "PCKEY_DIVIDE" },
	{ PCKEY_F1, "PCKEY_F1" },
	{ PCKEY_F2, "PCKEY_F2" },
	{ PCKEY_F3, "PCKEY_F3" },
	{ PCKEY_F4, "PCKEY_F4" },
	{ PCKEY_F5, "PCKEY_F5" },
	{ PCKEY_F6, "PCKEY_F6" },
	{ PCKEY_F7, "PCKEY_F7" },
	{ PCKEY_F8, "PCKEY_F8" },
	{ PCKEY_F9, "PCKEY_F9" },
	{ PCKEY_F10, "PCKEY_F10" },
	{ PCKEY_F11, "PCKEY_F11" },
	{ PCKEY_F12, "PCKEY_F12" },
	{ PCKEY_F13, "PCKEY_F13" },
	{ PCKEY_F14, "PCKEY_F14" },
	{ PCKEY_F15, "PCKEY_F15" },
	{ PCKEY_F16, "PCKEY_F16" },
	{ PCKEY_F17, "PCKEY_F17" },
	{ PCKEY_F18, "PCKEY_F18" },
	{ PCKEY_F19, "PCKEY_F19" },
	{ PCKEY_F20, "PCKEY_F20" },
	{ PCKEY_F21, "PCKEY_F21" },
	{ PCKEY_F22, "PCKEY_F22" },
	{ PCKEY_F23, "PCKEY_F23" },
	{ PCKEY_F24, "PCKEY_F24" },
	{ PCKEY_NUMLOCK, "PCKEY_NUMLOCK" },
	{ PCKEY_OEM_SCROLL, "PCKEY_OEM_SCROLL" },
	{ PCKEY_OEM_1, "PCKEY_OEM_1" },
	{ PCKEY_OEM_PLUS, "PCKEY_OEM_PLUS" },
	{ PCKEY_OEM_COMMA, "PCKEY_OEM_COMMA" },
	{ PCKEY_OEM_MINUS, "PCKEY_OEM_MINUS" },
	{ PCKEY_OEM_PERIOD, "PCKEY_OEM_PERIOD" },
	{ PCKEY_OEM_2, "PCKEY_OEM_2" },
	{ PCKEY_OEM_3, "PCKEY_OEM_3" },
	{ PCKEY_OEM_4, "PCKEY_OEM_4" },
	{ PCKEY_OEM_5, "PCKEY_OEM_5" },
	{ PCKEY_OEM_6, "PCKEY_OEM_6" },
	{ PCKEY_OEM_7, "PCKEY_OEM_7" },
	{ PCKEY_OEM_8, "PCKEY_OEM_8" },
	{ PCKEY_ICO_F17, "PCKEY_ICO_F17" },
	{ PCKEY_ICO_F18, "PCKEY_ICO_F18" },
	{ PCKEY_OEM102, "PCKEY_OEM102" },
	{ PCKEY_ICO_HELP, "PCKEY_ICO_HELP" },
	{ PCKEY_ICO_00, "PCKEY_ICO_00" },
	{ PCKEY_ICO_CLEAR, "PCKEY_ICO_CLEAR" },
	{ PCKEY_OEM_RESET, "PCKEY_OEM_RESET" },
	{ PCKEY_OEM_JUMP, "PCKEY_OEM_JUMP" },
	{ PCKEY_OEM_PA1, "PCKEY_OEM_PA1" },
	{ PCKEY_OEM_PA2, "PCKEY_OEM_PA2" },
	{ PCKEY_OEM_PA3, "PCKEY_OEM_PA3" },
	{ PCKEY_OEM_WSCTRL, "PCKEY_OEM_WSCTRL" },
	{ PCKEY_OEM_CUSEL, "PCKEY_OEM_CUSEL" },
	{ PCKEY_OEM_ATTN, "PCKEY_OEM_ATTN" },
	{ PCKEY_OEM_FINNISH, "PCKEY_OEM_FINNISH" },
	{ PCKEY_OEM_COPY, "PCKEY_OEM_COPY" },
	{ PCKEY_OEM_AUTO, "PCKEY_OEM_AUTO" },
	{ PCKEY_OEM_ENLW, "PCKEY_OEM_ENLW" },
	{ PCKEY_OEM_BACKTAB, "PCKEY_OEM_BACKTAB" },
	{ PCKEY_ATTN, "PCKEY_ATTN" },
	{ PCKEY_CRSEL, "PCKEY_CRSEL" },
	{ PCKEY_EXSEL, "PCKEY_EXSEL" },
	{ PCKEY_EREOF, "PCKEY_EREOF" },
	{ PCKEY_PLAY, "PCKEY_PLAY" },
	{ PCKEY_ZOOM, "PCKEY_ZOOM" },
	{ PCKEY_NONAME, "PCKEY_NONAME" },
	{ PCKEY_PA1, "PCKEY_PA1" },
	{ PCKEY_OEM_CLEAR, "PCKEY_OEM_CLEAR" },
    { -1, "" },
};
