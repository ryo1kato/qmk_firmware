#include "arkag.h"

// Start: Written by konstantin: vomindoraan
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void send_unicode_hex_string(const char *str) {
  if (!str) { return; } // Saftey net
  while (*str) {
          // Find the next code point (token) in the string
          for (; *str == ' '; str++);
          size_t n = strcspn(str, " "); // Length of the current token
          char code_point[n+1];
          strncpy(code_point, str, n);
          code_point[n] = '\0'; // Make sure it's null-terminated

          // Normalize the code point: make all hex digits lowercase
          for (char *p = code_point; *p; p++) {
                  *p = tolower(*p);
          }

          // Send the code point as a Unicode input string
          unicode_input_start();
          send_string(code_point);
          unicode_input_finish();
          str += n; // Move to the first ' ' (or '\0') after the current token
  }
}
// End: Written by konstantin: vomindoraan

uint8_t       current_os, mod_primary_mask, fade_delay;
uint16_t      flash_timer_one, flash_timer_two,
              fade_timer_one, fade_timer_two,
              active_timer_one, active_timer_two,
              elapsed               = 0,
              num_extra_flashes_off = 0;
Color         underglow,
              flash_color,
              saved_color,
              hsv_none      = {0,0,0},
              hsv_white     = {0,0,127};
flashState    flash_state   = no_flash;
fadeState     fade_state    = add_fade;
activityState state         = boot;

void set_color (Color new, bool update) {
  rgblight_sethsv_eeprom_helper(new.h, new.s, new.v, update);
}

void save_color(Color to_save) {
  saved_color = to_save;
}

void reset_color(void) {
  underglow = saved_color;
}

Color mod_color(Color current_color, bool should_add, uint8_t change_amount) {
  save_color(underglow);
  int addlim = 359 - change_amount;
  int sublim = change_amount;
  int leftovers;
  if (should_add) {
    if (current_color.h <= addlim) {
      current_color.h += change_amount;
    } else {
      leftovers = (359 + change_amount) % 359;
      current_color.h  = 0 + leftovers;
    }
  } else {
    if (current_color.h >= sublim) {
      current_color.h -= change_amount;
    } else {
      leftovers = change_amount - current_color.h;
      current_color.h  = 359 - leftovers;
    }
  }
  return current_color;
}

void reverse_fade (void) {
  if (fade_state == add_fade){
    fade_state = sub_fade;
  } else {
    fade_state = add_fade;
  }
}

void check_state (void) {
        static bool activated, deactivated, slept;
        switch (state) {
        case active:
          if (!activated) {
            fade_delay = LED_FADE_DELAY;
            reverse_fade();
            activated = true;
            deactivated = false;
          }
          active_timer_two = timer_read();
          elapsed = active_timer_two - active_timer_one;
          if (elapsed < INACTIVE_DELAY) {return;}
          state = inactive;
          return;

        case inactive:
          if (!deactivated) {
            fade_delay = LED_FADE_DELAY * 2;
            reverse_fade();
            deactivated = true;
            slept = false;
            activated = false;
          }
          active_timer_two = timer_read();
          elapsed = active_timer_two - active_timer_one;
          if (elapsed < SLEEP_DELAY) {return;}
          state = sleeping;
          return;

        case sleeping:
          if (!slept) {
            fade_delay = LED_FADE_DELAY * 6;
            reverse_fade();
            slept = true;
            deactivated = false;
            activated = false;
          }
          return;

        case boot:
          return;
        }
}

void fade_rgb (void) {
  static bool ran_once;
  if (flash_state != no_flash) {return;}
  if (state == boot) {return;}
  switch (fade_state) {
  case add_fade:
    if (!ran_once) {
      fade_timer_one = timer_read();
      ran_once = true;
    }
    fade_timer_two = timer_read();
    elapsed = fade_timer_two - fade_timer_one;
    if (elapsed < fade_delay) {return;}
    if (underglow.h == 359) {
      fade_state = sub_fade;
      return;
    }
    underglow.h = underglow.h + 1;
    set_color(underglow, false);
    // set_color_at(underglow, 0);
    fade_timer_one = fade_timer_two;
    return;

  case sub_fade:
    fade_timer_two = timer_read();
    elapsed = fade_timer_two - fade_timer_one;
    if (elapsed < fade_delay) {return;}
    if (underglow.h == 0) {
      fade_state = add_fade;
      return;
    }
    underglow.h = underglow.h - 1;
    set_color(underglow, false);
    // set_color_at(underglow, 0);
    fade_timer_one = fade_timer_two;
    return;
  }
}

