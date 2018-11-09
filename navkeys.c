#include "navputter.h"

#define KEY_MOD_LIST\
    MOD( HID_KEYBOARD_MODIFIER_LEFTCTRL  )\
    MOD( HID_KEYBOARD_MODIFIER_LEFTSHIFT)\
    MOD( HID_KEYBOARD_MODIFIER_LEFTALT )\
    MOD( HID_KEYBOARD_MODIFIER_LEFTGUI )\
    MOD( HID_KEYBOARD_MODIFIER_RIGHTCTRL )\
    MOD( HID_KEYBOARD_MODIFIER_RIGHTSHIFT )\
    MOD( HID_KEYBOARD_MODIFIER_RIGHTALT )\
    MOD( HID_KEYBOARD_MODIFIER_RIGHTGUI )

#define KEY_SCAN_LIST\
		KEY(HID_KEYBOARD_SC_RESERVED)\
		KEY(HID_KEYBOARD_SC_ERROR_ROLLOVER)\
		KEY(HID_KEYBOARD_SC_POST_FAIL)\
		KEY(HID_KEYBOARD_SC_ERROR_UNDEFINED)\
		KEY(HID_KEYBOARD_SC_A)\
		KEY(HID_KEYBOARD_SC_B)\
		KEY(HID_KEYBOARD_SC_C)\
		KEY(HID_KEYBOARD_SC_D)\
		KEY(HID_KEYBOARD_SC_E)\
		KEY(HID_KEYBOARD_SC_F)\
		KEY(HID_KEYBOARD_SC_G)\
		KEY(HID_KEYBOARD_SC_H)\
		KEY(HID_KEYBOARD_SC_I)\
		KEY(HID_KEYBOARD_SC_J)\
		KEY(HID_KEYBOARD_SC_K)\
		KEY(HID_KEYBOARD_SC_L)\
		KEY(HID_KEYBOARD_SC_M)\
		KEY(HID_KEYBOARD_SC_N)\
		KEY(HID_KEYBOARD_SC_O)\
		KEY(HID_KEYBOARD_SC_P)\
		KEY(HID_KEYBOARD_SC_Q)\
		KEY(HID_KEYBOARD_SC_R)\
		KEY(HID_KEYBOARD_SC_S)\
		KEY(HID_KEYBOARD_SC_T)\
		KEY(HID_KEYBOARD_SC_U)\
		KEY(HID_KEYBOARD_SC_V)\
		KEY(HID_KEYBOARD_SC_W)\
		KEY(HID_KEYBOARD_SC_X)\
		KEY(HID_KEYBOARD_SC_Y)\
		KEY(HID_KEYBOARD_SC_Z)\
		KEY(HID_KEYBOARD_SC_1_AND_EXCLAMATION)\
		KEY(HID_KEYBOARD_SC_2_AND_AT)\
		KEY(HID_KEYBOARD_SC_3_AND_HASHMARK)\
		KEY(HID_KEYBOARD_SC_4_AND_DOLLAR)\
		KEY(HID_KEYBOARD_SC_5_AND_PERCENTAGE)\
		KEY(HID_KEYBOARD_SC_6_AND_CARET)\
		KEY(HID_KEYBOARD_SC_7_AND_AMPERSAND)\
		KEY(HID_KEYBOARD_SC_8_AND_ASTERISK)\
		KEY(HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS)\
		KEY(HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS)\
		KEY(HID_KEYBOARD_SC_ENTER)\
		KEY(HID_KEYBOARD_SC_ESCAPE)\
		KEY(HID_KEYBOARD_SC_BACKSPACE)\
		KEY(HID_KEYBOARD_SC_TAB)\
		KEY(HID_KEYBOARD_SC_SPACE)\
		KEY(HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)\
		KEY(HID_KEYBOARD_SC_EQUAL_AND_PLUS)\
		KEY(HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE)\
		KEY(HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE)\
		KEY(HID_KEYBOARD_SC_BACKSLASH_AND_PIPE)\
		KEY(HID_KEYBOARD_SC_NON_US_HASHMARK_AND_TILDE)\
		KEY(HID_KEYBOARD_SC_SEMICOLON_AND_COLON)\
		KEY(HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE)\
		KEY(HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE)\
		KEY(HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN)\
		KEY(HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN)\
		KEY(HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK)\
		KEY(HID_KEYBOARD_SC_CAPS_LOCK)\
		KEY(HID_KEYBOARD_SC_F1)\
		KEY(HID_KEYBOARD_SC_F2)\
		KEY(HID_KEYBOARD_SC_F3)\
		KEY(HID_KEYBOARD_SC_F4)\
		KEY(HID_KEYBOARD_SC_F5)\
		KEY(HID_KEYBOARD_SC_F6)\
		KEY(HID_KEYBOARD_SC_F7)\
		KEY(HID_KEYBOARD_SC_F8)\
		KEY(HID_KEYBOARD_SC_F9)\
		KEY(HID_KEYBOARD_SC_F10)\
		KEY(HID_KEYBOARD_SC_F11)\
		KEY(HID_KEYBOARD_SC_F12)\
		KEY(HID_KEYBOARD_SC_PRINT_SCREEN)\
		KEY(HID_KEYBOARD_SC_SCROLL_LOCK)\
		KEY(HID_KEYBOARD_SC_PAUSE)\
		KEY(HID_KEYBOARD_SC_INSERT)\
		KEY(HID_KEYBOARD_SC_HOME)\
		KEY(HID_KEYBOARD_SC_PAGE_UP)\
		KEY(HID_KEYBOARD_SC_DELETE)\
		KEY(HID_KEYBOARD_SC_END)\
		KEY(HID_KEYBOARD_SC_PAGE_DOWN)\
		KEY(HID_KEYBOARD_SC_RIGHT_ARROW)\
		KEY(HID_KEYBOARD_SC_LEFT_ARROW)\
		KEY(HID_KEYBOARD_SC_DOWN_ARROW)\
		KEY(HID_KEYBOARD_SC_UP_ARROW)\
		KEY(HID_KEYBOARD_SC_NUM_LOCK)\
		KEY(HID_KEYBOARD_SC_KEYPAD_SLASH)\
		KEY(HID_KEYBOARD_SC_KEYPAD_ASTERISK)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MINUS)\
		KEY(HID_KEYBOARD_SC_KEYPAD_PLUS)\
		KEY(HID_KEYBOARD_SC_KEYPAD_ENTER)\
		KEY(HID_KEYBOARD_SC_KEYPAD_1_AND_END)\
		KEY(HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW)\
		KEY(HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN)\
		KEY(HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW)\
		KEY(HID_KEYBOARD_SC_KEYPAD_5)\
		KEY(HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW)\
		KEY(HID_KEYBOARD_SC_KEYPAD_7_AND_HOME)\
		KEY(HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW)\
		KEY(HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP)\
		KEY(HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT)\
		KEY(HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE)\
		KEY(HID_KEYBOARD_SC_NON_US_BACKSLASH_AND_PIPE)\
		KEY(HID_KEYBOARD_SC_APPLICATION)\
		KEY(HID_KEYBOARD_SC_POWER)\
		KEY(HID_KEYBOARD_SC_KEYPAD_EQUAL_SIGN)\
		KEY(HID_KEYBOARD_SC_F13)\
		KEY(HID_KEYBOARD_SC_F14)\
		KEY(HID_KEYBOARD_SC_F15)\
		KEY(HID_KEYBOARD_SC_F16)\
		KEY(HID_KEYBOARD_SC_F17)\
		KEY(HID_KEYBOARD_SC_F18)\
		KEY(HID_KEYBOARD_SC_F19)\
		KEY(HID_KEYBOARD_SC_F20)\
		KEY(HID_KEYBOARD_SC_F21)\
		KEY(HID_KEYBOARD_SC_F22)\
		KEY(HID_KEYBOARD_SC_F23)\
		KEY(HID_KEYBOARD_SC_F24)\
		KEY(HID_KEYBOARD_SC_EXECUTE)\
		KEY(HID_KEYBOARD_SC_HELP)\
		KEY(HID_KEYBOARD_SC_MENU)\
		KEY(HID_KEYBOARD_SC_SELECT)\
		KEY(HID_KEYBOARD_SC_STOP)\
		KEY(HID_KEYBOARD_SC_AGAIN)\
		KEY(HID_KEYBOARD_SC_UNDO)\
		KEY(HID_KEYBOARD_SC_CUT)\
		KEY(HID_KEYBOARD_SC_COPY)\
		KEY(HID_KEYBOARD_SC_PASTE)\
		KEY(HID_KEYBOARD_SC_FIND)\
		KEY(HID_KEYBOARD_SC_MUTE)\
		KEY(HID_KEYBOARD_SC_VOLUME_UP)\
		KEY(HID_KEYBOARD_SC_VOLUME_DOWN)\
		KEY(HID_KEYBOARD_SC_LOCKING_CAPS_LOCK)\
		KEY(HID_KEYBOARD_SC_LOCKING_NUM_LOCK)\
		KEY(HID_KEYBOARD_SC_LOCKING_SCROLL_LOCK)\
		KEY(HID_KEYBOARD_SC_KEYPAD_COMMA)\
		KEY(HID_KEYBOARD_SC_KEYPAD_EQUAL_SIGN_AS400)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL1)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL2)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL3)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL4)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL5)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL6)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL7)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL8)\
		KEY(HID_KEYBOARD_SC_INTERNATIONAL9)\
		KEY(HID_KEYBOARD_SC_LANG1)\
		KEY(HID_KEYBOARD_SC_LANG2)\
		KEY(HID_KEYBOARD_SC_LANG3)\
		KEY(HID_KEYBOARD_SC_LANG4)\
		KEY(HID_KEYBOARD_SC_LANG5)\
		KEY(HID_KEYBOARD_SC_LANG6)\
		KEY(HID_KEYBOARD_SC_LANG7)\
		KEY(HID_KEYBOARD_SC_LANG8)\
		KEY(HID_KEYBOARD_SC_LANG9)\
		KEY(HID_KEYBOARD_SC_ALTERNATE_ERASE)\
		KEY(HID_KEYBOARD_SC_SYSREQ)\
		KEY(HID_KEYBOARD_SC_CANCEL)\
		KEY(HID_KEYBOARD_SC_CLEAR)\
		KEY(HID_KEYBOARD_SC_PRIOR)\
		KEY(HID_KEYBOARD_SC_RETURN)\
		KEY(HID_KEYBOARD_SC_SEPARATOR)\
		KEY(HID_KEYBOARD_SC_OUT)\
		KEY(HID_KEYBOARD_SC_OPER)\
		KEY(HID_KEYBOARD_SC_CLEAR_AND_AGAIN)\
		KEY(HID_KEYBOARD_SC_CRSEL_AND_PROPS)\
		KEY(HID_KEYBOARD_SC_EXSEL)\
		KEY(HID_KEYBOARD_SC_KEYPAD_00)\
		KEY(HID_KEYBOARD_SC_KEYPAD_000)\
		KEY(HID_KEYBOARD_SC_THOUSANDS_SEPARATOR)\
		KEY(HID_KEYBOARD_SC_DECIMAL_SEPARATOR)\
		KEY(HID_KEYBOARD_SC_CURRENCY_UNIT)\
		KEY(HID_KEYBOARD_SC_CURRENCY_SUB_UNIT)\
		KEY(HID_KEYBOARD_SC_KEYPAD_OPENING_PARENTHESIS)\
		KEY(HID_KEYBOARD_SC_KEYPAD_CLOSING_PARENTHESIS)\
		KEY(HID_KEYBOARD_SC_KEYPAD_OPENING_BRACE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_CLOSING_BRACE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_TAB)\
		KEY(HID_KEYBOARD_SC_KEYPAD_BACKSPACE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_A)\
		KEY(HID_KEYBOARD_SC_KEYPAD_B)\
		KEY(HID_KEYBOARD_SC_KEYPAD_C)\
		KEY(HID_KEYBOARD_SC_KEYPAD_D)\
		KEY(HID_KEYBOARD_SC_KEYPAD_E)\
		KEY(HID_KEYBOARD_SC_KEYPAD_F)\
		KEY(HID_KEYBOARD_SC_KEYPAD_XOR)\
		KEY(HID_KEYBOARD_SC_KEYPAD_CARET)\
		KEY(HID_KEYBOARD_SC_KEYPAD_PERCENTAGE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_LESS_THAN_SIGN)\
		KEY(HID_KEYBOARD_SC_KEYPAD_GREATER_THAN_SIGN)\
		KEY(HID_KEYBOARD_SC_KEYPAD_AMP)\
		KEY(HID_KEYBOARD_SC_KEYPAD_AMP_AMP)\
		KEY(HID_KEYBOARD_SC_KEYPAD_PIPE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_PIPE_PIPE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_COLON)\
		KEY(HID_KEYBOARD_SC_KEYPAD_HASHMARK)\
		KEY(HID_KEYBOARD_SC_KEYPAD_SPACE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_AT)\
		KEY(HID_KEYBOARD_SC_KEYPAD_EXCLAMATION_SIGN)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_STORE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_RECALL)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_CLEAR)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_ADD)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_SUBTRACT)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_MULTIPLY)\
		KEY(HID_KEYBOARD_SC_KEYPAD_MEMORY_DIVIDE)\
		KEY(HID_KEYBOARD_SC_KEYPAD_PLUS_AND_MINUS)\
		KEY(HID_KEYBOARD_SC_KEYPAD_CLEAR)\
		KEY(HID_KEYBOARD_SC_KEYPAD_CLEAR_ENTRY)\
		KEY(HID_KEYBOARD_SC_KEYPAD_BINARY)\
		KEY(HID_KEYBOARD_SC_KEYPAD_OCTAL)\
		KEY(HID_KEYBOARD_SC_KEYPAD_DECIMAL)\
		KEY(HID_KEYBOARD_SC_KEYPAD_HEXADECIMAL)\
		KEY(HID_KEYBOARD_SC_LEFT_CONTROL)\
		KEY(HID_KEYBOARD_SC_LEFT_SHIFT)\
		KEY(HID_KEYBOARD_SC_LEFT_ALT)\
		KEY(HID_KEYBOARD_SC_LEFT_GUI)\
		KEY(HID_KEYBOARD_SC_RIGHT_CONTROL)\
		KEY(HID_KEYBOARD_SC_RIGHT_SHIFT)\
		KEY(HID_KEYBOARD_SC_RIGHT_ALT)\
		KEY(HID_KEYBOARD_SC_RIGHT_GUI)\
		KEY(HID_KEYBOARD_SC_MEDIA_PLAY)\
		KEY(HID_KEYBOARD_SC_MEDIA_STOP)\
		KEY(HID_KEYBOARD_SC_MEDIA_PREVIOUS_TRACK)\
		KEY(HID_KEYBOARD_SC_MEDIA_NEXT_TRACK)\
		KEY(HID_KEYBOARD_SC_MEDIA_EJECT)\
		KEY(HID_KEYBOARD_SC_MEDIA_VOLUME_UP)\
		KEY(HID_KEYBOARD_SC_MEDIA_VOLUME_DOWN)\
		KEY(HID_KEYBOARD_SC_MEDIA_MUTE)\
		KEY(HID_KEYBOARD_SC_MEDIA_WWW)\
		KEY(HID_KEYBOARD_SC_MEDIA_BACKWARD)\
		KEY(HID_KEYBOARD_SC_MEDIA_FORWARD)\
		KEY(HID_KEYBOARD_SC_MEDIA_CANCEL)\
		KEY(HID_KEYBOARD_SC_MEDIA_SEARCH)\
		KEY(HID_KEYBOARD_SC_MEDIA_SLEEP)\
		KEY(HID_KEYBOARD_SC_MEDIA_LOCK)\
		KEY(HID_KEYBOARD_SC_MEDIA_RELOAD)\
		KEY(HID_KEYBOARD_SC_MEDIA_CALCULATOR)

#define PAD_SZ 20

void dump_keycodes(FILE *fp)
{
    fprintf(fp,"Key modifier codes. These can be ORed together\n\r");
#define MOD(e) fprintf(fp,"%s%*d\n\r", #e, PAD_SZ - strlen( #e ), e );
KEY_MOD_LIST
#undef MOD

    fprintf(fp,"Key scan codes.\n\r");
#define KEY(e) fprintf(fp, "%s%*d\n\r", #e, PAD_SZ - strlen( #e ), e );
KEY_SCAN_LIST
#undef KEY
}
