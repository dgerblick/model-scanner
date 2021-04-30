# To build and run examples
```
mkdir build && cd build
cmake ..
make
mkdir out
./model-scanner -c ../examples/camera_info.yml -s ../examples/scissors.mp4 -d6 -o out/scissors.stl
./model-scanner -c ../examples/camera_info.yml -s ../examples/charger.mp4 -d7 -o out/charger.stl
./model-scanner -c ../examples/camera_info.yml -s ../examples/sharpie.mp4 -d5 -o out/sharpie.stl
./model-scanner -c ../examples/camera_info.yml -s ../examples/spoon.mp4 -d5 -o out/spoon.stl
./model-scanner -c ../examples/camera_info.yml -s ../examples/zip_tie.mp4 -d7 -o out/zip_tie.stl
```
