/**
* @file GamePad.h
*/
#ifndef GAMEPAD_H_INCLUDED
#define GAMEPAD_H_INCLUDED
#include <stdint.h>

/**
* ゲームパッド情報.
*/
struct GamePad
{
  // キーとボタンに対応するビット定数.
  static const uint32_t DPAD_UP = 0x0001;
  static const uint32_t DPAD_DOWN = 0x0002;
  static const uint32_t DPAD_LEFT = 0x0004;
  static const uint32_t DPAD_RIGHT = 0x0008;
  static const uint32_t START = 0x0010;
  static const uint32_t A = 0x0020;
  static const uint32_t B = 0x0040;
  static const uint32_t X = 0x0080;
  static const uint32_t Y = 0x0100;
  static const uint32_t L = 0x0200;
  static const uint32_t R = 0x0400;

  uint32_t buttons = 0; ///< 押されているあいだフラグが立つ.
  uint32_t buttonDown = 0; ///< 押された瞬間だけフラグが立つ.
};

#endif // GAMEPAD_H_INCLUDED