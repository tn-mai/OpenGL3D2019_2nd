/**
* @file Camera.h
*/
#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include <GL/glew.h>
#include <glm/glm.hpp>
// windows.h����`���Ă���near,far�𖳌���.
#undef far
#undef near

/**
* �J����.
*/
struct Camera
{
  glm::vec3 target = glm::vec3(100, 0, 100);
  glm::vec3 position = glm::vec3(100, 50, 150);
  glm::vec3 up = glm::vec3(0, 1, 0);
  glm::vec3 velocity = glm::vec3(0);

  // ��ʃp�����[�^.
  float width = 1280; ///< ��ʂ̕�(�s�N�Z����).
  float height = 720; ///< ��ʂ̍���(�s�N�Z����).
  float near = 1; ///< �ŏ�Z�l(���[�g��).
  float far = 500; ///< �ő�Z�l(���[�g��).

  // �ύX�\�ȃJ�����p�����[�^.
  float fNumber = 1.4f; ///< �G�t�E�i���o�[ = �J������F�l.
  float fov = glm::radians(30.0f); ///< �t�B�[���h�E�I�u�E�r���[ = �J�����̎���p(���W�A��)
  float sensorSize = 36.0f; ///< �Z���T�[�E�T�C�Y = �J�����̃Z���T�[�̉���(�~��).

  // Update�֐��Ōv�Z����J�����p�����[�^.
  float focalLength = 50.0f; ///< �t�H�[�J���E�����O�X = �œ_����(�~��).
  float aperture = 20.0f; ///< �A�p�[�`�� = �J��(�~��).
  float focalPlane = 10000.0f; ///< �t�H�[�J���E�v���[�� = �s���g�̍�������.

  void Update(const glm::mat4& matView);
};

#endif // CAMERA_H_INCLUDED
