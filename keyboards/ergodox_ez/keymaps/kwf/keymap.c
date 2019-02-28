#include "ergodox_ez.h"
#include "debug.h"
#include "action_layer.h"
#include "version.h"

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
  PERCENT, // '%'

  DF_ENT,    // ENTER, hold for CMD
  DF_TAB,    // TAB, hold for SHIFT
  DF_ESC,    // ESC, hold for SHIFT
  DF_CMD_ENT,  // CMD+ENTER, hold for CTL
  DF_SFT_ENT,  // SFT+ENTER, hold for ALT
};

#define DUAL_KEY_TIMEOUT 300

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [0] = LAYOUT_ergodox(
        XXXXXXX, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    XXXXXXX,
        LPAREN,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    BANG,
        KC_BSLS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
        LANGLE,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    PERIOD,
        XXXXXXX, XXXXXXX, XXXXXXX, KC_EQL,  LT(1, KC_QUOT),

                                                     XXXXXXX, XXXXXXX,
                                                              DF_CMD_ENT,
                                            DF_ESC,  DF_ENT,  DF_SFT_ENT,

        XXXXXXX, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    XXXXXXX,
        QUES,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    RPAREN,
                 KC_H,    KC_J,    KC_K,    KC_L,    ATSIGN,  SLASH,
        COMMA,   KC_N,    KC_M,    CARET,   DOLLAR,  PERCENT, RANGLE,
                          KC_MINS, KC_GRV,  XXXXXXX, XXXXXXX, XXXXXXX,

        XXXXXXX, XXXXXXX,
        KC_LCTL,
        DF_TAB,  KC_BSPC, KC_SPC),

  [1] = LAYOUT_ergodox(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, KC_BTN3, KC_BTN2, KC_BTN1, XXXXXXX, XXXXXXX,
        XXXXXXX, KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,

                                                     TO(0),   _______,
                                                              _______,
                                           _______,  _______, _______,

        XXXXXXX, XXXXXXX, KC_WH_U, KC_WH_D, XXXXXXX, XXXXXXX, KC_MPLY,
        XXXXXXX, KC_MS_L, KC_MS_D, KC_MS_U, KC_MS_R, XXXXXXX, KC_VOLU,
                 KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX, KC_VOLD,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_MNXT,
                          XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_MPRV,

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

// Press a keycode (true = press down, false = release)
void press_code(bool down, uint16_t keycode) {
  if (down) {
    register_code(keycode);
  } else {
    unregister_code(keycode);
  }
}

// Tap a keycode (press and immediately release)
void tap_code(uint16_t keycode) {
  press_code(true, keycode);
  press_code(false, keycode);
}

// Press (or release) the modifier keys corresponding to the mods specified
void press_mods(bool down, uint16_t mods) {
  if (mods & MOD_LSFT) { press_code(down, KC_LSFT); }
  if (mods & MOD_LCTL) { press_code(down, KC_LCTL); }
  if (mods & MOD_LALT) { press_code(down, KC_LALT); }
  if (mods & MOD_LGUI) { press_code(down, KC_LGUI); }
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
                 uint16_t lo_mods,
                 char lo,
                 uint16_t hi_mods,
                 char hi,
                 bool shift_down,
                 uint16_t keycode,
                 keyrecord_t *record) {
  if (keycode == cap_code) {
    // Press or unpress at the right shift level
    if (record->event.pressed) {
      press_mods(true, shift_down ? hi_mods : lo_mods);
      press_char(shift_down, true, shift_down ? hi : lo);
    } else {
      // But if unpress, unpress both
      // This prevents a stuck-key bug that happened with the sequence
      // shift down, key down, shift up, key up
      press_char(shift_down, false, hi);
      press_char(shift_down, false, lo);
      press_mods(false, hi_mods | lo_mods);
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
// is often something my typing produces when I'm going fast. Currently only
// supports one possible mod to press while pressing release-key.
bool mod_tap_key(uint16_t dual_code,
                 uint16_t mod_keycode,
                 uint16_t tap_mods,
                 uint16_t tap_keycode,
                 bool *tap_on_up,
                 uint16_t keycode,
                 keyrecord_t *record) {
  static uint16_t key_timer;
  if (keycode == dual_code) {
    if (record->event.pressed) {
      // Put down the mod key
      register_code(mod_keycode);
      *tap_on_up = true;
      key_timer = timer_read();
    } else {
      // Release the mod key
      unregister_code(mod_keycode);
      if (*tap_on_up && timer_elapsed(key_timer) < DUAL_KEY_TIMEOUT) {
        // Tap the tap key
        press_mods(true, tap_mods);
        tap_code(tap_keycode);
        press_mods(false, tap_mods);
      }
    }
    return true;
  } else {
    *tap_on_up = false;
    return false;
  }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  // Keep track of whether shift is currently pressed
  static bool shift_down = false;
  track_key(DF_ESC, &shift_down, keycode, record);

  static bool ent_on_up = false;
  static bool tab_on_up = false;
  static bool esc_on_up = false;
  static bool sft_ent_on_up = false;
  static bool cmd_ent_on_up = false;

  bool match
    // mod-tap keys must come first, in case they press shift
    =  mod_tap_key(DF_ENT,     KC_LGUI, 0,        KC_ENT, &ent_on_up,     keycode, record)
    || mod_tap_key(DF_TAB,     KC_LSFT, 0,        KC_TAB, &tab_on_up,     keycode, record)
    || mod_tap_key(DF_ESC,     KC_LSFT, 0,        KC_ESC, &esc_on_up,     keycode, record)
    || mod_tap_key(DF_SFT_ENT, KC_LALT, MOD_LSFT, KC_ENT, &sft_ent_on_up, keycode, record)
    || mod_tap_key(DF_CMD_ENT, KC_LCTL, MOD_LGUI, KC_ENT, &cmd_ent_on_up, keycode, record)
    // capitalized keys must come second, since they might rely on a mod-tap shift
    || capitalized(PERIOD, 0, '.', 0, ':', shift_down, keycode, record)
    || capitalized(COMMA,  0, ',', 0, ';', shift_down, keycode, record)
    || capitalized(BANG,   0, '!', 0, '`', shift_down, keycode, record)
    || capitalized(QUES,   0, '?', 0, '~', shift_down, keycode, record)
    || capitalized(LPAREN, 0, '(', 0, '[', shift_down, keycode, record)
    || capitalized(RPAREN, 0, ')', 0, ']', shift_down, keycode, record)
    || capitalized(LANGLE, 0, '<', 0, '{', shift_down, keycode, record)
    || capitalized(RANGLE, 0, '>', 0, '}', shift_down, keycode, record)
    || capitalized(ATSIGN, 0, '@', 0, '#', shift_down, keycode, record)
    || capitalized(CARET,  0, '^', 0, '&', shift_down, keycode, record)
    || capitalized(DOLLAR, 0, '$', 0, '*', shift_down, keycode, record)
    || capitalized(SLASH,  0, '/', MOD_LALT | MOD_LSFT, '_', shift_down, keycode, record)
    || capitalized(PERCENT, 0, '%', 0, '%', shift_down, keycode, record)
    ;

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
