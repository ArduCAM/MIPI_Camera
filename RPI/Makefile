CROSS_COMPILE 	?=
CROSS_PREFIX	?=
CC		:= $(CROSS_COMPILE)gcc
CXX		:= $(CROSS_COMPILE)g++
CFLAGS 	?= -I. -g -O0 -std=gnu11 
# CXXFLAGS ?= -I. -g -std=gnu++11 /usr/local/lib/libopencv_core.so -lopencv_core /usr/local/lib/libopencv_line_descriptor.so -lopencv_line_descriptor `pkg-config --cflags --libs opencv` 
CXXFLAGS ?= -I. -g -std=gnu++11 \
/usr/local/lib/libopencv_core.so \
/usr/local/lib/libopencv_line_descriptor.so  \
/usr/local/lib/libopencv_imgproc.so \
-I/usr/include/opencv /usr/lib/arm-linux-gnueabihf/libopencv_calib3d.so -lopencv_calib3d /usr/lib/arm-linux-gnueabihf/libopencv_contrib.so -lopencv_contrib /usr/lib/arm-linux-gnueabihf/libopencv_features2d.so -lopencv_features2d /usr/lib/arm-linux-gnueabihf/libopencv_flann.so -lopencv_flann /usr/lib/arm-linux-gnueabihf/libopencv_gpu.so -lopencv_gpu /usr/lib/arm-linux-gnueabihf/libopencv_highgui.so -lopencv_highgui /usr/lib/arm-linux-gnueabihf/libopencv_legacy.so -lopencv_legacy /usr/lib/arm-linux-gnueabihf/libopencv_ml.so -lopencv_ml /usr/lib/arm-linux-gnueabihf/libopencv_objdetect.so -lopencv_objdetect /usr/lib/arm-linux-gnueabihf/libopencv_ocl.so -lopencv_ocl /usr/lib/arm-linux-gnueabihf/libopencv_photo.so -lopencv_photo /usr/lib/arm-linux-gnueabihf/libopencv_stitching.so -lopencv_stitching /usr/lib/arm-linux-gnueabihf/libopencv_superres.so -lopencv_superres /usr/lib/arm-linux-gnueabihf/libopencv_ts.so -lopencv_ts /usr/lib/arm-linux-gnueabihf/libopencv_video.so -lopencv_video /usr/lib/arm-linux-gnueabihf/libopencv_videostab.so -lopencv_videostab 
# CXXFLAGS ?= -I. -g -std=gnu++11 -I/usr/local/include/opencv2 /usr/local/lib/{libopencv_aruco.so,libopencv_bgsegm.so,libopencv_bioinspired.so,libopencv_calib3d.so,libopencv_ccalib.so,libopencv_core.so,libopencv_cvv.so,libopencv_datasets.so,libopencv_dnn_objdetect.so,libopencv_dnn.so,libopencv_dnn_superres.so,libopencv_dpm.so,libopencv_face.so,libopencv_features2d.so,libopencv_flann.so,libopencv_freetype.so,libopencv_fuzzy.so,libopencv_gapi.so,libopencv_hfs.so,libopencv_highgui.so,libopencv_imgcodecs.so,libopencv_img_hash.so,libopencv_imgproc.so,libopencv_intensity_transform.so,libopencv_line_descriptor.so,libopencv_ml.so,libopencv_objdetect.so,libopencv_optflow.so,libopencv_phase_unwrapping.so,libopencv_photo.so,libopencv_plot.so,libopencv_quality.so,libopencv_rapid.so,libopencv_reg.so,libopencv_rgbd.so,libopencv_saliency.so,libopencv_shape.so,libopencv_stereo.so,libopencv_stitching.so,libopencv_structured_light.so,libopencv_superres.so,libopencv_surface_matching.so,libopencv_text.so,libopencv_tracking.so,libopencv_videoio.so,libopencv_video.so,libopencv_videostab.so,libopencv_xfeatures2d.so,libopencv_ximgproc.so,libopencv_xobjdetect.so,libopencv_xphoto.so} 
LDFLAGS	?=
LIBS	:= -larducam_mipicamera -lpthread
OLIB	:= lib
examples:= video4cameras preview_setMode arducamstill capture video list_format capture_raw raw_callback yuv_callback read_write_sensor_reg ov9281_external_trigger 2MPGlobalShuterDemo preview-camera0 preview-dualcam capture-dualcam video2stdout capture2opencv qrcode_detection opencvGui take_ardu_pic
# examples:= video4cameras preview_setMode arducamstill capture video list_format capture_raw raw_callback yuv_callback read_write_sensor_reg ov9281_external_trigger 2MPGlobalShuterDemo preview-camera0 preview-dualcam capture-dualcam video2stdout take_ardu_pic
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(examples)

arducamstill: arducamstill.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
preview: preview.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
preview_setMode: preview_setMode.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
video4cameras: video4cameras.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
take_ardu_pic: take_ardu_pic.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
capture: capture.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
capture_raw: capture_raw.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

video: video.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

list_format: list_format.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

raw_callback: raw_callback.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

read_write_sensor_reg : read_write_sensor_reg.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

video2stdout : video2stdout.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

capture2opencv : capture2opencv.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

opencvGui : opencvGui.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

qrcode_detection: qrcode_detection.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS) -lzbar

preview-camera0 : preview-camera0.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

preview-dualcam : preview-dualcam.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

capture-dualcam : capture-dualcam.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

ov9281_external_trigger : ov9281_external_trigger.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
	
2MPGlobalShuterDemo: 2MPGlobalShuterDemo.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
	
yuv_callback: yuv_callback.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f *.o
	-rm -f $(examples)

.PHONY: install

install: 
	sudo install -m 644 $(OLIB)/libarducam_mipicamera.so /usr/lib/
	install take_ardu_pic $(HOME)/bin/
	rsync -r lens_shading_table $(HOME)/bin/

