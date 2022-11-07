mkdir -p debug
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build .
cp src/UXP UXP
cd ..
