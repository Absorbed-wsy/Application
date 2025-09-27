# Application

# build x86
mkdir build
cd build
cmake ..
make

# build arm
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-toolchain.cmake ..
make