#include QMK_KEYBOARD_H
#include "bootloader.h"
#ifdef PROTOCOL_LUFA
  #include "lufa.h"
  #include "split_util.h"
#endif
#ifdef SSD1306OLED
  #include "ssd1306.h"
#endif
#include "rev1.h"

extern keymap_config_t keymap_config;

#ifdef RGBLIGHT_ENABLE
//Following line allows macro to read current RGB settings
extern rgblight_config_t rgblight_config;
#endif

extern uint8_t is_master;

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
#define _QWERTY  0
#define _DIGIT   1
#define _SYMBOL  2
#define _VIM     3
#define _EMACS   4
#define _MOUSE   5

enum custom_keycodes {
  QWERTY = SAFE_RANGE
  ,DIGIT
  ,SYMBOL
  ,VIM
  ,EMACS
#ifdef MOUSEKEY_ENABLE
  ,MOUSE
#endif
};

enum macro_keycodes {
  KC_SAMPLEMACRO,
};

#define KC______  KC_TRNS
#define KC_XXXXX  KC_NO
#define KC_QWERTY QWERTY
#define KC_DIGIT  DIGIT
#define KC_SYMBOL SYMBOL
#define KC_VIM    VIM
#define KC_EMACS  EMACS

#define KC_RESET  RESET

// Layer
#define KC__MINS  LT(_DIGIT, KC_MINS)
#define KC__BSPC  LT(_SYMBOL, KC_BSPC)
#define KC__SCLN  LT(_VIM, KC_SCLN)
#define KC__TAB   LT(_EMACS, KC_TAB)
#ifdef MOUSEKEY_ENABLE
#  define KC__P   LT(_MOUSE, KC_P)
#else
#  define KC__P   KC_P
#endif

// Modifier
#define KC__LSFT  LSFT_T(KC_GRAVE)
#define KC__RSFT  RSFT_T(KC_BSLASH)
#define KC__Z     LSFT_T(KC_Z)
#define KC__SLSH  RSFT_T(KC_SLSH)
#define KC__EQL   RALT_T(KC_EQL)

// Shortcuts
#define KC_HEAD   LCMD(KC_HOME) // macOS start of line
#define KC_TAIL   LCMD(KC_END)  // macOS end of line
#define KC_SCSHOT SCMD(4)       // macOS ranged screen shot
#define KC_PREV   SCMD(KC_LBRC)
#define KC_NEXT   SCMD(KC_RBRC)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_QWERTY] = LAYOUT_kc( \
  //,-----------------------------------------.                ,-----------------------------------------.
       _TAB,     Q,     W,     E,     R,     T,                      Y,     U,     I,     O,    _P,  BSPC,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
       LCTL,     A,     S,     D,     F,     G,                      H,     J,     K,     L, _SCLN,  QUOT,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _LSFT,    _Z,     X,     C,     V,     B,                      N,     M,  COMM,   DOT, _SLSH, _RSFT,\
  //|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
                                   LCMD,   SPC, _BSPC,      ENT, _MINS,  _EQL \
                              //`--------------------'  `--------------------'
  ),

  [_DIGIT] = LAYOUT_kc( \
  //,-----------------------------------------.                ,-----------------------------------------.
        ESC,     1,     2,     3,     4,     5,                      7,     8,     9,     0,  MINS,  BSPC,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____,    F1,    F2,    F3,    F4,     6,                   BSPC,  MINS,  LBRC,  RBRC,  COLN, _____,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX,                   PLUS,   ENT,  COMM,   DOT, _____, _____,\
  //|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
                                  _____, _____,  LSFT,    _____, _____, _____ \
                              //`--------------------'  `--------------------'
  ),

  [_SYMBOL] = LAYOUT_kc( \
  //,-----------------------------------------.                ,-----------------------------------------.
        GRV,  EXLM,    AT,  HASH,   DLR,  PERC,                   AMPR,  ASTR,  LPRN,  RPRN,  MINS,  BSPC,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
       TILD, XXXXX, XXXXX, XXXXX, XXXXX,  CIRC,                   BSPC, XXXXX,  LBRC,  RBRC,  PLUS, XXXXX,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, XXXXX, XXXXX, XXXXX, XXXXX,   ENT,                  XXXXX,   ENT,  LBRC,  RBRC, _____, _____,\
  //|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
                                  _____, _____, _____,    _____, _____, _____ \
                              //`--------------------'  `--------------------'
  ),

  [_VIM] = LAYOUT_kc( \
  //,-----------------------------------------.                ,-----------------------------------------.
      _____, _____, _____, _____, RESET, _____,                  _____, _____, _____, _____, _____, _____,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, _____, _____, _____, _____, _____,                   LEFT,  DOWN,    UP, RIGHT, _____, _____,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, _____, _____, _____, _____, _____,                  _____, _____, _____, _____, _____, _____,\
  //|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
                                  _____, _____, _____,    _____, _____, _____ \
                              //`--------------------'  `--------------------'
  ),

  [_EMACS] = LAYOUT_kc( \
  //,-----------------------------------------.                ,-----------------------------------------.
      _____, XXXXX, XXXXX,  TAIL,SCSHOT, XXXXX,                  XXXXX, XXXXX, XXXXX, XXXXX,    UP, _____,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____,  HEAD, XXXXX,   DEL, RIGHT, XXXXX,                   BSPC, XXXXX, XXXXX, XXXXX, XXXXX, XXXXX,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, XXXXX, XXXXX, XXXXX,  PGDN,  LEFT,                   DOWN,   ENT,  PREV,  NEXT, XXXXX, XXXXX,\
  //|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
                                  _____, _____, _____,    _____, _____, _____ \
                              //`--------------------'  `--------------------'
  ),

  [_MOUSE] = LAYOUT_kc( \
  //,-----------------------------------------.                ,-----------------------------------------.
      _____, _____, _____,  WH_U,  ACL0,  ACL1,                   MS_U,  MS_D,  MS_U,  MS_R, _____, _____,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, _____,  WH_L,  WH_D,  WH_R,  ACL2,                   BTN1,  BTN2,  BTN3, _____, _____, _____,\
  //|------+------+------+------+------+------|                |------+------+------+------+------+------|
      _____, _____, _____, _____, _____, _____,                  _____, _____, _____, _____, _____, _____,\
  //|------+------+------+------+------+------+------|  |------+------+------+------+------+------+------|
                                  _____, _____, _____,    _____, _____, _____ \
                              //`--------------------'  `--------------------'
  ),
};

