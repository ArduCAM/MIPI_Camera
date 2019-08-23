'''
This script is a wrapper for the libarducam_mipicamera.so dynamic library. 
To use this script you need to pre-install libarducam_mipicamera.so
'''
from ctypes import *
import numpy as np
import sys

try:
    camera_lib = cdll.LoadLibrary("libarducam_mipicamera.so")
except:
    print("Load libarducam_mipicamera fail.")
    sys.exit(0)


def FOURCC(a, b, c, d):
    return (ord(a) | (ord(b) << 8) | (ord(c) << 16) | (ord(d) << 24))

IMAGE_ENCODING_I420 = FOURCC('I', '4', '2', '0')
IMAGE_ENCODING_JPEG = FOURCC('J', 'P', 'E', 'G')
IMAGE_ENCODING_RAW_BAYER = FOURCC('R', 'A', 'W', ' ')

VIDEO_ENCODING_H264 = FOURCC('H', '2', '6', '4')

image_encodings = {
    'i420' : IMAGE_ENCODING_I420,
    'jpeg' : IMAGE_ENCODING_JPEG,
    'raw' : IMAGE_ENCODING_RAW_BAYER,
}
video_encodings = {
    'h264' : VIDEO_ENCODING_H264,
}


# H264 level
VIDEO_LEVEL_H264_4                  = 0x1C
VIDEO_LEVEL_H264_41                 = 0x1D
VIDEO_LEVEL_H264_42                 = 0x1E
# H264 profile
VIDEO_PROFILE_H264_BASELINE         = 0x19
VIDEO_PROFILE_H264_MAIN             = 0x1A
VIDEO_PROFILE_H264_HIGH             = 0x1C

VIDEO_INTRA_REFRESH_CYCLIC          = 0x00
VIDEO_INTRA_REFRESH_ADAPTIVE        = 0x01
VIDEO_INTRA_REFRESH_BOTH            = 0x02
VIDEO_INTRA_REFRESH_CYCLIC_MROWS    = 0x7F000001

MMAL_TIME_UNKNOWN = c_int64(1<<63).value

MMAL_BUFFER_HEADER_FLAG_EOS                    = (1<<0)
MMAL_BUFFER_HEADER_FLAG_FRAME_START            = (1<<1)
MMAL_BUFFER_HEADER_FLAG_FRAME_END              = (1<<2)
MMAL_BUFFER_HEADER_FLAG_FRAME                  = (MMAL_BUFFER_HEADER_FLAG_FRAME_START|MMAL_BUFFER_HEADER_FLAG_FRAME_END)
MMAL_BUFFER_HEADER_FLAG_KEYFRAME               = (1<<3)
MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY          = (1<<4)
MMAL_BUFFER_HEADER_FLAG_CONFIG                 = (1<<5)
MMAL_BUFFER_HEADER_FLAG_ENCRYPTED              = (1<<6)
MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO          = (1<<7)
MMAL_BUFFER_HEADER_FLAGS_SNAPSHOT              = (1<<8)
MMAL_BUFFER_HEADER_FLAG_CORRUPTED              = (1<<9)
MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED    = (1<<10)
MMAL_BUFFER_HEADER_FLAG_DECODEONLY             = (1<<11)
MMAL_BUFFER_HEADER_FLAG_NAL_END                = (1<<12)   

class FRACT(Structure):
    _fields_ = [
        ("numerator",c_uint32),
        ("denominator",c_uint32),
    ]
    def getdict(struct):
        return dict((field, getattr(struct, field)) for field, _ in struct._fields_)

class FORMAT(Structure):
    _fields_ = [
        ("mode", c_int),
        ("width", c_int),
        ("height", c_int),
        ("pixelformat", c_uint32),
        ("frameintervals", FRACT),
        ("description", c_char_p),
        ("reserved", c_uint32 * 4),
    ]
    def getdict(struct):
        return dict((field, getattr(struct, field) if field != "frameintervals" else struct.frameintervals.getdict()) \
            for field, _ in struct._fields_)

