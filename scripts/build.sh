rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j12

mkdir -p ../bin
mv game_client ../bin/game_client