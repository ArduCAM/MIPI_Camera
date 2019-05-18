import numpy as np
import cv2
import sys


def save_image(img, name):
    cv2.imwrite(name, img)


def read_image(name, width, height):
    with open(name, "rb") as f:
        data = f.read()

    img = np.frombuffer(data, np.uint8).reshape(height, width, 1)
    return img


if __name__ == "__main__":
    print("Notice:This program only support 8bit data convert to gray iamge.\n")
    if len(sys.argv) != 5:
        print("python {} <input_name> <output_name> <width> <height>".format(
            sys.argv[0]))
        exit(-1)

    input_name = sys.argv[1]
    output_name = sys.argv[2]
    width = int(sys.argv[3])
    height = int(sys.argv[4])
    img = read_image(input_name, width, height)
    save_image(img, output_name)
