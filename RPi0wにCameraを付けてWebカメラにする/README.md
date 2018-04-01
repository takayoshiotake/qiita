# RPi0wにCameraを付けてWebカメラにする

**Pre-conditions**

- RPi0wは2018-03-13-raspbian-stretch-lite.imgからセットアップ済み

**MEMO**: RPi3と同様にセットアップ可能でした。


## Camera装着

CameraはRaspberry Pi PiNoir Camera V2を使用します。

**NOTICE**: RPi0のカメラケーブルはRPi3等よりも小型で、カメラに付属するケーブルは使用できません。Raspberry Pi公式のRPi0用ケースに付属する短めのカメラケーブルを使用するか、別途RPi0用の長めのカメラケーブルが必要になります。

最初にカメラを有効にします。

```shell
pi $ sudo raspi-config
```

からの`5 Interfacing Options` > `P1 Camera`と進んでカメラを有効にします。その後、再起動を要求されます。

次のコマンドでカメラを使用可能か確認できます。

```shell
pi $ vcgencmd get_camera
supported=1 detected=1
```


## テスト撮影

静止画撮影用のコマンドがOS標準で用意されているため、とりあえず撮影します。

```shell
pi $ raspistill -o /tmp/still.jpg
```

ちなみに、`/tmp`はRamDisk化してますが、問題なく3280x2464(5.3MB)のJPEGが作成できました。

タイムアウト引数を省略すると5秒待たされるようです。(`-t 5000`)

```shell
pi $ time raspistill -o /tmp/still.jpg -t 1

real	0m1.020s
user	0m0.072s
sys	0m0.059s
```

これで待ち時間がなくなります。しかし、コマンド完了まで1秒かかってるのが気になります。。。


## ライブビュー

RPi0wでPythonのサーバーを実行します。このサーバーはhttp(8080)に応答し、実行ディレクトリ基準の`./web`以下を静的ウェブサイトホスティングし、`GET /api/camera.liveiew.jpg`で640x480の画像を返します。

*server-v1.py*:

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import http.server
import socketserver
import json

import subprocess
import sys

PORT = 8080


