/**
* @file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>
#include <vector>
#include <math.h>

namespace Collision {

/**
* ���`����쐬����.
*
* @param center ���̒��S���W.
* @param r      ���̔��a.
*
* @return ����ێ�����ėp�Փˌ`��I�u�W�F�N�g.
*/
Shape CreateSphere(const glm::vec3& center, float r)
{
  Shape result;
  result.type = Shape::Type::sphere;
  result.s = Sphere{ center, r };
  return result;
}

/**
* �J�v�Z���`����쐬����.
*
* @param a  ���S�̐����̎n�_���W.
* @param b  ���S�̐����̏I�_���W.
* @param r  �J�v�Z���̔��a.
*
* @return �J�v�Z����ێ�����ėp�Փˌ`��I�u�W�F�N�g.
*/
Shape CreateCapsule(const glm::vec3& a, const glm::vec3& b, float r) {
  Shape result;
  result.type = Shape::Type::capsule;
  result.c = Capsule{ { a, b }, r };
  return result;
}

/**
* �L�����E�{�b�N�X�`����쐬����.
*
* @param center �L�����E�{�b�N�X�̒��S���W.
* @param axisX  X���̌���.
* @param axisY  Y���̌���.
* @param axisZ  Z���̌���.
* @param e      XYZ�������̕�.
*
* @return �L�����E�{�b�N�X��ێ�����ėp�Փˌ`��I�u�W�F�N�g.
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
* ���Ƌ����Փ˂��Ă��邩���ׂ�.
*
* @param s0 ����Ώۂ̋����̂P.
* @param s1 ����Ώۂ̋����̂Q.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��ĂȂ�.
*/
bool TestSphereSphere(const Sphere& s0, const Sphere& s1)
{
  const glm::vec3 m = s0.center - s1.center;
  const float radiusSum = s0.r + s1.r;
  return glm::dot(m, m) <= radiusSum * radiusSum;
}

/**
* �����Ɠ_�̍ŋߐړ_�𒲂ׂ�.
*
* @param seg ����.
* @param p   �_.
*
* @return seg��p�̍ŋߐړ_.
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
* ���ƃJ�v�Z�����Փ˂��Ă��邩���ׂ�.
*
* @param s ��.
* @param c �J�v�Z��.
* @param p �ŋߐړ_�̊i�[��.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p)
{
  *p = ClosestPointSegment(c.seg, s.center);
  const glm::vec3 distance = *p - s.center;
  const float radiusSum = s.r + c.r;
  return glm::dot(distance, distance) <= radiusSum * radiusSum;
}

/**
* OBB�Ɠ_�̍ŋߐړ_�𒲂ׂ�.
*
* @param obb �L�����E�{�b�N�X.
* @param p   �_.
*
* @return obb��p�̍ŋߐړ_.
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
* ����OBB���Փ˂��Ă��邩���ׂ�.
*
* @param s   ��.
* @param obb �L�����E�{�b�N�X.
* @param p   �ŋߐړ_�̊i�[��.
*
* @retval true  �Փ˂��Ă���.
* @retval false �Փ˂��Ă��Ȃ�.
*/
bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p)
{
  *p = ClosestPointOBB(obb, s.center);
  const glm::vec3 distance = *p - s.center;
  return dot(distance, distance) <= s.r * s.r;
}

