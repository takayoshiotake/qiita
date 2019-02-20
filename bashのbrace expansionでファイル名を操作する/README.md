# BashのBrace Expansionでファイル名を操作する

BashのBrace Expansionなる機能をさっき知りました。こんな便利な機能があったとは。。。

https://www.gnu.org/software/bash/manual/html_node/Brace-Expansion.html

## ファイルをバックアップする

例えば

```bash
$ cp settings.json settings.json.bak
```

が

```bash
$ cp settings.json{,.bak}
```

で済む。これは

```bash
$ echo settings.json{,.bak}
settings.json settings.json.bak
```

と言う具合にBrace Expansionされるためです。

## バックアップファイルから復元する

```bash
$ ls -1
settings.json
settings.json.bak
$ mv settings.json{.bak,}
$ ls -1
settings.json
```

これは

```bash
$ echo settings.json{.bak,}
settings.json.bak settings.json
```

なので。

## APPENDIX

```
$ touch {0..2}.txt
$ ls -1
0.txt
1.txt
2.txt
$ echo {0..2} | xargs -n 1 -I % -t mv %.{txt,dat}
mv 0.txt 0.dat
mv 1.txt 1.dat
mv 2.txt 2.dat
$ ls -1
0.dat
1.dat
2.dat
```
