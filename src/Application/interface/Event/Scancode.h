#pragma once
enum EVGGScancode
{
  VGG_SCANCODE_UNKNOWN = 0,

  /**
   *  \name Usage page 0x07
   *
   *  These values are from usage page 0x07 (USB keyboard page).
   */
  /* @{ */

  VGG_SCANCODE_A = 4,
  VGG_SCANCODE_B = 5,
  VGG_SCANCODE_C = 6,
  VGG_SCANCODE_D = 7,
  VGG_SCANCODE_E = 8,
  VGG_SCANCODE_F = 9,
  VGG_SCANCODE_G = 10,
  VGG_SCANCODE_H = 11,
  VGG_SCANCODE_I = 12,
  VGG_SCANCODE_J = 13,
  VGG_SCANCODE_K = 14,
  VGG_SCANCODE_L = 15,
  VGG_SCANCODE_M = 16,
  VGG_SCANCODE_N = 17,
  VGG_SCANCODE_O = 18,
  VGG_SCANCODE_P = 19,
  VGG_SCANCODE_Q = 20,
  VGG_SCANCODE_R = 21,
  VGG_SCANCODE_S = 22,
  VGG_SCANCODE_T = 23,
  VGG_SCANCODE_U = 24,
  VGG_SCANCODE_V = 25,
  VGG_SCANCODE_W = 26,
  VGG_SCANCODE_X = 27,
  VGG_SCANCODE_Y = 28,
  VGG_SCANCODE_Z = 29,

  VGG_SCANCODE_1 = 30,
  VGG_SCANCODE_2 = 31,
  VGG_SCANCODE_3 = 32,
  VGG_SCANCODE_4 = 33,
  VGG_SCANCODE_5 = 34,
  VGG_SCANCODE_6 = 35,
  VGG_SCANCODE_7 = 36,
  VGG_SCANCODE_8 = 37,
  VGG_SCANCODE_9 = 38,
  VGG_SCANCODE_0 = 39,

  VGG_SCANCODE_RETURN = 40,
  VGG_SCANCODE_ESCAPE = 41,
  VGG_SCANCODE_BACKSPACE = 42,
  VGG_SCANCODE_TAB = 43,
  VGG_SCANCODE_SPACE = 44,

