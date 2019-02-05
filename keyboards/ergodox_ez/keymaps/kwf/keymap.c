#include "ergodox_ez.h"
#include "debug.h"
#include "action_layer.h"
#include "version.h"


#include "keymap_german.h"

#include "keymap_nordic.h"



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
  PERIOD,  // '.' and ':'
  COMMA,   // ',' and ';'
  SLASH,   // '/' and (em-dash)
  ATSIGN,  // '@' and '#'
  CARET,   // '^' and '&'
  DOLLAR,  // '$' and '%'

};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [0] = LAYOUT_ergodox(
        KC_BSLASH, KC_1,      KC_2,      KC_3,      KC_4,      KC_5,      KC_NO,
        LPAREN,    KC_Q,      KC_W,      KC_E,      KC_R,      KC_T,      KC_EXLM,
        KC_ESCAPE, KC_A,      KC_S,      KC_D,      KC_F,      KC_G,
        LANGLE,    KC_Z,      KC_X,      KC_C,      KC_V,      KC_B,      PERIOD,
        KC_NO,     KC_NO,     KC_NO,     KC_EQUAL,  KC_QUOTE,

                                                               TT(1),     KC_LSHIFT,
                                                                          KC_LCTL,
                                                    KC_LSHIFT, KC_LGUI,   KC_LALT,

        KC_ENTER,  KC_6,      KC_7,      KC_8,      KC_9,      KC_0,      SLASH,
        KC_QUES,   KC_Y,      KC_U,      KC_I,      KC_O,      KC_P,      RPAREN,
                   KC_H,      KC_J,      KC_K,      KC_L,      ATSIGN,    KC_ENTER,
        COMMA,     KC_N,      KC_M,      CARET,     DOLLAR,    KC_UP,     RANGLE,
                              KC_MINUS,  KC_GRAVE,  KC_LEFT,   KC_DOWN,   KC_RIGHT,

        KC_LALT, TT(1),
        KC_LCTL,
        KC_TAB,  KC_BSPACE, KC_SPACE),

  [1] = LAYOUT_ergodox(
        KC_NO,     KC_1,    KC_2,    KC_3,      KC_MS_BTN2,   KC_MS_BTN1,   KC_MS_BTN3,
        KC_NO,     KC_4,    KC_5,    KC_6,      KC_NO,        KC_MS_ACCEL0, KC_MS_WH_DOWN,
        TO(0),     KC_7,    KC_8,    KC_9,      KC_NO,        KC_MS_ACCEL1,
        KC_NO,     KC_KP_0, KC_0,    KC_KP_DOT, KC_NO,        KC_MS_ACCEL2, KC_MS_WH_UP,
        RGB_TOG,   RGB_VAD, RGB_VAI, RGB_HUD,   RGB_HUI,

                                                               TO(0), KC_LSHIFT,
                                                                      KC_LCTL,
                                                  KC_LSHIFT, KC_LGUI, KC_LALT,

        KC_MS_BTN3,    KC_MS_BTN1,          KC_MS_BTN2,    KC_NO,          KC_MEDIA_PREV_TRACK,  KC_MEDIA_NEXT_TRACK, KC_MEDIA_PLAY_PAUSE,
        KC_MS_WH_DOWN, KC_MS_LEFT,          KC_MS_DOWN,    KC_MS_UP,       KC_MS_RIGHT,          KC_NO,               KC_AUDIO_MUTE,
                       KC_LEFT,             KC_DOWN,       KC_UP,          KC_RIGHT,             KC_NO,               KC_AUDIO_VOL_UP,
        KC_MS_WH_UP,   LALT(LSFT(KC_LEFT)), LALT(KC_LEFT), LALT(KC_RIGHT), LALT(LSFT(KC_RIGHT)), KC_NO,               KC_AUDIO_VOL_DOWN,
                                            KC_NO,         KC_NO,          KC_NO,                KC_NO,               KC_NO,

        KC_LALT, TO(0),
        KC_LCTL,
        KC_TAB, KC_BSPACE, KC_SPACE),

};