int RGB_current_mode;

void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
}

// Setting ADJUST layer RGB back to default
void update_tri_layer_RGB(uint8_t layer1, uint8_t layer2, uint8_t layer3) {
  if (IS_LAYER_ON(layer1) && IS_LAYER_ON(layer2)) {
    layer_on(layer3);
  } else {
    layer_off(layer3);
  }
}

void matrix_init_user(void) {
    #ifdef RGBLIGHT_ENABLE
      RGB_current_mode = rgblight_config.mode;
    #endif
    //SSD1306 OLED init, make sure to add #define SSD1306OLED in config.h
    #ifdef SSD1306OLED
        iota_gfx_init(!has_usb());   // turns on the display
    #endif
}

//SSD1306 OLED update loop, make sure to add #define SSD1306OLED in config.h
#ifdef SSD1306OLED

// When add source files to SRC in rules.mk, you can use functions.
const char *read_layer_state(void);
const char *read_logo(void);
void set_keylog(uint16_t keycode, keyrecord_t *record);
const char *read_keylog(void);
const char *read_keylogs(void);

// const char *read_mode_icon(bool swap);
// const char *read_host_led_state(void);
// void set_timelog(void);
// const char *read_timelog(void);

void matrix_scan_user(void) {
   iota_gfx_task();
}

void matrix_render_user(struct CharacterMatrix *matrix) {
  if (is_master) {
    // If you want to change the display of OLED, you need to change here
    matrix_write_ln(matrix, read_layer_state());
    matrix_write_ln(matrix, read_keylog());
    matrix_write_ln(matrix, read_keylogs());
    //matrix_write_ln(matrix, read_mode_icon(keymap_config.swap_lalt_lgui));
    //matrix_write_ln(matrix, read_host_led_state());
    //matrix_write_ln(matrix, read_timelog());
  } else {
    matrix_write(matrix, read_logo());
  }
}

void matrix_update(struct CharacterMatrix *dest, const struct CharacterMatrix *source) {
  if (memcmp(dest->display, source->display, sizeof(dest->display))) {
    memcpy(dest->display, source->display, sizeof(dest->display));
    dest->dirty = true;
  }
}

void iota_gfx_task_user(void) {
  struct CharacterMatrix matrix;
  matrix_clear(&matrix);
  matrix_render_user(&matrix);
  matrix_update(&display, &matrix);
}

define case_PROCESS_LT_break(layer) \
    case layer:                     \
      if (record->event.pressed) {  \
        layer_on(_ ## layer);       \
      } else {                      \
        layer_off(_ ## layer);      \
      }                             \
      return false;                 \
      break;


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    set_keylog(keycode, record);
    // set_timelog();
  }

  switch (keycode) {
    case QWERTY:
      if (record->event.pressed) {
        persistent_default_layer_set(1UL<<_QWERTY);
      }
      return false;
      break;
    case_PROCESS_LT_break(DIGIT)
    case_PROCESS_LT_break(SYMBOL)
    case_PROCESS_LT_break(VIM)
    case_PROCESS_LT_break(EMACS)
    case_PROCESS_LT_break(MOUSE)
    case RGB_MOD:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          rgblight_mode(RGB_current_mode);
          rgblight_step();
          RGB_current_mode = rgblight_config.mode;
        }
      #endif
      return false;
      break;
    case RGBRST:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          eeconfig_update_rgblight_default();
          rgblight_enable();
          RGB_current_mode = rgblight_config.mode;
        }
      #endif
      break;
  }
  return true;
}

#endif
