/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace Collision {

/**
* ��.
*/
struct Sphere {
  glm::vec3 center = glm::vec3(0); ///< ���S���W.
  float r = 0; ///< ���a.
};

/**
* ����.
*/
struct Segment {
  glm::vec3 a = glm::vec3(0); ///< �����̎n�_.
  glm::vec3 b = glm::vec3(0); ///< �����̏I�_.
};

/**
* �J�v�Z��.
*/
struct Capsule {
  Segment seg; ///< �~�����̒��S�̐���.
  float r = 0; ///< �J�v�Z���̔��a.
};

/**
* �����s���E�{�b�N�X.
*/
struct AABB {
  glm::vec3 min;
  glm::vec3 max;
};

/**
* �L�����E�{�b�N�X.
*/
struct OrientedBoundingBox {
  glm::vec3 center = glm::vec3(0); ///< �{�b�N�X�̒��S.
  glm::vec3 axis[3] = { {1,0,0}, {0,1,0}, {0,0,1} }; ///< �{�b�N�X�̎�.
  glm::vec3 e = glm::vec3(0); ///< �e���̔��a.
};

/**
* �ėp�Փˌ`��.
*/
struct Shape
{
  enum class Type {
    none, ///< �`��Ȃ�.
    sphere, ///< ��.
    capsule, ///< �J�v�Z��.
    obb, ///< �L�����E�{�b�N�X.
  };
  Type type = Type::none; ///< ���ۂ̌`��.
  Sphere s; ///< ���̌`��f�[�^.
  Capsule c; ///< �J�v�Z���̌`��f�[�^.
  OrientedBoundingBox obb; ///< �L�����E�{�b�N�X�̌`��f�[�^.
};

// �`��쐬�֐�.
Shape CreateSphere(const glm::vec3&, float);
Shape CreateCapsule(const glm::vec3&, const glm::vec3&, float);
Shape CreateOBB(const glm::vec3& center, const glm::vec3& axisX,
  const glm::vec3& axisY, const glm::vec3& axisZ, const glm::vec3& e);

/**
* �Փˌ��ʂ�\���\����.
*/
struct Result
{
  bool isHit = false; ///< �Փ˂̗L��.
  glm::vec3 pa; ///< �`��A��̏Փ˓_.
  glm::vec3 na; ///< �`��A��̏Փ˕��ʂ̖@��.
  glm::vec3 pb; ///< �`��B��̏Փ˓_.
  glm::vec3 nb; ///< �`��B��̏Փ˕��ʂ̖@��.
};

bool TestSphereSphere(const Sphere&, const Sphere&);
bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p);
bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p);
bool TestShapeShape(const Shape&, const Shape&, glm::vec3* pa, glm::vec3* pb);
Result TestShapeShape(const Shape&, const Shape&);

glm::vec3 ClosestPointSegment(const Segment& seg, const glm::vec3& p);

#ifndef NDEBUG
bool Test();
#endif

} // namespace Collision

#endif // COLLISION_H_INCLUDED