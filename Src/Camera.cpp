/**
* @file Camera.cpp
*/
#include "Camera.h"

/**
* カメラのパラメータを更新する.
*
* @param matView 更新に使用するビュー行列.
*/
void Camera::Update(const glm::mat4& matView)
{
  const float scale = -1000.0f;
  const glm::vec4 pos = matView * glm::vec4(target, 1);
  focalPlane = pos.z * scale;
#if 1
  const float imageDistance = sensorSize * 0.5f / glm::tan(fov * 0.5f); // 焦点距離.
  //const float fov = 2.0f * glm::atan(sensorSize * 0.5f / imageDistance);
  focalLength = 1.0f / ((1.0f / focalPlane) + (1.0f / imageDistance));
#else
  // 上の式を変形して除算を減らしたバージョン.
  // `https://www.slideshare.net/siliconstudio/cedec-2010`で提示されている式.
  const float f = (focalPlane * sensorSize * 0.5f) / (glm::tan(fov * 0.5f) * focalPlane + sensorSize * 0.5f);
#endif
  aperture = focalLength / fNumber;
}
