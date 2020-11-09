import v4l2
import fcntl
import array
import ctypes
import cv2
import numpy as np

_IOC_NRBITS = 8
_IOC_TYPEBITS = 8
_IOC_SIZEBITS = 14
_IOC_DIRBITS = 2

_IOC_NRSHIFT = 0
_IOC_TYPESHIFT = _IOC_NRSHIFT + _IOC_NRBITS
_IOC_SIZESHIFT = _IOC_TYPESHIFT + _IOC_TYPEBITS
_IOC_DIRSHIFT = _IOC_SIZESHIFT + _IOC_SIZEBITS

_IOC_NONE = 0
_IOC_WRITE = 1
_IOC_READ  = 2


def _IOC(dir_, type_, nr, size):
    return (
        ctypes.c_int32(dir_ << _IOC_DIRSHIFT).value |
        ctypes.c_int32(ord(type_) << _IOC_TYPESHIFT).value |
        ctypes.c_int32(nr << _IOC_NRSHIFT).value |
        ctypes.c_int32(size << _IOC_SIZESHIFT).value)

def _IOC_TYPECHECK(t):
    return ctypes.sizeof(t)

def _IO(type_, nr):
    return _IOC(_IOC_NONE, type_, nr, 0)

def _IOW(type_, nr, size):
    return _IOC(_IOC_WRITE, type_, nr, _IOC_TYPECHECK(size))

def _IOR(type_, nr, size):
    return _IOC(_IOC_READ, type_, nr, _IOC_TYPECHECK(size))

def _IOWR(type_, nr, size):
    return _IOC(_IOC_READ | _IOC_WRITE, type_, nr, _IOC_TYPECHECK(size))

BASE_VIDIOC_PRIVATE = 192

class arducam_i2c(ctypes.Structure):
    _fields_ = [
        ('reg', ctypes.c_uint16),
        ('val', ctypes.c_uint16),
    ]

class arducam_dev(ctypes.Structure):
    _fields_ = [
        ('reg', ctypes.c_uint16),
        ('val', ctypes.c_uint32),
    ]


VIDIOC_R_I2C = _IOWR('V', BASE_VIDIOC_PRIVATE + 0, arducam_i2c)
VIDIOC_W_I2C = _IOWR('V', BASE_VIDIOC_PRIVATE + 1, arducam_i2c)
VIDIOC_R_DEV = _IOWR('V', BASE_VIDIOC_PRIVATE + 2, arducam_dev)
VIDIOC_W_DEV = _IOWR('V', BASE_VIDIOC_PRIVATE + 3, arducam_dev)