class RequestHandler(http.server.SimpleHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    
    def do_HEAD(self):
        self.send_error(405)
        

    def do_POST(self):
        if self.path == '/api' or self.path.startswith('/api/'):
            self.do_api('POST')
            return
        self.send_error(404)


    def do_GET(self):
        if self.path == '/api' or self.path.startswith('/api/'):
            self.do_api('GET')
            return
        super().do_GET()


    def do_api(self, method):
        if method == 'GET' and self.path == '/api/camera.liveview.jpg':
            subargs = 'raspistill -w 640 -h 480 -q 20 -o - -t 1 -n'
            try:
                res = subprocess.run(subargs, shell=True, stdout=subprocess.PIPE).stdout
                self.send_response(200)
                self.send_header('Content-Type', 'image/jpeg')
                self.send_header('Content-Length', len(res))
                self.end_headers()
                self.wfile.write(res)
            except:
                self.send_error(503)
        else:
            self.send_error(404)


    def send_response(self, code, message=None):
        super().send_response_only(code, message)


    def send_error(self, code, message=None, explain=None):
        try:
            shortmsg, _ = self.responses[code]
        except KeyError:
            shortmsg, longmsg = '???', '???'
        if message is None:
            message = shortmsg
        self.log_error('code {0}, message {1}'.format(code, message))
        response = {
            'code': code,
            'message': message
        }
        response_bytes = json.dumps(response).encode('utf-8')
        self.send_response(code, message)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', len(response_bytes))
        self.end_headers()
        self.wfile.write(response_bytes)


class ThreadingServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    pass


if __name__ == '__main__':
    os.chdir('./web')
    socketserver.TCPServer.allow_reuse_address = True
    server_address = ("", PORT)
    
    httpd = ThreadingServer(server_address, RequestHandler)
    print("serving at port", PORT)
    httpd.serve_forever()

```

*./web/index.html*:

```
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8"><!-- ✅ あ keeps utf-8 -->
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0">
<style>
html, body {
    width: 100%;
    height: 100%;
    padding: 0;
    margin: 0;
}
img {
    display: block;
}
.flex-box {
    display: flex;
    width: 100%;
    height: 100%;
    justify-content: center;
    align-items: center;
    background-color: black;
}
.flex-item {
    width: 100%;
    height: 100%;
    object-fit: contain;
}
</style>
<script>
class IndexPage {
    constructor() {
        this.img = document.querySelector('.flex-item')
        this.start_liveview()
    }
    
    start_liveview() {
        var is_updating = false
        let update_liveview = async () => {
            if (is_updating) {
                return
            }
            is_updating = true
            
            let webapi = new WebAPIRequest()
            try {
                await webapi.request('GET', '/api/camera.liveview.jpg')
                if (webapi.status == 200) {
                    this.img.src = await webapi.response_as('image/jpeg')
                }
                else {
                    console.log(await webapi.response_as('json'))
                }
            }
            catch (e) {
            }
            
            is_updating = false
        }
        setInterval(update_liveview, 100)
        update_liveview()
    }
}

let indexPage
window.addEventListener('load', () => {
    indexPage = new IndexPage()
})

class WebAPIRequest {
    constructor() {
        this.xhr = new XMLHttpRequest()
        this.xhr.responseType = 'arraybuffer'
    }
    
    get status() {
        return this.xhr.status
    }
    
    async request(method, uri, timeout = 3000) {
        return new Promise((resolve, reject) => {
            this.xhr.ontimeout = (e) => { reject(e) }
            this.xhr.onerror = (e) => { reject(e) }
            this.xhr.onload = (e) => {
                if (this.xhr.readyState == 4) {
                    resolve()
                }
            }
                
            this.xhr.open(method, uri)
            this.xhr.setRequestHeader('Pragma', 'no-cache')
            this.xhr.setRequestHeader('Cache-Control', 'no-cache')
            this.xhr.send()
        })
    }
    
    async response_as(type) {
        switch (type) {
        case 'image/jpeg':
            return new Promise((resolve) => {
                let reader = new FileReader()
                reader.onloadend = () => {
                    resolve(reader.result)
                }
                reader.readAsDataURL(new Blob([this.xhr.response], {type: 'image/jpeg'}))
            })
        case 'json':
            return JSON.parse(new TextDecoder('utf-8').decode(this.xhr.response))
        default:
            return this.xhr.response
        }
    }
}
</script>
</head>
<body>
<div class="flex-box">
  <img class="flex-item">
</div>
</body>
</html>

```

これを実行してWebブラウザで`http://<You RPi IP>:8080/`にアクセスするとライブビューが見れます。

```shell
pi $ ./server-v1.py
serving at port 8080
```

しかし、Chromeで確認するとFPSが1程度。。。例えば

- Request sent: 84 us
- Waiting (TTFB): 861.90 ms
- Content Download 29.93 ms

こんな感じです。

```shell
pi $ time raspistill -w 640 -h 480 -q 20 -o /tmp/still.jpg -t 1 -n

real	0m0.845s
user	0m0.019s
sys	0m0.034s
```

やはり、`raspistill`の実行に時間がかかっているのは明白です。


## ライブビュー with mjpg-streamer

mjpg-streamerはカメラの映像をMJPG(Motion JPEG)で簡単に配信できる素晴らしいソフトウェアです。

- SeeAlso: https://github.com/jacksonliam/mjpg-streamer

サクッとmakeして、このソフトからライブビューを取得してみます。

```shell
pi $ sudo cat /etc/apt/sources.list
deb http://raspbian.raspberrypi.org/raspbian/ stretch main contrib non-free rpi
# Uncomment line below then 'apt-get update' to enable 'apt-get source'
#deb-src http://raspbian.raspberrypi.org/raspbian/ stretch main contrib non-free rpi

deb http://ftp.jaist.ac.jp/raspbian stretch main contrib non-free
pi $ sudo apt-get install cmake libjpeg8-dev

pi $ cd ~
pi $ curl -L https://github.com/jacksonliam/mjpg-streamer/archive/master.zip > mjpg-streamer-master.zip
pi $ unzip mjpg-streamer-master.zip
pi $ cd mjpg-streamer-master/mjpg-streamer-experimental
pi $ make
```

簡単に実行するために、スクリプトを作成しました。

*mjpg-server.sh*:

```shell
#!/bin/bash

MJPEG_STREAMER_PATH=/home/pi/mjpg-streamer-master/mjpg-streamer-experimental

export LD_LIBRARY_PATH=$MJPEG_STREAMER_PATH
$MJPEG_STREAMER_PATH/mjpg_streamer -o "output_http.so -p 8081" -i "input_raspicam.so -x 640 -y 480 -quality 20"

```

また、Pythonのサーバーを一部変更して、`GET /api/camera.liveview.jpg`を`http://localhost:8081/?action=snapshot`のリバースプロキシにします。(CORS対応が面倒なので)

*server-v2.py*:

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import http.server
import socketserver
import json

import urllib.request

PORT = 8080


class RequestHandler(http.server.SimpleHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    
    def do_HEAD(self):
        self.send_error(405)
        

    def do_POST(self):
        if self.path == '/api' or self.path.startswith('/api/'):
            self.do_api('POST')
            return
        self.send_error(404)


    def do_GET(self):
        if self.path == '/api' or self.path.startswith('/api/'):
            self.do_api('GET')
            return
        super().do_GET()


    def do_api(self, method):
        if method == 'GET' and self.path == '/api/camera.liveview.jpg':
            with urllib.request.urlopen('http://localhost:8081/?action=snapshot') as f:
                res = f.read()
                self.send_response(200)
                self.send_header('Content-Type', 'image/jpeg')
                self.send_header('Content-Length', len(res))
                self.end_headers()
                self.wfile.write(res)
        else:
            self.send_error(404)


    def send_response(self, code, message=None):
        super().send_response_only(code, message)


    def send_error(self, code, message=None, explain=None):
        try:
            shortmsg, _ = self.responses[code]
        except KeyError:
            shortmsg, longmsg = '???', '???'
        if message is None:
            message = shortmsg
        self.log_error('code {0}, message {1}'.format(code, message))
        response = {
            'code': code,
            'message': message
        }
        response_bytes = json.dumps(response).encode('utf-8')
        self.send_response(code, message)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', len(response_bytes))
        self.end_headers()
        self.wfile.write(response_bytes)


class ThreadingServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    pass


if __name__ == '__main__':
    os.chdir('./web')
    socketserver.TCPServer.allow_reuse_address = True
    server_address = ("", PORT)
    
    httpd = ThreadingServer(server_address, RequestHandler)
    print("serving at port", PORT)
    httpd.serve_forever()

```

この場合ではライブビューのFPSが5程度になるのが確認できました。

```shell
pi $ pwd
/home/pi
pi $ ./mjpg-server.sh &
MJPG Streamer Version.: 2.0
 i: fps.............: 5
 i: resolution........: 640 x 480
 i: camera parameters..............:

Sharpness 0, Contrast 0, Brightness 50
Saturation 0, ISO 0, Video Stabilisation No, Exposure compensation 0
Exposure Mode 'auto', AWB Mode 'auto', Image Effect 'none'
Metering Mode 'average', Colour Effect Enabled No with U = 128, V = 128
Rotation 0, hflip No, vflip No
ROI x 0.000000, y 0.000000, w 1.000000 h 1.000000
 o: www-folder-path......: disabled
 o: HTTP TCP port........: 8081
 o: HTTP Listen Address..: (null)
 o: username:password....: disabled
 o: commands.............: enabled
 i: Starting Camera
Encoder Buffer Size 81920

pi $ ./server-v2.py
serving at port 8080
```


## まとめと今後の課題

RPi0wにCameraを付けて、Webページ上でカメラのライブビューを実現することができましたが、ライブビューの性能があまりよろしくなかったため、撮影機能の実現はペンディングです。

mjpg-streamerを実行中は`raspistill`を実行することができないため、VGA画質ライブビューからのフルサイズ撮影を行うためには、一度mjpg-streamerを停止する必要があります。この停止には数秒要するため、撮影までのラグが長くなってしまうのがネックです。また、ライブビュー中に露光条件やWB等を変更することも難しそうです。

一方`raspistill`だけを利用する場合は、VGA画質ライブビューからのフルサイズ撮影に余計な待ち時間は発生しませんが、そもそも`raspistill`の実行に時間がかかるためライブビューのFPSが出ません。

結論として、別の方法でカメラを制御する必要がありそうですが、Raspberry Pi用のカメラはMMAL (Multi-Media Abstraction Layer)というAPIで制御可能なことがわかりました。mjpg-streamerもraspistillもMMALでカメラを制御してます。

- SeeAlso: https://github.com/jacksonliam/mjpg-streamer/tree/master/mjpg-streamer-experimental/plugins/input_raspicam
- SeeAlso: https://github.com/raspberrypi/userland/tree/master/host_applications/linux/apps/raspicam

ということで今後の課題はMMALということになりますが、iPhone, AndroidにWebカメラアプリを作る方が簡単そう。。。その前にRPi3の性能とも比較したい。。。
