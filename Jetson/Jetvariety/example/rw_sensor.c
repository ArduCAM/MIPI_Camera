#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

struct arducam_i2c {
	uint16_t reg;
	uint16_t val;
};

struct arducam_dev {
	uint16_t reg;
	uint32_t val;
};

#define VIDIOC_R_I2C	_IOWR('V', BASE_VIDIOC_PRIVATE + 0,  struct arducam_i2c)
#define VIDIOC_W_I2C	_IOWR('V', BASE_VIDIOC_PRIVATE + 1,  struct arducam_i2c)
#define VIDIOC_R_DEV	_IOWR('V', BASE_VIDIOC_PRIVATE + 2,  struct arducam_dev)
#define VIDIOC_W_DEV	_IOWR('V', BASE_VIDIOC_PRIVATE + 3,  struct arducam_dev)

static int read_sensor(int fd, uint16_t reg, uint16_t *val) {
	struct arducam_i2c i2c = {reg, 0};
	int ret = ioctl(fd, VIDIOC_R_I2C, &i2c);
	*val = i2c.val;
	return ret;
}

static int write_sensor(int fd, uint16_t reg, uint16_t val) {
	struct arducam_i2c i2c = {reg, val};
	return ioctl(fd, VIDIOC_W_I2C, &i2c);
}

static int read_device(int fd, uint16_t reg, uint32_t *val) {
	struct arducam_dev dev = {reg, 0};
	int ret = ioctl(fd, VIDIOC_R_DEV, &dev);
	*val = dev.val;
	return ret;
}

static int write_device(int fd, uint16_t reg, uint32_t val) {
	struct arducam_dev dev = {reg, val};
	return ioctl(fd, VIDIOC_W_DEV, &dev);
}


int main() {
	printf("Read ID: 0x%lx, Write ID: 0x%lx\n", VIDIOC_R_I2C, VIDIOC_W_I2C);
	const char *video_device = "/dev/video0";
	// open video device
	int fd = open(video_device, O_RDWR);
	if (fd < 0) {
        if (errno == ENOENT)
            printf("%s: File does not exist.\n", video_device);
        else if(errno == EACCES) 
            printf("%s: Permission denied.\n", video_device);
		return -1;
	}

	uint16_t val;
	uint32_t val32;
	int ret = read_sensor(fd, 0x3500, &val);
	printf("Read reg: 0x%02X, val: 0x%02X, ret: %d\n", 0x3500, val, ret);

	ret = write_sensor(fd, 0x3500, 0x03);
	printf("Write reg: 0x%02X, val: 0x%02X, ret: %d\n", 0x3500, 0x03, ret);

	ret = read_device(fd, 0x0105, &val32);
	printf("Read device reg: 0x%02X, val: 0x%02X, ret: %d\n", 0x0105, val32, ret);

	ret = read_device(fd, 0x0106, &val32);
	printf("Read device reg: 0x%02X, val: 0x%02X, ret: %d\n", 0x0106, val32, ret);
	close(fd);

	return 0;
}
