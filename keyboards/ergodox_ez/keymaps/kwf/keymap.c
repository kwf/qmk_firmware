#include "ergodox_ez.h"
#include "debug.h"
#include "action_layer.h"
#include "version.h"
#include "keycode.h"

/* { TOGGLE_KEY(PERIOD, 0, KC_DOT,   MOD_LSFT, KC_SCLN), */
/*   TOGGLE_KEY(COMMA,  0, KC_COMM,  0,        KC_SCLN), */
/*   TOGGLE_KEY(SLASH,  0, KC_SLASH, MOD_LSFT | MOD_LALT, KC_MINUS) }; */

typedef struct dual_key {
  uint16_t input_code;   // which logical key?
  uint16_t hold_mods;    // hold these mods down when this key is held down
  // when released, if no other key has been touched...
  uint16_t toggle_mods;  // which set of mods is the "threshold" b/t lo/hi?
  // if toggle_mods not satisfied, tap lo_code with lo_mods down
  uint16_t lo_mods;
  uint16_t lo_code;
  // if toggle_mods satisfied, tap hi_code with hi_mods down
  uint16_t hi_mods;
  uint16_t hi_code;
  // is it currently being held down?
  bool pressed;
} dual_key_t;

typedef struct toggle_key {
  uint16_t input_code;   // which logical key?
  uint16_t toggle_mods;  // which set of mods is the "threshold" b/t lo/hi?
  // if toggle_mods not satisfied, press lo_code with lo_mods down
  uint16_t lo_mods;
  uint16_t lo_code;
  // if toggle_mods satisfied, press hi_code with hi_mods down
  uint16_t hi_mods;
  uint16_t hi_code;
  // is it currently being held down?
  bool pressed;
} toggle_key_t;

#define TOGGLE_KEY(IN, TOGGLE_MODS, LO_MODS, LO, HI_MODS, HI) \
  ((toggle_key_t) { (IN), (TOGGLE_MODS), (LO_MODS), (LO), (HI_MODS), (HI), false })

#define SHIFT_KEY(IN, LO_MODS, LO, HI_MODS, HI) \
  (TOGGLE_KEY((IN), (MOD_LSFT), (LO_MODS), (LO), (HI_MODS), (HI)))

#define DUAL_KEY(IN, HOLD, TOGGLE, LO_MODS, LO, HI_MODS, HI) \
  ((dual_key_t) { (IN), (HOLD), (TOGGLE), (LO_MODS), (LO), (HI_MODS), (HI), false })

#define SIMPLE_DUAL_KEY(IN, HOLD, MODS, TAP) \
  (DUAL_KEY((IN), (HOLD), 0, (MODS), (TAP), (MODS), (TAP)))

#define SHIFT_DUAL_KEY(IN, HOLD, LO_MODS, LO, HI_MODS, HI) \
  (DUAL_KEY((IN), (HOLD), (MOD_LSFT), (LO_MODS), (LO), (HI_MODS), (HI)))


enum custom_keycodes {
  PLACEHOLDER = SAFE_RANGE, // can always be here
  EPRM,
  VRSN,
  RGB_SLD,

  // Keycodes for custom capitalization
  LPAREN,  // '(' and '['
  RPAREN,  // ')' and ']'
  LANGLE,  // '<' and '{'
  RANGLE,  // '>' and '}'
  SLASH,   // '/' and (em-dash)
  ATSIGN,  // '@' and '#'
  CARET,   // '^' and '&'
  DOLLAR,  // '$' and '*'

  PERIOD,  // '.' and ':'
  COMMA,   // ',' and ';'
  BANG,    // '!' and '`'
  QUES,    // '?' and '~'

  REPORT,  // report some custom info

  // Dual-function keys
  DF_ENT,  // ENT, hold for CMD
  DF_TAB,  // TAB, hold for ALT
  DF_ESC,  // ESC, hold for SFT
};

static toggle_key_t TOGGLE_KEYS[] =
  { SHIFT_KEY(PERIOD, 0, KC_DOT, MOD_LSFT, KC_SCLN) };

static dual_key_t DUAL_KEYS[] = { };

