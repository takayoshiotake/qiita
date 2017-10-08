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
    print("")
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
