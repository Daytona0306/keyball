/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
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

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default (VIA)
  [0] = LAYOUT_universal(
    KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                            KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     ,
    KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                            KC_H     , KC_J     , KC_K     , KC_L     , KC_SCLN  ,
    KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                            KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    KC_LCTL  , KC_LGUI  , KC_LALT  ,LT(1,KC_LNG2),LT(2,KC_SPC),LT(3,KC_LNG1),KC_BSPC,LT(2,KC_ENT),LT(1,KC_LNG2),KC_RALT,KC_RGUI, KC_RSFT
  ),

  [1] = LAYOUT_universal(
    S(KC_Q)   , S(KC_W) , S(KC_E)  , S(KC_R)  , S(KC_T)  ,                            S(KC_Y)  , S(KC_U)  , S(KC_I)  , S(KC_O)  , S(KC_P)  ,
    S(KC_A)   , S(KC_S) , S(KC_D)  , S(KC_F)  , S(KC_G)  ,                            S(KC_H)  , S(KC_J)  , S(KC_K)  , S(KC_L)  , KC_QUOT  ,
    S(KC_Z)   , S(KC_X) , S(KC_C)  , S(KC_V)  , S(KC_B)  ,                            S(KC_N)  , S(KC_M)  ,S(KC_COMM), S(KC_DOT),S(KC_SLSH),
    KC_LCTL   , KC_LGUI , KC_LALT  , _______  , _______  , _______  ,      _______  , _______  , _______  , KC_RALT  , KC_RGUI  , KC_RSFT
  ),

  [2] = LAYOUT_universal(
    _______   , KC_7    , KC_8     , KC_9     , _______  ,                            _______  , KC_LEFT  , KC_UP    , KC_RGHT  , _______  ,
    _______   , KC_4    , KC_5     , KC_6     ,S(KC_SCLN),                            KC_PGUP  , KC_BTN1  , KC_DOWN  , KC_BTN2  , KC_BTN3  ,
    _______   , KC_1    , KC_2     , KC_3     ,S(KC_MINS),                            KC_PGDN  , _______  , _______  , _______  , _______  ,
    _______   , KC_0    , KC_DOT   , _______  , _______  , _______  ,      KC_DEL   , _______  , _______  , _______  , _______  , _______
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , _______  , _______  , _______  ,  _______  ,                           RGB_M_P  , RGB_M_B  , RGB_M_R  , RGB_M_SW , RGB_M_SN ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  ,  SCRL_DVI ,                           RGB_M_K  , RGB_M_X  , RGB_M_G  , RGB_M_T  , RGB_M_TW ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  ,  SCRL_DVD ,                           CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT    , KBC_RST  , _______  , _______  ,  _______  , _______  ,     _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
};
// clang-format on

bool pointing_device_task(void) {
   // ポインティングデバイスから現在のレポートを取得します。
   report_mouse_t mouse_report = pointing_device_get_report();

   // 現在の最も高いアクティブなレイヤーを取得します。
    // <<== get_highest_layer に layer_state を引数として渡すように修正
   uint8_t layer = get_highest_layer(layer_state);

   // トラックボールの移動量のしきい値を設定します。
   // この値より小さい移動は無視され、チャタリングなどを防止できます。
   // 実機でのテストに基づいて調整してください。
   int sensitivity_threshold = 5; // ZMKの 'tick' に相当する概xxxxxx念


   // レイヤー5の場合のみトラックボールの移動を処理します。(矢印キー)
   if (layer == 5) {

      // 元の移動量を一時的に保存
      int8_t original_delta_x = mouse_report.x;
      int8_t original_delta_y = mouse_report.y;

      // このレイヤーではデフォルトのマウス移動を無効にするため、レポートの移動量をゼロに設定
      mouse_report.x = 0;
      mouse_report.y = 0;
      mouse_report.v = 0; // スクロールも無効

      // しきい値を超えた移動量があるかチェック
      if (abs(original_delta_x) > sensitivity_threshold || abs(original_delta_y) > sensitivity_threshold) {
         // 支配的な移動方向を判定し、対応する矢印キーをタップします。
         // Y-方向が UP、Y+方向が DOWN に対応するのが一般的です。
         if (abs(original_delta_x) > abs(original_delta_y)) { // 水平移動が支配的
            if (original_delta_x > sensitivity_threshold) {
               tap_code(KC_RIGHT);
            } else if (original_delta_x < -sensitivity_threshold) {
               tap_code(KC_LEFT);
            }
         } else { // 垂直移動が支配的、または同じ
             if (original_delta_y < -sensitivity_threshold) { // Y- 方向
               tap_code(KC_UP);
            } else if (original_delta_y > sensitivity_threshold) { // Y+ 方向
               tap_code(KC_DOWN);
            }
         }
         // ZMKの 'wait-ms', 'tap-ms' のようなタイミング制御が必要な場合は、
         // ここにタイマーや状態管理のロジックを追加する必要があります。
         // この例は移動検出ごとに即座にキーをタップします。
      }

   }
   // レイヤー3の場合は Keyball 独自のスクロールモードが layer_state_set_user で有効になっている想定です。
    // pointing_device_task では特別なキー入力を生成せず、デフォルト処理に任せます。
    // mouse_report のデータはここでは変更しません。
    // else if (layer == 3) { /* 何もしない */ }
    // その他のレイヤーもデフォルト処理に任せます。
    // else { /* 何もしない */ }

    // レイヤー5で処理した場合のみ、レポートを独自処理したことを返します。
    // それ以外のレイヤーでは false を返すことで、QMKのデフォルトマウス処理に進みます。
    // ただし、pointing_device_task を定義すると通常はデフォルト処理を完全に置き換えるため、
    // ここではシンプルに常に true を返しておきます。
    // もしレイヤー3などでデフォルト処理を呼び出したい場合は、
    // pointing_device_execute_task(&mouse_report) のような関数を呼ぶ必要がありますが、
    // Keyballのファームウェア構造によるため、ここでは標準的な pointing_device_task オーバーライドの形式をとります。
    // シンプルに常に true を返すのが、オーバーライドの一般的な方法です。
    
   // 最終的なマウスレポートをQMKのUSBスタックに送信します。
   // レイヤー5の場合は移動量がゼロになり、矢印キー入力のみが行われます。
   // レイヤー3およびその他のレイヤーでは、取得した元の移動量が送信され、
   // デフォルトのマウス/スクロール処理（レイヤー3ではKeyballのスクロールモード）が行われます。
   pointing_device_set_report(mouse_report);
    
    // <<== 関数宣言に合わせて bool を返すように修正
   return true;
}



#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
