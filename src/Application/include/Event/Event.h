#pragma once

#include <cstdint>
#include "Keycode.h"
#include "Scancode.h"

enum EButtonState : uint8_t
{
  VGG_Release,
  VGG_Pressed
};

enum EEventType
{
  VGG_FIRSTEVENT = 0, /**< Unused (do not remove) */

  /* Application events */
  VGG_QUIT = 0x100, /**< User-requested quit */

  /* These application events have special meaning on iOS, see README-ios.md for details */
  VGG_APP_TERMINATING,         /**< The application is being terminated by the OS
                                    Called on iOS in applicationWillTerminate()
                                    Called on Android in onDestroy()
                               */
  VGG_APP_LOWMEMORY,           /**< The application is low on memory, free memory if possible.
                                    Called on iOS in applicationDidReceiveMemoryWarning()
                                    Called on Android in onLowMemory()
                               */
  VGG_APP_WILLENTERBACKGROUND, /**< The application is about to enter the background
                                   Called on iOS in applicationWillResignActive()
                                   Called on Android in onPause()
                              */
  VGG_APP_DIDENTERBACKGROUND,  /**< The application did enter the background and may not get CPU
                                  for some time  Called on iOS in applicationDidEnterBackground()
                                    Called on Android in onPause()
                               */
  VGG_APP_WILLENTERFOREGROUND, /**< The application is about to enter the foreground
                                   Called on iOS in applicationWillEnterForeground()
                                   Called on Android in onResume()
                              */
  VGG_APP_DIDENTERFOREGROUND,  /**< The application is now interactive
                                    Called on iOS in applicationDidBecomeActive()
                                    Called on Android in onResume()
                               */

  VGG_LOCALECHANGED, /**< The user's locale preferences have changed. */

  /* Display events */
  VGG_DISPLAYEVENT = 0x150, /**< Display state change */

  /* Window events */
  VGG_WINDOWEVENT = 0x200, /**< Window state change */
  VGG_SYSWMEVENT,          /**< System specific event */

  /* Keyboard events */
  VGG_KEYDOWN = 0x300, /**< Key pressed */
  VGG_KEYUP,           /**< Key released */
  VGG_TEXTEDITING,     /**< Keyboard text editing (composition) */
  VGG_TEXTINPUT,       /**< Keyboard text input */
  VGG_KEYMAPCHANGED,   /**< Keymap changed due to a system event such as an
                            input language or keyboard layout change.
                       */
  VGG_TEXTEDITING_EXT, /**< Extended keyboard text editing (composition) */

  /* Mouse events */
  VGG_MOUSEMOTION = 0x400, /**< Mouse moved */
  VGG_MOUSEBUTTONDOWN,     /**< Mouse button pressed */
  VGG_MOUSEBUTTONUP,       /**< Mouse button released */
  VGG_MOUSEWHEEL,          /**< Mouse wheel motion */

  /* Touch events */
  VGG_FINGERDOWN = 0x700,
  VGG_FINGERUP,
  VGG_FINGERMOTION,

  /* Clipboard events */
  VGG_CLIPBOARDUPDATE = 0x900, /**< The clipboard or primary selection changed */

  /* Drag and drop events */
  VGG_DROPFILE = 0x1000, /**< The system requests a file open */
  VGG_DROPTEXT,          /**< text/plain drag-and-drop event */
  VGG_DROPBEGIN,         /**< A new set of drops is beginning (NULL filename) */
  VGG_DROPCOMPLETE,      /**< Current set of drops is now complete (NULL filename) */

  VGG_PAINT = 0x1100,

  VGG_USEREVENT = 0x8000,

  /**
   *  This last event is only for bounding internal arrays
   */
  VGG_LASTEVENT = 0xFFFF
};

/**
 *  \brief Window state change event data (event.window.*)
 */

struct VKeysym
{
  EVGGScancode scancode; /**< SDL physical key code - see ::SDL_Scancode for details */
  EVGGKeyCode sym;       /**< SDL virtual key code - see ::SDL_Keycode for details */
  uint16_t mod;          /**< current key modifiers */
  uint32_t unused;
};

/**
 *  \brief Keyboard button event structure (event.key.*)
 */
struct VKeyboardEvent
{
  uint32_t type;      /**< ::VGG_KEYDOWN or ::SDL_KEYUP */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  EButtonState state; /**< ::VGG_Pressed or ::SDL_Release */
  uint8_t repeat;     /**< Non-zero if this is a key repeat */
  uint8_t padding2;
  uint8_t padding3;
  VKeysym keysym; /**< The key that was pressed or released */
};

#define VGG_TEXTEDITINGEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text editing event structure (event.edit.*)
 */
struct VTextEditingEvent
{
  uint32_t type;                             /**< ::VGG_TEXTEDITING */
  uint32_t timestamp;                        /**< In milliseconds, populated using VGG_GetTicks() */
  char text[VGG_TEXTEDITINGEVENT_TEXT_SIZE]; /**< The editing text */
  int32_t start;                             /**< The start cursor of selected editing text */
  int32_t length;                            /**< The length of selected editing text */
};

/**
 *  \brief Extended keyboard text editing event structure (event.editExt.*) when text would be
 *  truncated if stored in the text buffer VGG_TextEditingEvent
 */
struct VTextEditingExtEvent
{
  uint32_t type;      /**< ::VGG_TEXTEDITING_EXT */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  char* text; /**< The editing text, which should be freed with VGG_free(), and will not be NULL */
  int32_t start;  /**< The start cursor of selected editing text */
  int32_t length; /**< The length of selected editing text */
};