const int NUM_TOGGLE_KEYS = sizeof(TOGGLE_KEYS) / sizeof(toggle_key_t);
const int NUM_DUAL_KEYS   = sizeof(DUAL_KEYS)   / sizeof(dual_key_t);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [0] = LAYOUT_ergodox(
        XXXXXXX, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    XXXXXXX,
        LPAREN,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    BANG,
        KC_BSLS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
        LANGLE,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    PERIOD,
        XXXXXXX, XXXXXXX, XXXXXXX, KC_EQL,  KC_QUOT,

                                                     TT(1),   KC_LSFT,
                                                              KC_LCTL,
                                            DF_ESC,  DF_ENT,  KC_LALT,

        XXXXXXX, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    REPORT,
        QUES,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    RPAREN,
                 KC_H,    KC_J,    KC_K,    KC_L,    ATSIGN,  SLASH,
        COMMA,   KC_N,    KC_M,    CARET,   DOLLAR,  KC_UP,   RANGLE,
                          KC_MINS, KC_GRV,  KC_LEFT, KC_DOWN, KC_RGHT,

        KC_LSFT, TT(1),
        KC_LCTL,
        DF_TAB, KC_BSPC,  KC_SPC),

  [1] = LAYOUT_ergodox(
        XXXXXXX, KC_1,    KC_2,    KC_3,    KC_BTN2, KC_BTN1, KC_BTN3,
        XXXXXXX, KC_4,    KC_5,    KC_6,    XXXXXXX, KC_ACL0, KC_WH_D,
        XXXXXXX, KC_7,    KC_8,    KC_9,    XXXXXXX, KC_ACL1,
        XXXXXXX, KC_KP_0, KC_0,    KC_PDOT, XXXXXXX, KC_ACL2, KC_WH_U,
        RGB_TOG, RGB_VAD, RGB_VAI, RGB_HUD, RGB_HUI,

                                                     TO(0),   _______,
                                                              _______,
                                           _______,  _______, _______,

        KC_BTN3, KC_BTN1, KC_BTN2, XXXXXXX, XXXXXXX, XXXXXXX, KC_MPLY,
        KC_WH_D, KC_MS_L, KC_MS_D, KC_MS_U, KC_MS_R, XXXXXXX, XXXXXXX,
                 KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX, XXXXXXX,
        KC_WH_U, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_VOLU, XXXXXXX,
                          XXXXXXX, XXXXXXX, KC_MPRV, KC_VOLD, KC_MNXT,

        _______, TO(0),
        _______,
        _______, _______, _______),

};

const uint16_t PROGMEM fn_actions[] = {
  [1] = ACTION_LAYER_TAP_TOGGLE(1)
};

void matrix_init_user(void) {
#ifdef RGBLIGHT_COLOR_LAYER_0
  rgblight_setrgb(RGBLIGHT_COLOR_LAYER_0);
#endif
};

// Track whether a key is up or down by updating a boolean variable
void track_key(uint16_t keycode, bool *flag,
               uint16_t match, keyrecord_t *record) {
    if (keycode == match) {
        *flag = record->event.pressed;
    }
}

// Determines if the keycode corresponding to an ASCII character requires
// shift to be pressed when it is sent
bool is_shifted_char(char ascii_code) {
  return pgm_read_byte(&ascii_to_shift_lut[(uint8_t)ascii_code]);
}

// Returns the keycode corresponding to an ASCII character (may require shift
// to be pressed to correctly send it, see is_shifted_char)
uint16_t char_code(char ascii_code) {
  return pgm_read_byte(&ascii_to_keycode_lut[(uint8_t)ascii_code]);
}

// Presses (or un-presses) the keycode corresponding to an ASCII character,
// including shift if necessary (to achieve the right effect).
// This function is a decomposition of the functionality of send_char (in the
// file quantum/quantum.c) to allow separate up/down presses, with the addition
// of behaving sensibly in the presence of shift being held down.
void press_char(bool shift_down, bool press, char ascii_code) {
  const uint8_t shift = KC_LSFT;
  // if pressing the key, set shift appropriately
  if (press) {
    if (is_shifted_char(ascii_code)) {
      if (!shift_down) {
        register_code(shift);
      }
    } else {
      if (shift_down) {
        unregister_code(shift);
      }
    }
  }
  // actually press/unpress the key
  const uint8_t key = char_code(ascii_code);
  if (press) {
    register_code(key);
  } else {
    unregister_code(key);
  }
  // if unpressing the key, unset shift appropriately
  if (!press) {
    if (is_shifted_char(ascii_code)) {
      if (!shift_down) {
        unregister_code(shift);
      }
    } else {
      if (shift_down) {
        register_code(shift);
      }
    }
  }
}

// Sends a down/up-stroke including shift if shift was pressed
bool capitalized(uint16_t cap_code,
                 char no,
                 char yes,
                 bool shift_down,
                 uint16_t keycode,
                 keyrecord_t *record) {
  if (keycode == cap_code) {
    // Press or unpress at the right shift level
    if (record->event.pressed) {
      press_char(shift_down, true, shift_down ? yes : no);
    } else {
      // But if unpress, unpress both
      // This prevents a stuck-key bug that happened with the sequence
      // shift down, key down, shift up, key up
      press_char(shift_down, false, yes);
      press_char(shift_down, false, no);
    }
    return true;
  } else {
    return false;
  }
}