const uint16_t PROGMEM fn_actions[] = {
  [1] = ACTION_LAYER_TAP_TOGGLE(1)
};

// leaving this in place for compatibilty with old keymaps cloned and re-compiled.
const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt)
{
      switch(id) {
        case 0:
        if (record->event.pressed) {
          SEND_STRING (QMK_KEYBOARD "/" QMK_KEYMAP " @ " QMK_VERSION);
        }
        break;
      }
    return MACRO_NONE;
};

void matrix_init_user(void) {
#ifdef RGBLIGHT_COLOR_LAYER_0
  rgblight_setrgb(RGBLIGHT_COLOR_LAYER_0);
#endif
};

// keep track of modifier state
static bool shift_down = false;
static bool ctrl_down  = false;
static bool alt_down   = false;
static bool cmd_down   = false;

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
uint16_t keycode(char ascii_code) {
  return pgm_read_byte(&ascii_to_keycode_lut[(uint8_t)ascii_code]);
}

// Presses (or un-presses) the keycode corresponding to an ASCII character,
// including shift if necessary (to achieve the right effect).
// This function is a decomposition of the functionality of send_char (in the
// file quantum/quantum.c) to allow separate up/down presses, with the addition
// of behaving sensibly in the presence of shift being held down.
void press_char(bool press, char ascii_code) {
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
  const uint8_t key = keycode(ascii_code);
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
bool capitalized(char no, char yes, keyrecord_t *record) {
  // Press or unpress at the right shift level
  if (record->event.pressed) {
    press_char(true, (shift_down) ? yes : no);
  } else {
    // But if unpress, unpress both
    // This prevents a stuck-key bug that happened with the sequence
    // shift down, key down, shift up, key up
    press_char(false, yes);
    press_char(false, no);
  }
  return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  // track whether modifiers are down
  track_key(KC_LSHIFT, &shift_down, keycode, record);
  track_key(KC_LCTL,   &ctrl_down,  keycode, record);
  track_key(KC_LALT,   &alt_down,   keycode, record);
  track_key(KC_LGUI,   &cmd_down,   keycode, record);

  switch (keycode) {

    // Keys with custom capitalization
    case LPAREN:
      return capitalized('(', '[', record);
      break;
    case RPAREN:
      return capitalized(')', ']', record);
      break;
    case LANGLE:
      return capitalized('<', '{', record);
      break;
    case RANGLE:
      return capitalized('>', '}', record);
      break;
    case PERIOD:
      return capitalized('.', ':', record);
      break;
    case COMMA:
      return capitalized(',', ';', record);
      break;
    case ATSIGN:
      return capitalized('@', '#', record);
      break;
    case CARET:
      return capitalized('^', '&', record);
      break;
    case DOLLAR:
      return capitalized('$', '%', record);
      break;
    case SLASH:
      // Extra modifiers aren't (yet) supported by the 'capitalized' function,
      // which means it's tricky to get key-repeat for em-dash. We punt on this
      // and don't give it repeat behavior.
      if (record->event.pressed) {
        if (!shift_down) {
          SEND_STRING("/");
        } else {
          // em-dash
          SEND_STRING(SS_DOWN(X_LSHIFT)); // don't lift shift, let user do that
          SEND_STRING(SS_LALT("-"));
        }
      }
      return false;
      break;

    case EPRM:
      if (record->event.pressed) {
        eeconfig_init();
      }
      return false;
      break;
    case VRSN:
      if (record->event.pressed) {
        SEND_STRING (QMK_KEYBOARD "/" QMK_KEYMAP " @ " QMK_VERSION);
      }
      return false;
      break;
    case RGB_SLD:
      if (record->event.pressed) {
        rgblight_mode(1);
      }
      return false;
      break;

  }
  return true;
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
