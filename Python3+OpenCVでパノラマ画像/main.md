# Python3+OpenCVでパノラマ写真

コードはGitHubにあります。
https://github.com/takayoshiotake/qiita/tree/master/Python3%2BOpenCV%E3%81%A7%E3%83%91%E3%83%8E%E3%83%A9%E3%83%9E%E7%94%BB%E5%83%8F/src


## 画像用意

実験用の画像を用意します。有名な女性の写真を利用させていただきます。

```shell-session
$ curl -O http://www.cs.cmu.edu/~chuck/lennapg/lena_std.tif
```

今回はこれを2倍して1024x1024にし、そこから左上、右上、左下、右下からそれぞれ640x640で切り出した4枚の画像を利用します。

![fig_1.png](https://qiita-image-store.s3.amazonaws.com/0/97440/2638286c-e1e0-2718-163b-540cb8887529.png)


## Stitcher

OpenCVのStitcherを利用すると簡単にパノラマ画像が作れます。

**stitchimg.py**

```python
import argparse
import cv2

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Stitch given images.')
    parser.add_argument("inputs", metavar='input', type=str, nargs='+', help='input file')
    parser.add_argument("--output", type=str, default='a.png', help='output file (default=a.png)')
    args = parser.parse_args()

    input_images = []
    for i in args.inputs:
        image = cv2.imread(i)
        if image is None:
            print(f'Error: Unable to open file "{i}".')
            exit()
        input_images.append(image)

    if len(input_images) == 1:
        cv2.imwrite(args.output, input_images[0])
    else:
        stitcher = cv2.createStitcher(True)
        stitched = stitcher.stitch(input_images)
        cv2.imwrite(args.output, stitched[1])

```

```shell-session
$ python3 stitchimg.py --output fig_2.png test_tl.tif test_tr.tif test_bl.tif test_br.tif
```

そこそこ時間がかかります。画像はほぼ完璧です。

![fig_2.png](https://qiita-image-store.s3.amazonaws.com/0/97440/281cd3c6-5b90-bde6-4e18-108b199edb99.png)

1024x1024を期待していたのですが、1025x1023になってしまいました。


## 完全一致

完全一致する行または列を探して、画像を合成します。1回の合成では水平方向(入力は左,右)か垂直方向(入力は上,下)のどちらかのみ、かつ、合成する方向に依存しますが画像の横幅か縦幅が一致することが条件です。

**matchimg.py**

```python
import argparse
import cv2
import numpy as np

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Overlay given images with perfect matching.')
    parser.add_argument("inputs", metavar='input', type=str, nargs=2,
                        help='input file (inputs are (left, right) or (top, bottom))')
    parser.add_argument("--output", type=str, default='a.png',
                        help='output file (default=a.png)')
    parser.add_argument("--direction", choices=['h', 'v'], default='h',
                        help='overlay direction (default=h, h=horizontal, v=vertical)')
    parser.add_argument("--threshold", type=float, default=0,
                        help='matching threshold (default=0)')
    args = parser.parse_args()

    input_images = []
    for i in args.inputs:
        image = cv2.imread(i)
        if image is None:
            print(f'Error: Unable to open file "{i}".')
            exit()
        input_images.append(image)

    if args.direction == 'h':
        if input_images[0].shape[0] != input_images[1].shape[0]:
            print('Error: height not matched')
            exit()
        if input_images[0].shape[2] != input_images[1].shape[2]:
            print('Error: channels not matched')
            exit()
        match_h = -1
        for h in range(input_images[1].shape[1]):
            # if np.array_equal(input_images[0][:, -1], input_images[1][:, h]):
            if np.allclose(input_images[0][:, -1], input_images[1][:, h], args.threshold):
                match_h = h + 1
                break
        if match_h == -1:
            print(f'Error: Unable to match.')
            exit()
        # print(f'match_h={match_h}')
        stitched = np.hstack((input_images[0], input_images[1][:, match_h:]))
        cv2.imwrite(args.output, stitched)
    else:
        if input_images[0].shape[1] != input_images[1].shape[1]:
            print('Error: width not matched')
            exit()
        if input_images[0].shape[2] != input_images[1].shape[2]:
            print('Error: channels not matched')
            exit()
        match_v = -1
        for v in range(input_images[1].shape[0]):
            # if np.array_equal(input_images[0][-1], input_images[1][v]):
            if np.allclose(input_images[0][-1], input_images[1][v], args.threshold):
                match_v = v + 1
                break
        if match_v == -1:
            print(f'Error: Unable to match.')
            exit()
        # print(f'match_v={match_v}')
        stitched = np.vstack((input_images[0], input_images[1][match_v:]))
        cv2.imwrite(args.output, stitched)

```

```shell-session
$ python3 matchimg.py --output fig_3.png --direction h test_tl.tif test_tr.tif
$ python3 matchimg.py --output fig_4.png --direction h test_bl.tif test_br.tif
$ python3 matchimg.py --output fig_5.png --direction v fig_3.png fig_4.png
```

(画像大きいので fig_3.png と fig_4.png は省略します)

![fig_5.png](https://qiita-image-store.s3.amazonaws.com/0/97440/1cf87d63-fabe-fbc7-401e-2cafe9e6660c.png)

元の画像に戻りましたが、現実的には完全一致するシーンはほぼないでしょう。一応 `--threshold` オプションで一致判定を緩くすることも可能にしておきました。
