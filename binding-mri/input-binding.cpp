/*
** input-binding.cpp
**
** This file is part of mkxp.
**
** Copyright (C) 2013 Jonas Kulla <Nyocurio@gmail.com>
**
** mkxp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** mkxp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "input.h"
#include "sharedstate.h"
#include "eventthread.h"
#include "exception.h"
#include "binding-util.h"
#include "util.h"
#include "SDL.h"

RB_METHOD(inputUpdate)
{
	RB_UNUSED_PARAM;

	shState->input().update();

	return Qnil;
}

static int getButtonArg(int argc, VALUE *argv)
{
	int num;

	rb_check_argc(argc, 1);

	if (FIXNUM_P(argv[0]))
	{
		num = FIX2INT(argv[0]);
	}
	else if (SYMBOL_P(argv[0]) && rgssVer >= 3)
	{
		VALUE symHash = getRbData()->buttoncodeHash;
		num = FIX2INT(rb_hash_lookup2(symHash, argv[0], INT2FIX(Input::None)));
	}
	else
	{
		// FIXME: RMXP allows only few more types that
		// don't make sense (symbols in pre 3, floats)
		num = 0;
	}

	return num;
}

RB_METHOD(inputPress)
{
	RB_UNUSED_PARAM;

	int num = getButtonArg(argc, argv);

	return rb_bool_new(shState->input().isPressed(num));
}

RB_METHOD(inputTrigger)
{
	RB_UNUSED_PARAM;

	int num = getButtonArg(argc, argv);

	return rb_bool_new(shState->input().isTriggered(num));
}

RB_METHOD(inputRepeat)
{
	RB_UNUSED_PARAM;

	int num = getButtonArg(argc, argv);

	return rb_bool_new(shState->input().isRepeated(num));
}

RB_METHOD(inputDir4)
{
	RB_UNUSED_PARAM;

	return rb_fix_new(shState->input().dir4Value());
}

RB_METHOD(inputDir8)
{
	RB_UNUSED_PARAM;

	return rb_fix_new(shState->input().dir8Value());
}

/* Non-standard extensions */
RB_METHOD(inputMouseX)
{
	RB_UNUSED_PARAM;

	return rb_fix_new(shState->input().mouseX());
}

RB_METHOD(inputMouseY)
{
	RB_UNUSED_PARAM;

	return rb_fix_new(shState->input().mouseY());
}

RB_METHOD(inputQuit)
{
	RB_UNUSED_PARAM;

	return rb_bool_new(shState->input().hasQuit());
}

RB_METHOD(inputModkeys) {
	RB_UNUSED_PARAM;
	return INT2FIX(shState->input().modkeys);
}

RB_METHOD(inputStartTextInput) {
	int limit = 4096;
	rb_get_args(argc, argv, "|i", &limit RB_ARG_END);
	SDL_LockMutex(EventThread::inputMut);
	shState->rtData().acceptingTextInput = true;
	shState->rtData().inputTextLimit = limit;
	shState->rtData().inputText.clear();
	SDL_UnlockMutex(EventThread::inputMut);
	return Qnil;
}

RB_METHOD(inputStopTextInput) {
	RB_UNUSED_PARAM;
	SDL_LockMutex(EventThread::inputMut);
	shState->rtData().acceptingTextInput = false;
	SDL_UnlockMutex(EventThread::inputMut);
	return Qnil;
}

RB_METHOD(inputTextInput) {
	RB_UNUSED_PARAM;
	SDL_LockMutex(EventThread::inputMut);
	VALUE retval = rb_utf8_str_new_cstr(shState->rtData().inputText.c_str());
	SDL_UnlockMutex(EventThread::inputMut);
	return retval;
}

RB_METHOD(inputSetTextInput) {
	RB_UNUSED_PARAM;
	const char *newinput;
	rb_get_args(argc, argv, "z", &newinput RB_ARG_END);
	SDL_LockMutex(EventThread::inputMut);
	shState->rtData().inputText = newinput;
	SDL_UnlockMutex(EventThread::inputMut);
	return Qnil;
}

RB_METHOD(inputSetKey)
{
	RB_UNUSED_PARAM;
	int num = getButtonArg(argc, argv);
	shState->input().setKey(num);
	return Qnil;
}

