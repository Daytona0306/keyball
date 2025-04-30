/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "pointing_device.h"
#include "lib/keyball/keyball.h"
#include <math.h>

#define QMK_SCROLL_SENSITIVITY 2.0f

static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

static void adjust_mouse_speed(keyball_motion_t *m) {
    int16_t movement_size = abs(m->x) + abs(m->y);
    float speed_multiplier = 1.0;
    if (movement_size > 60) {
        speed_multiplier = 3.0;
    } else if (movement_size > 30) {
        speed_multiplier = 1.5;
    } else if (movement_size > 5 ) {
        speed_multiplier = 1.0;
    } else if (movement_size > 4 ) {
        speed_multiplier = 0.9;
    } else if (movement_size > 3 ) {
        speed_multiplier = 0.7;
    } else if (movement_size > 2 ) {
        speed_multiplier = 0.5;
    } else if (movement_size > 1 ){
        speed_multiplier = 0.2;
    }
    m->x = clip2int8((int16_t)(m->x * speed_multiplier));
    m->y = clip2int8((int16_t)(m->y * speed_multiplier));
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_universal(
    KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                            KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     ,
    KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                            KC_H     , KC_J     , KC_K     , KC_L     , KC_MINS  ,
    KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                            KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    KC_LCTL  , KC_LGUI  , KC_LALT  ,LSFT_T(KC_LNG2),LT(1,KC_SPC),LT(3,KC_LNG1),KC_BSPC,LT(2,KC_ENT),LSFT_T(KC_LNG2),KC_RALT,KC_RGUI, KC_RSFT
  ),
  [1] = LAYOUT_universal(
    KC_F1    , KC_F2    , KC_F3    , KC_F4    , KC_RBRC  ,                            KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   ,
    KC_F5    , KC_EXLM  , S(KC_6)  ,S(KC_INT3), S(KC_8)  ,                           S(KC_INT1), KC_BTN1  , KC_PGUP  , KC_BTN2  , KC_SCLN  ,
    S(KC_EQL),S(KC_LBRC),S(KC_7)   , S(KC_2)  ,S(KC_RBRC),                            KC_LBRC  , KC_DLR   , KC_PGDN  , KC_BTN3  , KC_F11   ,
    KC_INT1  , KC_EQL   , S(KC_3)  , _______  , _______  , _______  ,      TO(2)    , TO(0)    , _______  , KC_RALT  , KC_RGUI  , KC_F12
  ),
  [2] = LAYOUT_universal(
    KC_TAB   , KC_7     , KC_8     , KC_9     , KC_MINS  ,                            KC_NUHS  , _______  , KC_BTN3  , _______  , KC_BSPC  ,
   S(KC_QUOT), KC_4     , KC_5     , KC_6     ,S(KC_SCLN),                            S(KC_9)  , KC_BTN1  , KC_UP    , KC_BTN2  , KC_QUOT  ,
    KC_SLSH  , KC_1     , KC_2     , KC_3     ,S(KC_MINS),                           S(KC_NUHS), KC_LEFT  , KC_DOWN  , KC_RGHT  , _______  ,
    KC_ESC   , KC_0     , KC_DOT   , KC_DEL   , KC_ENT   , KC_BSPC  ,      _______  , _______  , _______  , _______  , _______  , _______
  ),
  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  ,                            _______  , _______  , SSNP_HOR , SSNP_VRT , SSNP_FRE ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  , SCRL_DVI ,                            _______  , _______  , _______  , _______  , _______  ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , SCRL_DVD ,                            CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT  , KBC_RST  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    if (get_highest_layer(state) == 5) {
        keyball_set_scroll_mode(false);
    }
    return state;
}

void keyball_on_apply_motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    uint8_t layer = get_highest_layer(layer_state);
    int sensitivity_threshold = 5;

    if (layer == 5) {
        int16_t original_delta_x = m->x;
        int16_t original_delta_y = m->y;
        m->x = 0;
        m->y = 0;
        r->x = 0;
        r->y = 0;
        r->v = 0;

        if (abs(original_delta_x) > sensitivity_threshold || abs(original_delta_y) > sensitivity_threshold) {
            if (abs(original_delta_x) > abs(original_delta_y)) {
                 if (original_delta_x < -sensitivity_threshold) { tap_code(KC_UP); }
                else if (original_delta_x > sensitivity_threshold) { tap_code(KC_DOWN); }
            } else {
                 if (original_delta_y < -sensitivity_threshold) { tap_code(KC_LEFT); }
                else if (original_delta_y > sensitivity_threshold) { tap_code(KC_RIGHT); }
            }
        }
    } else {
        adjust_mouse_speed(m);
        #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
            r->x = clip2int8(m->y);
            r->y = clip2int8(m->x);
            if (is_left) {
                r->x = -r->x;
                r->y = -r->y;
            }
        #elif KEYBALL_MODEL == 46
            r->x = clip2int8(m->x);
            r->y = -clip2int8(m->y);
        #else
    #    error("unknown Keyball model")
        #endif
        m->x = 0;
        m->y = 0;
    }
}

void keyball_on_apply_motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    static uint32_t last_scroll_time = 0;
    uint32_t current_time = timer_read();

    float movement = sqrtf((float)m->x * m->x + (float)m->y * m->y);
    float accel_x = m->x;
    float accel_y = m->y;

    uint32_t delta_time = timer_elapsed(last_scroll_time);

    if (delta_time > 0) {
        float speed = movement / (float)delta_time;
        float base_sensitivity = QMK_SCROLL_SENSITIVITY / 2.5f;
        float acceleration_factor = 1.0f / (1.0f + expf(-0.5f * (speed - 8.0f)));
        float acceleration = 1.0f + 9.0f * acceleration_factor;
        acceleration *= base_sensitivity;

        accel_x = (float)m->x * acceleration;
        accel_y = (float)m->y * acceleration;

        if (abs(m->x) <= 1) accel_x = m->x;
        if (abs(m->y) <= 1) accel_y = m->y;
    }

    last_scroll_time = current_time;
    m->x = (int16_t)roundf(accel_x);
    m->y = (int16_t)roundf(accel_y);

    int16_t div = 1 << (keyball_get_scroll_div() - 1);
    int16_t x = divmod16(&m->x, div);
    int16_t y = divmod16(&m->y, div);

    #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
        r->h = clip2int8(y);
        r->v = -clip2int8(x);
        if (is_left) {
            r->h = -r->h;
            r->v = -r->v;
        }
    #elif KEYBALL_MODEL == 46
        r->h = clip2int8(x);
        r->v = clip2int8(y);
    #else
    #    error("unknown Keyball model")
    #endif
}

#ifdef OLED_ENABLE
#    include "lib/oledkit/oledkit.h"
void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
