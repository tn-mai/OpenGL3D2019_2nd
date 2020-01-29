/**
* @file EventScript.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "EventScript.h"
#include "GLFWEW.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

/**
* スクリプトエンジンのシングルトン・インスタンスを取得する.
*
* @return スクリプトエンジンのシングルトン・インスタンス.
*/
EventScriptEngine& EventScriptEngine::Instance()
{
  static EventScriptEngine instance;
  return instance;
}

/**
* スクリプトエンジンを初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool EventScriptEngine::Init(int maxVariableCount)
{
  if (isInitialized) {
    std::cerr << "[エラー] EventScriptEngineは既に初期化されています.\n";
    return false;
  }

  variables.resize(maxVariableCount, 0);
  filename.reserve(256);
  script.reserve(2048);
  if (!textWindow.Init("Res/TextWindow.tga", glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0))) {
    std::cerr << "[エラー]" << __func__ << ":スクリプトエンジンの初期化に失敗.\n";
    return false;
  }
  isInitialized = true;
  return true;
}

/**
* スクリプト命令の引数に値を設定する.
*
* @param arg スクリプト命令の引数.
* @param str 設定する値を含む文字列.
*/
void EventScriptEngine::Set(Argument& arg, const char* s)
{
  if (s[0] == '[') {
    VariableId id;
    if (sscanf(s, "[%d]", &id) >= 1) {
      arg = id;
    }
  } else {
    Number n;
    if (sscanf(s, "%lf", &n) >= 1) {
      arg = n;
    }
  }
}

/**
* スクリプト命令の引数から値を取得する.
*
* @param arg スクリプト命令の引数.
*
* @return 引数から直接・間接に得られた値.
*/
EventScriptEngine::Number EventScriptEngine::Get(const Argument& arg) const
{
  if (const auto p = std::get_if<VariableId>(&arg)) {
    return variables[*p];
  } else if (const auto p = std::get_if<Number>(&arg)) {
    return *p;
  }
  return 0;
}

/**
* スクリプト命令の引数に関係演算子を設定する.
*
* @param arg スクリプト命令の引数.
* @param str 設定する値を含む文字列.
*/
void EventScriptEngine::SetOperator(Argument& arg, const char* str)
{
  if (str[1] == '\0') {
    switch (str[0]) {
    case '<': arg = Operator::less; break;
    case '>': arg = Operator::greater; break;
    case '+': arg = Operator::add; break;
    case '-': arg = Operator::sub; break;
    case '*': arg = Operator::mul; break;
    case '/': arg = Operator::div; break;
    default: /* 何もしない */ break;
    }
  } else if (str[1] == '=' && str[2] == '\0') {
    switch (str[0]) {
    case '=': arg = Operator::equal; break;
    case '!': arg = Operator::notEqual; break;
    case '<': arg = Operator::lessEqual; break;
    case '>': arg = Operator::greaterEqual; break;
    default: /* 何もしない */ break;
    }
  }
}