class IMAGE_FORMAT(Structure):
    _fields_ = [
        ("encoding",c_uint32),
        ("quality",c_int),
    ]

class RECTANGLE(Structure):
    _fields_ = [
        ("x",c_int32),
        ("y",c_int32),
        ("width",c_int32),
        ("height",c_int32),
    ]

class PREVIEW_PARAMS(Structure):
    _fields_ = [
        ("fullscreen",c_int),
        ("opacity",c_int),
        ("window",RECTANGLE),
    ]

class VIDEO_ENCODER_STATE(Structure):
    _fields_ = [
        ("encoding",c_uint32),              # Requested codec video encoding (MJPEG or H264)  
        ("bitrate",c_int),                  # Requested bitrate
        ("intraperiod",c_int),              # Intra-refresh period (key frame rate)
        ("quantisationParameter",c_int),    # Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
        ("bInlineHeaders",c_int),           # Insert inline headers to stream (SPS, PPS)
        ("immutableInput",c_int),           # Not working               
        ("profile",c_int),                  # H264 profile to use for encoding
        ("level",c_int),                    # H264 level to use for encoding
        ("inlineMotionVectors",c_int),      # Encoder outputs inline Motion Vectors
        ("intra_refresh_type",c_int),       # What intra refresh type to use. -1 to not set.
        ("addSPSTiming",c_int),             # 0 or 1
        ("slices",c_int),                   # Horizontal slices per frame. Default 1 (off)
    ]

class BUFFER(Structure):
    _fields_ = [
        ("priv",c_void_p),
        ("data",POINTER(c_ubyte)),
        ("alloc_size",c_uint32),
        ("length",c_uint32),
        ("flags",c_uint32),
        ("pts",c_uint64),
        ("userdata",c_void_p),
    ]

class CAMERA_CTRL(Structure):
    _fields_ = [
        ("id",c_int),
        ("desc",c_char_p),
        ("max_value",c_int),
        ("min_value",c_int),
        ("default_value",c_int),
    ]

class CAMERA_INTERFACE(Structure):
    _fields_ = [
        ("i2c_bus",c_int),
        ("camera_num",c_int),
        ("sda_pins",c_int * 2),
        ("scl_pins",c_int * 2),
        ("shutdown_pins",c_int * 2),
        ("led_pins",c_int * 2),
    ]


OUTPUT_CALLBACK = CFUNCTYPE(c_int, POINTER(BUFFER))

arducam_init_camera = camera_lib.arducam_init_camera
arducam_init_camera.argtypes = [POINTER(c_void_p)]
arducam_init_camera.restype = c_int

arducam_init_camera2 = camera_lib.arducam_init_camera2
arducam_init_camera2.argtypes = [POINTER(c_void_p), CAMERA_INTERFACE]
arducam_init_camera2.restype = c_int

arducam_set_resolution = camera_lib.arducam_set_resolution
arducam_set_resolution.argtypes = [c_void_p, POINTER(c_int), POINTER(c_int)]
arducam_set_resolution.restype = c_int

arducam_set_mode = camera_lib.arducam_set_mode
arducam_set_mode.argtypes = [c_void_p, c_int]
arducam_set_mode.restype = c_int

arducam_get_format = camera_lib.arducam_get_format
arducam_get_format.argtypes = [c_void_p, POINTER(FORMAT)]
arducam_get_format.restype = c_int

arducam_start_preview = camera_lib.arducam_start_preview
arducam_start_preview.argtypes = [c_void_p, POINTER(PREVIEW_PARAMS)]
arducam_start_preview.restype = c_int

arducam_stop_preview = camera_lib.arducam_stop_preview
arducam_start_preview.argtypes = [c_void_p]
arducam_start_preview.restype = c_int

arducam_capture = camera_lib.arducam_capture
arducam_capture.argtypes = [c_void_p, POINTER(IMAGE_FORMAT), c_int]
arducam_capture.restype = POINTER(BUFFER)

arducam_release_buffer = camera_lib.arducam_release_buffer
arducam_release_buffer.argtypes = [POINTER(BUFFER)]
arducam_release_buffer.restype = None