void flash_rgb (void) {
  static bool ran_once;
  switch(flash_state) {
  case no_flash:
    return;

  case flash_off:
    if (!ran_once) {
      set_color(hsv_none, false);
      flash_timer_one = timer_read();
      ran_once = true;
      flash_state = flash_on;
      return;
    }
    flash_timer_two = timer_read();
    elapsed = flash_timer_two - flash_timer_one;
    if (elapsed >= LED_FLASH_DELAY) {
      set_color(hsv_none, false);
      flash_timer_one = timer_read();
      flash_state = flash_on;
    }
    return;

  case flash_on:
    flash_timer_two = timer_read();
    elapsed = flash_timer_two - flash_timer_one;
    if (elapsed >= LED_FLASH_DELAY) {
      set_color(flash_color, false);
      flash_timer_one = timer_read();
      if (num_extra_flashes_off > 0) {
        flash_state = flash_off;
        num_extra_flashes_off--;
      } else {
        set_color(underglow, false);
        flash_state = no_flash;
        ran_once = false;
      }
    }
    return;
  }
}

void set_os (uint8_t os, bool update) {
  current_os = os;
  if (update) {
    eeprom_update_byte(EECONFIG_USERSPACE, current_os);
  }
  switch (os) {
  case OS_MAC:
    set_unicode_input_mode(UC_OSX);
    underglow = (Color){ 300, 255, 255 };
    mod_primary_mask = MOD_GUI_MASK;
    break;
  case OS_WIN:
    set_unicode_input_mode(UC_WINC);
    underglow = (Color){ 180, 255, 255 };
    mod_primary_mask = MOD_CTL_MASK;
    break;
  case OS_NIX:
    set_unicode_input_mode(UC_LNX);
    underglow = (Color){ 60, 255, 255 };
    mod_primary_mask = MOD_CTL_MASK;
    break;
  default:
    underglow = (Color){ 0, 0, 255 };
    mod_primary_mask = MOD_CTL_MASK;
  }
  set_color(underglow, update);
  flash_color = underglow;
  flash_state = flash_off;
  num_extra_flashes_off = 1;
}

void tap_key(uint8_t keycode) {
  register_code(keycode);
  unregister_code(keycode);
}

// register GUI if Mac or Ctrl if other
void pri_mod(bool press) {
  if (press) {
    if (current_os == OS_MAC) {
      register_code(KC_LGUI);
    } else {
      register_code(KC_LCTL);
    }
  } else {
    if (current_os == OS_MAC) {
      unregister_code(KC_LGUI);
    } else {
      unregister_code(KC_LCTL);
    }
  }
}

// register Ctrl if Mac or GUI if other
void sec_mod(bool press) {
  if (press) {
    if (current_os == OS_MAC) {
      register_code(KC_LCTL);
    } else {
      register_code(KC_LGUI);
    }
  } else {
    if (current_os == OS_MAC) {
      unregister_code(KC_LCTL);
    } else {
      unregister_code(KC_LGUI);
    }
  }
}

void surround_type(uint8_t num_of_chars, uint16_t keycode, bool use_shift) {
  if (use_shift) {
    register_code(KC_LSFT);
  }
  for (int i = 0; i < num_of_chars; i++) {
    tap_key(keycode);
  }
  if (use_shift) {
    unregister_code(KC_LSFT);
  }
  for (int i = 0; i < (num_of_chars/2); i++) {
    tap_key(KC_LEFT);
  }
}

