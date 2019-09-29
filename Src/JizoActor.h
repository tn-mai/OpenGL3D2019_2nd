/**
* @file JizoActor.h
*/
#ifndef JIZOACTOR_H_INCLUDED
#define JIZOACTOR_H_INCLUDED
#include "Actor.h"
#include "Mesh.h"

// êÊçsêÈåæ.
class MainGameScene;

/**
* Ç®ínë†ól.
*/
class JizoActor : public StaticMeshActor
{
public:
  JizoActor(const Mesh::FilePtr& m, const glm::vec3& pos, int id,
    MainGameScene* p);
  virtual ~JizoActor() = default;

  virtual void OnHit(const ActorPtr& other, const glm::vec3& p) override;

private:
  int id = -1;
  MainGameScene* parent = nullptr;
  bool isActivated = false;
};
using JizoActorPtr = std::shared_ptr<JizoActor>;

#endif // JIZOACTOR_H_INCLUDED