RB_METHOD(inputUnsetKey)
{
	RB_UNUSED_PARAM;
	int num = getButtonArg(argc, argv);
	shState->input().unsetKey(num);
	return Qnil;
}
RB_METHOD(inputSetKeyPressed)
{
	RB_UNUSED_PARAM;
	int num = getButtonArg(argc, argv);
	shState->input().setPressed(num);
	return Qnil;
}
RB_METHOD(inputSetKeyTriggered)
{
	RB_UNUSED_PARAM;
	int num = getButtonArg(argc, argv);
	shState->input().setTriggered(num);
	return Qnil;
}
RB_METHOD(inputSetKeyRepeated)
{
	RB_UNUSED_PARAM;
	int num = getButtonArg(argc, argv);
	shState->input().setRepeated(num);
	return Qnil;
}

struct
{
	const char *str;
	Input::ButtonCode val;
}

#define INPUT_DEF_BUTTONCODE(name) \
	{ #name, Input::name },
static buttonCodes[] =
{
	{ "DOWN",       Input::Down       },
	{ "LEFT",       Input::Left       },
	{ "RIGHT",      Input::Right      },
	{ "UP",         Input::Up         },

	{ "ACTION",     Input::Action     },
	{ "CANCEL",     Input::Cancel     },
	{ "MENU",       Input::Menu       },
	{ "ITEMS",      Input::Items      },
	{ "RUN",        Input::Run        },
	{ "DEACTIVATE", Input::Deactivate },

	{ "L",          Input::L          },
	{ "R",          Input::R          },

	{ "SETTINGS", 	Input::Settings   },
	{ "PAUSE", 		Input::Pause	  },
	
	{ "F5",         Input::F5         },
	{ "F6",         Input::F6         },
	{ "F7",         Input::F7         },
	{ "F8",         Input::F8         },
	{ "F9",         Input::F9         },

	{ "MOUSELEFT",   Input::MouseLeft   },
	{ "MOUSEMIDDLE", Input::MouseMiddle },
	{ "MOUSERIGHT",  Input::MouseRight  },

		// CLOVERLINK KEYBINDS
	
	{ "CHAT",		 Input::Chat		},
	{ "TOGGLECHAT",	 Input::ToggleChat	},
	{ "MAP",		 Input::Map			},
	{ "PLAYING",	 Input::Playing		},
	{ "SENDMSG",	 Input::MSG			},
	{ "E1",		 	 Input::E1			},
	{ "E2",		 	 Input::E2			},
	{ "E3",		 	 Input::E3			},
	{ "E4",		 	 Input::E4			},
	{ "FAVORITE",	 Input::Favorite	},

	INPUT_DEF_BUTTONCODE(KEY_UNKNOWN)
	INPUT_DEF_BUTTONCODE(KEY_BACKSPACE)
	INPUT_DEF_BUTTONCODE(KEY_TAB)
	INPUT_DEF_BUTTONCODE(KEY_RETURN)
	INPUT_DEF_BUTTONCODE(KEY_ESCAPE)
	INPUT_DEF_BUTTONCODE(KEY_SPACE)
	INPUT_DEF_BUTTONCODE(KEY_EXCLAIM)
	INPUT_DEF_BUTTONCODE(KEY_QUOTEDBL)
	INPUT_DEF_BUTTONCODE(KEY_HASH)
	INPUT_DEF_BUTTONCODE(KEY_DOLLAR)
	INPUT_DEF_BUTTONCODE(KEY_PERCENT)
	INPUT_DEF_BUTTONCODE(KEY_AMPERSAND)
	INPUT_DEF_BUTTONCODE(KEY_QUOTE)
	INPUT_DEF_BUTTONCODE(KEY_LEFTPAREN)
	INPUT_DEF_BUTTONCODE(KEY_RIGHTPAREN)
	INPUT_DEF_BUTTONCODE(KEY_ASTERISK)
	INPUT_DEF_BUTTONCODE(KEY_PLUS)
	INPUT_DEF_BUTTONCODE(KEY_COMMA)
	INPUT_DEF_BUTTONCODE(KEY_MINUS)
	INPUT_DEF_BUTTONCODE(KEY_PERIOD)
	INPUT_DEF_BUTTONCODE(KEY_SLASH)
	INPUT_DEF_BUTTONCODE(KEY_0)
	INPUT_DEF_BUTTONCODE(KEY_1)
	INPUT_DEF_BUTTONCODE(KEY_2)
	INPUT_DEF_BUTTONCODE(KEY_3)
	INPUT_DEF_BUTTONCODE(KEY_4)
	INPUT_DEF_BUTTONCODE(KEY_5)
	INPUT_DEF_BUTTONCODE(KEY_6)
	INPUT_DEF_BUTTONCODE(KEY_7)
	INPUT_DEF_BUTTONCODE(KEY_8)
	INPUT_DEF_BUTTONCODE(KEY_9)
	INPUT_DEF_BUTTONCODE(KEY_COLON)
	INPUT_DEF_BUTTONCODE(KEY_SEMICOLON)
	INPUT_DEF_BUTTONCODE(KEY_LESS)
	INPUT_DEF_BUTTONCODE(KEY_EQUALS)
	INPUT_DEF_BUTTONCODE(KEY_GREATER)
	INPUT_DEF_BUTTONCODE(KEY_QUESTION)
	INPUT_DEF_BUTTONCODE(KEY_AT)
	INPUT_DEF_BUTTONCODE(KEY_LEFTBRACKET)
	INPUT_DEF_BUTTONCODE(KEY_BACKSLASH)
	INPUT_DEF_BUTTONCODE(KEY_RIGHTBRACKET)
	INPUT_DEF_BUTTONCODE(KEY_CARET)
	INPUT_DEF_BUTTONCODE(KEY_UNDERSCORE)
	INPUT_DEF_BUTTONCODE(KEY_BACKQUOTE)
	INPUT_DEF_BUTTONCODE(KEY_A)
	INPUT_DEF_BUTTONCODE(KEY_B)
	INPUT_DEF_BUTTONCODE(KEY_C)
	INPUT_DEF_BUTTONCODE(KEY_D)
	INPUT_DEF_BUTTONCODE(KEY_E)
	INPUT_DEF_BUTTONCODE(KEY_F)
	INPUT_DEF_BUTTONCODE(KEY_G)
	INPUT_DEF_BUTTONCODE(KEY_H)
	INPUT_DEF_BUTTONCODE(KEY_I)
	INPUT_DEF_BUTTONCODE(KEY_J)
	INPUT_DEF_BUTTONCODE(KEY_K)
	INPUT_DEF_BUTTONCODE(KEY_L)
	INPUT_DEF_BUTTONCODE(KEY_M)
	INPUT_DEF_BUTTONCODE(KEY_N)
	INPUT_DEF_BUTTONCODE(KEY_O)
	INPUT_DEF_BUTTONCODE(KEY_P)
	INPUT_DEF_BUTTONCODE(KEY_Q)
	INPUT_DEF_BUTTONCODE(KEY_R)
	INPUT_DEF_BUTTONCODE(KEY_S)
	INPUT_DEF_BUTTONCODE(KEY_T)
	INPUT_DEF_BUTTONCODE(KEY_U)
	INPUT_DEF_BUTTONCODE(KEY_V)
	INPUT_DEF_BUTTONCODE(KEY_W)
	INPUT_DEF_BUTTONCODE(KEY_X)
	INPUT_DEF_BUTTONCODE(KEY_Y)
	INPUT_DEF_BUTTONCODE(KEY_Z)
	INPUT_DEF_BUTTONCODE(KEY_DELETE)
	INPUT_DEF_BUTTONCODE(KEY_CAPSLOCK)
	INPUT_DEF_BUTTONCODE(KEY_F1)
	INPUT_DEF_BUTTONCODE(KEY_F2)
	INPUT_DEF_BUTTONCODE(KEY_F3)
	INPUT_DEF_BUTTONCODE(KEY_F4)
	INPUT_DEF_BUTTONCODE(KEY_F5)
	INPUT_DEF_BUTTONCODE(KEY_F6)
	INPUT_DEF_BUTTONCODE(KEY_F7)
	INPUT_DEF_BUTTONCODE(KEY_F8)
	INPUT_DEF_BUTTONCODE(KEY_F9)
	INPUT_DEF_BUTTONCODE(KEY_F10)
	INPUT_DEF_BUTTONCODE(KEY_F11)
	INPUT_DEF_BUTTONCODE(KEY_F12)
	INPUT_DEF_BUTTONCODE(KEY_PRINTSCREEN)
	INPUT_DEF_BUTTONCODE(KEY_SCROLLLOCK)
	INPUT_DEF_BUTTONCODE(KEY_PAUSE)
	INPUT_DEF_BUTTONCODE(KEY_INSERT)
	INPUT_DEF_BUTTONCODE(KEY_HOME)
	INPUT_DEF_BUTTONCODE(KEY_PAGEUP)
	INPUT_DEF_BUTTONCODE(KEY_END)
	INPUT_DEF_BUTTONCODE(KEY_PAGEDOWN)
	INPUT_DEF_BUTTONCODE(KEY_RIGHT)
	INPUT_DEF_BUTTONCODE(KEY_LEFT)
	INPUT_DEF_BUTTONCODE(KEY_DOWN)
	INPUT_DEF_BUTTONCODE(KEY_UP)
	INPUT_DEF_BUTTONCODE(KEY_NUMLOCKCLEAR)
	INPUT_DEF_BUTTONCODE(KEY_KP_DIVIDE)
	INPUT_DEF_BUTTONCODE(KEY_KP_MULTIPLY)
	INPUT_DEF_BUTTONCODE(KEY_KP_MINUS)
	INPUT_DEF_BUTTONCODE(KEY_KP_PLUS)
	INPUT_DEF_BUTTONCODE(KEY_KP_ENTER)
	INPUT_DEF_BUTTONCODE(KEY_KP_1)
	INPUT_DEF_BUTTONCODE(KEY_KP_2)
	INPUT_DEF_BUTTONCODE(KEY_KP_3)
	INPUT_DEF_BUTTONCODE(KEY_KP_4)
	INPUT_DEF_BUTTONCODE(KEY_KP_5)
	INPUT_DEF_BUTTONCODE(KEY_KP_6)
	INPUT_DEF_BUTTONCODE(KEY_KP_7)
	INPUT_DEF_BUTTONCODE(KEY_KP_8)
	INPUT_DEF_BUTTONCODE(KEY_KP_9)
	INPUT_DEF_BUTTONCODE(KEY_KP_0)
	INPUT_DEF_BUTTONCODE(KEY_KP_PERIOD)
	INPUT_DEF_BUTTONCODE(KEY_APPLICATION)
	INPUT_DEF_BUTTONCODE(KEY_POWER)
	INPUT_DEF_BUTTONCODE(KEY_KP_EQUALS)
	INPUT_DEF_BUTTONCODE(KEY_F13)
	INPUT_DEF_BUTTONCODE(KEY_F14)
	INPUT_DEF_BUTTONCODE(KEY_F15)
	INPUT_DEF_BUTTONCODE(KEY_F16)
	INPUT_DEF_BUTTONCODE(KEY_F17)
	INPUT_DEF_BUTTONCODE(KEY_F18)
	INPUT_DEF_BUTTONCODE(KEY_F19)
	INPUT_DEF_BUTTONCODE(KEY_F20)
	INPUT_DEF_BUTTONCODE(KEY_F21)
	INPUT_DEF_BUTTONCODE(KEY_F22)
	INPUT_DEF_BUTTONCODE(KEY_F23)
	INPUT_DEF_BUTTONCODE(KEY_F24)
	// KEY_EXECUTE is used in windows, so it is not exposed. not that anyone really needs it.
	// INPUT_DEF_BUTTONCODE(KEY_EXECUTE)
	INPUT_DEF_BUTTONCODE(KEY_HELP)
	INPUT_DEF_BUTTONCODE(KEY_MENU)
	INPUT_DEF_BUTTONCODE(KEY_SELECT)
	INPUT_DEF_BUTTONCODE(KEY_STOP)
	INPUT_DEF_BUTTONCODE(KEY_AGAIN)
	INPUT_DEF_BUTTONCODE(KEY_UNDO)
	INPUT_DEF_BUTTONCODE(KEY_CUT)
	INPUT_DEF_BUTTONCODE(KEY_COPY)
	INPUT_DEF_BUTTONCODE(KEY_PASTE)
	INPUT_DEF_BUTTONCODE(KEY_FIND)
	INPUT_DEF_BUTTONCODE(KEY_MUTE)
	INPUT_DEF_BUTTONCODE(KEY_VOLUMEUP)
	INPUT_DEF_BUTTONCODE(KEY_VOLUMEDOWN)
	INPUT_DEF_BUTTONCODE(KEY_KP_COMMA)
	INPUT_DEF_BUTTONCODE(KEY_KP_EQUALSAS400)
	INPUT_DEF_BUTTONCODE(KEY_ALTERASE)
	INPUT_DEF_BUTTONCODE(KEY_SYSREQ)
	INPUT_DEF_BUTTONCODE(KEY_CANCEL)
	INPUT_DEF_BUTTONCODE(KEY_CLEAR)
	INPUT_DEF_BUTTONCODE(KEY_PRIOR)
	INPUT_DEF_BUTTONCODE(KEY_RETURN2)
	INPUT_DEF_BUTTONCODE(KEY_SEPARATOR)
	INPUT_DEF_BUTTONCODE(KEY_OUT)
	INPUT_DEF_BUTTONCODE(KEY_OPER)
	INPUT_DEF_BUTTONCODE(KEY_CLEARAGAIN)
	INPUT_DEF_BUTTONCODE(KEY_CRSEL)
	INPUT_DEF_BUTTONCODE(KEY_EXSEL)
	INPUT_DEF_BUTTONCODE(KEY_KP_00)
	INPUT_DEF_BUTTONCODE(KEY_KP_000)
	INPUT_DEF_BUTTONCODE(KEY_THOUSANDSSEPARATOR)
	INPUT_DEF_BUTTONCODE(KEY_DECIMALSEPARATOR)
	INPUT_DEF_BUTTONCODE(KEY_CURRENCYUNIT)
	INPUT_DEF_BUTTONCODE(KEY_CURRENCYSUBUNIT)
	INPUT_DEF_BUTTONCODE(KEY_KP_LEFTPAREN)
	INPUT_DEF_BUTTONCODE(KEY_KP_RIGHTPAREN)
	INPUT_DEF_BUTTONCODE(KEY_KP_LEFTBRACE)
	INPUT_DEF_BUTTONCODE(KEY_KP_RIGHTBRACE)
	INPUT_DEF_BUTTONCODE(KEY_KP_TAB)
	INPUT_DEF_BUTTONCODE(KEY_KP_BACKSPACE)
	INPUT_DEF_BUTTONCODE(KEY_KP_A)
	INPUT_DEF_BUTTONCODE(KEY_KP_B)
	INPUT_DEF_BUTTONCODE(KEY_KP_C)
	INPUT_DEF_BUTTONCODE(KEY_KP_D)
	INPUT_DEF_BUTTONCODE(KEY_KP_E)
	INPUT_DEF_BUTTONCODE(KEY_KP_F)
	INPUT_DEF_BUTTONCODE(KEY_KP_XOR)
	INPUT_DEF_BUTTONCODE(KEY_KP_POWER)
	INPUT_DEF_BUTTONCODE(KEY_KP_PERCENT)
	INPUT_DEF_BUTTONCODE(KEY_KP_LESS)
	INPUT_DEF_BUTTONCODE(KEY_KP_GREATER)
	INPUT_DEF_BUTTONCODE(KEY_KP_AMPERSAND)
	INPUT_DEF_BUTTONCODE(KEY_KP_DBLAMPERSAND)
	INPUT_DEF_BUTTONCODE(KEY_KP_VERTICALBAR)
	INPUT_DEF_BUTTONCODE(KEY_KP_DBLVERTICALBAR)
	INPUT_DEF_BUTTONCODE(KEY_KP_COLON)
	INPUT_DEF_BUTTONCODE(KEY_KP_HASH)
	INPUT_DEF_BUTTONCODE(KEY_KP_SPACE)
	INPUT_DEF_BUTTONCODE(KEY_KP_AT)
	INPUT_DEF_BUTTONCODE(KEY_KP_EXCLAM)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMSTORE)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMRECALL)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMCLEAR)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMADD)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMSUBTRACT)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMMULTIPLY)
	INPUT_DEF_BUTTONCODE(KEY_KP_MEMDIVIDE)
	INPUT_DEF_BUTTONCODE(KEY_KP_PLUSMINUS)
	INPUT_DEF_BUTTONCODE(KEY_KP_CLEAR)
	INPUT_DEF_BUTTONCODE(KEY_KP_CLEARENTRY)
	INPUT_DEF_BUTTONCODE(KEY_KP_BINARY)
	INPUT_DEF_BUTTONCODE(KEY_KP_OCTAL)
	INPUT_DEF_BUTTONCODE(KEY_KP_DECIMAL)
	INPUT_DEF_BUTTONCODE(KEY_KP_HEXADECIMAL)
	INPUT_DEF_BUTTONCODE(KEY_LCTRL)
	INPUT_DEF_BUTTONCODE(KEY_LSHIFT)
	INPUT_DEF_BUTTONCODE(KEY_LALT)
	INPUT_DEF_BUTTONCODE(KEY_LGUI)
	INPUT_DEF_BUTTONCODE(KEY_RCTRL)
	INPUT_DEF_BUTTONCODE(KEY_RSHIFT)
	INPUT_DEF_BUTTONCODE(KEY_RALT)
	INPUT_DEF_BUTTONCODE(KEY_RGUI)
	INPUT_DEF_BUTTONCODE(KEY_MODE)
	INPUT_DEF_BUTTONCODE(KEY_AUDIONEXT)
	INPUT_DEF_BUTTONCODE(KEY_AUDIOPREV)
	INPUT_DEF_BUTTONCODE(KEY_AUDIOSTOP)
	INPUT_DEF_BUTTONCODE(KEY_AUDIOPLAY)
	INPUT_DEF_BUTTONCODE(KEY_AUDIOMUTE)
	INPUT_DEF_BUTTONCODE(KEY_MEDIASELECT)
	INPUT_DEF_BUTTONCODE(KEY_WWW)
	INPUT_DEF_BUTTONCODE(KEY_MAIL)
	INPUT_DEF_BUTTONCODE(KEY_CALCULATOR)
	INPUT_DEF_BUTTONCODE(KEY_COMPUTER)
	INPUT_DEF_BUTTONCODE(KEY_AC_SEARCH)
	INPUT_DEF_BUTTONCODE(KEY_AC_HOME)
	INPUT_DEF_BUTTONCODE(KEY_AC_BACK)
	INPUT_DEF_BUTTONCODE(KEY_AC_FORWARD)
	INPUT_DEF_BUTTONCODE(KEY_AC_STOP)
	INPUT_DEF_BUTTONCODE(KEY_AC_REFRESH)
	INPUT_DEF_BUTTONCODE(KEY_AC_BOOKMARKS)
	INPUT_DEF_BUTTONCODE(KEY_BRIGHTNESSDOWN)
	INPUT_DEF_BUTTONCODE(KEY_BRIGHTNESSUP)
	INPUT_DEF_BUTTONCODE(KEY_DISPLAYSWITCH)
	INPUT_DEF_BUTTONCODE(KEY_KBDILLUMTOGGLE)
	INPUT_DEF_BUTTONCODE(KEY_KBDILLUMDOWN)
	INPUT_DEF_BUTTONCODE(KEY_KBDILLUMUP)
	INPUT_DEF_BUTTONCODE(KEY_EJECT)
	INPUT_DEF_BUTTONCODE(KEY_SLEEP)

};

