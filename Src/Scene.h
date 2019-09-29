/**
* @file Scene.h
*/
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include <memory>
#include <string>
#include <vector>

class SceneStack;

/**
* シーンの基底クラス.
*/
class Scene {
public:
  Scene(const char* name);
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  virtual ~Scene();

  virtual bool Initialize() = 0 {}
  virtual void ProcessInput() = 0 {}
  virtual void Update(float) = 0 {}
  virtual void Render() = 0 {}
  virtual void Finalize() = 0 {}

  virtual void Play();
  virtual void Stop();
  virtual void Show();
  virtual void Hide();

  const std::string& Name() const;
  bool IsActive() const;
  bool IsVisible() const;

private:
  std::string name;
  bool isVisible = true;
  bool isActive = true;
};
using ScenePtr = std::shared_ptr<Scene>;

/**
* シーン管理クラス.
*/
class SceneStack
{
public:
  static SceneStack& Instance();

  void Push(ScenePtr);
  void Pop();
  void Replace(ScenePtr);
  Scene& Current();
  const Scene& Current() const;
  size_t Size() const;
  bool Empty() const;

  void Update(float);
  void Render();

private:
  SceneStack();
  SceneStack(const SceneStack&) = delete;
  SceneStack& operator=(const SceneStack&) = delete;
  ~SceneStack() = default;

  std::vector<ScenePtr> stack;
};

#endif // SCENE_H_INCLUDED