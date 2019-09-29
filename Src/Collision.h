/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <GL/glew.h>
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
* カプセル.
*/
struct Capsule {
  Segment seg; ///< 円柱部の中心の線分.
  float r = 0; ///< カプセルの半径.
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

bool TestSphereSphere(const Sphere&, const Sphere&);
bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p);
bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p);
bool TestShapeShape(const Shape&, const Shape&, glm::vec3* pa, glm::vec3* pb);

} // namespace Collision

#endif // COLLISION_H_INCLUDED