arducam_set_raw_callback = camera_lib.arducam_set_raw_callback
arducam_set_raw_callback.argtypes = [c_void_p, OUTPUT_CALLBACK, c_void_p]
arducam_set_raw_callback.restype = c_int

arducam_set_video_callback = camera_lib.arducam_set_video_callback
arducam_set_video_callback.argtypes = [c_void_p, POINTER(VIDEO_ENCODER_STATE), OUTPUT_CALLBACK, c_void_p]
arducam_set_video_callback.restype = c_int

arducam_reset_control = camera_lib.arducam_reset_control
arducam_reset_control.argtypes = [c_void_p, c_int]
arducam_reset_control.restype = c_int

arducam_set_control = camera_lib.arducam_set_control
arducam_set_control.argtypes = [c_void_p, c_int, c_int]
arducam_set_control.restype = c_int

arducam_get_control = camera_lib.arducam_get_control
arducam_get_control.argtypes = [c_void_p, c_int, POINTER(c_int)]
arducam_get_control.restype = c_int

arducam_get_support_formats = camera_lib.arducam_get_support_formats
arducam_get_support_formats.argtypes = [c_void_p, POINTER(FORMAT), c_int]
arducam_get_support_formats.restype = c_int

arducam_get_support_controls = camera_lib.arducam_get_support_controls
arducam_get_support_controls.argtypes = [c_void_p, POINTER(CAMERA_CTRL), c_int]
arducam_get_support_controls.restype = c_int


arducam_software_auto_exposure = camera_lib.arducam_software_auto_exposure
arducam_software_auto_exposure.argtypes = [c_void_p, c_int]
arducam_software_auto_exposure.restype = c_int

arducam_software_auto_white_balance = camera_lib.arducam_software_auto_white_balance
arducam_software_auto_white_balance.argtypes = [c_void_p, c_int]
arducam_software_auto_white_balance.restype = c_int

arducam_read_sensor_reg = camera_lib.arducam_read_sensor_reg
arducam_read_sensor_reg.argtypes = [c_void_p, c_uint16, POINTER(c_uint16)]
arducam_read_sensor_reg.restype = c_int

arducam_write_sensor_reg = camera_lib.arducam_write_sensor_reg
arducam_write_sensor_reg.argtypes = [c_void_p, c_uint16, c_uint16]
arducam_write_sensor_reg.restype = c_int

arducam_close_camera = camera_lib.arducam_close_camera
arducam_close_camera.argtypes = [c_void_p]
arducam_close_camera.restype = c_int

arducam_unpack_raw10_to_raw8 = camera_lib.arducam_unpack_raw10_to_raw8
arducam_unpack_raw10_to_raw8.argtypes = [POINTER(c_ubyte), c_int, c_int]
arducam_unpack_raw10_to_raw8.restype = POINTER(BUFFER)

arducam_unpack_raw10_to_raw16 = camera_lib.arducam_unpack_raw10_to_raw16
arducam_unpack_raw10_to_raw16.argtypes = [POINTER(c_ubyte), c_int, c_int]
arducam_unpack_raw10_to_raw16.restype = POINTER(BUFFER)

def align_down(size, align):
    return (size & ~((align)-1))

def align_up(size, align):
    return align_down(size + align - 1, align)

