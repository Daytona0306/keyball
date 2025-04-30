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

// 数学関数（expf, sqrtf など）を使うために必要
#include <math.h>

// マクロ機能を使う場合は必要になる場合がありますが、今回は M() を KC_NO に置き換えているため必須ではありません
// #include "action_macro.h"


// === カスタマイズ可能な設定値 ===

// スクロール加速の感度 (1-10程度で調整)
// ZMKの CONFIG_PMW3610_SCROLL_SENSITIVITY に相当
#define QMK_SCROLL_SENSITIVITY 2.0f // デフォルトは2.0f。浮動小数点数として定義


// === ヘルパー関数 ===

// Keyball デフォルト処理で使用されている静的ヘルパー関数をコピー
// divmod16 は使用されているため残します
static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

// Keyball デフォルト処理で使用されている速度調整関数
// シグモイド加速導入に伴い、スクロール処理ではこの関数は直接使用しなくなりますが、
// Layer 5以外のマウス移動処理では引き続き使用します。
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
  // keymap for default
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
    // Auto enable scroll mode when the highest layer is 3
    // この行はKeyball独自のスクロールモードに必要とのことなので、そのままにします。
    keyball_set_scroll_mode(get_highest_layer(state) == 3);

    // レイヤー5のトラックボール矢印キー機能を使う場合、Layer 5に切り替わったらスクロールモードを無効にする必要があります。
    // keymaps 配列で Layer 4, 5 は定義されていませんが、
    // keyball_on_apply_motion_to_mouse_move 関数で Layer 5 をチェックしているため、
    // Layer 5 に切り替えるキーを割り当てていれば、トラックボールは矢印キーとして機能します。
    // Layer 5 のチェックは get_highest_layer(state) == 5 で行います。
    if (get_highest_layer(state) == 5) {
        keyball_set_scroll_mode(false);
    }

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
    int sensitivity_threshold = 2; // ZMKの 'tick' に相当する概念。調整が必要。

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
// スクロールにシグモイド関数を使った加速度を適用
void keyball_on_apply_motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    // レイヤー3（スクロールモード）の場合のみ処理を行います。
    // Layer 3 で keyball.scroll_mode が true に設定されている前提です。
    // この関数は Keyball 内部で keyball.scroll_mode が true の場合に呼ばれます。

    // シグモイド加速を適用
    // Based on ZMK code provided by user
    static uint32_t last_scroll_time = 0; // Static variable to store the time of the last scroll event (using uint32_t for QMK time)
    uint32_t current_time = timer_read(); // Use QMK's timer_read() for milliseconds

    // Calculate movement magnitude (using float for calculation)
    float movement = sqrtf((float)m->x * m->x + (float)m->y * m->y); // Use sqrtf for float

    float accel_x = m->x;
    float accel_y = m->y;

    // Calculate time delta (handle initial call and timer wrap-around)
    uint32_t delta_time = timer_elapsed(last_scroll_time);

    // Calculate speed (movement per millisecond)
    if (delta_time > 0) { // Avoid division by zero
        float speed = movement / (float)delta_time;

        // Sigmoid acceleration function (adapted from ZMK code)
        // ZMK code uses a base sensitivity multiplier: acceleration *= base_sensitivity;
        // base_sensitivity = (float)CONFIG_PMW3610_SCROLL_SENSITIVITY / 2.5f;

        float base_sensitivity = QMK_SCROLL_SENSITIVITY / 2.5f;
        // ZMK formula: 1.0f + 9.0f * (1.0f / (1.0f + expf(-0.5f * (speed - 8.0f))))
        // Base acceleration 1.0x to 10.0x, centered around speed 8
        float acceleration_factor = 1.0f / (1.0f + expf(-0.5f * (speed - 8.0f)));
        float acceleration = 1.0f + 9.0f * acceleration_factor;

        // Apply sensitivity
        acceleration *= base_sensitivity;

        // Apply acceleration factor to original deltas
        accel_x = (float)m->x * acceleration;
        accel_y = (float)m->y * acceleration;

        // Maintain small movements (optional, based on ZMK code - keeping original delta for <= 1)
        if (abs(m->x) <= 1) accel_x = m->x;
        if (abs(m->y) <= 1) accel_y = m->y;

    }

    // Update last scroll time
    last_scroll_time = current_time;

    // Accumulate accelerated deltas back into m? Or use accel_x/y directly for report?
    // The original Keyball logic (and divmod16) works with m as the accumulator.
    // Update m with the calculated accelerated deltas before the scroll tick check.
    m->x = (int16_t)roundf(accel_x); // Convert back to int16, using roundf for better accuracy
    m->y = (int16_t)roundf(accel_y); // Convert back to int16, using roundf for better accuracy


    // The rest of the scroll logic from the original override remains to handle scroll ticks and reporting
    // consume motion of trackball.
    // keyball_get_scroll_div() は keyball.h をインクルードしていれば使えます。
    // 注: divmod16 は m->x/y を変更して残余を保持します。
    int16_t div = 1 << (keyball_get_scroll_div() - 1);
    int16_t x = divmod16(&m->x, div);
    int16_t y = divmod16(&m->y, div);

    // apply to mouse report.
    // r->h と r->v にスクロール量を格納
    #if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
        r->h = clip2int8(y); // Horizontal scroll from Y movement (Keyball default mapping)
        r->v = -clip2int8(x); // Vertical scroll from X movement (Keyball default mapping)
        if (is_left) {
            r->h = -r->h;
            r->v = -r->v;
        }
    #elif KEYBALL_MODEL == 46
        r->h = clip2int8(x); // Horizontal scroll from X movement
        r->v = clip2int8(y); // Vertical scroll from Y movement
    #else
    #    error("unknown Keyball model")
    #endif

    // ZMKコードにあった残り値の減衰処理やイベントレート制限はここには含めていません。
    // Keyballの既存ロジックが divmod16 で残余を扱います。

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
