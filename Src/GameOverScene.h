/**
* @file GameOverScene.h
*/
#ifndef GAMEOVERSCENE_H_INCLUDED
#define GAMEOVERSCENE_H_INCLUDED
#include "Scene.h"
#include "Sprite.h"
#include "Font.h"
#include <vector>

/**
* ゲームオーバー画面.
*/
class GameOverScene : public Scene
{
public:
  GameOverScene() : Scene("GameOverScene") {}
  virtual ~GameOverScene() = default;

  virtual bool Initialize() override;
  virtual void ProcessInput() override;
  virtual void Update(float) override;
  virtual void Render() override;
  virtual void Finalize() override {}

private:
  std::vector<Sprite> sprites;
  SpriteRenderer spriteRenderer;
  FontRenderer fontRenderer;
};

#endif // GAMEOVERSCENE_H_INCLUDED