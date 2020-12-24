import numpy as np
import cv2
import sys


def align_down(size, align):
    return (size & ~((align)-1))

def align_up(size, align):
    return align_down(size + align - 1, align)

def remove_padding(data, width, height, bit_width):
    buff = np.frombuffer(data, np.uint8)
    real_width = width / 8 * bit_width
    align_width = align_up(real_width, 32)
    align_height = align_up(height, 16)
    
    buff = buff.reshape(align_height, align_width)
    buff = buff[:height, :real_width]
    buff = buff.reshape(height * real_width)
    return buff


def save_image(img, name):
    cv2.imwrite(name, img)


def read_image(name, width, height):
    with open(name, "rb") as f:
        data = f.read()

    img = remove_padding(data, width, height, 8).reshape(height, width, 1)
    # img = np.frombuffer(data, np.uint8).reshape(height, width, 1)
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
