/**
* @file Camera.h
*/
#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include <GL/glew.h>
#include <glm/glm.hpp>
// windows.hが定義しているnear,farを無効化.
#undef far
#undef near

/**
* カメラ.
*/
struct Camera
{
  glm::vec3 target = glm::vec3(100, 0, 100);
  glm::vec3 position = glm::vec3(100, 50, 150);
  glm::vec3 up = glm::vec3(0, 1, 0);
  glm::vec3 velocity = glm::vec3(0);

  // 画面パラメータ.
  float width = 1280; ///< 画面の幅(ピクセル数).
  float height = 720; ///< 画面の高さ(ピクセル数).
  float near = 1; ///< 最小Z値(メートル).
  float far = 500; ///< 最大Z値(メートル).

  // 変更可能なカメラパラメータ.
  float fNumber = 1.4f; ///< エフ・ナンバー = カメラのF値.
  float fov = glm::radians(30.0f); ///< フィールド・オブ・ビュー = カメラの視野角(ラジアン)
  float sensorSize = 36.0f; ///< センサー・サイズ = カメラのセンサーの横幅(ミリ).

  // Update関数で計算するカメラパラメータ.
  float focalLength = 50.0f; ///< フォーカル・レングス = 焦点距離(ミリ).
  float aperture = 20.0f; ///< アパーチャ = 開口(ミリ).
  float focalPlane = 10000.0f; ///< フォーカル・プレーン = ピントの合う距離.

  void Update(const glm::mat4& matView);
};

#endif // CAMERA_H_INCLUDED
