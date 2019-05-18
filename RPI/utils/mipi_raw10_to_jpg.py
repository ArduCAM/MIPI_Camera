import numpy as np
import cv2
import sys
global COLOR_BayerGB2BGR, COLOR_BayerRG2BGR, COLOR_BayerGR2BGR, COLOR_BayerBG2BGR
global bayer_order_maps

COLOR_BayerBG2BGR = 46
COLOR_BayerGB2BGR = 47
COLOR_BayerRG2BGR = 48
COLOR_BayerGR2BGR = 49

bayer_order_maps = {
    "bayer_bg": COLOR_BayerBG2BGR,
    "bayer_gb": COLOR_BayerGB2BGR,
    "bayer_rg": COLOR_BayerRG2BGR,
    "bayer_gr": COLOR_BayerGR2BGR,
}


def read_uint10(byte_buf):
    data = np.frombuffer(byte_buf, dtype=np.uint8)
    # 5 bytes contain 4 10-bit pixels (5x8 == 4x10)
    b1, b2, b3, b4, b5 = np.reshape(
        data, (data.shape[0]//5, 5)).astype(np.uint16).T
    o1 = (b1 << 2) + (b2 >> 6)
    o2 = ((b2 % 64) << 4) + (b3 >> 4)
    o3 = ((b3 % 16) << 6) + (b4 >> 2)
    o4 = ((b4 % 4) << 8) + b5
    unpacked = np.reshape(np.concatenate(
        (o1[:, None], o2[:, None], o3[:, None], o4[:, None]), axis=1),  4*o1.shape[0])
    return unpacked


def unpack_mipi_raw10(byte_buf):
    data = np.frombuffer(byte_buf, dtype=np.uint8)
    # 5 bytes contain 4 10-bit pixels (5x8 == 4x10)
    b1, b2, b3, b4, b5 = np.reshape(
        data, (data.shape[0]//5, 5)).astype(np.uint16).T
    o1 = (b1 << 2) + ((b5) & 0x3)
    o2 = (b2 << 2) + ((b5 >> 2) & 0x3)
    o3 = (b3 << 2) + ((b5 >> 4) & 0x3)
    o4 = (b4 << 2) + ((b5 >> 6) & 0x3)
    unpacked = np.reshape(np.concatenate(
        (o1[:, None], o2[:, None], o3[:, None], o4[:, None]), axis=1),  4*o1.shape[0])
    return unpacked


def save_image(img, name):
    cv2.imwrite(name, img)


if __name__ == "__main__":
    print("Notice:This program only support csi-2 raw 10bit packet data convert to jpg.\n")
    if len(sys.argv) != 6:
        print("python {} <input_name> <output_name> <width> <height> <bayer_order>".format(
            sys.argv[0]))
        print("Bayer Order:")
        for key, value in bayer_order_maps.iteritems():
            print("    " + key)
        exit(-1)

    input_name = sys.argv[1]
    output_name = sys.argv[2]
    width = int(sys.argv[3])
    height = int(sys.argv[4])
    bayer_order = bayer_order_maps[sys.argv[5].lower()]

    with open(input_name, "rb") as f:
        data = f.read()

    img = unpack_mipi_raw10(data).reshape(height, width, 1)
    img = img >> 2
    img = img.astype(np.uint8)
    img = cv2.cvtColor(img, bayer_order)
    save_image(img, output_name)