class ArducamUtils(object):
    pixfmt_map = {
        v4l2.V4L2_PIX_FMT_SBGGR10:{ "depth":10, "cvt_code": cv2.COLOR_BAYER_RG2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SGBRG10:{ "depth":10, "cvt_code": cv2.COLOR_BAYER_GR2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SGRBG10:{ "depth":10, "cvt_code": cv2.COLOR_BAYER_GB2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SRGGB10:{ "depth":10, "cvt_code": cv2.COLOR_BAYER_BG2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_Y10:{ "depth":10, "cvt_code": -1, "convert2rgb": 0},
    }
    pixfmt_map_xavier_nx = {
        v4l2.V4L2_PIX_FMT_SBGGR10:{ "depth":16, "cvt_code": cv2.COLOR_BAYER_RG2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SGBRG10:{ "depth":16, "cvt_code": cv2.COLOR_BAYER_GR2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SGRBG10:{ "depth":16, "cvt_code": cv2.COLOR_BAYER_GB2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SRGGB10:{ "depth":16, "cvt_code": cv2.COLOR_BAYER_BG2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_Y10:{ "depth":16, "cvt_code": -1, "convert2rgb": 0},
    }

    pixfmt_map_raw8 = {
        v4l2.V4L2_PIX_FMT_SBGGR8:{ "depth":8, "cvt_code": cv2.COLOR_BAYER_RG2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SGBRG8:{ "depth":8, "cvt_code": cv2.COLOR_BAYER_GR2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SGRBG8:{ "depth":8, "cvt_code": cv2.COLOR_BAYER_GB2BGR, "convert2rgb": 0},
        v4l2.V4L2_PIX_FMT_SRGGB8:{ "depth":8, "cvt_code": cv2.COLOR_BAYER_BG2BGR, "convert2rgb": 0},
    }

    AUTO_CONVERT_TO_RGB = { "depth":-1, "cvt_code": -1, "convert2rgb": 1}

    DEVICE_REG_BASE     = 0x0100
    PIXFORMAT_REG_BASE  = 0x0200
    FORMAT_REG_BASE     = 0x0300
    CTRL_REG_BASE       = 0x0400
    SENSOR_REG_BASE     = 0x500

    STREAM_ON                = (DEVICE_REG_BASE | 0x0000)
    FIRMWARE_VERSION_REG     = (DEVICE_REG_BASE | 0x0001)
    SENSOR_ID_REG            = (DEVICE_REG_BASE | 0x0002)
    DEVICE_ID_REG            = (DEVICE_REG_BASE | 0x0003)
    FIRMWARE_SENSOR_ID_REG   = (DEVICE_REG_BASE | 0x0005)
    SERIAL_NUMBER_REG        = (DEVICE_REG_BASE | 0x0006)

    PIXFORMAT_INDEX_REG     = (PIXFORMAT_REG_BASE | 0x0000)
    PIXFORMAT_TYPE_REG      = (PIXFORMAT_REG_BASE | 0x0001)
    PIXFORMAT_ORDER_REG     = (PIXFORMAT_REG_BASE | 0x0002)
    MIPI_LANES_REG          = (PIXFORMAT_REG_BASE | 0x0003)

    RESOLUTION_INDEX_REG    = (FORMAT_REG_BASE | 0x0000)
    FORMAT_WIDTH_REG        = (FORMAT_REG_BASE | 0x0001)
    FORMAT_HEIGHT_REG       = (FORMAT_REG_BASE | 0x0002)

    CTRL_INDEX_REG  = (CTRL_REG_BASE | 0x0000)
    CTRL_ID_REG     = (CTRL_REG_BASE | 0x0001)
    CTRL_MIN_REG    = (CTRL_REG_BASE | 0x0002)
    CTRL_MAX_REG    = (CTRL_REG_BASE | 0x0003)
    CTRL_STEP_REG   = (CTRL_REG_BASE | 0x0004)
    CTRL_DEF_REG    = (CTRL_REG_BASE | 0x0005)
    CTRL_VALUE_REG  = (CTRL_REG_BASE | 0x0006)

    SENSOR_RD_REG = (SENSOR_REG_BASE |0x0001)
    SENSOR_WR_REG = (SENSOR_REG_BASE |0x0002)

    NO_DATA_AVAILABLE = 0xFFFFFFFE

    DEVICE_ID = 0x0030

    def __init__(self, device_num):
        import subprocess
        command = ['bash', '-c', 'source scripts/jetson_variables.sh && env']

        proc = subprocess.Popen(command, stdout = subprocess.PIPE)
        environment_vars = {}
        for line in proc.stdout:
            (key, _, value) = line.partition(b"=")
            environment_vars[key] = value

        proc.communicate() 

        # Jetson Model
        if b"Xavier NX" in environment_vars[b"JETSON_TYPE"].strip():
            ArducamUtils.pixfmt_map = ArducamUtils.pixfmt_map_xavier_nx

        self.vd = open('/dev/video{}'.format(device_num), 'w')
        self.refresh()

    def refresh(self):
        self.config = self.get_pixfmt_cfg()

    def read_sensor(self, reg):
        i2c = arducam_i2c()
        i2c.reg = reg
        fcntl.ioctl(self.vd, VIDIOC_R_I2C, i2c)
        return i2c.val

    def write_sensor(self, reg, val):
        i2c = arducam_i2c()
        i2c.reg = reg
        i2c.val = val
        return fcntl.ioctl(self.vd, VIDIOC_W_I2C, i2c)

    def read_dev(self, reg):
        dev = arducam_dev()
        dev.reg = reg
        ret = fcntl.ioctl(self.vd, VIDIOC_R_DEV, dev)
        return ret, dev.val

    def write_dev(self, reg, val):
        dev = arducam_dev()
        dev.reg = reg
        dev.val = val
        return fcntl.ioctl(self.vd, VIDIOC_W_DEV, dev)

    def get_device_info(self):
        fw_sensor_id = self.read_dev(ArducamUtils.FIRMWARE_SENSOR_ID_REG)
        sensor_id = self.read_dev(ArducamUtils.SENSOR_ID_REG)
        fw_version = self.read_dev(ArducamUtils.FIRMWARE_VERSION_REG)
        serial_number = self.read_dev(ArducamUtils.SERIAL_NUMBER_REG)

        pass

    def convert(self, frame):
        if self.convert2rgb == 1:
            return frame
        
        if self.depth != -1:
            frame = cv2.convertScaleAbs(frame, None, 256.0 / (1 << self.depth))
            frame = frame.astype(np.uint8)

        if self.cvt_code != -1:
            frame = cv2.cvtColor(frame, self.cvt_code)

        return frame

    def get_pixelformat(self):
        fmt = v4l2.v4l2_format()
        fmt.type = v4l2.V4L2_BUF_TYPE_VIDEO_CAPTURE
        ret = fcntl.ioctl(self.vd, v4l2.VIDIOC_G_FMT, fmt)
        return ret, fmt.fmt.pix.pixelformat

    # Find the actual pixel format
    def get_pixfmt_cfg(self):
        ret, pixfmt = self.get_pixelformat()

        pf = ArducamUtils.pixfmt_map_raw8.get(pixfmt, None)
        if pf != None:
            return pf

        if pixfmt != v4l2.V4L2_PIX_FMT_Y16:
            return ArducamUtils.AUTO_CONVERT_TO_RGB
        fmtdesc = v4l2.v4l2_fmtdesc()
        fmtdesc.index = 0
        fmtdesc.type = v4l2.V4L2_BUF_TYPE_VIDEO_CAPTURE
        while True:
            try:
                fcntl.ioctl(self.vd, v4l2.VIDIOC_ENUM_FMT, fmtdesc)
                pixfmt = ArducamUtils.pixfmt_map.get(fmtdesc.pixelformat, None)
                if pixfmt != None:
                    return pixfmt
                fmtdesc.index += 1
            except Exception as e:
                break
        return ArducamUtils.AUTO_CONVERT_TO_RGB

    def get_pixelformats(self):
        pixfmts = []
        fmtdesc = v4l2.v4l2_fmtdesc()
        fmtdesc.index = 0
        fmtdesc.type = v4l2.V4L2_BUF_TYPE_VIDEO_CAPTURE
        while True:
            try:
                fcntl.ioctl(self.vd, v4l2.VIDIOC_ENUM_FMT, fmtdesc)
                pixfmts.append((fmtdesc.pixelformat, fmtdesc.description))
                fmtdesc.index += 1
            except Exception as e:
                break
        return pixfmts

    def get_framesizes(self, pixel_format = v4l2.V4L2_PIX_FMT_Y16):
        framesizes = []
        framesize = v4l2.v4l2_frmsizeenum()
        framesize.index = 0
        framesize.pixel_format = pixel_format
        while True:
            try:
                fcntl.ioctl(self.vd, v4l2.VIDIOC_ENUM_FRAMESIZES, framesize)
                framesizes.append((framesize.discrete.width, framesize.discrete.height))
                framesize.index += 1
            except Exception as e:
                break
        return framesizes

    def __getattr__(self, key):
        return self.config.get(key)