/**
* イベント・スクリプトを実行する.
*
* @param filename スクリプト・ファイル名.
*
* @retval true  実行に成功.
* @retval false 実行に失敗.
*/
bool EventScriptEngine::RunScript(const char* filename)
{
  if (!isInitialized) {
    return false;
  }

  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":スクリプトファイル" << filename << "を読み込めません.\n";
    return false;
  }

  script.clear();

  size_t lineCount = 0;
  std::string line;
  char buf[1000];
  char a[20], b[20], c[20], op[20];
  std::vector<size_t> stackIfElse;
  stackIfElse.reserve(100);
  while (std::getline(ifs, line)) {
    line.erase(0, line.find_first_not_of(" \t\n"));
    ++lineCount;

    Instruction inst;
    int n = sscanf(line.c_str(), "p %999[^\n]", buf);
    if (n >= 1) {
      const std::string s = std::regex_replace(buf, std::regex(R"(\\n)"), "\n");
      setlocale(LC_CTYPE, "ja-JP");
      const size_t size = mbstowcs(nullptr, s.c_str(), 0);
      std::wstring text(size, L'\0');
      mbstowcs(&text[0], s.c_str(), size);
      inst.type = InstructionType::print;
      inst.arguments[0] = text;
      script.push_back(inst);
      continue;
    }
    n = sscanf(line.c_str(), "set_camera_position %19s %19s %19s", a, b, c);
    if (n >= 3) {
      inst.type = InstructionType::setCameraPosition;
      Set(inst.arguments[0], a);
      Set(inst.arguments[1], b);
      Set(inst.arguments[2], c);
      script.push_back(inst);
      continue;
    }
    n = sscanf(line.c_str(), "set_camera_target %19s %19s %19s", a, b, c);
    if (n >= 3) {
      inst.type = InstructionType::setCameraTarget;
      Set(inst.arguments[0], a);
      Set(inst.arguments[1], b);
      Set(inst.arguments[2], c);
      script.push_back(inst);
      continue;
    }
    n = sscanf(line.c_str(), "[%19[^]]] = %19[^=!<>+-*/] %19[=!<>+-*/] %19[^=!<>+-*/]", a, b, op, c);
    if (n >= 4) {
      inst.type = InstructionType::expression;
      inst.arguments[0] = static_cast<VariableId>(atoi(a));
      Set(inst.arguments[1], b);
      SetOperator(inst.arguments[2], op);
      Set(inst.arguments[3], c);
      script.push_back(inst);
      continue;
    }
    n = sscanf(line.c_str(), "[%19[^]]] = %19s", a, b);
    if (n >= 2) {
      inst.type = InstructionType::assign;
      inst.arguments[0] = static_cast<VariableId>(atoi(a));
      Set(inst.arguments[1], b);
      script.push_back(inst);
      continue;
    }
    n = sscanf(line.c_str(), "if %19s %19s %19s", a, b, c);
    if (n >= 3) {
      inst.type = InstructionType::beginIf;
      Set(inst.arguments[0], a);
      SetOperator(inst.arguments[1], b);
      Set(inst.arguments[2], c);
      stackIfElse.push_back(script.size());
      script.push_back(inst);
      continue;
    }
    if (strncmp(line.c_str(), "endif", 5) == 0) {
      const size_t p = stackIfElse.back();
      stackIfElse.pop_back();
      script[p].jump = script.size();
      continue;
    }
    if (strncmp(line.c_str(), "else", 4) == 0) {
      const size_t posIf = stackIfElse.back();
      stackIfElse.back() = script.size();
      inst.type = InstructionType::jump;
      script.push_back(inst);
      script[posIf].jump = script.size();
      continue;
    }
  }

  this->filename = filename;
  programCounter = 0;
  isFinished = false;
  std::cout << "[INFO]" << __func__ << ":スクリプトファイル" << filename << "を実行.\n";
  return true;
}