/**
* 2�̐����̍ŋߐړ_�𒲂ׂ�.
*
* @param s0 ����A.
* @param s1 ����B.
* @param c0 ����A��̍ŋߐړ_.
* @param c1 ����B��̍ŋߐړ_.
*
* @return c0, c1�Ԃ̂̒�����2��.
*/
float ClosestPoint(const Segment& s0, const Segment& s1, glm::vec3& c0, glm::vec3& c1)
{
  const glm::vec3 d1 = s0.b - s0.a;
  const glm::vec3 d2 = s1.b - s1.a;
  const glm::vec3 r = s0.a - s1.a;

  const float a = glm::dot(d1, d1);
  const float b = glm::dot(d1, d2);
  const float c = glm::dot(d1, r);
  const float e = glm::dot(d2, d2);
  const float f = glm::dot(d2, r);
  const float d = a * e - b * b;

  // 2�����̍ŋߐړ_d1*v(s,t)�y��d2*v(s,t)�����߂�ȉ��̍s��
  // |a -b| |s|   |-c|
  // |b -e| |t| = |-f|
  // ���A�N�������̌�����p���Ĉȉ��ɕϊ�����.
  // d = ae - b^2
  // s = (bf - ce) / d
  // t = (af - bc) / d
  //
  // �����̏ꍇ�A�㎮�͈ȉ��̌`�ɕω�����
  // s = (bt - c) / a
  // t = (bs + f) / e
  // ���̂܂܂ł͌v�Z�ł��Ȃ��̂ŁA�܂�s(�܂���t)���v�Z���A�����͈͓̔��ɃN�����v���ĉ���s�����߂�.
  // ���ɉ�s���㎮�ɑ������t�����߂�.
  // t�������O�ɂ���Ȃ�t���N�����v���A�㎮��p����s���Čv�Z����.
  // t���������ɂ���Ȃ�m�肷��.

  float s = 0;
  if (d) {
    s = (b * f - c * e) / d;
    s = glm::clamp(s, 0.0f, 1.0f);
  }
  //const float t = (a * f - b * c) / d;

  float t = (b * s + f) / e;
  if (t < 0) {
    t = 0;
    // s = (tb - c) / a���t = 0�Ȃ̂�s = -c / a�ƂȂ�
    s = glm::clamp(-c / a, 0.0f, 1.0f);
  } else if (t > 1) {
    t = 1;
    // s = (tb - c) / a���t = 1�Ȃ̂�s = (b - c) / a�ƂȂ�
    s = glm::clamp((b - c) / a, 0.0f, 1.0f);
  }

  c0 = s0.a + d1 * s;
  c1 = s1.a + d2 * t;

  return glm::dot(c0 - c1, c0 - c1);
}

/**
* �J�v�Z���ƃJ�v�Z�����Փ˂��Ă��邩���ׂ�.
*
* @param c0 �J�v�Z��0.
* @param c1 �J�v�Z��1.
*
* @return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleCapsule(const Capsule& c0, const Capsule& c1)
{
  glm::vec3 p0, p1;
  const float d = ClosestPoint(c0.seg, c1.seg, p0, p1);
  const float r = c0.r + c1.r;
  Result result;
  if (d < r * r) {
    result.isHit = true;
    result.normal = glm::normalize(p0 - p1);
    result.pa = p1 + result.normal * c1.r;
    result.pb = p0 - result.normal * c0.r;
  }
  return result;
}

/**
*
*/
struct AABB {
  glm::vec3 min;
  glm::vec3 max;
};

/**
*
*/
bool IntersectRayAABB(const glm::vec3& p, const glm::vec3& d, const AABB& a, float& tmin, glm::vec3& q)
{
  tmin = 0;
  float tmax = FLT_MAX;

  for (int i = 0; i < 3; ++i) {
    if (std::abs(d[i]) < FLT_EPSILON) {
      if (p[i] < a.min[i] || p[i] > a.max[i]) {
        return false;
      }
    } else {
      const float ood = 1.0f / d[i];
      float t1 = (a.min[i] - p[i]) * ood;
      float t2 = (a.max[i] - p[i]) * ood;
      if (t1 > t2) {
        std::swap(t1, t2);
      }
      tmin = std::max(tmin, t1);
      tmax = std::min(tmax, t2);
      if (tmin > tmax) {
        return false;
      }
    }
  }
  q = p + d * tmin;
  return true;
}

/**
*
*/
glm::vec3 Corner(const AABB& aabb, int n)
{
  glm::vec3 p = aabb.min;
  if (n & 1) { p.x = aabb.max.x; }
  if (n & 2) { p.y = aabb.max.y; }
  if (n & 4) { p.z = aabb.max.z; }
  return p;
}