  VGG_SCANCODE_MINUS = 45,
  VGG_SCANCODE_EQUALS = 46,
  VGG_SCANCODE_LEFTBRACKET = 47,
  VGG_SCANCODE_RIGHTBRACKET = 48,
  VGG_SCANCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                *   key on ISO keyboards and at the right end
                                *   of the QWERTY row on ANSI keyboards.
                                *   Produces REVERSE SOLIDUS (backslash) and
                                *   VERTICAL LINE in a US layout, REVERSE
                                *   SOLIDUS and VERTICAL LINE in a UK Mac
                                *   layout, NUMBER SIGN and TILDE in a UK
                                *   Windows layout, DOLLAR SIGN and POUND SIGN
                                *   in a Swiss German layout, NUMBER SIGN and
                                *   APOSTROPHE in a German layout, GRAVE
                                *   ACCENT and POUND SIGN in a French Mac
                                *   layout, and ASTERISK and MICRO SIGN in a
                                *   French Windows layout.
                                */
  VGG_SCANCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                *   instead of 49 for the same key, but all
                                *   OSes I've seen treat the two codes
                                *   identically. So, as an implementor, unless
                                *   your keyboard generates both of those
                                *   codes and your OS treats them differently,
                                *   you should generate VGG_SCANCODE_BACKSLASH
                                *   instead of this code. As a user, you
                                *   should not rely on this code because SDL
                                *   will never generate it with most (all?)
                                *   keyboards.
                                */
  VGG_SCANCODE_SEMICOLON = 51,
  VGG_SCANCODE_APOSTROPHE = 52,
  VGG_SCANCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                            *   and ISO keyboards). Produces GRAVE ACCENT and
                            *   TILDE in a US Windows layout and in US and UK
                            *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                            *   and NOT SIGN in a UK Windows layout, SECTION
                            *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                            *   layouts on ISO keyboards, SECTION SIGN and
                            *   DEGREE SIGN in a Swiss German layout (Mac:
                            *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                            *   DEGREE SIGN in a German layout (Mac: only on
                            *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                            *   French Windows layout, COMMERCIAL AT and
                            *   NUMBER SIGN in a French Mac layout on ISO
                            *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                            *   SIGN in a Swiss German, German, or French Mac
                            *   layout on ANSI keyboards.
                            */
  VGG_SCANCODE_COMMA = 54,
  VGG_SCANCODE_PERIOD = 55,
  VGG_SCANCODE_SLASH = 56,

  VGG_SCANCODE_CAPSLOCK = 57,

  VGG_SCANCODE_F1 = 58,
  VGG_SCANCODE_F2 = 59,
  VGG_SCANCODE_F3 = 60,
  VGG_SCANCODE_F4 = 61,
  VGG_SCANCODE_F5 = 62,
  VGG_SCANCODE_F6 = 63,
  VGG_SCANCODE_F7 = 64,
  VGG_SCANCODE_F8 = 65,
  VGG_SCANCODE_F9 = 66,
  VGG_SCANCODE_F10 = 67,
  VGG_SCANCODE_F11 = 68,
  VGG_SCANCODE_F12 = 69,

  VGG_SCANCODE_PRINTSCREEN = 70,
  VGG_SCANCODE_SCROLLLOCK = 71,
  VGG_SCANCODE_PAUSE = 72,
  VGG_SCANCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                 does send code 73, not 117) */
  VGG_SCANCODE_HOME = 74,
  VGG_SCANCODE_PAGEUP = 75,
  VGG_SCANCODE_DELETE = 76,
  VGG_SCANCODE_END = 77,
  VGG_SCANCODE_PAGEDOWN = 78,
  VGG_SCANCODE_RIGHT = 79,
  VGG_SCANCODE_LEFT = 80,
  VGG_SCANCODE_DOWN = 81,
  VGG_SCANCODE_UP = 82,

  VGG_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                   */
  VGG_SCANCODE_KP_DIVIDE = 84,
  VGG_SCANCODE_KP_MULTIPLY = 85,
  VGG_SCANCODE_KP_MINUS = 86,
  VGG_SCANCODE_KP_PLUS = 87,
  VGG_SCANCODE_KP_ENTER = 88,
  VGG_SCANCODE_KP_1 = 89,
  VGG_SCANCODE_KP_2 = 90,
  VGG_SCANCODE_KP_3 = 91,
  VGG_SCANCODE_KP_4 = 92,
  VGG_SCANCODE_KP_5 = 93,
  VGG_SCANCODE_KP_6 = 94,
  VGG_SCANCODE_KP_7 = 95,
  VGG_SCANCODE_KP_8 = 96,
  VGG_SCANCODE_KP_9 = 97,
  VGG_SCANCODE_KP_0 = 98,
  VGG_SCANCODE_KP_PERIOD = 99,

  VGG_SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                      *   keyboards have over ANSI ones,
                                      *   located between left shift and Y.
                                      *   Produces GRAVE ACCENT and TILDE in a
                                      *   US or UK Mac layout, REVERSE SOLIDUS
                                      *   (backslash) and VERTICAL LINE in a
                                      *   US or UK Windows layout, and
                                      *   LESS-THAN SIGN and GREATER-THAN SIGN
                                      *   in a Swiss German, German, or French
                                      *   layout. */
  VGG_SCANCODE_APPLICATION = 101,    /**< windows contextual menu, compose */
  VGG_SCANCODE_POWER = 102,          /**< The USB document says this is a status flag,
                                      *   not a physical key - but some Mac keyboards
                                      *   do have a power key. */
  VGG_SCANCODE_KP_EQUALS = 103,
  VGG_SCANCODE_F13 = 104,
  VGG_SCANCODE_F14 = 105,
  VGG_SCANCODE_F15 = 106,
  VGG_SCANCODE_F16 = 107,
  VGG_SCANCODE_F17 = 108,
  VGG_SCANCODE_F18 = 109,
  VGG_SCANCODE_F19 = 110,
  VGG_SCANCODE_F20 = 111,
  VGG_SCANCODE_F21 = 112,
  VGG_SCANCODE_F22 = 113,
  VGG_SCANCODE_F23 = 114,
  VGG_SCANCODE_F24 = 115,
  VGG_SCANCODE_EXECUTE = 116,
  VGG_SCANCODE_HELP = 117, /**< AL Integrated Help Center */
  VGG_SCANCODE_MENU = 118, /**< Menu (show menu) */
  VGG_SCANCODE_SELECT = 119,
  VGG_SCANCODE_STOP = 120,  /**< AC Stop */
  VGG_SCANCODE_AGAIN = 121, /**< AC Redo/Repeat */
  VGG_SCANCODE_UNDO = 122,  /**< AC Undo */
  VGG_SCANCODE_CUT = 123,   /**< AC Cut */
  VGG_SCANCODE_COPY = 124,  /**< AC Copy */
  VGG_SCANCODE_PASTE = 125, /**< AC Paste */
  VGG_SCANCODE_FIND = 126,  /**< AC Find */
  VGG_SCANCODE_MUTE = 127,
  VGG_SCANCODE_VOLUMEUP = 128,
  VGG_SCANCODE_VOLUMEDOWN = 129,
  /* not sure whether there's a reason to enable these */
  /*     VGG_SCANCODE_LOCKINGCAPSLOCK = 130,  */
  /*     VGG_SCANCODE_LOCKINGNUMLOCK = 131, */
  /*     VGG_SCANCODE_LOCKINGSCROLLLOCK = 132, */
  VGG_SCANCODE_KP_COMMA = 133,
  VGG_SCANCODE_KP_EQUALSAS400 = 134,

  VGG_SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                          footnotes in USB doc */
  VGG_SCANCODE_INTERNATIONAL2 = 136,
  VGG_SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
  VGG_SCANCODE_INTERNATIONAL4 = 138,
  VGG_SCANCODE_INTERNATIONAL5 = 139,
  VGG_SCANCODE_INTERNATIONAL6 = 140,
  VGG_SCANCODE_INTERNATIONAL7 = 141,
  VGG_SCANCODE_INTERNATIONAL8 = 142,
  VGG_SCANCODE_INTERNATIONAL9 = 143,
  VGG_SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
  VGG_SCANCODE_LANG2 = 145, /**< Hanja conversion */
  VGG_SCANCODE_LANG3 = 146, /**< Katakana */
  VGG_SCANCODE_LANG4 = 147, /**< Hiragana */
  VGG_SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
  VGG_SCANCODE_LANG6 = 149, /**< reserved */
  VGG_SCANCODE_LANG7 = 150, /**< reserved */
  VGG_SCANCODE_LANG8 = 151, /**< reserved */
  VGG_SCANCODE_LANG9 = 152, /**< reserved */

  VGG_SCANCODE_ALTERASE = 153, /**< Erase-Eaze */
  VGG_SCANCODE_SYSREQ = 154,
  VGG_SCANCODE_CANCEL = 155, /**< AC Cancel */
  VGG_SCANCODE_CLEAR = 156,
  VGG_SCANCODE_PRIOR = 157,
  VGG_SCANCODE_RETURN2 = 158,
  VGG_SCANCODE_SEPARATOR = 159,
  VGG_SCANCODE_OUT = 160,
  VGG_SCANCODE_OPER = 161,
  VGG_SCANCODE_CLEARAGAIN = 162,
  VGG_SCANCODE_CRSEL = 163,
  VGG_SCANCODE_EXSEL = 164,

  VGG_SCANCODE_KP_00 = 176,
  VGG_SCANCODE_KP_000 = 177,
  VGG_SCANCODE_THOUSANDSSEPARATOR = 178,
  VGG_SCANCODE_DECIMALSEPARATOR = 179,
  VGG_SCANCODE_CURRENCYUNIT = 180,
  VGG_SCANCODE_CURRENCYSUBUNIT = 181,
  VGG_SCANCODE_KP_LEFTPAREN = 182,
  VGG_SCANCODE_KP_RIGHTPAREN = 183,
  VGG_SCANCODE_KP_LEFTBRACE = 184,
  VGG_SCANCODE_KP_RIGHTBRACE = 185,
  VGG_SCANCODE_KP_TAB = 186,
  VGG_SCANCODE_KP_BACKSPACE = 187,
  VGG_SCANCODE_KP_A = 188,
  VGG_SCANCODE_KP_B = 189,
  VGG_SCANCODE_KP_C = 190,
  VGG_SCANCODE_KP_D = 191,
  VGG_SCANCODE_KP_E = 192,
  VGG_SCANCODE_KP_F = 193,
  VGG_SCANCODE_KP_XOR = 194,
  VGG_SCANCODE_KP_POWER = 195,
  VGG_SCANCODE_KP_PERCENT = 196,
  VGG_SCANCODE_KP_LESS = 197,
  VGG_SCANCODE_KP_GREATER = 198,
  VGG_SCANCODE_KP_AMPERSAND = 199,
  VGG_SCANCODE_KP_DBLAMPERSAND = 200,
  VGG_SCANCODE_KP_VERTICALBAR = 201,
  VGG_SCANCODE_KP_DBLVERTICALBAR = 202,
  VGG_SCANCODE_KP_COLON = 203,
  VGG_SCANCODE_KP_HASH = 204,
  VGG_SCANCODE_KP_SPACE = 205,
  VGG_SCANCODE_KP_AT = 206,
  VGG_SCANCODE_KP_EXCLAM = 207,
  VGG_SCANCODE_KP_MEMSTORE = 208,
  VGG_SCANCODE_KP_MEMRECALL = 209,
  VGG_SCANCODE_KP_MEMCLEAR = 210,
  VGG_SCANCODE_KP_MEMADD = 211,
  VGG_SCANCODE_KP_MEMSUBTRACT = 212,
  VGG_SCANCODE_KP_MEMMULTIPLY = 213,
  VGG_SCANCODE_KP_MEMDIVIDE = 214,
  VGG_SCANCODE_KP_PLUSMINUS = 215,
  VGG_SCANCODE_KP_CLEAR = 216,
  VGG_SCANCODE_KP_CLEARENTRY = 217,
  VGG_SCANCODE_KP_BINARY = 218,
  VGG_SCANCODE_KP_OCTAL = 219,
  VGG_SCANCODE_KP_DECIMAL = 220,
  VGG_SCANCODE_KP_HEXADECIMAL = 221,

  VGG_SCANCODE_LCTRL = 224,
  VGG_SCANCODE_LSHIFT = 225,
  VGG_SCANCODE_LALT = 226, /**< alt, option */
  VGG_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
  VGG_SCANCODE_RCTRL = 228,
  VGG_SCANCODE_RSHIFT = 229,
  VGG_SCANCODE_RALT = 230, /**< alt gr, option */
  VGG_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

  VGG_SCANCODE_MODE = 257, /**< I'm not sure if this is really not covered
                            *   by any of the above, but since there's a
                            *   special KMOD_MODE for it I'm adding it here
                            */

  /* @} */ /* Usage page 0x07 */

  /**
   *  \name Usage page 0x0C
   *
   *  These values are mapped from usage page 0x0C (USB consumer page).
   *  See https://usb.org/sites/default/files/hut1_2.pdf
   *
   *  There are way more keys in the spec than we can represent in the
   *  current scancode range, so pick the ones that commonly come up in
   *  real world usage.
   */
  /* @{ */

  VGG_SCANCODE_AUDIONEXT = 258,
  VGG_SCANCODE_AUDIOPREV = 259,
  VGG_SCANCODE_AUDIOSTOP = 260,
  VGG_SCANCODE_AUDIOPLAY = 261,
  VGG_SCANCODE_AUDIOMUTE = 262,
  VGG_SCANCODE_MEDIASELECT = 263,
  VGG_SCANCODE_WWW = 264, /**< AL Internet Browser */
  VGG_SCANCODE_MAIL = 265,
  VGG_SCANCODE_CALCULATOR = 266, /**< AL Calculator */
  VGG_SCANCODE_COMPUTER = 267,
  VGG_SCANCODE_AC_SEARCH = 268,    /**< AC Search */
  VGG_SCANCODE_AC_HOME = 269,      /**< AC Home */
  VGG_SCANCODE_AC_BACK = 270,      /**< AC Back */
  VGG_SCANCODE_AC_FORWARD = 271,   /**< AC Forward */
  VGG_SCANCODE_AC_STOP = 272,      /**< AC Stop */
  VGG_SCANCODE_AC_REFRESH = 273,   /**< AC Refresh */
  VGG_SCANCODE_AC_BOOKMARKS = 274, /**< AC Bookmarks */

  /* @} */ /* Usage page 0x0C */

  /**
   *  \name Walther keys
   *
   *  These are values that Christian Walther added (for mac keyboard?).
   */
  /* @{ */

  VGG_SCANCODE_BRIGHTNESSDOWN = 275,
  VGG_SCANCODE_BRIGHTNESSUP = 276,
  VGG_SCANCODE_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                         switch, video mode switch */
  VGG_SCANCODE_KBDILLUMTOGGLE = 278,
  VGG_SCANCODE_KBDILLUMDOWN = 279,
  VGG_SCANCODE_KBDILLUMUP = 280,
  VGG_SCANCODE_EJECT = 281,
  VGG_SCANCODE_SLEEP = 282, /**< SC System Sleep */

  VGG_SCANCODE_APP1 = 283,
  VGG_SCANCODE_APP2 = 284,

  /* @} */ /* Walther keys */

  /**
   *  \name Usage page 0x0C (additional media keys)
   *
   *  These values are mapped from usage page 0x0C (USB consumer page).
   */
  /* @{ */

  VGG_SCANCODE_AUDIOREWIND = 285,
  VGG_SCANCODE_AUDIOFASTFORWARD = 286,

  /* @} */ /* Usage page 0x0C (additional media keys) */

  /**
   *  \name Mobile keys
   *
   *  These are values that are often used on mobile phones.
   */
  /* @{ */

  VGG_SCANCODE_SOFTLEFT = 287,  /**< Usually situated below the display on phones and
                                     used as a multi-function feature key for selecting
                                     a software defined function shown on the bottom left
                                     of the display. */
  VGG_SCANCODE_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                     used as a multi-function feature key for selecting
                                     a software defined function shown on the bottom right
                                     of the display. */
  VGG_SCANCODE_CALL = 289,      /**< Used for accepting phone calls. */
  VGG_SCANCODE_ENDCALL = 290,   /**< Used for rejecting phone calls. */

  /* @} */ /* Mobile keys */

  /* Add any other keys here. */

  VGG_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                               for array bounds */
};