/**
* スクリプトエンジンの状態を更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*
* 1行に1命令. 改行で行の終了.
* すべての変数はdouble型. 変数は[変数番号]と書く.
*
* 命令一覧:
* - p           テキストウィンドウに文章を表示.
*               文中に[変数番号]と書くと変数の内容が出力される.
* - if a op b   条件分岐. 条件が真のときendifまで実行. opは=,!=,<,>,<=,>=のいずれか.
* - else        条件分岐の中間.
* - endif       条件分岐の終端.
* - a = b       変数の代入. aは変数、bは変数または数値.
* - a = b op c   四則演算. aは変数、b, cは変数または数値. opは+,-,*,/のいずれか.
* - select a b...N                選択肢の表示. aは変数、b～Nは文章.
* - spawn_actor name type pos rot アクターを出現させる.
* - kill_actor name               アクターを消す.
* - move_actor name pos time      アクターを移動させる. 
* - wait time                     指定された時間待機.
* - # comment                     行コメント.
*/
void EventScriptEngine::Update(float deltaTime)
{
  if (!isInitialized) {
    return;
  }

  if (script.empty() || isFinished) {
    return;
  }

  for (bool yield = false; !yield; ) {
    if (programCounter >= script.size()) {
      isFinished = true;
      return;
    }

    const auto& inst = script[programCounter];
    switch (inst.type) {
    case InstructionType::print:
      if (textWindow.IsOpen()) {
        if (textWindow.IsFinished()) {
          const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
          if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
            textWindow.Close();
            ++programCounter;
            continue;
          }
        }
        yield = true;
      } else {
        const auto p = std::get_if<Text>(&inst.arguments[0]);
        if (!p) {
          std::cerr << "[エラー]" << __func__ << "print命令の引数はText型でなくてはなりません.\n";
          ++programCounter;
          break;
        }
        // 文章中の変数番号を数値に置き換える.
        const std::wregex re(LR"*(\[(\d+)\])*");
        std::wsregex_iterator itr(p->begin(), p->end(), re);
        std::wsregex_iterator end;
        if (itr != end) {
          std::wstring tmp;
          tmp.reserve(p->size());
          for (;;) {
            const auto& m = *itr;
            tmp += m.prefix();
            wchar_t* p = nullptr;
            const size_t n = static_cast<size_t>(std::wcstol(m[1].str().c_str(), &p, 10));
            if (n < variables.size()) {
              wchar_t buf[20];
              if (std::floor(variables[n]) == variables[n]) {
                swprintf(buf, 20, L"%d", static_cast<int>(variables[n]));
              } else {
                swprintf(buf, 20, L"%.2lf", variables[n]);
              }
              tmp += buf;
            }
            ++itr;
            if (itr == end) {
              tmp += m.suffix();
              break;
            }
          }
          textWindow.Open(tmp.c_str());
        } else {
          textWindow.Open(p->c_str());
        }
        yield = true;
      }
      break;

    case InstructionType::expression:
      if (const auto a = std::get_if<VariableId>(&inst.arguments[0])) {
        if (const auto op = std::get_if<Operator>(&inst.arguments[2])) {
          const Number b = Get(inst.arguments[1]);
          const Number c = Get(inst.arguments[3]);
          switch (*op) {
          case Operator::add: variables[*a] = b + c; break;
          case Operator::sub: variables[*a] = b - c; break;
          case Operator::mul: variables[*a] = b * c; break;
          case Operator::div: variables[*a] = b / c; break;
          }
        }
      }
      ++programCounter;
      break;

    case InstructionType::assign:
      if (const auto a = std::get_if<VariableId>(&inst.arguments[0])) {
        variables[*a] = Get(inst.arguments[1]);
      }
      ++programCounter;
      break;

    case InstructionType::beginIf:
      if (const auto op = std::get_if<Operator>(&inst.arguments[1])) {
        const Number a = Get(inst.arguments[0]);
        const Number b = Get(inst.arguments[2]);
        bool result = false;
        switch (*op) {
        case Operator::equal: result = a == b; break;
        case Operator::notEqual: result = a != b; break;
        case Operator::less: result = a < b; break;
        case Operator::lessEqual: result = a <= b; break;
        case Operator::greater: result = a > b; break;
        case Operator::greaterEqual: result = a >= b; break;
        }
        if (!result) {
          programCounter = inst.jump;
          break;
        }
      }
      ++programCounter;
      break;

    case InstructionType::jump:
      programCounter = inst.jump;
      break;

    case InstructionType::setCameraPosition:
      if (camera) {
        camera->position.x = static_cast<float>(Get(inst.arguments[0]));
        camera->position.y = static_cast<float>(Get(inst.arguments[1]));
        camera->position.z = static_cast<float>(Get(inst.arguments[2]));
      }
      ++programCounter;
      break;

    case InstructionType::setCameraTarget:
      if (camera) {
        camera->target.x = static_cast<float>(Get(inst.arguments[0]));
        camera->target.y = static_cast<float>(Get(inst.arguments[1]));
        camera->target.z = static_cast<float>(Get(inst.arguments[2]));
      }
      ++programCounter;
      break;

    default:
      ++programCounter;
      break;
    }
  }

  textWindow.Update(deltaTime);
}

/**
* スクリプトエンジンを描画する.
*/
void EventScriptEngine::Draw()
{
  if (!isInitialized) {
    return;
  }

  textWindow.Draw();
}

/**
* スクリプトの実行が完了したか調べる.
*
* @retval true  実行完了.
* @retval false 実行中、またはスクリプトが読み込まれていない.
*/
bool EventScriptEngine::IsFinished() const
{
  return isFinished;
}

/**
* スクリプト変数に値を設定する.
*
* @param no    変数番号(0～初期化時に設定した最大数).
* @param value 設定する値.
*/
void EventScriptEngine::SetVariable(int no, double value)
{
  if (no < 0 || no >= static_cast<int>(variables.size())) {
    return;
  }
  variables[no] = value;
}

/**
* スクリプト変数の値を取得する.
*
* @param no    変数番号(0～初期化時に設定した最大数).
*
* @return no番の変数に設定されている値.
*/
double EventScriptEngine::GetVariable(int no) const
{
  if (no < 0 || no >= static_cast<int>(variables.size())) {
    return 0;
  }
  return variables[no];
}
