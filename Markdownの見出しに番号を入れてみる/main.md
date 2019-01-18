# Markdownの見出しに番号を入れてみる

普段 Markdown を書くのに Visual Studio Code + Markdown Preview Enhanced を使ってとても捗ってます。

しかし、見出しに番号を自動挿入する方法がわからず、とりあえず次のように見出しに番号を振ってみました。

```sample0.md
# 1. HELLO
## 1.1. WORLD
# 2. FOO
## 2.1. BAR
```

短いドキュメントならこれでも良いと思う。

## CSS will help me

見出しは h1, h2 なので、style を書けばいけるはず！

```sample1.md
# PREFACE
#
## (NO TITLE)
##
# TITLE
## SUBTITLE
# A1. APPENDIX
## A1.1. omake-1
## A1.2. omake-2
# A2. APPENDIX

<style>
body {
    counter-reset: h1;
}
h1 {
    counter-reset: h2;
}
h1:before {
    counter-increment: h1;
    content: counter(h1) ". ";
}
h2:before {
    counter-increment: h2;
    content: counter(h1) "." counter(h2) ". ";
}
</style>
```

↓ (HTML にするとこんな感じになるはず; Chrome で確認; 4つめの ## が h1 になって 3. になるのは見なかった事にする)

```
1. PREFACE
2.
2.1. (NO TITLE)
3.
4. TITLE
4.1. SUBTITLE
5. A1. APPENDIX
5.1. A1.1. omake-1
5.2. A1.2. omake-2
6. A2. APPENDIX
```

これだと APPENDIX にも番号が振られてしまう。PREFACE も番号振りたくないかも。

しかも、1. とか 2. とかは before であるからか、マウスで選択(コピペ)できないっぽい。(Chrome で確認)

## JavaScript will help me

CSS の隣接セレクタを駆使してみたりしたけど、script 書くのが好きだなぁと自己納得しました。

```sample2.md
# PREFACE
# {#}.
## {#}. (NO TITLE)
## {#}.
# {#}. TITLE
## {#}. SUBTITLE

<hr class="md-reset-numbering">

# A{#}. APPENDIX
## A{#}. omake-1
## A{#}. omake-2
# A{#}. APPENDIX

<style>
hr.md-reset-numbering {
    display: none;
}
</style>
<script>
var h1Count = 0
var h2Count = 0
for (let hx of document.querySelectorAll('h1,h2,hr.md-reset-numbering')) {
    const f = {
        "h1": () => {
            if (hx.innerText.match(/{#}/)) {
                ++h1Count
                h2Count = 0
                hx.innerText = hx.innerText.replace(/{#}/, `${h1Count}`)
            }
        },
        "h2": () => {
            if (hx.innerText.match(/{#}/)) {
                ++h2Count
                hx.innerText = hx.innerText.replace(/{#}/, `${h1Count}.${h2Count}`)
            }
        },
        "hr": () => {
            h1Count = 0
        }
    }
    f[hx.tagName.toLowerCase()]()
}
</script>
```

↓ (HTML にするとこんな感じになるはず; Chrome で確認)

```
PREFACE
1.
1.1. (NO TITLE)
1.2.
2. TITLE
2.1. SUBTITLE
A1. APPENDIX
A1.1. omake-1
A1.2. omake-2
A2. APPENDIX
```

ただし、Visual Studio Code + Markdown Preview Enhanced の MPE Preview では script が実行されないので

```
PREFACE
{#}.
{#}. (NO TITLE)

{#}.

{#}. TITLE
{#}. SUBTITLE

A{#}. APPENDIX
A{#}. omake-1

A{#}. omake-2

A{#}. APPENDIX
```

のように見えてしまうのが残念です。

## まとめ

JavaScript パターンが良さそうに感じましたが、MPE Preview では script で振った番号が表示されないのでちょっと不安になります。

ある程度の規模のドキュメントになると、手で番号を振るのは肉体的にもつらいところです。

ある程度の規模のドキュメントは Word で書くのが良いのかも知れませんね。
