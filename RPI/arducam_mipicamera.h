#ifndef _ARDUCAM_MIPI_CAMERA_H__
#define _ARDUCAM_MIPI_CAMERA_H__
#include "stdint.h"
#define FOURCC(a, b, c, d) ((a) | (b << 8) | (c << 16) | (d << 24))

#define IMAGE_ENCODING_I420 FOURCC('I', '4', '2', '0')
#define IMAGE_ENCODING_JPEG FOURCC('J', 'P', 'E', 'G')

#define VIDEO_ENCODING_H264 FOURCC('H', '2', '6', '4')

#define OUTPUT_FLAG_KEEP_BUFFER_REQUIREMENTS 0x08
#define OUTPUT_FLAG_BUFFER_ALLOCATION_USE_MMAL_CORE 0x10

#define VIDEO_LEVEL_H264_4 0x1C
#define VIDEO_LEVEL_H264_41 0x1D
#define VIDEO_LEVEL_H264_42 0x1E

#define VIDEO_PROFILE_H264_BASELINE 0x19
#define VIDEO_PROFILE_H264_MAIN 0x1A
#define VIDEO_PROFILE_H264_HIGH 0x1C

#define VIDEO_INTRA_REFRESH_CYCLIC 0x00
#define VIDEO_INTRA_REFRESH_ADAPTIVE 0x01
#define VIDEO_INTRA_REFRESH_BOTH 0x02
#define VIDEO_INTRA_REFRESH_CYCLIC_MROWS 0x7F000001
/** \name Special Unknown Time Value
 * Timestamps in MMAL are defined as signed 64 bits integer values representing microseconds.
 * However a pre-defined special value is used to signal that a timestamp is not known. */
/* @{ */
#define TIME_UNKNOWN (INT64_C(1) << 63) /**< Special value signalling that time is not known */
/* @} */

/** \name Buffer header flags
 * \anchor bufferheaderflags
 * The following flags describe properties of a buffer header */
/* @{ */
/** Signals that the current payload is the end of the stream of data */
#define MMAL_BUFFER_HEADER_FLAG_EOS (1 << 0)
/** Signals that the start of the current payload starts a frame */
#define MMAL_BUFFER_HEADER_FLAG_FRAME_START (1 << 1)
/** Signals that the end of the current payload ends a frame */
#define MMAL_BUFFER_HEADER_FLAG_FRAME_END (1 << 2)
/** Signals that the current payload contains only complete frames (1 or more) */
#define MMAL_BUFFER_HEADER_FLAG_FRAME (MMAL_BUFFER_HEADER_FLAG_FRAME_START | MMAL_BUFFER_HEADER_FLAG_FRAME_END)
/** Signals that the current payload is a keyframe (i.e. self decodable) */
#define MMAL_BUFFER_HEADER_FLAG_KEYFRAME (1 << 3)
/** Signals a discontinuity in the stream of data (e.g. after a seek).
 * Can be used for instance by a decoder to reset its state */
#define MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY (1 << 4)
/** Signals a buffer containing some kind of config data for the component
 * (e.g. codec config data) */
#define MMAL_BUFFER_HEADER_FLAG_CONFIG (1 << 5)
/** Signals an encrypted payload */
#define MMAL_BUFFER_HEADER_FLAG_ENCRYPTED (1 << 6)
/** Signals a buffer containing side information */
#define MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO (1 << 7)
/** Signals a buffer which is the snapshot/postview image from a stills capture */
#define MMAL_BUFFER_HEADER_FLAGS_SNAPSHOT (1 << 8)
/** Signals a buffer which contains data known to be corrupted */
#define MMAL_BUFFER_HEADER_FLAG_CORRUPTED (1 << 9)
/** Signals that a buffer failed to be transmitted */
#define MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED (1 << 10)
/** Signals the output buffer won't be used, just update reference frames */
#define MMAL_BUFFER_HEADER_FLAG_DECODEONLY (1 << 11)
/** Signals that the end of the current payload ends a NAL */
#define MMAL_BUFFER_HEADER_FLAG_NAL_END (1 << 12)
/* @} */

typedef struct {
    uint32_t encoding;
    int quality; // JPEG quality setting (1-100)
} IMAGE_FORMAT;

/** Describes a rectangle */
typedef struct {
    int32_t x;      /**< x coordinate (from left) */
    int32_t y;      /**< y coordinate (from top) */
    int32_t width;  /**< width */
    int32_t height; /**< height */
} RECTANGLE;

typedef struct {
    int fullscreen;   // 0 is use previewRect, non-zero to use full screen
    int opacity;      // Opacity of window - 0 = transparent, 255 = opaque
    RECTANGLE window; // Destination rectangle for the preview window.
} PREVIEW_PARAMS;

typedef struct
{
    uint32_t encoding;         /// Requested codec video encoding (MJPEG or H264)
    int bitrate;               /// Requested bitrate
    int intraperiod;           /// Intra-refresh period (key frame rate)
    int quantisationParameter; /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
    int bInlineHeaders;        /// Insert inline headers to stream (SPS, PPS)
    int immutableInput;        /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                               /// the camera output or the encoder output (with compression artifacts)
    int profile;               /// H264 profile to use for encoding
    int level;                 /// H264 level to use for encoding

    int inlineMotionVectors; /// Encoder outputs inline Motion Vectors
    int intra_refresh_type;  /// What intra refresh type to use. -1 to not set.
    int addSPSTiming;        /// 0 or 1
    int slices;              /// Horizontal slices per frame. Default 1 (off)
} VIDEO_ENCODER_STATE;

typedef struct {
    void *priv;
    uint8_t *data;
    uint32_t alloc_size;
    uint32_t length;
    uint32_t flags;
    int64_t pts;
    void *userdata;
} BUFFER;

struct format {
    int width;
    int height;
};
struct camera_ctrl {
    int id;
    const char *desc;
    int max_value;
    int min_value;
    int default_value;
};

typedef int (*OUTPUT_CALLBACK)(BUFFER *buffer);

typedef void *CAMERA_INSTANCE;

int arducam_init_camera(CAMERA_INSTANCE *camera_instance);
int arducam_set_resolution(CAMERA_INSTANCE camera_instance, int *width, int *height);
int arducam_set_video_callback(CAMERA_INSTANCE camera_instance, VIDEO_ENCODER_STATE *encoder_state, OUTPUT_CALLBACK callback, void *userdata);
BUFFER *arducam_capture(CAMERA_INSTANCE camera_instance, IMAGE_FORMAT *format, int timeout);
void arducam_release_buffer(BUFFER *buffer);
int arducam_start_preview(CAMERA_INSTANCE camera_instance, PREVIEW_PARAMS *preview_params);
int arducam_stop_preview(CAMERA_INSTANCE camera_instance);
int arducam_close_camera(CAMERA_INSTANCE camera_instance);

int arducam_set_control(CAMERA_INSTANCE camera_instance, int ctrl_id, int value);
int arducam_get_control(CAMERA_INSTANCE camera_instance, int ctrl_id, int *value);

int arducam_get_support_formats(CAMERA_INSTANCE camera_instance, struct format *fmt, int index);
int arducam_get_support_controls(CAMERA_INSTANCE camera_instance, struct camera_ctrl *cam_ctrl, int index);

#endif