static elementsN(buttonCodes);

#define INPUT_EXPOSE_KMOD(name) \
	rb_const_set(module, rb_intern(#name), INT2FIX(name));

RB_METHOD(inputGetAllPressed)
{
	RB_UNUSED_PARAM;
	VALUE res = rb_ary_new();
    for (size_t i = 0; i < buttonCodesN; i++) {
		if (shState->input().isPressed(buttonCodes[i].val)) {
            rb_ary_push(res, INT2FIX(buttonCodes[i].val));
		}
	}
	return res;
}
RB_METHOD(inputGetAllTriggered)
{
	RB_UNUSED_PARAM;
	VALUE res = rb_ary_new();
    for (size_t i = 0; i < buttonCodesN; i++) {
		if (shState->input().isTriggered(buttonCodes[i].val)) {
            rb_ary_push(res, INT2FIX(buttonCodes[i].val));
		}
	}
	return res;
}
RB_METHOD(inputGetAllRepeated)
{
	RB_UNUSED_PARAM;
	VALUE res = rb_ary_new();
    for (size_t i = 0; i < buttonCodesN; i++) {
		if (shState->input().isRepeated(buttonCodes[i].val)) {
            rb_ary_push(res, INT2FIX(buttonCodes[i].val));
		}
	}
	return res;
}
RB_METHOD(inputSetAllPressedUnPressed)
{
	RB_UNUSED_PARAM;
	VALUE res = rb_ary_new();
    for (size_t i = 0; i < buttonCodesN; i++) {
		if (shState->input().isPressed(buttonCodes[i].val)) {
            shState->input().unsetKey(buttonCodes[i].val);
		}
	}
	return Qnil;
}
RB_METHOD(inputSetAllUnPressed)
{
	RB_UNUSED_PARAM;
	VALUE res = rb_ary_new();
    for (size_t i = 0; i < buttonCodesN; i++) {
        shState->input().unsetKey(buttonCodes[i].val);		
	}
	return Qnil;
}
void
inputBindingInit()
{
	VALUE module = rb_define_module("Input");

	_rb_define_module_function(module, "update", inputUpdate);
	_rb_define_module_function(module, "press?", inputPress);
	_rb_define_module_function(module, "trigger?", inputTrigger);
	_rb_define_module_function(module, "repeat?", inputRepeat);
	_rb_define_module_function(module, "dir4", inputDir4);
	_rb_define_module_function(module, "dir8", inputDir8);
	_rb_define_module_function(module, "modkeys", inputModkeys);

	_rb_define_module_function(module, "mouse_x", inputMouseX);
	_rb_define_module_function(module, "mouse_y", inputMouseY);

	_rb_define_module_function(module, "quit?", inputQuit);

	_rb_define_module_function(module, "start_text_input", inputStartTextInput);
	_rb_define_module_function(module, "stop_text_input", inputStopTextInput);
	_rb_define_module_function(module, "text_input", inputTextInput);
	_rb_define_module_function(module, "set_text_input", inputSetTextInput);

	_rb_define_module_function(module, "get_all_pressed", inputGetAllPressed);
	_rb_define_module_function(module, "get_all_triggered", inputGetAllTriggered);
	_rb_define_module_function(module, "get_all_repeated", inputGetAllRepeated);
	_rb_define_module_function(module, "set_all_pressed_unpressed", inputSetAllPressedUnPressed);
	_rb_define_module_function(module, "set_all_unpressed", inputSetAllUnPressed);
	_rb_define_module_function(module, "set_key", inputSetKey);
	_rb_define_module_function(module, "unset_key", inputUnsetKey);

	_rb_define_module_function(module, "set_key_pressed", inputSetKeyPressed);
	_rb_define_module_function(module, "set_key_repeated", inputSetKeyRepeated);
	_rb_define_module_function(module, "set_key_triggered", inputSetKeyTriggered);

	if (rgssVer >= 3)
	{
		VALUE symHash = rb_hash_new();

		for (size_t i = 0; i < buttonCodesN; ++i)
		{
			ID sym = rb_intern(buttonCodes[i].str);
			VALUE val = INT2FIX(buttonCodes[i].val);

			/* In RGSS3 all Input::XYZ constants are equal to :XYZ symbols,
			 * to be compatible with the previous convention */
			rb_const_set(module, sym, ID2SYM(sym));
			rb_hash_aset(symHash, ID2SYM(sym), val);
		}

		rb_iv_set(module, "buttoncodes", symHash);
		getRbData()->buttoncodeHash = symHash;
	}
	else
	{
		for (size_t i = 0; i < buttonCodesN; ++i)
		{
			ID sym = rb_intern(buttonCodes[i].str);
			VALUE val = INT2FIX(buttonCodes[i].val);

			rb_const_set(module, sym, val);
		}
	}

	// expose kmods
	INPUT_EXPOSE_KMOD(KMOD_NONE)
	INPUT_EXPOSE_KMOD(KMOD_LSHIFT)
	INPUT_EXPOSE_KMOD(KMOD_RSHIFT)
	INPUT_EXPOSE_KMOD(KMOD_LCTRL)
	INPUT_EXPOSE_KMOD(KMOD_RCTRL)
	INPUT_EXPOSE_KMOD(KMOD_LALT)
	INPUT_EXPOSE_KMOD(KMOD_RALT)
	INPUT_EXPOSE_KMOD(KMOD_LGUI)
	INPUT_EXPOSE_KMOD(KMOD_RGUI)
	INPUT_EXPOSE_KMOD(KMOD_NUM)
	INPUT_EXPOSE_KMOD(KMOD_CAPS)
	INPUT_EXPOSE_KMOD(KMOD_MODE)
	INPUT_EXPOSE_KMOD(KMOD_CTRL)
	INPUT_EXPOSE_KMOD(KMOD_SHIFT)
	INPUT_EXPOSE_KMOD(KMOD_ALT)
	INPUT_EXPOSE_KMOD(KMOD_GUI)
}
