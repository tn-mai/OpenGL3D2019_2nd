/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include <GL/glew.h>
#include "Mesh.h"
#include "Collision.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>

class Actor;
using ActorPtr = std::shared_ptr<Actor>;

/**
* �A�N�^�[�Ƀ��C�g�̉e�����󂯂�\�͂�t�^����\����.
*
* ���C�g���󂯂����A�N�^�[�͂�����܂܂��͌p������.
*/
struct LightReceiver
{
  void SetPointLightList(const std::vector<int>& v);
  void SetSpotLightList(const std::vector<int>& v);
  void UploadLightList(const Shader::ProgramPtr&);

  int pointLightCount = 0;
  int pointLightIndex[8] = { -1 };

  int spotLightCount = 0;
  int spotLightIndex[8] = { -1 };
};

/**
* �V�[���ɔz�u����I�u�W�F�N�g.
*/
class Actor
{
public:
  Actor(const std::string& name, int hp, const glm::vec3& pos,
    const glm::vec3& rot = glm::vec3(0), const glm::vec3& scale = glm::vec3(1));
  virtual ~Actor() = default;

  virtual void Update(float);
  virtual void UpdateDrawData(float);

  virtual void Draw(Mesh::DrawType = Mesh::DrawType::color);
  virtual void OnHit(const ActorPtr&, const glm::vec3&) {}
  virtual void OnHit(const ActorPtr& other, const Collision::Result& result) {
    OnHit(other, result.pb);
  }

public:
  std::string name; ///< �A�N�^�[�̖��O.
  glm::vec3 position = glm::vec3(0);
  glm::vec3 rotation = glm::vec3(0);
  glm::vec3 scale = glm::vec3(1);
  glm::vec3 velocity = glm::vec3(0); ///< ���x.
  int health = 0; ///< �̗�.
  Collision::Shape colLocal;
  Collision::Shape colWorld;

  // ������Ƃ̏Փ˔���p.
  Collision::Sphere bounds = Collision::Sphere{ glm::vec3(0), 0 };
};
using ActorPtr = std::shared_ptr<Actor>;

/**
* ���b�V���\���@�\�t���̃A�N�^�[.
*/
class StaticMeshActor : public Actor, public LightReceiver
{
public:
  StaticMeshActor(const Mesh::FilePtr& m, const std::string& name, int hp,
    const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0),
    const glm::vec3& scale = glm::vec3(1));
  virtual ~StaticMeshActor() = default;

  virtual void Draw(Mesh::DrawType) override;

  const Mesh::FilePtr& GetMesh() const { return mesh; }

private:
  Mesh::FilePtr mesh;
};
using StaticMeshActorPtr = std::shared_ptr<StaticMeshActor>;

/**
* �A�N�^�[���܂Ƃ߂đ��삷��N���X.
*/
class ActorList
{
public:
  // �C�e���[�^�[���`����.
  using iterator = std::vector<ActorPtr>::iterator;
  using const_iterator = std::vector<ActorPtr>::const_iterator;

  ActorList() = default;
  ~ActorList() = default;

  void Reserve(size_t);
  void Add(const ActorPtr&);
  bool Remove(const ActorPtr&);
  void Update(float);
  void UpdateDrawData(float);
  void Draw(Mesh::DrawType = Mesh::DrawType::color);
  void Draw(const Collision::Frustum& frustum, Mesh::DrawType = Mesh::DrawType::color);
  bool Empty() const { return actors.empty(); }

  // �C�e���[�^�[���擾����֐�.
  iterator begin() { return actors.begin(); }
  iterator end() { return actors.end(); }
  const_iterator begin() const { return actors.begin(); }
  const_iterator end() const { return actors.end(); }

  std::vector<ActorPtr> FindNearbyActors(const glm::vec3& pos, float maxDistance) const;

private:
  std::vector<ActorPtr> actors;

  static const int mapGridSizeX = 10;
  static const int mapGridSizeY = 10;
  static const int sepalationSizeY = 20;
  static const int sepalationSizeX = 20;
  std::vector<ActorPtr> grid[sepalationSizeY][sepalationSizeX];
  glm::ivec2 CalcMapIndex(const glm::vec3& pos) const;
};

using CollisionHandlerType =
  std::function<void(const ActorPtr&, const ActorPtr&, const glm::vec3&)>;
void DetectCollision(const ActorPtr& a, const ActorPtr& b,
  CollisionHandlerType handler = nullptr);
void DetectCollision(const ActorPtr& a, ActorList& b,
  CollisionHandlerType handler = nullptr);
void DetectCollision(ActorList& a, ActorList& b,
  CollisionHandlerType handler = nullptr);

#endif // ACTOR_H_INCLUDED