/**
* �J�v�Z����AABB���Փ˂��Ă��邩���ׂ�.
*
* @param c    �J�v�Z��.
* @param aabb �����s���E�{�b�N�X.
*
* @return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleAABB(const Capsule& c, const AABB& aabb)
{
  AABB e = aabb;
  e.min -= c.r;
  e.max += c.r;

  const glm::vec3 d(c.seg.b - c.seg.a); // 0 <= t < 1�Ŕ���ł���悤�ɂ��邽�߁A���K���͂��Ȃ�.

  float t;
  glm::vec3 p;
  if (!IntersectRayAABB(c.seg.a, d, e, t, p) || t > 1) {
    return {};
  }

  int u = 0, v = 0;
  if (p.x < aabb.min.x) { u |= 1; }
  else if (p.x > aabb.max.x) { v |= 1; }

  if (p.y < aabb.min.y) { u |= 2; }
  else if (p.y > aabb.max.y) { v |= 2; }

  if (p.z < aabb.min.z) { u |= 4; }
  else if (p.z > aabb.max.z) { v |= 4; }

  const int m = u + v;

  // m = 0�Ȃ�΁A��_p��AABB�̓����ɂ���.
  // p���ł��߂��ʂɎˉe���A���̓_���Փ˓_�Ƃ���.
  if (!m) {
    glm::vec3 q(aabb.min.x, p.y, p.z);
    float d = p.x - aabb.min.x;
    if (aabb.max.x - p.x < d) {
      d = aabb.max.x - p.x;
      q = glm::vec3(aabb.max.x, p.y, p.z);
    }
    if (p.y - aabb.min.y < d) {
      d = p.y - aabb.min.y;
      q = glm::vec3(p.x, aabb.min.y, p.z);
    }
    if (aabb.max.y - p.y < d) {
      d = aabb.max.y - p.y;
      q = glm::vec3(p.x, aabb.max.y, p.z);
    }
    if (p.z - aabb.min.z < d) {
      d = p.z - aabb.min.z;
      q = glm::vec3(p.x, p.y, aabb.min.z);
    }
    if (aabb.max.z - p.z < d) {
      d = aabb.max.z - p.z;
      q = glm::vec3(p.x, p.y, aabb.max.z);
    }
    Result result;
    result.isHit = true;
    result.normal = glm::normalize(q - p);
    result.pa = q;
    result.pb = p + result.normal * c.r;
    return result;
  }

  if ((m & (m - 1)) == 0) {
    Result result;
    result.isHit = true;
    if (u & 1) {
      result.normal = glm::vec3(-1, 0, 0);
    } else if (u & 2) {
      result.normal = glm::vec3(0,-1, 0);
    } else if (u & 4) {
      result.normal = glm::vec3(0, 0,-1);
    } else if (v & 1) {
      result.normal = glm::vec3(1, 0, 0);
    } else if (v & 2) {
      result.normal = glm::vec3(0, 1, 0);
    } else if (v & 4) {
      result.normal = glm::vec3(0, 0, 1);
    }
    glm::vec3 q = aabb.min;
    if (v) {
      q = aabb.max;
    }
    result.pa = p - result.normal * glm::dot(p - q, result.normal);
    result.pb = p - result.normal * c.r;
    return result;
  }

  if (m == 7) {
    const glm::vec3 bv = Corner(aabb, v);
    glm::vec3 c0, c1, c2, c3;
    float d = ClosestPoint(c.seg, Segment{ bv, Corner(aabb, v ^ 1) }, c0, c1);
    float d0 = ClosestPoint(c.seg, Segment{ bv, Corner(aabb, v ^ 2) }, c2, c3);
    if (d0 < d) {
      d = d0;
      c0 = c2;
      c1 = c3;
    }
    d0 = ClosestPoint(c.seg, Segment{ bv, Corner(aabb, v ^ 4) }, c2, c3);
    if (d0 < d) {
      d = d0;
      c0 = c2;
      c1 = c3;
    }
    if (d > c.r * c.r) {
      return {};
    }
    Result result;
    result.isHit = true;
    result.normal = glm::normalize(c0 - c1);
    result.pa = c1;
    result.pb = c0 - result.normal * c.r;
    return result;
  }

  {
    glm::vec3 c0, c1;
    const float d = ClosestPoint(c.seg, Segment{ Corner(aabb, u ^ 7), Corner(aabb, v) }, c0, c1);
    if (d > c.r * c.r) {
      return {};
    }
    Result result;
    result.isHit = true;
    result.normal = glm::normalize(c0 - c1);
    result.pa = c1;
    result.pb = c0 - result.normal * c.r;
    return result;
  }
}

/**
* �J�v�Z����OBB���Փ˂��Ă��邩���ׂ�.
*
* @param c   �J�v�Z��.
* @param obb �L�����E�{�b�N�X.
*
* @return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleOBB(const Capsule& c, const OrientedBoundingBox& obb)
{
  // ������OBB�̃��[�J�����W�n�ɕϊ�.
  Capsule cc = c;
  cc.seg.a -= obb.center;
  cc.seg.b -= obb.center;
  const glm::mat3 matOBB(glm::transpose(glm::mat3(obb.axis[0], obb.axis[1], obb.axis[2])));
  cc.seg.a = matOBB * cc.seg.a;
  cc.seg.b = matOBB * cc.seg.b;
  Result result = TestCapsuleAABB(cc, AABB{ -obb.e, obb.e });
  if (result.isHit) {
    const glm::mat3 matInvOBB(glm::inverse(matOBB));
    result.pa = matInvOBB * result.pa + obb.center;
    result.pb = matInvOBB * result.pb + obb.center;
    result.normal = matInvOBB * result.normal;
  }
  return result;
}

glm::vec3 FindFurthestPoint(const OrientedBoundingBox& obb, const glm::vec3& normal)
{
  glm::vec3 p(0);
  for (int i = 0; i < 3; ++i) {
    if (glm::dot(obb.axis[i], normal) >= 0) {
      p += obb.axis[i] * obb.e[i];
    } else {
      p -= obb.axis[i] * obb.e[i];
    }
  }
  return p + obb.center;
}

enum Face {
  plusX,
  plusY,
  plusZ,
  minusX,
  minusY,
  minusZ,
};

struct Plane {
  glm::vec3 p;
  glm::vec3 n;
};

struct FaceQuery {
  Face face;
  float separationDistance;
  glm::vec3 pa;
  glm::vec3 pb;
  glm::vec3 normal;
};

struct EdgeQuery {
  float separationDistance;
  Segment edgeA;
  Segment edgeB;
  glm::vec3 normal;
};

/**
*/
bool QueryFaceDirections(const OrientedBoundingBox& a, const OrientedBoundingBox& b, FaceQuery& query)
{
  query.separationDistance = -FLT_MAX;
  float sign = 1;
  for (int s = 0; s < 2; ++s) {
    for (int i = 0; i < 3; ++i) {
      const glm::vec3 n = a.axis[i] * sign;
      const glm::vec3 pa = a.center + n * a.e[i];
      const glm::vec3 pb = FindFurthestPoint(b, -n);
      const float distance = glm::dot(pb - pa, n);
      if (distance > 0) {
        return false;
      }
      if (distance > query.separationDistance) {
        query.face = static_cast<Face>(i);
        query.separationDistance = distance;
        query.pa = pa;
        query.pb = pb;
        query.normal = n;
      }
    }
    sign = -1;
  }
  return true;
}

