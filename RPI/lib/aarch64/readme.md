## Build mmal
```bash
git clone https://github.com/raspberrypi/userland
```
```bash
cd userland
```
```bash
git revert f97b1af1b3e653f9da2c1a3643479bfd469e3b74
```
```bash
git revert e31da99739927e87707b2e1bc978e75653706b9c
```
```bash
export LDFLAGS="-Wl,--no-as-needed"
```
```bash
./buildme --aarch64
```
```bash
sudo cp build/lib/*.so /usr/lib/aarch64-linux-gnu/
```

## Install the libarducam_mipicamera.so 

```bash
cd MIPI_Camera/RPI
```
```bash
sudo cp /lib/libarducam_mipicamera /usr/lib/
```
## Test

```bash
cd MIPI_Camera/RPI/
```
```bash
make arducamstill
```
```bash
./aruducamstill -t 0 
```