class buffer(object):
    buffer_ptr = None
    def __init__(self, buff):
        if not isinstance(buff, POINTER(BUFFER)):
            raise TypeError("Expected parameter type is POINTER(BUFFER).")
        self.buffer_ptr = buff

    @property
    def as_array(self):
        return np.ctypeslib.as_array(self.buffer_ptr[0].data, shape=(self.length,))
    
    @property
    def data(self):
        return string_at(self.buffer_ptr[0].data,self.length)

    @property
    def length(self):
        return self.buffer_ptr[0].length
    @length.setter
    def length(self, value):
        self.buffer_ptr[0].length = value

    @property
    def alloc_size(self):
        return self.buffer_ptr[0].alloc_size
    @alloc_size.setter
    def alloc_size(self, value):
        self.buffer_ptr[0].alloc_size = value

    @property
    def flags(self):
        return self.buffer_ptr[0].flags
    @flags.setter
    def flags(self, value):
        self.buffer_ptr[0].flags = value

    @property
    def pts(self):
        return self.buffer_ptr[0].pts
    @pts.setter
    def pts(self, value):
        self.buffer_ptr[0].pts = value

    @property
    def userdata(self):
        if self.buffer_ptr[0].userdata == None:
            return None
        return cast(self.buffer_ptr[0].userdata, POINTER(py_object))[0]

    def release(self):
        arducam_release_buffer(self.buffer_ptr)

    def __del__(self):
        self.release()

def check_status(status,func_name):
    if status != 0:
        raise RuntimeError("{}: Unexpected result.".format(func_name))

