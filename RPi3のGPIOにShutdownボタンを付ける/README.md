# RPi3のGPIOにShutdownボタンを付ける

**Pre-conditions**

- RPi3は2018-03-13-raspbian-stretch-lite.imgからセットアップ済み


## GPIO実験

GPIOの仕様は公式ページを参照しました。

- https://www.raspberrypi.org/documentation/usage/gpio-plus-and-raspi2/README.md

GPIO5, GPIO6にそれぞれLED, プッシュSWを繋いで実験します。

```
[GPIO5]---[LED+抵抗]---[GND]
[GPIO6]---[プッシュSW]---[GND]
```

GPIO6はpull-upされてます。

**MEMO**: RPi3はpull-up/downを設定できるような気配ですが、良い方法がわかりませんでした。。。(`direction`に`high`または`low`を書き込むとそれっぽいのですが、そうすると`edge`が`none`しか選択できなくなる。。。)


### Test LED

GPIOはデバイスファイルでアクセスできます。

```shell
pi $ ls /sys/class/gpio/
export  gpiochip0  gpiochip100  gpiochip128  unexport
pi $ echo 5 > /sys/class/gpio/export
pi $ ls /sys/class/gpio/
export  gpio5  gpiochip0  gpiochip100  gpiochip128  unexport
pi $ ls /sys/class/gpio/gpio5
active_low  device  direction  edge  power  subsystem  uevent  value
```

`export`に利用するGPIOの番号を書き込むと該当するGPIOのデバイスファイルが生成されます。

```shell
pi $ echo out > /sys/class/gpio/gpio5/direction
pi $ echo 1 > /sys/class/gpio/gpio5/value
```

これでLEDが点灯して、

```shell
pi $ echo 0 > /sys/class/gpio/gpio5/value
```

これでLEDが消灯します。`unexport`でGPIOの利用をやめることができます。

```shell
pi $ echo 5 > /sys/class/gpio/unexport
```


### Test switch

プッシュSWは`direction`を`in`にします。

```shell
pi $ echo 6 > /sys/class/gpio/export
pi $ echo in > /sys/class/gpio/gpio6/direction
```

`value`がプッシュSWの状態で変化します。

```shell
pi $ cat /sys/class/gpio/gpio6/value
? (1 or 0)
```


### Polling switch

プッシュSWの状態をポーリングによって監視し、プッシュ検出でLEDを点滅させます。

**trigger-polling.sh**

```shell
#!/bin/bash

echo Start Button Trigger

echo 6 > /sys/class/gpio/export 2> /dev/null
echo 5 > /sys/class/gpio/export 2> /dev/null
sleep 0.1
echo in > /sys/class/gpio/gpio6/direction
echo out > /sys/class/gpio/gpio5/direction

gpio_in=/sys/class/gpio/gpio6/value
led_out=/sys/class/gpio/gpio5/value

while :
do
    echo 1 > $led_out
    
    while [ `cat $gpio_in` = "0" ]
    do
        sleep 0.01
    done
    while [ `cat $gpio_in` = "1" ]
    do
        sleep 0.01
    done
    
    echo Falling Edge Detected
    echo 0 > $led_out
    
    sleep 0.5
    echo 1 > $led_out
    sleep 0.1
    echo 0 > $led_out
    sleep 0.1
    echo 1 > $led_out
    sleep 0.1
    echo 0 > $led_out
    sleep 0.1
    echo 1 > $led_out
    sleep 0.1
    echo 0 > $led_out
done
```


### Shutdownスクリプト

ほぼtrigger-polling.shですが、最後に`shutdown`します。

**shutdown-button.sh**

```shell
#!/bin/bash

echo 6 > /sys/class/gpio/export 2> /dev/null
echo 5 > /sys/class/gpio/export 2> /dev/null
sleep 0.1
echo in > /sys/class/gpio/gpio6/direction
echo out > /sys/class/gpio/gpio5/direction

gpio_in=/sys/class/gpio/gpio6/value
led_out=/sys/class/gpio/gpio5/value

echo 1 > $led_out
    
while [ `cat $gpio_in` = "0" ]
do
    sleep 0.01
done
while [ `cat $gpio_in` = "1" ]
do
    sleep 0.01
done
echo 0 > $led_out
sleep 0.5
echo 1 > $led_out
sleep 0.1
echo 0 > $led_out
sleep 0.1
echo 1 > $led_out
sleep 0.1
echo 0 > $led_out
sleep 0.1
echo 1 > $led_out
sleep 0.1
echo 0 > $led_out

shutdown -h now
```

これを起動時に実行されるようサービスとして登録します。とりあえず動く設定です。

**pi:/etc/systemd/system/shutdown-button.service**

```
[Unit]
Description=Shutdown Button
After=local-fs.target
ConditionPathExists=/home/pi

[Service]
ExecStart=/home/pi/shutdown-button.sh
Restart=no
Type=simple
User=root
Group=root

[Install]
WantedBy=multi-user.target
```

```shell
pi $ ls /home/pi
shutdown-button.sh  trigger_edge  trigger_edge.c  trigger-polling.sh
pi $ sudo systemctl daemon-reload
pi $ sudo systemctl enable shutdown-button
pi $ sudo shutdown -r now
```

これで起動時にshutdown-button.shが起動されます。


# Appendix

RPi3はGPIOのエッジトリガーの検出が可能で、ポーリングよりリソースに優しいです。Shellscriptで書く方法がわからなかったため今回は不採用としましたが、C言語のコードを書き残しておきます。

**trigger_edge.c**

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

static char const * PATH = "/sys/class/gpio/gpio6/value";

int main(void) {
    int fd;

    puts("Start Button Trigger");
    fd = open(PATH, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error Failed to the file: %s\n", PATH);
        return -1;
    }

    {
        struct pollfd polls;
        char ch;
        polls.fd = fd;
        polls.events = POLLPRI;

        read(fd, &ch, 1);
        lseek(fd, 0, SEEK_SET);
        poll(&polls, 1, -1);
    }
    puts("Falling Edge Detected");

    close(fd);
    return 0;
}
```

```shell
$ echo 6 > /sys/class/gpio/export 2> /dev/null
$ echo in > /sys/class/gpio/gpio6/direction
$ echo falling > /sys/class/gpio/gpio6/edge
$ gcc trigger_edge.c -o trigger_edge
$ ./trigger_edge
Start Button Trigger
Falling Edge Detected
```

ちなみに、`edge`を`none`(defualt)にするとエッジ検出しなくなります。

チャタリング対策していないためRising Edgeも拾ってるようです。。。
