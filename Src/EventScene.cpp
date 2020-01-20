/**
* @file EventScene.cpp
*/
#include "EventScene.h"
#include "EventScript.h"
#include <fstream>
#include <sstream>

/**
* コンストラクタ.
*
* @param filename スクリプトファイル名.
*/
EventScene::EventScene(const char* filename) :
  Scene("EventScene"), filename(filename)
{
}

/**
*
*/
bool EventScene::Initialize()
{
  return EventScriptEngine::Instance().Execute(filename.c_str());
}

/**
*
*
* スクリプト一覧:
* - TEXT                文章を表示し、表示終了を待つ.
* - WAIT                キー入力待ち.
* - IF a op b ～ ENDIF  aとbが比較演算子opを満たすならENDIFまでを実行.
* - a = b op c              aにbを代入演算子opに従って代入.
*
* 100個のdouble型変数を用意. スクリプト中では'var0'～'var99'の名前で参照できる. 初期値は0.
* 
*
*/
void EventScene::Update(float deltaTime)
{
  if (EventScriptEngine::Instance().IsFinished()) {
    SceneStack::Instance().Pop();
  }
}