void long_keystroke(size_t num_of_keys, uint16_t keys[]) {
  for (int i = 0; i < num_of_keys-1; i++) {
    register_code(keys[i]);
  }
  tap_key(keys[num_of_keys-1]);
  for (int i = 0; i < num_of_keys-1; i++) {
    unregister_code(keys[i]);
  }
}

void dance_grv (qk_tap_dance_state_t *state, void *user_data) {
  if (state->count == 1) {
    tap_key(KC_GRV);
  } else if (state->count == 2) {
    surround_type(2, KC_GRAVE, false);
  } else {
    surround_type(6, KC_GRAVE, false);
  }
}

void dance_quot (qk_tap_dance_state_t *state, void *user_data) {
  if (state->count == 1) {
    tap_key(KC_QUOT);
  } else if (state->count == 2) {
    surround_type(2, KC_QUOTE, false);
  } else if (state->count == 3) {
    surround_type(2, KC_QUOTE, true);
  }
}

void dance_strk (qk_tap_dance_state_t *state, void *user_data) {
  if (state->count == 1) {
    surround_type(4, KC_TILDE, true);
  } else if (state->count == 2) {
    if (current_os == OS_MAC) {
      long_keystroke(3, (uint16_t[]){KC_LGUI, KC_LSFT, KC_4});
    } else if (current_os == OS_WIN) {
      long_keystroke(3, (uint16_t[]){KC_LGUI, KC_LSFT, KC_S});
    } else {
      return;
    }
  }
}

void dance_3 (qk_tap_dance_state_t *state, void *user_data) {
  if (state->count == 1) {
    tap_key(KC_3);
  } else if (state->count == 2) {
    send_unicode_hex_string("00E8");
  } else if (state->count == 3) {
    send_unicode_hex_string("00E9");
  }
}

void dance_c (qk_tap_dance_state_t *state, void *user_data) {
  if (state->count == 1) {
    tap_key(KC_C);
  } else if (state->count == 2) {
    send_unicode_hex_string("00E7");
  }
}

void matrix_init_user(void) {
  current_os = eeprom_read_byte(EECONFIG_USERSPACE);
  set_os(current_os, false);
}

