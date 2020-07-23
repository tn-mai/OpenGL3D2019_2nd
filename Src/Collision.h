/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <GL/glew.h>
#include "Camera.h"
#include <glm/glm.hpp>

namespace Collision {

/**
* 球.
*/
struct Sphere {
  glm::vec3 center = glm::vec3(0); ///< 中心座標.
  float r = 0; ///< 半径.
};

/**
* 線分.
*/
struct Segment {
  glm::vec3 a = glm::vec3(0); ///< 線分の始点.
  glm::vec3 b = glm::vec3(0); ///< 線分の終点.
};

/**
* 平面.
*/
struct Plane
{
  glm::vec3 p; ///< 平面上の点.
  glm::vec3 n; ///< 平面の法線.
};

/**
* カプセル.
*/
struct Capsule {
  Segment seg; ///< 円柱部の中心の線分.
  float r = 0; ///< カプセルの半径.
};

/**
* 軸平行境界ボックス.
*/
struct AABB {
  glm::vec3 min;
  glm::vec3 max;
};

/**
* 有向境界ボックス.
*/
struct OrientedBoundingBox {
  glm::vec3 center = glm::vec3(0); ///< ボックスの中心.
  glm::vec3 axis[3] = { {1,0,0}, {0,1,0}, {0,0,1} }; ///< ボックスの軸.
  glm::vec3 e = glm::vec3(0); ///< 各軸の半径.
};

/**
* 錐台.
*/
struct Frustum
{
  Frustum() = default;
  explicit Frustum(const Camera&);
  Frustum(const Camera&, float left, float right, float bottom, float top, float near, float far);

  enum {
    nearPlane,
    farPlane,
    topPlane,
    bottomPlane,
    leftPlane,
    rightPlane,
  };
  Plane planes[6];
};

/**
* 汎用衝突形状.
*/
struct Shape
{
  enum class Type {
    none, ///< 形状なし.
    sphere, ///< 球.
    capsule, ///< カプセル.
    obb, ///< 有向境界ボックス.
  };
  Type type = Type::none; ///< 実際の形状.
  Sphere s; ///< 球の形状データ.
  Capsule c; ///< カプセルの形状データ.
  OrientedBoundingBox obb; ///< 有向境界ボックスの形状データ.
};

// 形状作成関数.
Shape CreateSphere(const glm::vec3&, float);
Shape CreateCapsule(const glm::vec3&, const glm::vec3&, float);
Shape CreateOBB(const glm::vec3& center, const glm::vec3& axisX,
  const glm::vec3& axisY, const glm::vec3& axisZ, const glm::vec3& e);
Frustum CreateFrustum(const Camera&);

// 衝突判定関数.
bool TestSphereSphere(const Sphere&, const Sphere&);
bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p);
bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p);
bool TestShapeShape(const Shape&, const Shape&, glm::vec3* pa, glm::vec3* pb);
bool Test(const Frustum&, const glm::vec3&);
bool Test(const Frustum&, const Sphere&);

/**
* 衝突結果を表す構造体.
*/
struct Result
{
  bool isHit = false; ///< 衝突の有無.
  glm::vec3 pa; ///< 形状A上の衝突点.
  glm::vec3 na; ///< 形状A上の衝突平面の法線.
  glm::vec3 pb; ///< 形状B上の衝突点.
  glm::vec3 nb; ///< 形状B上の衝突平面の法線.
};

Result TestShapeShape(const Shape&, const Shape&);

glm::vec3 ClosestPointSegment(const Segment& seg, const glm::vec3& p);

/**
* 視錐台その2.
*
* 錘台の形に着目した最適化を施したバージョン.
*/
struct ViewFrustum2 {
  ViewFrustum2() = default;
  ViewFrustum2(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up, float fov, float aspectRatio, float near, float far);
  bool Test(const Sphere& s) const;

  glm::vec3 position;
  glm::vec3 axis[3];
  float tanY; // tan(fov)
  float aspectRatio;
  glm::vec2 sphereFactor; // y=1/cos(fov), x=1/cos(acos(tanY*aspectRatio))
  float near;
  float far;
};

#ifndef NDEBUG
bool Test();
#endif

} // namespace Collision

#endif // COLLISION_H_INCLUDED