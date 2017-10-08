# Python3+OpenCVでエッジ抽出するまで

## 環境構築

macOSで表題の実験を行いますが、環境はHomebrewで構築します。

Homebewは[公式](https://brew.sh/index_ja.html)にある通りにインストールします。一応、インストール方法をコピペします。(2017-10-07)

> ```shell-session
> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
> ```
> 
> このスクリプトをターミナルに貼り付け実行して下さい。

そして、Python3をインストールします。

*Warning*: この状態でインストール開始するとPermission deniedで失敗する可能生があります。
> Error: An unexpected error occurred during the `brew link` step
> The formula built, but is not symlinked into /usr/local
> Permission denied @ dir_s_mkdir - /usr/local/Frameworks
> Error: Permission denied @ dir_s_mkdir - /usr/local/Frameworks

あらかじめ次のコマンドを実行しておきます。

```shell-session
$ sudo mkdir -p /usr/local/Frameworks
$ sudo chown $(whoami) /usr/local/Frameworks
```

改めて、Python3をインストールします。(4分かかりました)

```shell-session
$ brew install python3
...
==> Summary
🍺  /usr/local/Cellar/python3/3.6.3: 7,973 files, 111MB, built in 4 minutes 8 seconds
$ which python3
/usr/local/bin/python3
$ which pip3
/usr/local/bin/pip3
```

Python3のインストールが成功していればpip3もインストールされます。

必要なパッケージはpip3でインストールします。ちょっと時間がかかります。

```shell-session
$ pip3 install opencv-python
$ pip3 install matplotlib
$ pip3 list --format=columns
Package         Version 
--------------- --------
cycler          0.10.0  
matplotlib      2.0.2   
numpy           1.13.3  
opencv-python   3.3.0.10
pip             9.0.1   
pyparsing       2.2.0   
python-dateutil 2.6.1   
pytz            2017.2  
setuptools      36.5.0  
six             1.11.0  
wheel           0.30.0
```

関連するパッケージもインストールされました。

## 画像用意

実験用の画像を用意します。有名な女性の写真を利用させていただきます。

```shell-session
$ curl -O http://www.cs.cmu.edu/~chuck/lennapg/lena_std.tif
```

## 画像表示

OpenCVで画像を読み込み、Matplotlibで画像を表示します。まだ、エッジ抽出はおあずけです。

**main1.py**

```python
import cv2
import matplotlib.pyplot as plt

if __name__ == "__main__":
    img = cv2.imread("lena_std.tif")
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    plt.imshow(img)
    plt.show()

```

コマンドを間違えないように実行します。コマンドは `python3` です。間違って `python` と打つと python 2.7.* で実行され、おそらく `import cv2` でエラーになります。

```shell-session
$ python3 main1.py
```

Memo: 初回起動時はMatplotlibがフォントキャッシュを構築するため時間がかかるようです。

![fig_1.png](https://qiita-image-store.s3.amazonaws.com/0/97440/da5c6818-7087-158b-2417-b114b359790f.png)


## エッジ抽出

ラプラシアンで抽出します。

**main2.py**

```python
import cv2
import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    img = cv2.imread("lena_std.tif")
    grayimg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
    laplacianimg = cv2.Laplacian(grayimg, cv2.CV_32F, ksize=3)
    kernel = [[-1,-1,-1],[-1,8,-1],[-1,-1,-1]]
    edgeimg = cv2.filter2D(grayimg, cv2.CV_32F, np.array(kernel, np.float32))
    edgeimg = np.sqrt(np.multiply(edgeimg, edgeimg))

    plt.subplot(2, 2, 1)
    plt.imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    plt.subplot(2, 2, 2)
    plt.imshow(laplacianimg, cmap="gray")
    plt.subplot(2, 2, 3)
    plt.imshow(edgeimg, cmap="gray")
    plt.show()

```

![fig_2.png](https://qiita-image-store.s3.amazonaws.com/0/97440/cb5140c8-b779-0d45-f329-deff45bd49f3.png)

右上の画像がOpenCVのラプラシアンでエッジ抽出した結果になります。左下の画像はエッジ強度になります。

## 3次元表示

このまま終わるのは味気ないので、エッジ強度の画像を3次元表示してみます。

**main3.py**

```python
import cv2
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

if __name__ == "__main__":
    img = cv2.imread("lena_std.tif")
    grayimg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
    kernel = [[-1,-1,-1],[-1,8,-1],[-1,-1,-1]]
    edgeimg = cv2.filter2D(grayimg, cv2.CV_32F, np.array(kernel, np.float32))
    edgeimg = np.sqrt(np.multiply(edgeimg, edgeimg))
    
    edgeimg = cv2.resize(edgeimg, (128, 128))
    x = np.arange(edgeimg.shape[0])
    y = np.arange(edgeimg.shape[1])
    X, Y = np.meshgrid(x, y)
    Z = edgeimg[X, Y]

    plt.subplot(2, 3, 1)
    plt.imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    plt.subplot(2, 3, 4)
    plt.imshow(edgeimg, cmap="magma")
    ax = plt.subplot(2, 3, (2, 6), projection="3d") # requires Axes3D
    ax.grid(False)
    ax.view_init(60, -30)
    ax.plot_surface(X, Y, Z, rstride=1, cstride=1, linewidth=0, antialiased=True, cmap="magma")
    plt.show()

```

![fig_3.png](https://qiita-image-store.s3.amazonaws.com/0/97440/a9aeb4b5-280c-eb36-c1d3-76d215b83a88.png)


## もっと3次元表示

もっと3次元表示するため、PLYファイルを出力します。([PLY - Polygone File Format](http://paulbourke.net/dataformats/ply/))


```python
import cv2
import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    img = cv2.imread("lena_std.tif")
    grayimg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
    kernel = [[-1,-1,-1],[-1,8,-1],[-1,-1,-1]]
    edgeimg = cv2.filter2D(grayimg, cv2.CV_32F, np.array(kernel, np.float32))
    edgeimg = np.sqrt(np.multiply(edgeimg, edgeimg))
    
    width, height = edgeimg.shape
    ymax = height - 1
    xmax = width - 1
    print("ply")
    print("format ascii 1.0")
    print("element vertex %d" % edgeimg.size)
    print("property double x")
    print("property double y")
    print("property double z")
    print("property uchar red")
    print("property uchar green")
    print("property uchar blue")
    print("element face %d" % ((width - 1) * (height - 1)))
    print("property list uchar int vertex_index")
    print("end_header")
    for y in range(height):
        for x in range(width):
            z = edgeimg[x, y] / 255
            c = img[x, y]
            print("%f %f %f %d %d %d" % (x / xmax, y / ymax, z, c[2], c[1], c[0]))
    for y in range(height - 1):
        for x in range(width - 1):
            v1 = (y + 0) * width + (x + 0)
            v2 = (y + 0) * width + (x + 1)
            v3 = (y + 1) * width + (x + 1)
            v4 = (y + 1) * width + (x + 0)
            print("4 %d %d %d %d" % (v1, v2, v3, v4))

```

```shell-session
$ python3 main4.py > edge3d.ply
```

MeshLabで表示しました。

![fig_4.png](https://qiita-image-store.s3.amazonaws.com/0/97440/7aecf861-c577-e292-6268-f9637285ad04.png)
