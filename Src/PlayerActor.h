/**
* @file PlayerActor.h
*/
#ifndef PLAYERACTOR_H_INCLUDED
#define PLAYERACTOR_H_INCLUDED
#include "GLFWEW.h"
#include "SkeletalMeshActor.h"
#include "Terrain.h"
#include <memory>

/**
* �v���C���[�A�N�^�[.
*/
class PlayerActor : public SkeletalMeshActor
{
public:
  PlayerActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
    const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0));
  virtual ~PlayerActor() = default;

  virtual void Update(float) override;
  virtual void OnHit(const ActorPtr&, const glm::vec3&) override;
  virtual void OnHit(const ActorPtr&, const Collision::Result&) override;
  void Jump();
  void ProcessInput();
  void SetBoardingActor(ActorPtr);
  
  const ActorPtr& GetAttackCollision() const { return attackCollision; }

private:
  void CheckRun(const GamePad& gamepad);
  void CheckJump(const GamePad& gamepad);
  void CheckAttack(const GamePad& gamepad);

  /// �A�j���[�V�������.
  enum class State {
    idle, ///< ��~.
    run,  ///< �ړ�.
    jump, ///< �W�����v.
    attack, ///< �U��.
  };
  State state = State::idle; ///< ���݂̃A�j���[�V�������.
  bool isInAir = false;      ///< �󒆔���t���O.
  ActorPtr boardingActor;    ///< ����Ă���A�N�^�[.
  float moveSpeed = 5.0f;    ///< �ړ����x.
  ActorPtr attackCollision;  ///< �U������.
  float attackTimer = 0;     ///< �U������.
  
  glm::vec3 prevPosition = glm::vec3(0);

  const Terrain::HeightMap* heightMap = nullptr;
};
using PlayerActorPtr = std::shared_ptr<PlayerActor>;

#endif // PLAYERACTOR_H_INCLUDED