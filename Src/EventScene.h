/**
* @file EventScene.h
*/
#ifndef EVENTSCENE_H_INCLUDED
#define EVENTSCENE_H_INCLUDED
#include "Scene.h"
#include "TextWindow.h"
#include <string>

/**
* イベントシーン.
*/
class EventScene : public Scene
{
public:
  explicit EventScene(const char* filename);
  ~EventScene() = default;

  virtual bool Initialize() override;
  virtual void ProcessInput() override {}
  virtual void Update(float) override;
  virtual void Render() override {}
  virtual void Finalize() override {}

private:
  std::string filename;
};

#endif // EVENTSCENE_H_INCLUDED