Segment GetEdge(const OrientedBoundingBox& obb, int n)
{
  static const float sign0[4][3] = {
    { 1, 1, 1}, {-1,-1, 1}, {-1, 1,-1}, { 1,-1,-1},
  };
  static const float sign1[3][3] = {
    {-1, 1, 1}, { 1,-1, 1}, { 1, 1,-1}
  };
  const int x = n / 3;
  const int a = n % 3;

  Segment seg;
  for (int i = 0; i < 3; ++i) {
    const glm::vec3 v = obb.axis[i] * obb.e[i] * sign0[x][i];
    seg.a += v;
    seg.b += v * sign1[a][i];
  }
  seg.a += obb.center;
  seg.b += obb.center;
  return seg;
}

float Project(const OrientedBoundingBox& obb, const glm::vec3& axis)
{
  static const float sign[8][3] = {
    { 1, 1, 1}, { 1, 1, -1}, { 1,-1, 1}, { 1,-1,-1},
    {-1, 1, 1}, {-1, 1, -1}, {-1,-1, 1}, {-1,-1,-1},
  };
  float bestD = -FLT_MAX;
  for (int i = 0; i < 8; ++i) {
    const glm::vec3 p =
      (obb.axis[0] * obb.e[0] * sign[i][0]) +
      (obb.axis[1] * obb.e[1] * sign[i][1]) +
      (obb.axis[2] * obb.e[2] * sign[i][2]);
    const float d = glm::dot(axis, p);
    if (d > bestD) {
      bestD = d;
    }
  }
  return bestD;
}

