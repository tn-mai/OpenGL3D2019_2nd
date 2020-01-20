/**
* @file EventScript.h
*/
#ifndef EVENTSCRIPT_H_INCLUDED
#define EVENTSCRIPT_H_INCLUDED
#include "TextWindow.h"
#include <string>
#include <vector>

/**
* イベントスクリプト制御クラス.
*/
class EventScriptEngine
{
public:
static EventScriptEngine& Instance();

  bool Init(int maxVariableCount = 100);
  bool Execute(const char* filename);
  void Update(float deltaTime);
  void Draw();

  void SetVariable(int no, double value);
  double GetVariable(int no) const;
  bool IsFinished() const;
  
private:
  EventScriptEngine() = default;
  ~EventScriptEngine() = default;
  EventScriptEngine(const EventScriptEngine&) = delete;
  EventScriptEngine& operator=(const EventScriptEngine&) = delete;

  std::vector<double>  variables;
  std::string filename;
  std::wstring script;

  bool waitForInput = false;

  TextWindow textWindow;
};

#endif // EVENTSCRIPT_H_INCLUDED

