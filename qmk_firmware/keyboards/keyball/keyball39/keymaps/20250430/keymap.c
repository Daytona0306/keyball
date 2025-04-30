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
// Keyball ライブラリのヘッダーをインクルード
// Keyball の lib ディレクトリからのパスを指定
#include "lib/keyball/keyball.h"

// Keyball デフォルト処理で使用されている静的ヘルパー関数をコピー
// これらは keyball.c から持ってきています
static int16_t add16(int16_t a, int16_t b) {
    int16_t r = a + b;
    if (a >= 0 && b >= 0 && r < 0) {
        r = 32767;
    } else if (a < 0 && b < 0 && r >= 0) {
        r = -32768;
    }
    return r;
}

static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

// Keyball デフォルト処理で使用されている速度調整関数をコピー
// これも keyball.c から持ってきています
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
  // Layer 0: Base Layer (from PDF Cheat Sheet)
  [0] = LAYOUT_universal(
    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,
    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                        KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN,
    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,
    KC_LCTL, KC_LGUI, KC_LALT, MO(4),   LT(2,KC_SPC),LT(3,KC_LNG1),  KC_BSPC, LT(2,KC_ENT),MO(5), KC_RALT, KC_RGUI, KC_RSFT
  ),

  // Layer 1: Shift Layer (from PDF Cheat Sheet) - Assumed to be Shift + Base Layer
  [1] = LAYOUT_universal(
    S(KC_Q), S(KC_W), S(KC_E), S(KC_R), S(KC_T),                     S(KC_Y), S(KC_U), S(KC_I), S(KC_O), S(KC_P),
    S(KC_A), S(KC_S), S(KC_D), S(KC_F), S(KC_G),                     S(KC_H), S(KC_J), S(KC_K), S(KC_L), KC_QUOT, // PDF shows ' for KC_QUOT
    S(KC_Z), S(KC_X), S(KC_C), S(KC_V), S(KC_B),                     S(KC_N), S(KC_M), S(KC_COMM),S(KC_DOT),S(KC_SLSH),
    KC_LCTL, KC_LGUI, KC_LALT, _______, _______, _______,           _______, _______, _______, KC_RALT, KC_RGUI, KC_RSFT
  ),

  // Layer 2: Number/Symbol/Mouse Layer (from PDF Cheat Sheet - Image 5 interpretation)
  [2] = LAYOUT_universal(
    S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5),                     KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_SLASH, M(3),   // Image 5 R0: S(1)-S(5) | KP7-KP/, Macro 4
    S(KC_6), S(KC_7), S(KC_8), S(KC_9), S(KC_0),                     KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_ASTERISK,M(4),   // Image 5 R1: S(6)-S(0) | KP4-KP*, Macro 5
    S(KC_MINS),S(KC_EQUAL),S(KC_BSLS), M(1),    M(0),                KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_MINUS, M(2),   // Image 5 R2: S(-)=,~,\, M1,M0 | KP1-KP-, Macro 3
    KC_CAPS, LCTL(KC_6),LCTL(KC_2), KC_LSFT, KC_LALT, KC_TAB,        KC_LANG1,S(KC_LANG2),KC_DEL,  KC_RALT, KC_RGUI, KC_RSFT // Image 5 R3: Mods, Lang, Del, Right Mods
  ),

  // Layer 3: Function/RGB/CPI/Scroll Div Layer (Prioritizing user's previous functional L3 and PDF text over potentially inconsistent images 6/7)
  [3] = LAYOUT_universal(
    RGB_TOG  , _______  , _______  , _______  ,  _______  ,                           RGB_M_P  , RGB_M_B  , RGB_M_R  , RGB_M_SW , RGB_M_SN ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  ,  SCRL_DVI ,                           RGB_M_K  , RGB_M_X  , RGB_M_G  , RGB_M_T  , RGB_M_TW ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  ,  SCRL_DVD ,                           CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT    , KBC_RST  , _______  , _______  ,  _______  , _______  ,     _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),

  // Layer 4: Num Pad / Audio / Function Keys (from PDF Cheat Sheet - Image 8 & 9 interpretation)
  [4] = LAYOUT_universal(
    KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,                       TO(0),   KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_SLASH,
    LCTL(KC_A),KC_F5,   KC_F6,   KC_F7,   KC_F8,                       KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_ASTERISK,_______,
    KC_LSFT, KC_F9,   KC_F10,  KC_F11,  KC_F12,                      KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_MINUS, _______,
    TO(4),   _______, _______, _______, _______, _______,           KC_KP_0, KC_KP_DOT, KC_KP_PLUS,KC_KP_ENTER,KC_DEL,  _______
  ),

  // Layer 5: Trackball Arrow Keys / Navigation / Edit (from PDF Cheat Sheet - Image 10 interpretation)
  [5] = LAYOUT_universal( // レイヤー5 (トラックボール矢印キー)
    KC_INS,  KC_HOME, KC_UP,   KC_END,  KC_PGUP,                     _______, _______, _______, _______, _______,
    KC_LEFT, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, // Trackball movement handled by code override
    KC_DEL,  KC_END,  KC_DOWN, KC_PGDN, _______,                     _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,           _______, _______, _______, _______, _______, _______ // Bottom row seems unassigned in PDF L5
  ),
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is 3
    // この行はKeyball独自のスクロールモードに必要とのことなので、そのままにします。
    keyball_set_scroll_mode(get_highest_layer(state) == 3);

    // (OLED表示など、他のレイヤー切り替え時の処理が必要であればここに追加)

    return state;
}

