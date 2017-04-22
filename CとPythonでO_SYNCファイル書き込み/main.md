# CとPythonでO_SYNCファイル書き込み

## 概要

CとPythonでファイル書き込みの時間コストを比較する。

存在する適当なファイル('DRIVER')に対して、毎回ファイルの先頭から適当なデータ('COMMAND')を、適当な回数(10000回)だけ書き込んだ場合の1回当たりの書き込みにかかる平均時間コストを計測する。  
(ファイルのopen/closeは時間コストに含めない。)

毎回ファイルの先頭からデータを書き込む理由は後述の[背景](#背景)のため。

## 背景

31313 マインドストーム EV3 で ev3dev を動作させ、モータ制御をする際にデバイスファイルに命令を送るのだが、その処理に限定してCとPythonで速度に差異があるか気になった。

- 31313 マインドストーム EV3 ([https://www.lego.com/ja-jp/mindstorms/products/mindstorms-ev3-31313](https://www.lego.com/ja-jp/mindstorms/products/mindstorms-ev3-31313))
- ev3dev ([http://www.ev3dev.org/](http://www.ev3dev.org/))

## 準備

環境は以下の通り。

```yaml
- macOS: 10.12.4
  software:
    - cc: Apple LLVM version 8.1.0 (clang-802.0.41)
    - python: 3.6.1
```

作成したコードは以下の通り。Pythonのファイル操作APIがいくつかあったので、3種類(io.FileIO, io.BaseIO, os)試してみる。

**test_syncw.c**

```c
// cc -Weverything -Ofast test_syncw.c -o test_syncw

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TEST_COUNT 10000

#define DRIVER_FILE "DRIVER"
#define COMMAND "COMMAND"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
    int fd = open(DRIVER_FILE, O_WRONLY | O_SYNC);
    do {
        clock_t elapsed_time;
        do {
            clock_t start_clock = clock();
            for (int i = 0; i < TEST_COUNT; ++i) {
                lseek(fd, 0, SEEK_SET);
                write(fd, COMMAND, sizeof(COMMAND));
            }
            elapsed_time = clock() - start_clock;
        } while(0);
        printf("%lfus\n", (double) elapsed_time / TEST_COUNT * (1000000.0 / CLOCKS_PER_SEC));
    } while(0);
    close(fd);
    return EXIT_SUCCESS;
}

```

**test_syncw.py**

```Python
import os
import io
import datetime
import time


TEST_COUNT = 10000
DRIVER_FILE = 'DRIVER'
COMMAND = b'COMMAND'


def test_with_fileio():
    elapsed_time = -1
    with io.FileIO(DRIVER_FILE, mode='w') as file:
        start = datetime.datetime.now()
        for i in range(TEST_COUNT):
            file.seek(0)
            file.write(COMMAND)
            file.flush()
        elapsed_time = (datetime.datetime.now() - start).microseconds
        file.close()
    print(f'io.FileIO: {elapsed_time / TEST_COUNT}us')


def test_with_iobase():
    elapsed_time = -1
    with open(DRIVER_FILE, mode='wb', buffering=0) as file:
        start = datetime.datetime.now()
        for i in range(TEST_COUNT):
            file.write(COMMAND)
            file.flush()
        elapsed_time = (datetime.datetime.now() - start).microseconds
        file.close()
    print(f'io.IOBase: {elapsed_time / TEST_COUNT}us')


def test_with_os():
    elapsed_time = -1
    fd = os.open(DRIVER_FILE, flags=(os.O_WRONLY | os.O_SYNC))
    start = datetime.datetime.now()
    for i in range(TEST_COUNT):
        os.lseek(fd, 0, os.SEEK_SET)
        os.write(fd, COMMAND)
    os.close(fd)
    elapsed_time = (datetime.datetime.now() - start).microseconds
    print(f'os: {elapsed_time / TEST_COUNT}us')


if __name__ == '__main__':
    test_with_fileio()
    test_with_iobase()
    test_with_os()

```

## 実験

コードを実行してみる。適当なファイル('DRIVER')は予め作成する必要がある。

```shell-session
$ touch DRIVER
$ cc -Weverything -Ofast test_syncw.c -o test_syncw
$ ./test_syncw
30.814900us
$ python test_syncw.py
io.FileIO: 3.4329us
io.IOBase: 3.3681us
os: 61.7254us
```

Pythonのio.FileIOとio.IOBaseはそもそもSYNCできていない。PythonのosはCよりも遅いが、これが比較するべき時間コストになる。

念のため5回繰り返してみた。

|   | C | Python (os) |
|:-:|:-:|:-----------:|
| 1 | 33.2873us | 58.1087us |
| 2 | 33.6538us | 72.6557us |
| 3 | 33.2398us | 64.4080us |
| 4 | 34.0902us | 60.7077us |
| 5 | 34.0923us | 62.3773us |

Pythonの方がCの2倍くらい時間がかかるけど、絶対値的にはそんなに気になるレベルではないと思う。
