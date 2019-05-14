#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

FILE *fd;
int video_callback(BUFFER *buffer) {
    if (TIME_UNKNOWN == buffer->pts) {
    }
    LOG("buffer length = %d, pts = %llu", buffer->length, buffer->pts);

    if (buffer->length) {
        if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG) {
            // SPS PPS
            if (fd) {
                fwrite(buffer->data, 1, buffer->length, fd);
                fflush(fd);
            }
        }
        if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
            /// Encoder outputs inline Motion Vectors
        } else {
            // MMAL_BUFFER_HEADER_FLAG_KEYFRAME
            // MMAL_BUFFER_HEADER_FLAG_FRAME_END
            if (fd) {
                int bytes_written = fwrite(buffer->data, 1, buffer->length, fd);
                fflush(fd);
            }
        }
    }
    return 0;
}
static void default_status(VIDEO_ENCODER_STATE *state) {
    // Default everything to zero
    memset(state, 0, sizeof(VIDEO_ENCODER_STATE));
    state->encoding = VIDEO_ENCODING_H264;
    state->bitrate = 17000000;
    state->immutableInput = 1; // Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                               // the camera output or the encoder output (with compression artifacts)
    /**********************H264 only**************************************/
    state->intraperiod = -1;                  // Not set
                                              // Specify the intra refresh period (key frame rate/GoP size).
                                              // Zero to produce an initial I-frame and then just P-frames.
    state->quantisationParameter = 0;         // Quantisation parameter. Use approximately 10-40. Default 0 (off)
    state->profile = VIDEO_PROFILE_H264_HIGH; // Specify H264 profile to use for encoding
    state->level = VIDEO_LEVEL_H264_4;        // Specify H264 level to use for encoding
    state->bInlineHeaders = 0;                // Insert inline headers (SPS, PPS) to stream
    state->inlineMotionVectors = 0;           // output motion vector estimates
    state->intra_refresh_type = -1;           // Set intra refresh type
    state->addSPSTiming = 0;                  // zero or one
    state->slices = 1;
    /**********************H264 only**************************************/
}

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int count = 0;
    int width = 0, height = 0;
    int res = arducam_init_camera(&camera_instance);
    LOG("init camera status = %d", res);

    width = 1920;
    height = 1080;
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    }

    fd = fopen("test.h264", "wb");
    VIDEO_ENCODER_STATE video_state;
    default_status(&video_state);
    // start video callback
    // Set video_state to NULL, using default parameters
    arducam_set_video_callback(camera_instance, &video_state, video_callback, NULL);
    usleep(1000 * 1000 * 10);

    // stop video callback
    arducam_set_video_callback(camera_instance, NULL, NULL, NULL);
    fclose(fd);

    res = arducam_close_camera(camera_instance);
    LOG("close camera status = %d", res);
    return 0;
}