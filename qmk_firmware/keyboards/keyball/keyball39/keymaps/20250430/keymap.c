/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "pointing_device.h"
#include "lib/keyball/keyball.h"
#include <math.h> // expf, sqrtf, roundf に必要

// QMK_SCROLL_SENSITIVITY はこのアプローチでは直接使用しませんが、
// 必要に応じて速度から除数へのマッピング関数内で感度調整に使うことも可能です。
// 今回は使用しないため定義を削除します。


static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

// adjust_mouse_speed 関数を再度定義し、含めます。
static void adjust_mouse_speed(keyball_motion_t *m) {
    int16_t movement_size = abs(m->x) + abs(m->y);

    float speed_multiplier = 1.0; // 基本速度
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
    int sensitivity_threshold = 2;

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
        // Layer 5 以外のマウス移動処理
        // adjust_mouse_speed 関数による速度調整を再度呼び出します。
        adjust_mouse_speed(m);


        // Keyball 39/61/147/44 のデフォルト軸マッピング (YをXに、XをYに) と反転
        #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
            r->x = clip2int8(m->y);
            r->y = clip2int8(m->x);
            if (is_left) {
                r->x = -r->x;
                r->y = -r->y;
            }
        #endif

        m->x = 0;
        m->y = 0;
    }
}

void keyball_on_apply_motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
   static uint32_t last_scroll_time = 0;
   uint32_t current_time = timer_read();

   // トラックボールの移動速度を計算 (magnitude per unit time)
   float movement = sqrtf((float)m->x * m->x + (float)m->y * m->y);

   uint32_t delta_time = timer_elapsed(last_scroll_time);

   // デフォルトのスクロール除数 (keyball_get_scroll_div() は 1-7 のインデックスを返す)
   // 実際の除数は 2^(index-1)
   int base_n = keyball_get_scroll_div(); // 1..7
   int dynamic_div = 1 << (base_n - 1); // デフォルトの除数 (1, 2, 4, 8, 16, 32, 64)

   if (delta_time > 0) {
       float speed = movement / (float)delta_time; // 速度 (単位時間あたりの移動量)

       // === 速度 (speed) に応じて動的な除数インデックス (dynamic_n) を計算 ===
       // シグモイド関数を使って、速度を 1..7 の除数インデックスにマッピングします。
       // 低速 -> 高いインデックス (7) -> 大きい除数 -> 遅いスクロール
       // 高速 -> 低いインデックス (1) -> 小さい除数 -> 速いスクロール

       // シグモイド関数の項 (速度に対して 0 から 1 へ変化, speed = 8.0 で 0.5)
       float sigmoid_term = 1.0f / (1.0f + expf(-0.5f * (speed - 20.0f)));

       // sigmoid_term (0..1) を 除数インデックスの範囲 (1..7) にマッピング
       // 高速側 (sigmoid_term が 1 に近い) を インデックス 1 に、
       // 低速側 (sigmoid_term が 0 に近い) を インデックス 7 にマッピングするため反転させます。
       // 例: base_n=4 (divisor 8) の場合、低速時は除数を大きく (index > 4)、高速時は除数を小さく (index < 4) する
       // マッピング式: base_n から index 7 までの範囲と base_n から index 1 までの範囲を sigmoid_term で補間
       float dynamic_n_float;
       if (sigmoid_term >= 0.5f) { // 速度が中心 (8.0) 以上の場合 (高速側)
            // sigmoid_term 0.5 -> base_n, 1 -> 1 に線形補間
            dynamic_n_float = (float)base_n + ((float)1.0f - (float)base_n) * (sigmoid_term - 0.5f) * 2.0f;
       } else { // 速度が中心 (8.0) 未満の場合 (低速側)
           // sigmoid_term 0 -> 7, 0.5 -> base_n に線形補間
           dynamic_n_float = (float)7.0f + ((float)base_n - (float)7.0f) * (sigmoid_term * 2.0f);
       }


       // シンプルに 1..7 の範囲にマッピングする場合はこちらを使います（調整が必要）
       // float dynamic_n_float = 7.0f - sigmoid_term * 6.0f; // sigmoid_term 0->7, 1->1


       // 計算された浮動小数点数のインデックスを整数に丸め、1から7の範囲にクランプ
       int dynamic_n = (int)roundf(dynamic_n_float);
       dynamic_n = MAX(1, MIN(10, dynamic_n));

       // 動的な除数を計算
       dynamic_div = 1 << (dynamic_n - 1);

       // デバッグ用: 計算されたインデックスと除数を一時的に表示するなど検討
       // #ifdef CONSOLE_ENABLE
       // print("Speed: "); pfloat(speed, 2); print(" sigmoid: "); pfloat(sigmoid_term, 2);
       // print(" n_float: "); pfloat(dynamic_n_float, 2); print(" n: "); print_int(dynamic_n);
       // print(" div: "); print_int(dynamic_div); print("\n");
       // #endif
   }

   last_scroll_time = current_time;

   // 動的に計算された除数を使用して divmod16 を適用
   // divmod16 は m->x/y を変更して残余を保持します
   int16_t x = divmod16(&m->x, dynamic_div);
   int16_t y = divmod16(&m->y, dynamic_div);

   // apply to mouse report.
   // r->h and r->v get the scroll amount
   #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
       r->h = clip2int8(y); // Horizontal scroll from Y movement (Keyball default mapping)
       r->v = -clip2int8(x); // Vertical scroll from X movement (Keyball default mapping)
       if (is_left) {
           r->h = -r->h;
           r->v = -r->v;
       }
   #endif

   // Keyballの既存ロジックと divmod16 が残余を扱います。

   // Function returns, Keyball's calling code uses r to send the report.
}

#ifdef OLED_ENABLE
#    include "lib/oledkit/oledkit.h"
void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
