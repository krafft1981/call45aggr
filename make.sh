scl enable devtoolset-7 bash << \EOF
printf "
[settings]
os_build=Linux
arch_build=x86_64
os=Linux
arch=x86_64
compiler=gcc
compiler.version=7
compiler.libcxx=libstdc++
" > profile

conan install . -pr profile --build bzip2 --build zlib  --build jsoncpp --build boost
EOF

export CC=/opt/rh/devtoolset-7/root/usr/bin/gcc
export CXX=/opt/rh/devtoolset-7/root/usr/bin/g++
cmake3 CMakeLists.txt
make -j $(nproc)
