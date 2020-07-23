/**
* @file Particle.h
*/
#ifndef PARTICLE_H_INCLUDED
#define PARTICLE_H_INCLUDED
#include <GL/glew.h>
#include "Texture.h"
#include "BufferObject.h"
#include "Shader.h"
#include "Sprite.h"
#include "Collision.h"
#include <glm/glm.hpp>
#include <list>
#include <memory>

class Particle;
class ParticleEmitter;
using ParticleEmitterPtr = std::shared_ptr<ParticleEmitter>;
class ParticleSystem;

/**
* �p�[�e�B�N���̃p�����[�^�[.
*/
struct ParticleParameter {
  float lifetime = 1; // ��������.
  glm::vec3 velocity = glm::vec3(0, 2, 0); // ���x.
  glm::vec3 acceleration = glm::vec3(0); // �����x.
  glm::vec2 scale = glm::vec2(1); // �傫��.
  float rotation = 0; // ��].
  glm::vec4 color = glm::vec4(1); // �F�ƕs�����x.
};

/**
* �p�[�e�B�N���E�G�~�b�^�[�̃p�����[�^�[.
*/
struct ParticleEmitterParameter {
  int id = 0;
  glm::vec3 position = glm::vec3(0);
  glm::vec3 rotation = glm::vec3(0);
  float duration = 1.0f; // �p�[�e�B�N������o����b��.
  bool loop = true; // duration�o�ߌ�Ƀ��[�v�Đ�����Ȃ�true�A��~����Ȃ�false.
  float emissionsPerSecond = 5.0f; // �b������̃p�[�e�B�N�����o��.
  float angle = glm::radians(15.0f); // �~���̊p�x.
  float radius = 0.5f; // �~���̔��a.
  float gravity = 9.8f; // �d��.
  std::string imagePath; // �e�N�X�`���E�t�@�C����.

  GLenum srcFactor = GL_SRC_ALPHA;
  GLenum dstFactor = GL_ONE_MINUS_SRC_ALPHA;
  glm::ivec2 tiles = glm::ivec2(1); // �e�N�X�`���̏c�Ɖ��̕�����.
};

/**
* �p�[�e�B�N��.
*/
class Particle {
public:
  friend ParticleSystem;
  friend ParticleEmitter;

  Particle(const ParticleParameter& pp, const glm::vec3& pos, const Rect& r);
  virtual ~Particle() = default;

  void Update(float deltatime);
#if 0
  // ���W�̐ݒ�E�擾
  void Position(const glm::vec3& p) { position = p; }
  const glm::vec3& Position() const { return position; }

  void Velocity(const glm::vec3& v) { velocity = v; }
  const glm::vec3& Velocity() const { return velocity; }

  void Acceleration(const glm::vec3& a) { acceleration = a; }
  const glm::vec3& Acceleration() const { return acceleration; }

  // ��]�̐ݒ�E�擾
  void Rotation(float r) { rotation = r; }
  float Rotation() const { return rotation; }

  // �g�嗦�̐ݒ�E�擾
  void Scale(const glm::vec2& s) { scale = s; }
  const glm::vec2& Scale() const { return scale; }

  // �F�̐ݒ�E�擾
  void Color(const glm::vec4& c) { color = c; }
  const glm::vec4& Color() const { return color; }

  // ��`�̐ݒ�E�擾
  void Rectangle(const Rect& r) { rect = r; }
  const Rect& Rectangle() const { return rect; }
#endif
  bool IsDead() const { return lifetime <= 0; }

private:
  Rect rect = { glm::vec2(0), glm::vec2(0) };
  glm::vec3 position = glm::vec3(0);
  float lifetime = 1; // ��������.
  glm::vec3 velocity = glm::vec3(0, 2, 0); // ���x.
  glm::vec3 acceleration = glm::vec3(0); // �����x.
  glm::vec2 scale = glm::vec2(1); // �傫��.
  float rotation = 0; // ��].
  glm::vec4 color = glm::vec4(1); // �F�ƕs�����x.
};

/**
* �p�[�e�B�N�����o�N���X.
*/
class ParticleEmitter
{
public:
  friend ParticleSystem;

  ParticleEmitter(const ParticleEmitterParameter& ep, const ParticleParameter& pp);
  ~ParticleEmitter() = default;

  void Update(float deltatime);
  void Draw();

  // ���W�̐ݒ�E�擾
  void Position(const glm::vec3& p) { ep.position = p; }
  const glm::vec3& Position() const { return ep.position; }
#if 0
  // ��]�̐ݒ�E�擾
  void Rotation(const glm::vec3& r) { ep.rotation = r; }
  const glm::vec3& Rotation() const { return ep.rotation; }

  void Angle(float a) { ep.angle = a; }
  float Angle() const { return ep.angle; }

  void Radius(float r) { ep.radius = r; }
  float Radius() const { return ep.radius; }

  void Gravity(float g) { ep.gravity = g; }
  float Gravity() const { return ep.gravity; }
#endif
  void Id(int n) { ep.id = n; }
  int Id() const { return ep.id; }

  bool IsDead() const { return !ep.loop && timer >= ep.duration && particles.empty(); }

private:
  void AddParticle();

  ParticleEmitterParameter ep; // �G�~�b�^�[�̃p�����[�^�[.
  ParticleParameter pp;        // �p�[�e�B�N���̃p�����[�^�[.

  Texture::Image2DPtr texture; // �p�[�e�B�N���p�e�N�X�`��.
  float interval = 0;          // �p�[�e�B�N���̔����Ԋu(�b).
  float timer = 0;             // �o�ߎ���(�b).
  float emissionTimer = 0;     // �p�[�e�B�N�������^�C�}�[(�b).

  size_t count = 0;      // �`�悷��C���f�b�N�X��.
  size_t baseVertex = 0; // �`��̊�ƂȂ钸�_�̃I�t�Z�b�g.

  std::list<Particle>  particles; // �p�[�e�B�N�����X�g.
};

/**
* �p�[�e�B�N���Ǘ��N���X.
*/
class ParticleSystem
{
public:
  ParticleSystem() = default;
  ~ParticleSystem() = default;

  bool Init(size_t maxParticleCount);
  ParticleEmitterPtr Add(const ParticleEmitterParameter& ep, const ParticleParameter& pp);
  ParticleEmitterPtr Find(int id) const;
  void Remove(const ParticleEmitterPtr&);
  void Clear();
  void Update(const Collision::Frustum& frustum, float deltatime);
  void Draw(const glm::mat4& matProj, const glm::mat4& matView);

private:
  BufferObject vbo;
  BufferObject ibo;
  VertexArrayObject vao;
  Shader::ProgramPtr program;

  std::list<ParticleEmitterPtr> emitters;
};

#endif // PARTICLE_H_INCLUDED
