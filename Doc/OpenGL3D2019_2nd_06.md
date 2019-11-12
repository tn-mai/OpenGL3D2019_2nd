[OpenGL 3D 2019 後期 第06回]

# OpenGLの長い午後

今回は第05回までテキストを使った復習となります。そのため、テキストはC言語練習問題のみです。

## C言語練習問題

文字列を操作するプログラムがある. このプログラムを実行し、以下の設問に答えなさい.

```c++
#include <stdio.h>

int length(const char* s) {
  const char* p = s;
  while (*p) {
    ++p;
  }
  return p - s;
}

char* copy(const char* s, char* d) {
  while (*s) {
    *d = *s;
    ++s;
    ++d;
  }
  *d = '\0';
  return d;
}

char* reverse(const char* s, char* d) {
  const char* p = s + length(s);
  while (p != s) {
    --p;
    *d = *p;
    ++d;
  }
  *d = '\0';
  return d;
}

char* concat(const char* s0, const char* s1, char* d) {
  d = copy(s0, d);
  return copy(s1, d);
}

int main()
{
  char mem_a[100] = "Hello ";
  char mem_b[100] = "world!";

  copy(mem_b, mem_a + length(mem_a));
  printf("mem_a=%s\n", mem_a);

  reverse(mem_a, mem_b + 10);
  printf("mem_b+10=%s\n", mem_b + 10);

  concat(mem_a, mem_b, mem_b + 30);
  printf("mem_b+30=%s\n", mem_b + 30);
}
```

<div style="page-break-after: always"></div>

［OpenGL 2019後期 第05回 C言語練習問題 解答用紙］

クラス：<br>氏名：

問1. 以下はプログラムで使われている関数の説明文です. 説明文の右に、説明に対応する関数名を書きなさい. 対応する関数がない場合は「なし」と書きなさい.

||説明文 |対応する関数の名前|
|:-:|:-:|:----------:|
|1| 文字列をコピーする ||
|2| 文字列を消去する ||
|3| 文字列の長さを調べる ||
|4| 文字列を逆順にコピーする ||
|5| 2つの文字列を連結する ||
|6| 2つの文字列を比較する ||

問2. プログラムを実行したとき、「mem_b+30=」の右に出力される文字列を書きなさい.

A2.______________________________

<br>

問3. プログラムが終了したとき、配列変数mem_aに格納されている文字列を書きなさい.

A3.______________________________

<br>

問4. プログラムが終了したとき、配列変数mem_bに格納されている文字を、先頭から5文字書きなさい.

A4.______________________________

<br>

問5. 配列変数mem_aの20バイト目に、mem_bの10バイト目に格納されている文字列をコピーするプログラムを書きなさい.

A5.______________________________
