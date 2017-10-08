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
