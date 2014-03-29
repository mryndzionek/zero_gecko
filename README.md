EFM32 Zero Gecko Starter Kit - CMake based development environment
==================================================================

![kit](images/kit.png?raw=true "Kit EFM32ZG_STK3200")

Introduction
------------

CMake scripts based on [this](https://github.com/alatarum/cmake-emlib) repo.

Build instructions
------------------

1. Clone the repo:

		git clone https://github.com/mryndzionek/zero_gecko.git

2. Download the ARM toolchain from [here](https://launchpad.net/gcc-arm-embedded/4.7/4.7-2012-q4-major)

3. Download the QP-nano framework (5.2.0) from [here](http://sourceforge.net/projects/qpc/files/QP-nano/5.2.0/) and unpack it into `qpn_5.2.0` subdirectory

4. Build emlib library:

		cd emlib
		cmake -DCMAKE_TOOLCHAIN_FILE=../gcc_cortex_m.cmake -DTOOLCHAIN_PREFIX=<path_to_toolchain> \
		-DEMLIB_DIR=<path_to_energymicro_dir> -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_INSTALL_PREFIX=<path_to_toolchain>/arm-none-eabi/ -DTARGET_MCU=EFM32ZG
		make -j<n>
		make install

5. Build the project:

		mkdir build
		cd build
		cmake -DEM_MCU=EFM32ZG222F32 -DCMAKE_TOOLCHAIN_FILE=../gcc_cortex_m.cmake \
		-DCMAKE_BUILD_TYPE=Debug -DTOOLCHAIN_PREFIX=<path_to_toolchain> \
		-DENMICRO_DIR=<path_to_energymicro_bundle> ../apps

Every `<target>` has also an `upload_<target>` using `start-eACommander.sh` to upload the binary to the starter kit.

Examples description
--------------------

* energymicro blink example

![blink_eap](images/blink_eap.png?raw=true "Energy aware profiler - blink example")

* QP-nano EM3 mode blink example

![blink_qpn_eap](images/blink_qpn_eap.png?raw=true "Energy aware profiler - blink QP-nano example")

* energymicro emlcd example

![emlcd](images/emlcd_eap.png?raw=true "Energy aware profiler - emlcd example")

Contact
-------
If you have questions, contact Mariusz Ryndzionek at:

<mryndzionek@gmail.com>
