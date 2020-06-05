import v4l2
import fcntl
import ctypes
import argparse

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

def read_sensor(vd, reg):
    i2c = arducam_i2c()
    i2c.reg = reg
    ret = fcntl.ioctl(vd, VIDIOC_R_I2C, i2c)
    return ret, i2c.val

def write_sensor(vd, reg, val):
    i2c = arducam_i2c()
    i2c.reg = reg
    i2c.val = val
    return fcntl.ioctl(vd, VIDIOC_W_I2C, i2c)

def read_regs(vd, regs):
    for reg in regs:
        ret, value = read_sensor(vd, reg)
        print("read reg: 0x{:02X}, value: 0x{:02X}, ret: {}".format(reg, value, ret))

def write_regs(vd, regs, values):
    for reg,val in zip(regs, values):
        ret = write_sensor(vd, reg, val)
        print("write reg: 0x{:02X}, value: 0x{:02X}, ret: {}".format(reg, val, ret))

def read_dev(vd, reg):
    dev = arducam_dev()
    dev.reg = reg
    ret = fcntl.ioctl(vd, VIDIOC_R_DEV, dev)
    return ret, dev.val

def write_dev(vd, reg, val):
    dev = arducam_dev()
    dev.reg = reg
    dev.val = val
    return fcntl.ioctl(vd, VIDIOC_W_DEV, dev)

def read_dev_regs(vd, regs):
    for reg in regs:
        ret, value = read_dev(vd, reg)
        print("read device reg: 0x{:02X}, value: 0x{:02X}, ret: {}".format(reg, value, ret))

def write_dev_regs(vd, regs, values):
    for reg,val in zip(regs, values):
        ret = write_dev(vd, reg, val)
        print("write device reg: 0x{:02X}, value: 0x{:02X}, ret: {}".format(reg, val, ret))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="This program is used to directly read and write the registers of \
            Arducam Jetson Nano MIPI Camera Sensor.")
    parser.add_argument('-d', '--device', default=0, type=int, nargs='?',
                        help='/dev/video<DEVICE> default is 0')

    parser.add_argument('-r', '--regs', type=lambda x: int(x,0), nargs='+',
                        help='The address of the register you want to read.')
    parser.add_argument('-v', '--values', type=lambda x: int(x,0), nargs='+',
                        help='The register value to be written.')
    parser.add_argument('-rd', '--dev-regs', type=lambda x: int(x,0), nargs='+',
                        help='The address of the register you want to read.')
    parser.add_argument('-vd', '--dev-values', type=lambda x: int(x,0), nargs='+',
                        help='The register value to be written.')

    args = parser.parse_args()

    if args.values != None and len(args.regs) != len(args.values):
        print("Requires the same number of regs and values.")
        exit(0)
    if args.dev_values != None and len(args.dev_regs) != len(args.dev_values):
        print("Requires the same number of dev-regs and dev-values.")
        exit(0)

    if args.regs == None and args.dev_regs == None:
        print("error: argument -r/--regs or -rd/--dev-regs is required")
        exit(0)

    vd = open('/dev/video{}'.format(args.device), 'w')

    if args.values != None:
        write_regs(vd, args.regs, args.values)
    elif args.regs != None:
        read_regs(vd, args.regs)

    if args.dev_values != None:
        write_dev_regs(vd, args.dev_regs, args.dev_values)
    elif args.dev_regs != None:
        read_dev_regs(vd, args.dev_regs)