// Keyball の keyball_on_apply_motion_to_mouse_move 弱い関数をオーバーライド
// レイヤー5での矢印キー変換と、その他のレイヤーでのデフォルト移動処理を実装
void keyball_on_apply_motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    // 現在のレイヤーを取得します。
    uint8_t layer = get_highest_layer(layer_state);

    // トラックボールの移動量のしきい値を設定します。
    // 実機でのテストに基づいて調整してください。
    int sensitivity_threshold = 5; // ZMKの 'tick' に相当する概念。調整が必要。

    // レイヤー5の場合のみトラックボールの移動を処理します。(矢印キー)
    if (layer == 5) {
        // Custom logic for Layer 5 (Arrows)

        // 使用する元の移動量を一時的に保存
        int16_t original_delta_x = m->x;
        int16_t original_delta_y = m->y;

        // トラックボールの蓄積された移動量 m をゼロにする
        // これにより、Keyball のデフォルトの処理がこの移動を使わないようにします。
        m->x = 0;
        m->y = 0;

        // マウスレポート r の移動量もゼロにする（念のため）
        r->x = 0;
        r->y = 0;
        r->v = 0; // スクロールもゼロにしておく

        // しきい値を超えた移動量があるかチェックし、矢印キーをタップ
        if (abs(original_delta_x) > sensitivity_threshold || abs(original_delta_y) > sensitivity_threshold) {
            // レイヤー5での矢印キーマッピング (ユーザーの観測に合わせた軸入れ替え)
            // 元のX移動量(horizontal)をVertical Arrow Keysに、元のY移動量(vertical)をHorizontal Arrow Keysにマップします。
            if (abs(original_delta_x) > abs(original_delta_y)) { // 支配的な元のX移動 (Physical Left/Right)
                // これをVertical Arrow Keys (UP/DOWN) にマップ
                 if (original_delta_x < -sensitivity_threshold) { tap_code(KC_UP); } // 元X- (Left) -> KC_UP
                else if (original_delta_x > sensitivity_threshold) { tap_code(KC_DOWN); } // 元X+ (Right) -> KC_DOWN

            } else { // 支配的な元のY移動 (Physical Up/Down)
                // これをHorizontal Arrow Keys (LEFT/RIGHT) にマップ
                 if (original_delta_y < -sensitivity_threshold) { tap_code(KC_LEFT); } // 元Y- (Up) -> KC_LEFT
                else if (original_delta_y > sensitivity_threshold) { tap_code(KC_RIGHT); } // 元Y+ (Down) -> KC_RIGHT
            }
        }
    } else {
        // レイヤーが5以外の場合、Keyball のデフォルトの移動処理を実行します。
        // このコードは keyball.c の keyball_on_apply_motion_to_mouse_move のデフォルト実装からコピーしています。
        // これにより、Layer 5 以外のレイヤーで通常のマウス移動が機能するようになります。

        // adjust_mouse_speed の呼び出しを追加
        adjust_mouse_speed(m);

        #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
            // Keyball 39/61/147/44 のデフォルト軸マッピング (YをXに、XをYに) と反転
            r->x = clip2int8(m->y);
            r->y = clip2int8(m->x);
            if (is_left) {
                r->x = -r->x;
                r->y = -r->y;
            }
        #elif KEYBALL_MODEL == 46
            // Keyball 46 のデフォルト軸マッピング
            r->x = clip2int8(m->x);
            r->y = -clip2int8(m->y);
        #else
            // 未知のKeyballモデルの場合のフォールバック
            r->x = clip2int8(m->x);
            r->y = clip2int8(m->y);
        #endif

        // デフォルト処理で移動量を使用した後にクリア
        m->x = 0;
        m->y = 0;
    }
    // 関数から戻ると、Keyball の calling code (pointing_device_driver_get_report) が
    // r の内容を使ってレポートを送信します。
}

// Keyball の keyball_on_apply_motion_to_mouse_scroll 弱い関数をオーバーライド
// スクロールに加速度を適用
void keyball_on_apply_motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    // 現在のレイヤーを取得します。（厳密にはスクロールモード有効時のみ呼ばれる想定ですが安全のため）
    uint8_t layer = get_highest_layer(layer_state); // この変数は現在使用されていませんが、今後使う可能性を考慮し残しています

    // レイヤー3（スクロールモード）の場合のみ処理を行います。
    // Layer 3 で keyball.scroll_mode が true に設定されている前提です。
    // この関数は Keyball 内部で keyball.scroll_mode が true の場合に呼ばれます。

    // スクロールにも adjust_mouse_speed を適用
    adjust_mouse_speed(m);

    // 以下は Keyball のデフォルトのスクロール処理ロジックをコピーしたものです。
    // consume motion of trackball.
    // keyball_get_scroll_div() は keyball.h をインクルードしていれば使えます。
    int16_t div = 1 << (keyball_get_scroll_div() - 1);
    int16_t x = divmod16(&m->x, div);
    int16_t y = divmod16(&m->y, div);

    // apply to mouse report.
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

    // KeyballのデフォルトにはScroll snappingのロジックも含まれますが、
    // 複雑になるためここでは省略します。必要に応じてkeyball.cからコピーしてください。
    // スクロール後に蓄積された移動量をクリア
    m->x = 0;
    m->y = 0;

    // 関数から戻ると、Keyball の calling code が r の内容を使ってレポートを送信します。
}


#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
