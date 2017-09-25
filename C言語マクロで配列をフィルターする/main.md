# C言語マクロで配列をフィルターする

## 概要

久しぶりのC言語との邂逅でマクロに触れることができた。
配列(begin, end)に対してfirstとfilterしてみたメモ。

## 実装

テストコードはGitHubにあります。
https://github.com/takayoshiotake/qiita/tree/master/C%E8%A8%80%E8%AA%9E%E3%83%9E%E3%82%AF%E3%83%AD%E3%81%A7%E9%85%8D%E5%88%97%E3%82%92%E3%83%95%E3%82%A3%E3%83%AB%E3%82%BF%E3%83%BC%E3%81%99%E3%82%8B/src

### first

配列を捜査して条件にマッチする最初の要素のポインタを取得します。

`_Type`には型名が入ります。`_begin`, `_end`は配列の先頭と末尾の次を指すポインタです。C++のイテレータと捉えていただければ。
`_first`には結果が格納されます。型は`_Type**`になります。残りの`_Match`と`_arg`は要素のチェックに利用します。

**bb_algorithm.h**

```C
#define BB_list_first(_Type, _begin, _end, _first, _Match, _arg) \
do { \
    _Type* _ptr; \
    for (_ptr = _begin; _ptr < _end; ++_ptr) { \
        if (_Match(_ptr, _arg)) { \
            *_first = _ptr; \
            break; \
        } \
    } \
} while (0)
```

このマクロで

```C
static int test_first(int* lhs, void const* arg) {
    return *lhs >= *(int*)arg ? 1 : 0;
}

int main(int argc, const char * argv[]) {
    int list[5] = { 2, 3, 5, 7, 11 };
    int* first = NULL;
    int arg = 5;
    BB_list_first(int, &list[0], &list[sizeof(list)/sizeof(*list)], &first, test_first, &arg);
    if (first) {
        printf("  index: %d\n", (int)(first - list));
    }
    return 0;
}
```

すると

```shell-session
index: 2
```

になります。(GitHubのコードから簡略化してます)

`_Match`にブロックスやラムダを利用したいのですが関数ポインタで妥協します。マクロに正しい引数を渡せるようになるまでコンパイラーと格闘が必要です。

### list

基本はfirstと同じ。条件にマッチする要素を`_filtered`が指す配列に格納していく。

**bb_algorithm.h**

```C
#define BB_list_filter(_Type, _begin, _end, _filtered, _Match, _arg) \
do { \
    _Type* _ptr; \
    for (_ptr = _begin; _ptr < _end; ++_ptr) { \
        if (_Match(_ptr, _arg)) { \
            *_filtered = *_ptr; \
            ++_filtered; \
        } \
    } \
} while (0)
```

このマクロで

```C
static int test_apple_filter(char const** lhs, void const* arg) {
    return strstr(*lhs, "apple") != NULL ? 1 : 0;
}

int main(int argc, const char * argv[]) {
    char const* list[4] = { "I have a pen", "I have an apple", "ugh", "apple pen" };
    char const* filtered[4];
    char const** filtered_end = filtered;
    BB_list_filter(char const*, &list[0], &list[sizeof(list)/sizeof(*list)], filtered_end, test_apple_filter, NULL);
    {
        char const** ptr;
        for (ptr = filtered; ptr < filtered_end; ++ptr) {
            printf("  %s\n", *ptr);
        }
    }
    return 0;
}
```

すると

```shell-session
I have an apple
apple pen
```

になります。(GitHubのコードから簡略化してます)

firstよりもマクロの引数が難しいです。`filtered_end`を初期化してから渡すことが要求されてます。

## まとめ

マクロをC++のテンプレートのように利用したかったのですが、コンパイルエラーを解消するのが難しいです。
