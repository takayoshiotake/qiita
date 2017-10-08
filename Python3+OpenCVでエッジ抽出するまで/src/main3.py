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
