// OpenGL3D2019_2nd.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <Windows.h>
#include <iostream>
#include "Src/TitleScene.h"
#include "Src/GLFWEW.h"
#include "Src/SkeletalMesh.h"
#include "Src/Audio/Audio.h"

int main()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  window.Init(1280, 720, u8"アクションゲーム");

  //音声再生プログラムを初期化する.
  Audio::Engine& audioEngine = Audio::Engine::Instance();
  if (!audioEngine.Initialize()) {
    return 1;
  }

  // スケルタル・アニメーションを利用可能にする.
  if (!Mesh::SkeletalAnimation::Initialize()) {
    return 1;
  }

  SceneStack& sceneStack = SceneStack::Instance();
  sceneStack.Push(std::make_shared<TitleScene>());

  while (!window.ShouldClose()) {
    const float deltaTime = static_cast<float>(window.DeltaTime());
    window.UpdateTimer();

    // ESCキーが押されたら終了ウィンドウを表示.
    if (window.IsKeyPressed(GLFW_KEY_ESCAPE)) {
      if (MessageBox(nullptr, L"ゲームを終了しますか？", L"終了", MB_OKCANCEL) == IDOK) {
        break;
      }
    }

    // スケルタル・アニメーション用データの作成準備.
    Mesh::SkeletalAnimation::ResetUniformData();

    sceneStack.Update(deltaTime);

    // スケルタル・アニメーション用データをGPUメモリに転送.
    Mesh::SkeletalAnimation::UploadUniformData();

    // 音声再生プログラムを更新する.
    audioEngine.Update();

    // バックバッファを消去する.
    glClearColor(0.8f, 0.2f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // GLコンテキストのパラメータを設定.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    sceneStack.Render();
    window.SwapBuffers();
  }

  // スケルタル・アニメーションの利用を終了する.
  Mesh::SkeletalAnimation::Finalize();

  // 音声再生プログラムを終了する.
  audioEngine.Finalize();
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