#define VGG_TEXTINPUTEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text input event structure (event.text.*)
 */
struct VTextInputEvent
{
  uint32_t type;                           /**< ::VGG_TEXTINPUT */
  uint32_t timestamp;                      /**< In milliseconds, populated using VGG_GetTicks() */
  char text[VGG_TEXTINPUTEVENT_TEXT_SIZE]; /**< The input text */
};

/**
 *  \brief Mouse motion event structure (event.motion.*)
 */
struct VMouseMotionEvent
{
  uint32_t type;      /**< ::VGG_MOUSEMOTION */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t which;     /**< The mouse instance id, or VGG_TOUCH_MOUSEID */
  uint32_t state;     /**< The current button state */
  int32_t x;          /**< X coordinate, relative to window */
  int32_t y;          /**< Y coordinate, relative to window */
  int32_t xrel;       /**< The relative motion in the X direction */
  int32_t yrel;       /**< The relative motion in the Y direction */
};

/**
 *  \brief Mouse button event structure (event.button.*)
 */
struct VMouseButtonEvent
{
  uint32_t type;      /**< ::VGG_MOUSEBUTTONDOWN or ::SDL_MOUSEBUTTONUP */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t which;     /**< The mouse instance id, or VGG_TOUCH_MOUSEID */
  uint8_t button;     /**< The mouse button index */
  EButtonState state; /**< ::VGG_PRESSED or ::SDL_RELEASED */
  uint8_t clicks;     /**< 1 for single-click, 2 for double-click, etc. */
  uint8_t padding1;
  int32_t x; /**< X coordinate, relative to window */
  int32_t y; /**< Y coordinate, relative to window */
};

/**
 *  \brief Mouse wheel event structure (event.wheel.*)
 */
struct VMouseWheelEvent
{
  uint32_t type;      /**< ::VGG_MOUSEWHEEL */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t which;     /**< The mouse instance id, or VGG_TOUCH_MOUSEID */
  int32_t
    x; /**< The amount scrolled horizontally, positive to the right and negative to the left */
  int32_t y; /**< The amount scrolled vertically, positive away from the user and negative toward
               the user */
  uint32_t direction; /**< Set to one of the VGG_MOUSEWHEEL_* defines. When FLIPPED the values in
                         X and Y will be opposite. Multiply by -1 to change them back */
  float preciseX; /**< The amount scrolled horizontally, positive to the right and negative to the
                     left, with float precision (added in 2.0.18) */
  float preciseY; /**< The amount scrolled vertically, positive away from the user and negative
                     toward the user, with float precision (added in 2.0.18) */
  int32_t mouseX; /**< X coordinate, relative to window (added in 2.26.0) */
  int32_t mouseY; /**< Y coordinate, relative to window (added in 2.26.0) */
};

/**
 *  \brief An event used to request a file open by the system (event.drop.*)
 *         This event is enabled by default, you can disable it with VGG_EventState().
 *  \note If this event is enabled, you must free the filename in the event.
 */
struct VDropEvent
{
  uint32_t type; /**< ::VGG_DROPBEGIN or ::SDL_DROPFILE or ::SDL_DROPTEXT or ::SDL_DROPCOMPLETE */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  char*
    file; /**< The file name, which should be freed with VGG_free(), is NULL on begin/complete */
};

/**
 *  \brief Sensor event structure (event.sensor.*)
 */

/**
 *  \brief The "quit requested" event
 */
struct VQuitEvent
{
  uint32_t type;      /**< ::VGG_QUIT */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
};

/**
 *  \brief A user-defined event type (event.user.*)
 */
struct VUserEvent
{
  uint32_t type;      /**< ::VGG_USEREVENT through ::SDL_LASTEVENT-1 */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  int32_t code;       /**< User defined event code */
  void* data1;        /**< User defined data pointer */
  void* data2;        /**< User defined data pointer */
};

struct VPaintEvent
{
  uint32_t type;
  uint32_t timestamp;
  void* data;
};

/**
 *  \brief General event structure
 */
union UEvent
{
  uint32_t type;                /**< Event type, shared with all events */
  VKeyboardEvent key;           /**< Keyboard event data */
  VTextEditingEvent edit;       /**< Text editing event data */
  VTextEditingExtEvent editExt; /**< Extended text editing event data */
  VTextInputEvent text;         /**< Text input event data */
  VMouseMotionEvent motion;     /**< Mouse motion event data */
  VMouseButtonEvent button;     /**< Mouse button event data */
  VMouseWheelEvent wheel;       /**< Mouse wheel event data */
  VQuitEvent quit;              /**< Quit request event data */
  VUserEvent user;              /**< Custom event data */
  VDropEvent drop;              /**< Drag and drop event data */
  VPaintEvent paint;

  /* This is necessary for ABI compatibility between Visual C++ and GCC.
     Visual C++ will respect the push pack pragma and use 52 bytes (size of
     VGG_TextEditingEvent, the largest structure for 32-bit and 64-bit
     architectures) for this union, and GCC will use the alignment of the
     largest datatype within the union, which is 8 bytes on 64-bit
     architectures.

     So... we'll add padding to force the size to be 56 bytes for both.

     On architectures where pointers are 16 bytes, this needs rounding up to
     the next multiple of 16, 64, and on architectures where pointers are
     even larger the size of VGG_UserEvent will dominate as being 3 pointers.
  */
  uint8_t padding[sizeof(void*) <= 8 ? 56 : sizeof(void*) == 16 ? 64 : 3 * sizeof(void*)];
};

/* vi: set ts=4 sw=4 expandtab: */
