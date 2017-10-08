import cv2
import matplotlib.pyplot as plt

if __name__ == "__main__":
    img = cv2.imread("lena_std.tif")
    plt.imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    plt.show()
