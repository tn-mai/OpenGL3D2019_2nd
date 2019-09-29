/**
* @file Collision.cpp
*/
#include "Collision.h"

namespace Collision {

/**
* 球形状を作成する.
*
* @param center 球の中心座標.
* @param r      球の半径.
*
* @return 球を保持する汎用衝突形状オブジェクト.
*/
Shape CreateSphere(const glm::vec3& center, float r)
{
  Shape result;
  result.type = Shape::Type::sphere;
  result.s = Sphere{ center, r };
  return result;
}

/**
* カプセル形状を作成する.
*
* @param a  中心の線分の始点座標.
* @param b  中心の線分の終点座標.
* @param r  カプセルの半径.
*
* @return カプセルを保持する汎用衝突形状オブジェクト.
*/
Shape CreateCapsule(const glm::vec3& a, const glm::vec3& b, float r) {
  Shape result;
  result.type = Shape::Type::capsule;
  result.c = Capsule{ { a, b }, r };
  return result;
}

/**
* 有向境界ボックス形状を作成する.
*
* @param center 有向境界ボックスの中心座標.
* @param axisX  X軸の向き.
* @param axisY  Y軸の向き.
* @param axisZ  Z軸の向き.
* @param e      XYZ軸方向の幅.
*
* @return 有向境界ボックスを保持する汎用衝突形状オブジェクト.
*/
Shape CreateOBB(const glm::vec3& center, const glm::vec3& axisX,
  const glm::vec3& axisY, const glm::vec3& axisZ, const glm::vec3& e) {
  Shape result;
  result.type = Shape::Type::obb;
  result.obb = OrientedBoundingBox{ center,
    { normalize(axisX), normalize(axisY), normalize(axisZ) }, e };
  return result;
}

/**
* 球と球が衝突しているか調べる.
*
* @param s0 判定対象の球その１.
* @param s1 判定対象の球その２.
*
* @retval true  衝突している.
* @retval false 衝突してない.
*/
bool TestSphereSphere(const Sphere& s0, const Sphere& s1)
{
  const glm::vec3 m = s0.center - s1.center;
  const float radiusSum = s0.r + s1.r;
  return glm::dot(m, m) <= radiusSum * radiusSum;
}

/**
* 線分と点の最近接点を調べる.
*
* @param seg 線分.
* @param p   点.
*
* @return segとpの最近接点.
*/
glm::vec3 ClosestPointSegment(const Segment& seg, const glm::vec3& p)
{
  const glm::vec3 ab = seg.b - seg.a;
  const glm::vec3 ap = p - seg.a;
  const float lenAQ = glm::dot(ab, ap);
  const float lenAB = glm::dot(ab, ab);
  if (lenAQ <= 0) {
    return seg.a;
  } else if (lenAQ >= lenAB) {
    return seg.b;
  }
  return seg.a + ab * (lenAQ / lenAB);
}

/**
* 球とカプセルが衝突しているか調べる.
*
* @param s 球.
* @param c カプセル.
* @param p 最近接点の格納先.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p)
{
  *p = ClosestPointSegment(c.seg, s.center);
  const glm::vec3 distance = *p - s.center;
  const float radiusSum = s.r + c.r;
  return glm::dot(distance, distance) <= radiusSum * radiusSum;
}

/**
* OBBと点の最近接点を調べる.
*
* @param obb 有向境界ボックス.
* @param p   点.
*
* @return obbとpの最近接点.
*/
glm::vec3 ClosestPointOBB(const OrientedBoundingBox& obb, const glm::vec3& p)
{
  const glm::vec3 d = p - obb.center;
  glm::vec3 q = obb.center;
  for (int i = 0; i < 3; ++i) {
    float distance = dot(d, obb.axis[i]);
    if (distance >= obb.e[i]) {
      distance = obb.e[i];
    } else if (distance <= -obb.e[i]) {
      distance = -obb.e[i];
    }
    q += distance * obb.axis[i];
  }
  return q;
}

/**
* 球とOBBが衝突しているか調べる.
*
* @param s   球.
* @param obb 有向境界ボックス.
* @param p   最近接点の格納先.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p)
{
  *p = ClosestPointOBB(obb, s.center);
  const glm::vec3 distance = *p - s.center;
  return dot(distance, distance) <= s.r * s.r;
}

/**
* シェイプ同士が衝突しているか調べる.
*
* @param a  判定対象のシェイプその１.
* @param b  判定対象のシェイプその２.
* @param pa 衝突した座標.
* @param pb 衝突した座標.
*
* @retval true  衝突した.
* @retval false 衝突しなかった.
*/
bool TestShapeShape(const Shape& a, const Shape& b, glm::vec3* pa, glm::vec3* pb)
{
  if (a.type == Shape::Type::sphere) {
    if (b.type == Shape::Type::sphere) {
      if (TestSphereSphere(a.s, b.s)) {
        *pa = a.s.center;
        *pb = b.s.center;
        return true;
      }
    } else if (b.type == Shape::Type::obb) {
      if (TestSphereOBB(a.s, b.obb, pb)) {
        *pa = a.s.center;
        return true;
      }
    } else if (b.type == Shape::Type::capsule) {
      if (TestSphereCapsule(a.s, b.c, pb)) {
        *pa = a.s.center;
        return true;
      }
    }
  } else if (a.type == Shape::Type::capsule) {
    if (b.type == Shape::Type::sphere) {
      if (TestSphereCapsule(b.s, a.c, pa)) {
        *pb = b.s.center;
        return true;
      }
    }
  } else if (a.type == Shape::Type::obb) {
    if (b.type == Shape::Type::sphere) {
      if (TestSphereOBB(b.s, a.obb, pa)) {
        *pb = b.s.center;
        return true;
      }
    }
  }
  return false;
}

} // namespace Collision