/**
*
*/
bool QueryEdgeDirections(const OrientedBoundingBox& a, const OrientedBoundingBox& b, EdgeQuery& query)
{
  query.separationDistance = -FLT_MAX;

  const glm::vec3 offsetDirection = glm::normalize(b.center - a.center) * 0.001f;

  const int edgeCount = 12;
  for (int ia = 0; ia < edgeCount; ++ia) {
    const Segment edgeA = GetEdge(a, ia);
    const glm::vec3 directionA = edgeA.b - edgeA.a;
    for (int ib = 0; ib < edgeCount; ++ib) {
      const Segment edgeB = GetEdge(b, ib);
      const glm::vec3 directionB = edgeB.b - edgeB.a;
      glm::vec3 axis = glm::cross(directionA, directionB);
      // 2�������s���Ă���ꍇ�͉������Ȃ�(���ʂ̔���ő�p�ł���̂�).
      if (glm::dot(axis, axis) < FLT_EPSILON) {
        continue;
      }

      axis = glm::normalize(axis);
      float f = glm::dot(axis, edgeA.a - a.center);
      if (f == 0) {
        const glm::vec3 v = edgeA.a - (a.center - offsetDirection);
        f = glm::dot(axis, v);
      }
      if (f < 0) {
        axis = -axis;
      }

      // ���ɒ��_���ˉe���ďd�Ȃ�𔻒肷��.
      const float t = std::abs(glm::dot(a.center - b.center, axis));
      const float da = Project(a, axis);
      const float db = Project(b, -axis);
      if (da + db > t) {
        continue;
      }

      {
        // �����������Ȃ��ꍇ�͏��O.
        glm::vec3 ba = glm::cross(edgeA.b, edgeA.a);
        float a = glm::dot(glm::dot(edgeB.a, ba), glm::dot(edgeB.b, ba));
        if (a >= 0) {
          continue;
        }
        glm::vec3 dc = glm::cross(edgeB.b, edgeB.a);
        float b = glm::dot(glm::dot(edgeA.a, dc), glm::dot(edgeA.b, dc));
        if (b >= 0) {
          continue;
        }
        // Ba-Ab��ʂ镽�ʂŕ��������Ƃ��A�c��2�_���������ɂȂ��ꍇ�͏��O.
        glm::vec3 cb = glm::cross(edgeB.a, edgeA.b);
        float c = glm::dot(glm::dot(edgeA.a, cb), glm::dot(edgeB.b, cb));
        if (c <= 0) {
          continue;
        }
      }

      const glm::vec3 pb = FindFurthestPoint(b, -axis);
      const float distance = glm::dot(axis, pb - edgeA.a);
      if (distance > 0) {
        return false;
      }
      if (distance > query.separationDistance) {
        query.separationDistance = distance;
        query.edgeA = edgeA;
        query.edgeB = edgeB;
        query.normal = axis;
      }
    }
  }
  return true;
}

