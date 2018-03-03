# MBRのIPLでHello world

PCは電源投入されるとBIOSが起動して起動ディスクのMBRにあるIPLを実行します。とりあえず、このIPLでHello worldしてみます。

- SeeAlso: [Basic Input/Output System - Wikipedia](https://ja.wikipedia.org/wiki/Basic_Input/Output_System)

---

- 2018-03: MEMO: BIOSはUEFI(Unified Extensible Firmware Interface)への置き換えが行われている最中で、BIOSは絶滅危惧種のようです。ただ、BIOS+MBRは仕組みが簡単で実験するにはもってこいだと思います。

## MBR

MBRはハードディスクの先頭セクタ(LBA 0)に配置される512Bytesのデータで、510BytesのIPL(Initial Program Loader)と、2BytesのMBR Signature(0x55, 0xAA)で構成されます。(パーティションテーブル云々は無視)

BIOSはハードディスクにMBR(Master Boot Record)を見つけると、そのハードディスクからの起動が可能と判断し、MBRをRAMの0x7c00-0x7dff(512Bytes)にロードしてIPLを実行することで任務完了します。

IPLは最大でも510Bytesしかないため、OSのロードと起動を行うには容量が足りません。そのため、IPLはGRUBなどのブートローダの起動までを職務とするのが通例です。

```
[HDD]
LBA 0 | 512Bytes <-- [MBR]
LBA 1 | 512Bytes
LBA ...
^^^ Logical Block Address

[MBR]
0x0000 | [IPL] ...
...
0x01f0 | xx xx xx xx xx xx xx xx xx xx xx xx xx xx 55 AA
                                                   ^^ ^^ MBR Signature
|
| BIOS loads MBR to RAM at 0x7c00, and runs IPL.
V

[RAM]
0x7c00 | [IPL] ...
...
0x7df0 | xx xx xx xx xx xx xx xx xx xx xx xx xx xx 55 AA
```

- SeeAlso: [マスターブートレコード - Wikipedia](https://ja.wikipedia.org/wiki/%E3%83%9E%E3%82%B9%E3%82%BF%E3%83%BC%E3%83%96%E3%83%BC%E3%83%88%E3%83%AC%E3%82%B3%E3%83%BC%E3%83%89)

---

QEMUを利用してMBRの有無で動作を確認してみます。macOSを利用しているため、HomebrewでQEMUをインストールしてます。

```shell
$ brew install qemu
$ brew link qemu
```

まず、LBA 0がall 0x00のハードディスクで確認してみます。QEMUはx86_64にしてます。

```shell
$ qemu-img create empty.bin 512
$ od -Ax -tx1 empty.bin 
0000000    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
0000200
$ qemu-system-x86_64 -hda empty.bin
...
```

QEMUが立ち上がり、

> Booting from Hard Disk...
> Boot failed: not a bootable disk

と表示されます。起動可能ディスクではないため、起動に失敗しました。次はMBR Signatureを付けてみます。

```shell
$ cp empty.bin mbr_signature.bin
$ echo -ne '\x55\xaa' | dd of=mbr_signature.bin bs=1 seek=510 conv=notrunc
$ od -Ax -tx1 mbr_signature.bin
0000000    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
00001f0    00  00  00  00  00  00  00  00  00  00  00  00  00  00  55  aa
0000200
$ qemu-system-x86_64 -hda mbr_signature.bin
...
```

QEMUが立ち上がり、

> Booting from Hard Disk...

で停止します。MBRとして認識してくれたようです。

## IPL

MBRがRAMの0x7c00-0x7dffにロードされているため、IPLは0x7c00で実行されます。x86_64ではBIOSはリアルモードで動作しているとのことで、IPLもまた16bitモードで動作します。

- SeeAlso: [リアルモード - Wikipedia](https://ja.wikipedia.org/wiki/%E3%83%AA%E3%82%A2%E3%83%AB%E3%83%A2%E3%83%BC%E3%83%89)

---

GNU Binutilsを使います。Homebrewのbinutilsではgldがインストールされないようなので、makeします。`/usr/local/Desk`で作業します。(思い付きで机をイメージしてます)

```shell
$ DESK=/usr/local/Desk
$ sudo mkdir $DESK && sudo chown `whoami` $DESK && chgrp wheel $DESK
$ mkdir $DESK/Drawer
```

```shell
$ pwd
/usr/local/Desk/Drawer
$ curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.30.tar.gz
... (37.8M) ...
$ tar xzf binutils-2.30.tar.gz
```

```shell
$ echo $DESK
/usr/local/Desk
$ pwd
/usr/local/Desk/Drawer/binutils-2.30
$ ./configure --prefix=$DESK/binutils/2.30 --target=x86_64-linux
...
checking where to find the target as... just compiled
...
checking where to find the target ld... just compiled
...
$ make
$ make install
```

- SeeAlso: [Binutils - GNU Project - Free Software Foundation](https://www.gnu.org/software/binutils/)

- !!!: makeする際のディレクトリ名にスペースが含まれてるとはまります。。。

---

以降、IPLを書いてみます。

### Hello,

- hello.s

```asm
        .file "hello.s"
        .code16gcc
        .intel_syntax noprefix

        .global _start

        .text # @ 7c00
_start:
        movb ah, 0x0e
        movb al, 'H'
        int 0x10
        movb al, 'e'
        int 0x10
        movb al, 'l'
        int 0x10
        movb al, 'l'
        int 0x10
        movb al, 'o'
        int 0x10
        movb al, ','
        int 0x10

_halt:
        hlt
        jmp _halt

        .data # @ 7dfe
mbr_signature:
        .byte 0x55
        .byte 0xAA

```

```shell
$ PREFIX=/usr/local/Desk/binutils/2.30/bin/x86_64-linux
$ $PREFIX-as -o hello.o hello.s
$ $PREFIX-ld --oformat binary -Ttext=7c00 -Tdata=7dfe -o hello.bin hello.o
$ od -Ax -tx1 hello.bin
0000000    b4  0e  b0  48  cd  10  b0  65  cd  10  b0  6c  cd  10  b0  6c
0000010    cd  10  b0  6f  cd  10  b0  2c  cd  10  f4  eb  fd  00  00  00
0000020    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
00001f0    00  00  00  00  00  00  00  00  00  00  00  00  00  00  55  aa
0000200
$ qemu-system-x86_64 -hda hello.bin
...
```

QEMUが立ち上がり、

> Booting from Hard Disk...
> Hello,

と表示されます。`ah=0x0e`で`int 0x10`すると`al`の文字が表示されます。BIOSのメモリマップに依存しないよう、割り込み発生で機能が利用できるようです。途中で力尽きました。。。

MBR Signatureはdataセクションに埋め込んでます。

- SeeAlso: [BIOS - OSDev Wiki](https://wiki.osdev.org/BIOS)
- SeeAlso: [Memory Map (x86) - OSDev Wiki](https://wiki.osdev.org/Memory_Map_(x86))

### Hello, world!

文字列を表示してみます。

```asm
        .file "hello_world.s"
        .code16gcc
        .intel_syntax noprefix

        .global _start

        .text # @ 7c00
_start:
        lea si, hello_world
        call puts
_halt:
        hlt
        jmp _halt

puts:
        pushw ax
        movb ah, 0x0e
_puts.l1:
        lodsb
        cmp al, 0x00
        jz _puts.l2
        int 0x10
        jmp _puts.l1
_puts.l2:
        popw ax
        ret

        .align 0x10
hello_world:
        .string "Hello, world!\r\n"

        .data # @ 7dfe
mgr_signature:
        .byte 0x55
        .byte 0xAA

```

```shell
$ PREFIX=/usr/local/Desk/binutils/2.30/bin/x86_64-linux
$ $PREFIX-as -o hello_world.o hello_world.s
$ $PREFIX-ld --oformat binary -Ttext=7c00 -Tdata=7dfe -o hello_world.bin hello_world.o
$ od -Ax -tx1 hello_world.bin
0000000    8d  36  20  7c  66  e8  03  00  00  00  f4  eb  fd  50  b4  0e
0000010    ac  3c  00  74  04  cd  10  eb  f7  58  66  c3  8d  b4  00  00
0000020    48  65  6c  6c  6f  2c  20  77  6f  72  6c  64  21  0d  0a  00
0000030    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
00001f0    00  00  00  00  00  00  00  00  00  00  00  00  00  00  55  aa
0000200
$ od -Ax -tc hello_world.bin
0000000  215   6       |   f 350 003  \0  \0  \0 364 353 375   P 264 016
0000010  254   <  \0   t 004 315 020 353 367   X   f   Í  ** 264  \0  \0
0000020    H   e   l   l   o   ,       w   o   r   l   d   !  \r  \n  \0
0000030   \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
*
00001f0   \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0   U 252
0000200
$ qemu-system-x86_64 -hda hello_world.bin
...
```

QEMUが立ち上がり、

> Booting from Hard Disk...
> Hello, world!

と表示されます。スタックポインタの初期化やらレジスタの初期化が必要らしいですが、とりあえず目的は達成したのでよしとします。

---

VirtualBoxでもやってみます。適当なVMを作成します。

- 名前: HelloWorld
- タイプ: Other
- バージョン: Other/Unknown
- メモリーサイズ: 4 MB
- ハードディスク: 4.00 MB
  - ファイルタイプ: VHD (Virtual Hard Disk)
  - ストレージ: 固定サイズ

仮想ハードディスクとしてHelloWorld.vhdが作成されます。VHDファイルはファイル先頭がそのままLBA 0になります。ファイルの末尾にはVHDファイルとしてのフッターが書き込まれています。

- SeeAlso: [About VHD (Windows)](https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd323654(v=vs.85).aspx)

```shell
$ od -Ax -tx1 HelloWorld.vhd
0000000    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
0400000    63  6f  6e  65  63  74  69  78  00  00  00  02  00  01  00  00
0400010    ff  ff  ff  ff  ff  ff  ff  ff  22  2d  25  04  76  62  6f  78
0400020    00  05  00  01  4d  61  63  20  00  00  00  00  00  40  00  00
0400030    00  00  00  00  00  40  00  00  00  78  04  11  00  00  00  02
0400040    ff  ff  e8  56  6e  bf  11  51  44  a9  58  4c  8d  2c  e1  8a
0400050    dc  2e  ac  da  00  00  00  00  00  00  00  00  00  00  00  00
0400060    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
0400200
```

作成後の仮想ハードディスクはall 0x00でした。とりあえずこのままRunして、

> FATAL: No bootable medium found! System halted.

です。MBR Signatureがないからですね。

HelloWorld.vhdのLBA 0にhello_world.binを書き込みます。

```shell
$ dd of=HelloWorld.vhd if=hello_world.bin conv=notrunc
$ od -Ax -tx1 HelloWorld.vhd
0000000    8d  36  20  7c  66  e8  03  00  00  00  f4  eb  fd  50  b4  0e
0000010    ac  3c  00  74  04  cd  10  eb  f7  58  66  c3  8d  b4  00  00
0000020    48  65  6c  6c  6f  2c  20  77  6f  72  6c  64  21  0d  0a  00
0000030    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
00001f0    00  00  00  00  00  00  00  00  00  00  00  00  00  00  55  aa
0000200    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
0400000    63  6f  6e  65  63  74  69  78  00  00  00  02  00  01  00  00
0400010    ff  ff  ff  ff  ff  ff  ff  ff  22  2d  25  04  76  62  6f  78
0400020    00  05  00  01  4d  61  63  20  00  00  00  00  00  40  00  00
0400030    00  00  00  00  00  40  00  00  00  78  04  11  00  00  00  02
0400040    ff  ff  e8  56  6e  bf  11  51  44  a9  58  4c  8d  2c  e1  8a
0400050    dc  2e  ac  da  00  00  00  00  00  00  00  00  00  00  00  00
0400060    00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00
*
0400200
```

ここでRunすると、

> Hello, world!

です。やったー。

---

- MEMO: VirtualBoxには拡張機能としてEFIを有効化するのものがあります。これでBIOSではなくUEFIでの動作が可能のため、UEFI+GPTの実験も可能です。(もちろん、本稿で作成したHelloWorld.vhdは動作しません。。。)
