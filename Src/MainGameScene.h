/**
* @file MainGameScene.h
*/
#ifndef MAINGAMESCENE_H_INCLUDED
#define MAINGAMESCENE_H_INCLUDED
#include "Scene.h"
#include "Sprite.h"
#include "Font.h"
#include "Mesh.h"
#include "Terrain.h"
#include "Actor.h"
#include "PlayerActor.h"
#include "JizoActor.h"
#include "Light.h"
#include "FrameBufferObject.h"
#include "Particle.h"
#include "TextWindow.h"
#include "Camera.h"
#include <random>
#include <vector>

/**
* メインゲーム画面.
*/
class MainGameScene : public Scene
{
public:
  MainGameScene() : Scene("MainGameScene") {}
  virtual ~MainGameScene() = default;

  virtual bool Initialize() override;
  virtual void ProcessInput() override;
  virtual void Update(float) override;
  virtual void Render() override;
  virtual void Finalize() override {}

  bool HandleJizoEffects(int id, const glm::vec3& pos);

private:
  void RenderMesh(Mesh::DrawType, const Collision::Frustum* = nullptr);

  std::mt19937 rand;
  int jizoId = -1; ///< 現在戦闘中のお地蔵様のID.
  bool achivements[4] = { false, false, false, false }; ///< 敵討伐状態.
  bool isClear = false;
  float clearTimer = 0;

  std::vector<Sprite> sprites;
  SpriteRenderer spriteRenderer;
  FontRenderer fontRenderer;
  Mesh::Buffer meshBuffer;
  Terrain::HeightMap heightMap;

  Camera camera;

  PlayerActorPtr player;
  ActorList enemies;
  ActorList trees;
  ActorList objects;

  LightBuffer lightBuffer;
  ActorList lights;

  ParticleSystem particleSystem;

  TextWindow textWindow;

  FrameBufferObjectPtr fboMain;
  FrameBufferObjectPtr fboDepthOfField;
  FrameBufferObjectPtr fboBloom[6][2];

  FrameBufferObjectPtr fboShadow;
};

#endif // MAINGAMESCENE_H_INCLUDED