#if 1
/**
*
*/
Result TestOBBOBB(const OrientedBoundingBox& a, const OrientedBoundingBox& b)
{
  FaceQuery qa;
  if (!QueryFaceDirections(a, b, qa)) {
    return {};
  }
  FaceQuery qb;
  if (!QueryFaceDirections(b, a, qb)) {
    return {};
  }
  qb.face = static_cast<Face>(qb.face + 3);
  EdgeQuery qe;
  if (!QueryEdgeDirections(a, b, qe)) {
    return {};
  }

  Result result;
  result.isHit = true;
  qa.separationDistance += 0.0002f;
  qb.separationDistance += 0.0001f;
  const bool isFaceContactA = qa.separationDistance > qe.separationDistance;
  const bool isFaceContactB = qb.separationDistance > qe.separationDistance;
  if (isFaceContactA && isFaceContactB) {
    // �ʂŐڐG.
    if (qa.separationDistance > qb.separationDistance) {
      result.normal = -qa.normal;
      result.pa = qa.pa;
      result.pb = qa.pb;
    } else {
      result.normal = qb.normal;
      result.pa = qb.pb;
      result.pb = qb.pa;
    }
#if 0
    // ���˖�(incident face)������.
    // ���˖ʂ�������ɂ́A���̑D�̂̂��ׂĂ̖ʂ��J��Ԃ��A�e�ʂ̖@���ƎQ�Ɩʂ̖@���̓��ς��v�Z���܂��B�ŏ��̃h�b�g�ς����ʂ����˖ʂ��`���܂��I

    // a�̖ʂɑ΂���b�̐N�������̂ق��������ꍇ�Ab�̓��˖ʂ�����.
    // b�ɑ΂���a�̐N�������ق��������ꍇ��a�̓��˖ʂ�����.
    if (qa.separationDistance > qb.separationDistance) {
      Face face;
      float minimumTheta = FLT_MAX;
      for (int i = 0; i < 3; ++i) {
        const float t0 = glm::dot(qa.normal, b.axis[i]);
        if (t0 < minimumTheta) {
          minimumTheta = t0;
          face = static_cast<Face>(i);
        }
        const float t1 = glm::dot(-qa.normal, b.axis[i]);
        if (t1 < minimumTheta) {
          minimumTheta = t1;
          face = static_cast<Face>(i + 3);
        }
      }
      // ���˖ʂ̒��_�����X�g�A�b�v.
      constexpr float sign[6] = { 1, 1, 1, -1, -1, -1 };
      constexpr int others[6][2] = {
        { 1, 2 }, { 1, 2 }, { 0, 2 }, { 0, 2 }, { 0, 1 }, { 0, 1 }
      };
      glm::vec3 v[4];
      const glm::vec3 axis0 = b.center + b.axis[face] * b.e[face] * sign[face];
      const glm::vec3 axis1 = b.axis[others[face][0]] * b.e[others[face][0]];
      const glm::vec3 axis2 = b.axis[others[face][1]] * b.e[others[face][1]];
      v[0] = axis0 + axis1 + axis2;
      v[1] = axis0 + axis1 - axis2;
      v[2] = axis0 - axis1 - axis2;
      v[3] = axis0 - axis1 + axis2;

      // �N���b�s���O���ʂ�I��.
      std::vector<Plane> planes;
      planes.reserve(4);
      for (int i = 0; i < 3; ++i) {
        if (i != (qa.face % 3)) {
          planes.push_back(Plane{ a.center + a.axis[i], a.axis[i] });
          planes.push_back(Plane{ a.center - a.axis[i], -a.axis[i] });
        }
      }

      // ���˖ʂ��N���b�s���O.
      std::vector<glm::vec3> points;
      points.reserve(8);
      for (auto& e : planes) {
        const float d0 = glm::dot(e.n, v[0] - e.p);
        const float d1 = glm::dot(e.n, v[1] - e.p);
        const float d2 = glm::dot(e.n, v[2] - e.p);
        const float d3 = glm::dot(e.n, v[3] - e.p);
        // d0 - d1
        // d1 - d2
        // d2 - d3
        // d3 - d0
      }
    }
#endif
  } else {
    // �ӂŐڐG.
    glm::vec3 c0, c1;
    ClosestPoint(qe.edgeA, qe.edgeB, c0, c1);
    result.normal = -qe.normal;
    result.pa = c1;
    result.pb = c0;
  }
  return result;
}
#else
/**
*
*/
Result TestOBBOBB(const OrientedBoundingBox& a, const OrientedBoundingBox& b)
{
  glm::mat3 R;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      R[i][j] = glm::dot(a.axis[i], b.axis[j]);
    }
  }
  glm::vec3 t = b.center - a.center;
  t = glm::vec3(glm::dot(t, a.axis[0]), glm::dot(t, a.axis[1]), glm::dot(t, a.axis[2]));

  glm::mat3 absR;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      absR[i][j] = std::abs(R[i][j]) + FLT_EPSILON;
    }
  }

  int bestIndex;
  float bestDistance = -FLT_MAX;
  glm::vec3 separatingAxis;

  for (int i = 0; i < 3; ++i) {
    const float ra = a.e[i];
    const float rb = b.e[0] * absR[i][0] + b.e[1] * absR[i][1] + b.e[2] * absR[i][2];
    const float d = std::abs(t[i]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = i;
      bestDistance = d;
      separatingAxis = a.axis[i];
    }
  }

  for (int i = 0; i < 3; ++i) {
    const float ra = a.e[0] * absR[0][i] + a.e[1] * absR[1][i] + a.e[2] * absR[2][i];
    const float rb = b.e[i];
    const float d = std::abs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = i + 3;
      bestDistance = d;
      separatingAxis = b.axis[i];
    }
  }

  { // L = A0 x B0
    const float ra = a.e[1] * absR[2][0] + a.e[2] * absR[1][0];
    const float rb = b.e[1] * absR[0][2] + b.e[2] * absR[0][1];
    const float d = std::abs(t[2] * R[1][0] - t[1] * R[2][0]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 6;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[0], b.axis[0]);
    }
  }

  { // L = A0 x B1
    const float ra = a.e[1] * absR[2][1] + a.e[2] * absR[1][1];
    const float rb = b.e[0] * absR[0][2] + b.e[2] * absR[0][0];
    const float d = std::abs(t[2] * R[1][1] - t[1] * R[2][1]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 7;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[0], b.axis[1]);
    }
  }

  { // L = A0 x B2
    const float ra = a.e[1] * absR[2][2] + a.e[2] * absR[1][2];
    const float rb = b.e[0] * absR[0][1] + b.e[1] * absR[0][0];
    const float d = std::abs(t[2] * R[1][2] - t[1] * R[2][2]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 8;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[0], b.axis[2]);
    }
  }

  { // L = A1 x B0
    const float ra = a.e[0] * absR[2][0] + a.e[2] * absR[0][0];
    const float rb = b.e[1] * absR[1][2] + b.e[2] * absR[1][1];
    const float d = std::abs(t[0] * R[2][0] - t[2] * R[0][0]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 9;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[1], b.axis[0]);
    }
  }

  { // L = A1 x B1
    const float ra = a.e[0] * absR[2][1] + a.e[2] * absR[0][1];
    const float rb = b.e[0] * absR[1][2] + b.e[2] * absR[1][0];
    const float d = std::abs(t[0] * R[2][1] - t[2] * R[0][1]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 10;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[1], b.axis[1]);
    }
  }

  { // L = A1 x B2
    const float ra = a.e[0] * absR[2][2] + a.e[2] * absR[0][2];
    const float rb = b.e[0] * absR[1][1] + b.e[1] * absR[1][0];
    const float d = std::abs(t[0] * R[2][2] - t[2] * R[0][2]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 11;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[1], b.axis[2]);
    }
  }

  { // L = A2 x B0
    const float ra = a.e[0] * absR[1][0] + a.e[1] * absR[0][0];
    const float rb = b.e[1] * absR[2][2] + b.e[2] * absR[2][1];
    const float d = std::abs(t[1] * R[0][0] - t[0] * R[1][0]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 12;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[2], b.axis[0]);
    }
  }

  { // L = A2 x B1
    const float ra = a.e[0] * absR[1][1] + a.e[1] * absR[0][1];
    const float rb = b.e[0] * absR[2][2] + b.e[2] * absR[2][0];
    const float d = std::abs(t[1] * R[0][1] - t[0] * R[1][1]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 13;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[2], b.axis[1]);
    }
  }

  { // L = A2 x B2
    const float ra = a.e[0] * absR[1][2] + a.e[1] * absR[0][2];
    const float rb = b.e[0] * absR[2][1] + b.e[1] * absR[2][0];
    const float d = std::abs(t[1] * R[0][2] - t[0] * R[1][2]);
    if (d > ra + rb) {
      return {};
    }
    if (d > bestDistance) {
      bestIndex = 14;
      bestDistance = d;
      separatingAxis = glm::cross(a.axis[2], b.axis[2]);
    }
  }

  // ��������������Ȃ�. �܂�AA��B�͌������Ă���.
  // ������Ԃ͈ȉ��̂����ꂩ:
  // - A�̂����ꂩ�̒��_��B�̓����ɂ���.
  // - B�̂����ꂩ�̒��_��A�̓����ɂ���.
  // - A�̂����ꂩ�̕ӂ�B�̖ʂƌ������Ă���.
  Result result;
  result.isHit = true;
  // TODO:�����ƌv�Z���邱��.
  result.normal = -glm::normalize(separatingAxis);
  switch (bestIndex) {
  case 0:
  case 1:
  case 2:
    result.pa = FindFurthestPoint(b, result.normal);
    result.pb = result.pa + result.normal * (glm::dot(result.pa - b.center, result.normal) - b.e[bestIndex]);
    break;
  case 3:
  case 4:
  case 5:
    result.pb = FindFurthestPoint(a, -result.normal);
    result.pa = result.pb - result.normal * (glm::dot(result.pb - a.center, -result.normal) - a.e[bestIndex - 3]);
    break;
  case 6:

    break;
  default:
    result.pa = b.center;
    result.pb = a.center;
    break;
  }
  return result;
}
#endif