// Sets up a "flexible" dual-function key, returns whether the keyrecord matched
// A tap on enter sends ENTER, but it's also a normal command key This works
// better than the built-in mod-tap functionality
// <https://docs.qmk.fm/#/feature_advanced_keycodes?id=mod-tap> because it
// allows quick DOWN(ENTER), DOWN(key), UP(ENTER), UP(key) interleaving, which
// is often something my typing produces when I'm going fast.
bool mod_tap_key(uint16_t dual_code,
                 uint16_t mod_keycode,
                 uint16_t tap_keycode,
                 bool *flag,
                 uint16_t keycode,
                 keyrecord_t *record) {
  if (keycode == dual_code) {
    if (record->event.pressed) {
      // Put down the mod key
      register_code(mod_keycode);
      *flag = true;
    } else {
      // Release the mod key
      unregister_code(mod_keycode);
      if (*flag) {
        // Tap the tap key
        register_code(tap_keycode);
        unregister_code(tap_keycode);
      }
    }
    return true;
  } else {
    *flag = false;
    return false;
  }
}

void press_code(bool down, uint16_t keycode) {
  if (down) {
    register_code(keycode);
  } else {
    unregister_code(keycode);
  }
}

bool superset(uint16_t super, uint16_t sub) {
  return (super & sub) == super;
}

uint16_t mod_for_code(uint16_t current_mods, uint16_t keycode) {
  uint16_t mods = 0;
  switch (keycode) {
  case KC_LSFT: mods = MOD_LSFT; break;
  case KC_LCTL: mods = MOD_LCTL; break;
  case KC_LALT: mods = MOD_LALT; break;
  case KC_LGUI: mods = MOD_LGUI; break;
  }
  for (int i = 0; i < NUM_DUAL_KEYS; i++) {
    dual_key_t key = DUAL_KEYS[i];
    if (keycode == key.input_code) {
      bool hi_state = superset(current_mods, key.toggle_mods);
      mods |= (hi_state ? key.hi_mods : key.lo_mods);
    }
  }
  return mod;
}

void press_mods(bool down, uint16_t mods) {
  if (mods & MOD_LSFT) { press_code(down, MOD_LSFT); }
  if (mods & MOD_LCTL) { press_code(down, MOD_LCTL); }
  if (mods & MOD_LALT) { press_code(down, MOD_LALT); }
  if (mods & MOD_LGUI) { press_code(down, MOD_LGUI); }
}

void print_code(uint16_t code) {
  for (int i = 0; i < 16; i++) {
    if (code & 1) {
      SEND_STRING("1");
    } else {
      SEND_STRING("0");
    }
    register_code(KC_LEFT);
    unregister_code(KC_LEFT);
    code = code >> 1;
  }
}

void transition_mods(uint16_t *current, uint16_t desired) {
  /* press_mods(false, *current); */
  /* press_mods(true, desired); */
  uint16_t new =  desired & ~(*current);
  uint16_t old = *current &   ~desired;
  press_mods(false, old);
  press_mods(true, new);
  // We have will now set current to desired. Why? The spec is: remove all
  // current modifiers, add all desired modifiers; instead we remove the ones
  // that are old and add the ones that are new. So we wish to show:
  // d = (c & ~old)           | new,    [ where old = ~d & c, new = ~c & d ]
  // d = (c & ~(c  & ~d))     | ~c & d  [ subst ]
  // d = (c & (~c  |  d))     | ~c & d  [ de Morgan ]
  // d = ((c & ~c) | (c & d)) | ~c & d  [ and-assoc ]
  // d = (false    | (c & d)) | ~c & d  [ excluded-middle ]
  // d =              c & d   | ~c & d  [ false-elim ]
  // d =             (c | ~c) &      d  [ common factor elim ]
  // d =               true   &      d  [ excluded middle ]
  // d =                             d  [ true-elim ]
  // QED. Also it makes intuitive sense :P
  *current = desired;
}

