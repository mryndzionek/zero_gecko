cd emlib
cmake -DCMAKE_TOOLCHAIN_FILE=../gcc_cortex_m.cmake -DTOOLCHAIN_PREFIX=<path_to_toolchain> -DEMLIB_DIR=<path_to_energymicro>  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=<path_to_toolchain>/arm-none-eabi/ -DTARGET_MCU=EFM32ZG
make install
cd ..