class mipi_camera(object):
    
    def __init__(self):
        self.camera_instance = c_void_p(0)

    def init_camera(self):
        check_status(
            arducam_init_camera(byref(self.camera_instance)),
            sys._getframe().f_code.co_name
        )

    def init_camera2(self, camera_interface):
        check_status(
            arducam_init_camera2(self.camera_instance, camera_interface),
            sys._getframe().f_code.co_name
        )

    def set_resolution(self, width, height):
        _width = c_int(width)
        _height = c_int(height)
        check_status(
            arducam_set_resolution(self.camera_instance, byref(_width), byref(_height)),
            sys._getframe().f_code.co_name
        )
        return _width.value, _height.value

    def set_mode(self, mode):
        check_status(
            arducam_set_mode(self.camera_instance, mode),
            sys._getframe().f_code.co_name
        )

    def get_format(self):
        fmt = FORMAT()
        check_status(
            arducam_get_format(self.camera_instance, byref(fmt)),
            sys._getframe().f_code.co_name
        )
        return fmt.getdict()

    def start_preview(self, fullscreen = True, opacity = 255, window = None):
        rect = RECTANGLE(0, 0, 640, 480)
        if window is not None:
            try:
                rect.x, rect.y, rect.width, rect.height = window
            except (TypeError, ValueError) as e:
                raise TypeError(
                    "Invalid window rectangle (x, y, w, h) tuple: %s" % window)
        preview_params = PREVIEW_PARAMS(int(fullscreen), opacity, rect)
        check_status(
            arducam_start_preview(self.camera_instance, byref(preview_params) if preview_params!=None else None),
            sys._getframe().f_code.co_name
        )
        
    def stop_preview(self):
        check_status(
            arducam_stop_preview(self.camera_instance),
            sys._getframe().f_code.co_name
        )
        
    def capture(self, time_out = 3000, encoding = 'jpeg', quality = 90):
        if image_encodings[encoding] == None:
            raise TypeError("Unknown image encoding type.")
        image_format = IMAGE_FORMAT(image_encodings[encoding], quality)
        return buffer(arducam_capture(self.camera_instance, byref(image_format), time_out))

    def set_raw_callback(self, func = None, userdata = None):
        '''
        Important:You need to keep a reference to userdata. 
                  If userdata is released, using userdata in 
                  the callback function will crash the program.
        '''
        if userdata != None and not isinstance(userdata, py_object):
            raise TypeError("Userdata must be of type py_object")
        cfunc = OUTPUT_CALLBACK(func) if func != None else cast(None, OUTPUT_CALLBACK)
        check_status(
            arducam_set_raw_callback(self.camera_instance, cfunc, byref(userdata) if userdata != None else None),
            sys._getframe().f_code.co_name
        )

    def set_video_callback(self, func = None, userdata = None, **kwargs):    
        '''
        Important:You need to keep a reference to userdata. 
                  If userdata is released, using userdata in 
                  the callback function will crash the program.
        '''
        if userdata != None and not isinstance(userdata, py_object):
            raise TypeError("Userdata must be of type py_object")

        cfunc = OUTPUT_CALLBACK(func) if func != None else cast(None, OUTPUT_CALLBACK)

        options = {
            'encoding': VIDEO_ENCODING_H264,
            'bitrate': 17000000,
            'intraperiod': -1,
            'quantisationParameter': 0,
            'bInlineHeaders': 0,
            'immutableInput': 1,
            'profile': VIDEO_PROFILE_H264_HIGH,
            'level': VIDEO_LEVEL_H264_4,
            'inlineMotionVectors': 0,
            'intra_refresh_type': -1,
            'addSPSTiming': 0,
            'slices':1,
            }
        for arg_name in options:
            options[arg_name] = kwargs.pop(arg_name, options[arg_name])
        video_state = VIDEO_ENCODER_STATE()

        for arg_name, _ in video_state._fields_:
            setattr(video_state, arg_name, options[arg_name])
        check_status(
            arducam_set_video_callback(
                self.camera_instance, byref(video_state), 
                cfunc, byref(userdata) if userdata != None else None
            ),
            sys._getframe().f_code.co_name
        )

    def reset_control(self, ctrl_id):
        check_status(
            arducam_reset_control(self.camera_instance, ctrl_id),
            sys._getframe().f_code.co_name
        )

    def set_control(self, ctrl_id, value):
        check_status(
            arducam_set_control(self.camera_instance, ctrl_id, value),
            sys._getframe().f_code.co_name
        )

    def get_control(self, ctrl_id):
        _value = c_int(0)
        check_status(
            arducam_get_control(self.camera_instance, ctrl_id, byref(_value)),
            sys._getframe().f_code.co_name
        )
        return _value.value

    def get_support_formats(self):
        fmt = FORMAT()
        fmts = []
        i = 0
        while arducam_get_support_formats(self.camera_instance, byref(fmt), i) == 0:
            i += 1
            fmts.append(fmt.getdict())
        return fmts

    def get_support_controls(self):
        ctrl = CAMERA_CTRL()
        ctrls = []
        i = 0
        while arducam_get_support_controls(self.camera_instance, byref(ctrl), i) == 0:
            i += 1
            ctrls.append(ctrl)
            ctrl = CAMERA_CTRL()
        return ctrls

    def software_auto_exposure(self, enable = True):
        check_status(
            arducam_software_auto_exposure(self.camera_instance, int(enable)),
            sys._getframe().f_code.co_name
        )

    def software_auto_white_balance(self, enable = True):
        check_status(
            arducam_software_auto_white_balance(self.camera_instance, int(enable)),
            sys._getframe().f_code.co_name
        )

    def read_sensor_reg(self, address):
        _value = c_uint16(0)
        check_status(
            arducam_read_sensor_reg(self.camera_instance, address, byref(_value)),
            sys._getframe().f_code.co_name
        )
        return _value.value

    def write_sensor_reg(self, address, value):
        check_status(
            arducam_write_sensor_reg(self.camera_instance, address, value),
            sys._getframe().f_code.co_name
        )

    def close_camera(self):
        check_status(
            arducam_close_camera(self.camera_instance),
            sys._getframe().f_code.co_name
        )

pass

def unpack_raw10_to_raw8(buff, width, height):
    if not isinstance(buff, POINTER(BUFFER)):
        raise TypeError("Expected parameter type is POINTER(BUFFER).")
    return buffer(arducam_unpack_raw10_to_raw8(buffer[0].data, width, height))

def unpack_raw10_to_raw16(buff, width, height):
    if not isinstance(buff, POINTER(BUFFER)):
        raise TypeError("Expected parameter type is POINTER(BUFFER).")
    return buffer(arducam_unpack_raw10_to_raw16(buffer[0].data, width, height))

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

def remove_padding(data, width, height, bit_width):
    buff = np.frombuffer(data, np.uint8)
    real_width = width / 8 * bit_width
    align_width = align_up(real_width, 32)
    align_height = align_up(height, 16)
    
    buff = buff.reshape(align_height, align_width)
    buff = buff[:height, :real_width]
    buff = buff.reshape(height * real_width)
    return buff