void report_mods(uint16_t current) {
  bool any = false;
  if (current & MOD_LSFT) { SEND_STRING("LSFT"); any = true; }
  if (current & MOD_LCTL) { SEND_STRING("LCTL"); any = true; }
  if (current & MOD_LALT) { SEND_STRING("LALT"); any = true; }
  if (current & MOD_LGUI) { SEND_STRING("LGUI"); any = true; }
  if (!any) { SEND_STRING("<none>"); }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  // Put debugging code here, it'll run only on first keypress
  static bool init = true;
  if (init) {
    // debug code here
    init = false;
  }

  // Keep track of whether shift is currently pressed
  static bool shift_down = false;
  track_key(DF_ESC, &shift_down, keycode, record);

  // Track which modifiers are currently down
  // Currently doesn't work with dual-function keys that map to mods
  static uint16_t current_mods = 0;
  uint16_t mod_update = mod_for_code(keycode);
  if (record->event.pressed) {
    current_mods |= mod_update;
  } else {
    current_mods &= ~mod_update;
  }
  if (mod_update) {
    if (record->event.pressed) {
      press_mods(false, current_mods);
      report_mods(current_mods);
      /* print_code(current_mods); */
      press_mods(true, current_mods);
    }
    // TODO: loop over all mask keys, adjusting their pressed codes and such
  }

  // Keep track of whether to release particular keys for dual-function keys
  static bool ent_on_up = false;
  static bool tab_on_up = false;
  static bool esc_on_up = false;

  bool match
     = mod_tap_key(DF_ENT, KC_LGUI, KC_ENT, &ent_on_up, keycode, record)
    || mod_tap_key(DF_TAB, KC_LALT, KC_TAB, &tab_on_up, keycode, record)
    || mod_tap_key(DF_ESC, KC_LSFT, KC_ESC, &esc_on_up, keycode, record)
    /* || capitalized(PERIOD, '.',     ':',    shift_down, keycode, record) */
    || capitalized(COMMA,  ',',     ';',    shift_down, keycode, record)
    || capitalized(BANG,   '!',     '`',    shift_down, keycode, record)
    || capitalized(QUES,   '?',     '~',    shift_down, keycode, record)
    || capitalized(LPAREN, '(',     '[',    shift_down, keycode, record)
    || capitalized(RPAREN, ')',     ']',    shift_down, keycode, record)
    || capitalized(LANGLE, '<',     '{',    shift_down, keycode, record)
    || capitalized(RANGLE, '>',     '}',    shift_down, keycode, record)
    || capitalized(ATSIGN, '@',     '#',    shift_down, keycode, record)
    || capitalized(CARET,  '^',     '&',    shift_down, keycode, record)
    || capitalized(DOLLAR, '$',     '*',    shift_down, keycode, record)
    ;

  // Save the modifier state before processing toggle keys
  uint16_t initial_mods = current_mods;

  for (int i = 0; i < NUM_TOGGLE_KEYS; i++) {
    toggle_key_t key = TOGGLE_KEYS[i];
    if (keycode == key.input_code) {
      // We've found a matching key; recall that
      match = true;
      // We're in "hi" (toggled) state when current_mods superset of toggle_mods
      bool hi_state = superset(current_mods, key.toggle_mods);
      // Switch the active mods to the ones for this key
      transition_mods(&current_mods, hi_state ? key.hi_mods : key.lo_mods);
      if (hi_state) {
        // if current_mods contain toggle_mods
        press_code(record->event.pressed, key.hi_code);
      } else {
        // if current_mods doesn't contain toggle_mods
        press_code(record->event.pressed, key.lo_code);
      }
    }
  }

  // Reset modifier state to whatever it was before processing toggle keys
  transition_mods(&current_mods, initial_mods);


  // Extra modifiers aren't (yet) supported by the 'capitalized' function,
  // which means it's tricky to get key-repeat for em-dash. We punt on this
  // and don't give it repeat behavior.
  if (keycode == SLASH) {
    match = true;
    if (record->event.pressed) {
      if (!shift_down) {
        SEND_STRING("/");
      } else {
        // em-dash
        SEND_STRING(SS_DOWN(X_LSHIFT)); // don't lift shift, let user do that
        SEND_STRING(SS_LALT("-"));
      }
    }
  }

  return !match; // If none of our custom processing fired, defer to system
}

uint32_t layer_state_set_user(uint32_t state) {

    uint8_t layer = biton32(state);

    ergodox_board_led_off();
    ergodox_right_led_1_off();
    ergodox_right_led_2_off();
    ergodox_right_led_3_off();
    switch (layer) {
      case 0:
        #ifdef RGBLIGHT_COLOR_LAYER_0
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_0);
        #endif
        break;
      case 1:
        ergodox_right_led_1_on();
        #ifdef RGBLIGHT_COLOR_LAYER_1
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_1);
        #endif
        break;
      case 2:
        ergodox_right_led_2_on();
        #ifdef RGBLIGHT_COLOR_LAYER_2
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_2);
        #endif
        break;
      case 3:
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_3
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_3);
        #endif
        break;
      case 4:
        ergodox_right_led_1_on();
        ergodox_right_led_2_on();
        #ifdef RGBLIGHT_COLOR_LAYER_4
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_4);
        #endif
        break;
      case 5:
        ergodox_right_led_1_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_5
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_5);
        #endif
        break;
      case 6:
        ergodox_right_led_2_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_6
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_6);
        #endif
        break;
      case 7:
        ergodox_right_led_1_on();
        ergodox_right_led_2_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_7
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_6);
        #endif
        break;
      default:
        break;
    }
    return state;

};
