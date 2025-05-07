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

// 関数プロトタイプの宣言 (Forward Declarations)
// これらの関数が後で定義されていることをコンパイラに知らせます。
static int16_t divmod16(int16_t *v, int16_t div);
static inline int8_t clip2int8(int16_t v);

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

    // Layer 5 (カーソルキーモード) の調整パラメータ
    // 連続での移動を不要とし、1回の動きで1ステップ移動させるための調整
    // ★★★ 最も重要な調整パラメータ ★★★
    int layer5_step_threshold = 25; // 1回のタップ（1ステップ）をトリガーするために必要なトラックボール移動量の「単位」。
                                   // この値を大きくすると、1タップに必要な物理的な移動量が大きくなり、鈍感になります。小さくすると敏感になります。
                                   // 例えば、10 なら 10 単位の移動で 1タップ、 30 なら 30 単位の移動で 1タップ発生。

    // Layer 5 のその他のパラメータ (max_taps_per_event, tap_delay_ms) は、
    // この新しいロジックではほとんど意味を持たなくなるため削除または無視できます。
    // int layer5_max_taps_per_event = 1; // 不要になる
    // int layer5_tap_delay_ms = 0;     // 不要になる

    if (layer == 5) {
        // Layer 5 では adjust_mouse_speed を使用しない
        // adjust_mouse_speed(m); // この行は削除またはコメントアウトされたまま

        // m->x, m->y には前回のレポートからの蓄積された移動量が入っている。
        // divmod16 を使用して、蓄積量から「ステップ数」を計算し、剰余を m->x/y に残す。
        int steps_x = divmod16(&m->x, layer5_step_threshold);
        int steps_y = divmod16(&m->y, layer5_step_threshold);

        // マウスレポート r の移動量はゼロにする（キー入力で制御するため）
        r->x = 0;
        r->y = 0;
        r->v = 0; // スクロールもゼロにしておく

        // どちらかの軸でステップが発生したかチェック
        if (steps_x != 0 || steps_y != 0) {
            uint16_t keycode_to_send = KC_NO;
            // int triggering_steps; // 使用しない

            // どちらの軸がステップをトリガーしたか（またはより多くステップを生成したか）を判断
            // Layer 5 の物理方向とキーコードのマッピングは既存ロジックを踏襲
            if (abs(steps_x) >= abs(steps_y) && steps_x != 0) {
                // X軸でステップが発生し、かつY軸より多いかY軸ではステップが発生していない場合
                // triggering_steps = steps_x; // 使用しない
                keycode_to_send = (steps_x < 0) ? KC_UP : KC_DOWN; // 右手: X-で上, X+で下
                if (is_left) { keycode_to_send = (steps_x < 0) ? KC_DOWN : KC_UP; } // 左手は反転
            } else if (steps_y != 0) {
                // Y軸でステップが発生し、かつX軸より多いかX軸ではステップが発生していない場合
                // triggering_steps = steps_y; // 使用しない
                keycode_to_send = (steps_y < 0) ? KC_LEFT : KC_RIGHT; // 右手: Y-で左, Y+で右
                if (is_left) { keycode_to_send = (steps_y < 0) ? KC_RIGHT : KC_LEFT; } // 左手は反転
            }

            // ステップがトリガーされた（keycode_to_send が設定された）場合、1回タップを送信
            if (keycode_to_send != KC_NO) {
                tap_code(keycode_to_send);
                // ここで wait_ms(layer5_tap_delay_ms) を入れても良いが、
                // 通常は1回のトリガーで1タップなので不要。連続タップさせたい場合に意味を持つ。
            }
        }
        // divmod16 によって m->x, m->y には「次のステップになりきれなかった」分の移動量が残っている。
        // これは自動的に次のレポートで加算される。

    } else {
        // Layer 5 以外のマウス移動処理（adjust_mouse_speed を使用する既存ロジック）
        adjust_mouse_speed(m); // この行は残す
        #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
            r->x = clip2int8(m->y);
            r->y = clip2int8(m->x);
            if (is_left) { r->x = -r->x; r->y = -r->y; }
        #endif
        m->x = 0; // Layer 5 以外では m をクリア（divmod16 による蓄積を使用しないため）
        m->y = 0;
    }
}

// divmod16 と clip2int8 の関数定義
static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
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
        // シグモイド関数を使って、速度を 0 から 1 へマッピングします。
        // 低速 -> 0 に近い値
        // 高速 -> 1 に近い値

        // シグモイド関数の項 (速度に対して 0 から 1 へ変化, speed = 80.0 で 0.5)
        // 傾き -0.05f, 中心 80.0f
        float sigmoid_term = 1.0f / (1.0f + expf(-0.05f * (speed - 1.2f)));

        // sigmoid_term (0..1) を 除数インデックスの範囲 (1..7) にマッピング
        // sigmoid_term 0 -> 7, 0.5 -> base_n, 1 -> 1 となるように線形補間
        float dynamic_n_float;
        if (sigmoid_term >= 0.5f) { // 速度が中心 (80.0) 以上の場合 (高速側)
             // sigmoid_term 0.5 から 1 の範囲を base_n から 1 に線形補間
             dynamic_n_float = (float)base_n + ((float)1.0f - (float)base_n) * (sigmoid_term - 0.5f) * 2.0f;
        } else { // 速度が中心 (80.0) 未満の場合 (低速側)
            // sigmoid_term 0 から 0.5 の範囲を 10 から base_n に線形補間
            dynamic_n_float = (float)10.0f + ((float)base_n - (float)10.0f) * (sigmoid_term * 2.0f);
        }

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

    // === スクロールスナップモードの適用 ===
    // keyball_scrollsnap_mode_t は lib/keyball/keyball.h で宣言されている型です
    keyball_scrollsnap_mode_t snap_mode = keyball_get_scrollsnap_mode();

    if (snap_mode == KEYBALL_SCROLLSNAP_MODE_VERTICAL) {
        // 垂直スナップモードの場合、水平方向のスクロール量 (x) をゼロにする
        x = 0;
    } else if (snap_mode == KEYBALL_SCROLLSNAP_MODE_HORIZONTAL) {
        // 水平スナップモードの場合、垂直方向のスクロール量 (y) をゼロにする
        y = 0;
    }
    // KEYBALL_SCROLLSNAP_MODE_FREE の場合、x と y はそのまま


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