/**
* �V�F�C�v���m���Փ˂��Ă��邩���ׂ�.
*
* @param a  ����Ώۂ̃V�F�C�v���̂P.
* @param b  ����Ώۂ̃V�F�C�v���̂Q.
* @param pa �Փ˂������W.
* @param pb �Փ˂������W.
*
* @retval true  �Փ˂���.
* @retval false �Փ˂��Ȃ�����.
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

/**
*
*
* @return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestSphereShape(const Sphere& a, const Shape& b)
{
  Result result;
  glm::vec3 p;
  switch (b.type) {
  case Shape::Type::sphere:
    if (TestSphereSphere(a, b.s)) {
      result.isHit = true;
      result.normal = glm::normalize(a.center - b.s.center);
      result.pa = b.s.center + result.normal * b.s.r;
      result.pb = a.center - result.normal * a.r;
    }
    break;
  case Shape::Type::capsule:
    if (TestSphereCapsule(a, b.c, &p)) {
      result.isHit = true;
      result.normal = glm::normalize(a.center - p);
      result.pa = p + result.normal * b.c.r;
      result.pb = a.center - result.normal * a.r;
    }
    break;
  case Shape::Type::obb:
    if (TestSphereOBB(a, b.obb, &p)) {
      result.isHit = true;
      result.pa = p;
      result.normal = glm::normalize(a.center - p);
    }
    break;
  }
  return result;
}

/**
*
*
* @return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleShape(const Capsule& a, const Shape& b)
{
  glm::vec3 p;
  switch (b.type) {
  case Shape::Type::sphere:
    if (TestSphereCapsule(b.s, a, &p)) {
      Result result;
      result.isHit = true;
      result.normal = glm::normalize(p - b.s.center);
      result.pa = b.s.center + result.normal * b.s.r;
      result.pb = p - result.normal * a.r;
      return result;
    }
    break;

  case Shape::Type::capsule:
    return TestCapsuleCapsule(a, b.c);

  case Shape::Type::obb:
    return TestCapsuleOBB(a, b.obb);
  }
  return Result{};
}

/**
*
*
* @return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestOBBShape(const OrientedBoundingBox& a, const Shape& b)
{
  Result result;
  glm::vec3 p;
  switch (b.type) {
  case Shape::Type::sphere:
    if (TestSphereOBB(b.s, a, &p)) {
      result.isHit = true;
      result.pa = p;
      result.normal = glm::normalize(p - b.s.center);
    }
    break;

  case Shape::Type::capsule:
    result = TestCapsuleOBB(b.c, a);
    result.normal *= -1;
    break;

  case Shape::Type::obb:
    return TestOBBOBB(a, b.obb);
  }
  return result;
}

/**
* �V�F�C�v���m���Փ˂��Ă��邩���ׂ�.
*
* @param a  ����Ώۂ̃V�F�C�v���̂P.
* @param b  ����Ώۂ̃V�F�C�v���̂Q.
*
* @return �Փˌ��ʂ�\��Result�^�̒l.
*/
Result TestShapeShape(const Shape& a, const Shape& b)
{
  switch (a.type) {
  case Shape::Type::sphere:
    return TestSphereShape(a.s, b);

  case Shape::Type::capsule:
    return TestCapsuleShape(a.c, b);

  case Shape::Type::obb:
    return TestOBBShape(a.obb, b);
  }
  return Result{};
}

/**
* �Փ˔��蓮��e�X�g.
*/
bool Test()
{
  struct local {
    static void test(bool a, bool b) {
      (a) == b ? printf("ok\n") : printf("fail\n");
    }
  };
  const AABB aabb{ glm::vec3(-2), glm::vec3(2) };
  const glm::vec3 p(-3, 0, 0);
  const glm::vec3 d(1, 0, 0);
  float tmin;
  glm::vec3 q;
  printf("IntersectRayAABB 0: ");
  local::test(IntersectRayAABB(p, d, aabb, tmin, q), true);

  const glm::vec3 d1(0, 1, 0);
  printf("IntersectRayAABB 1: ");
  local::test(IntersectRayAABB(p, d1, aabb, tmin, q), false);

  const glm::vec3 p2(2, 2, 2);
  const glm::vec3 d2(0,-1, 0);
  printf("IntersectRayAABB 2: ");
  local::test(IntersectRayAABB(p2, d2, aabb, tmin, q), true);
#undef TEST
  return true;
}

} // namespace Collision