void matrix_scan_user(void) {
  check_state();
  flash_rgb();
  fade_rgb();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
  case M_PMOD:
    if (record->event.pressed) {
      pri_mod(true);
    } else {
      pri_mod(false);
    }
    return false;

  case M_SMOD:
    if (record->event.pressed) {
      sec_mod(true);
    } else {
      sec_mod(false);
    }
    return false;

  case M_P_B:
    if (record->event.pressed) {
      if (current_os == OS_WIN) {
              SEND_STRING(SS_DOWN(X_LGUI) SS_TAP(X_PAUSE) SS_UP(X_LGUI));
      } else {
      }
    }
    return false;

  case M_C_A_D:
    if (record->event.pressed) {
      if (current_os == OS_WIN) {
              SEND_STRING(SS_DOWN(X_LCTRL) SS_DOWN(X_LALT) SS_TAP(X_DELETE) SS_UP(X_LALT) SS_UP(X_LCTRL));
      } else {
      }
    }
    return false;

  case M_CALC:
    if (record->event.pressed) {
      if (current_os == OS_WIN) {
        SEND_STRING(SS_TAP(X_CALCULATOR));
      } else if (current_os == OS_MAC) {
        SEND_STRING(SS_DOWN(X_LGUI) SS_TAP(X_SPACE) SS_UP(X_LGUI) "calculator" SS_TAP(X_ENTER));
      }
    }
    return false;

  case M_OS:
    if (record->event.pressed) {
      set_os((current_os+1) % _OS_COUNT, true);
    }
    return false;

  case M_LOD:
    if (record->event.pressed) {
      send_unicode_hex_string("0CA0 005F 005F 0CA0");
    }
    return false;

  case M_LENNY:
    if (record->event.pressed) {
      send_unicode_hex_string("0028 0020 0361 00B0 0020 035C 0296 0020 0361 00B0 0029");
    }
    return false;


  case M_TF:
    if (record->event.pressed) {
      send_unicode_hex_string("0028 256F 2035 0414 2032 0029 256F 5F61 253B 2501 253B");
    }
    return false;

  case M_UF:
    if (record->event.pressed) {
      send_unicode_hex_string("252C 2500 252C 30CE 0028 0020 00BA 0020 005F 0020 00BA 0020 30CE 0029");
    }
    return false;

  case M_SHRUG:
    if (record->event.pressed) {
      send_unicode_hex_string("00AF 005C 005F 0028 30C4 0029 005F 002F 00AF");
    }
    return false;

  case M_TM:
    if (record->event.pressed) {
      send_unicode_hex_string("2122");
    }
    return false;

  case M_REPO:
    if (record->event.pressed) {
      SEND_STRING("https://github.com/arkag/qmk_firmware/tree/master/keyboards/mechmini/v2/keymaps/arkag");
    }
    return false;

  case M_GGT:
    if (record->event.pressed) {
      SEND_STRING("@GrahamGoldenTech.com");
    }
    return false;

  case M_SNIPT:
    if (record->event.pressed) {
      surround_type(6, KC_GRAVE, false);
      pri_mod(true);
      tap_key(KC_V);
      pri_mod(false);
      tap_key(KC_RGHT);
      tap_key(KC_RGHT);
      tap_key(KC_RGHT);
      tap_key(KC_ENTER);
    }
    return false;

  case M_BOLD:
    if (record->event.pressed) {
      surround_type(4, KC_8, true);
    }
    return false;

  case M_ITAL:
    if (record->event.pressed) {
      surround_type(2, KC_8, true);
    }
    return false;

  case M_ULIN:
    if (record->event.pressed) {
      surround_type(4, KC_MINS, true);
    }
    return false;

  case KC_LSFT:
    if (record->event.pressed) {
      set_color(mod_color(underglow, true, 50), false);
      SEND_STRING(SS_DOWN(X_LSHIFT));
    } else {
      set_color(underglow, false);
      SEND_STRING(SS_UP(X_LSHIFT));
    }
    return false;

  case MEDIA:
  case LAZY:
  case KEEB:
  case RAISE:
  case LOWER:
    return true;

  default:
    if (record->event.pressed) {
      active_timer_one = timer_read();
      state = active;
    }
    return true;
  }
}

uint32_t layer_state_set_user(uint32_t state) {
  switch (biton32(state)) {
  case _LAZY:
    save_color(underglow);
    underglow = mod_color(underglow, true, 50);
    break;
  case _MEDIA:
    save_color(underglow);
    underglow = mod_color(underglow, true, 150);
    break;
  case _KEEB:
    save_color(underglow);
    underglow = mod_color(underglow, false, 150);
    break;
  case _LOWER:
    save_color(underglow);
    underglow = mod_color(underglow, false, 100);
    break;
  case _RAISE:
    save_color(underglow);
    underglow = mod_color(underglow, true, 100);
    break;
  default:
    reset_color();
    break;
  }
  set_color(underglow, false);
  return state;
}

//Tap Dance Definitions
qk_tap_dance_action_t tap_dance_actions[] = {
  [TD_3_GRV_ACT]      = ACTION_TAP_DANCE_FN (dance_3),
  [TD_C_CED]          = ACTION_TAP_DANCE_FN (dance_c),
  [TD_GRV_3GRV]       = ACTION_TAP_DANCE_FN (dance_grv),
  [TD_SING_DOUB]      = ACTION_TAP_DANCE_FN (dance_quot),
  [TD_STRK_SHOT]      = ACTION_TAP_DANCE_FN (dance_strk),
  [TD_HYPH_UNDR]      = ACTION_TAP_DANCE_DOUBLE (KC_MINS, LSFT(KC_MINS)),
  [TD_BRCK_PARN_O]    = ACTION_TAP_DANCE_DOUBLE (KC_LBRC, LSFT(KC_9)),
  [TD_BRCK_PARN_C]    = ACTION_TAP_DANCE_DOUBLE (KC_RBRC, LSFT(KC_0)),
  [TD_LALT_RALT]      = ACTION_TAP_DANCE_DOUBLE (KC_LALT, KC_RALT),
};
