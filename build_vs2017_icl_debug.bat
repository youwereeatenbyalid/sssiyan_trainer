mkdir build64
cd build64
cmake .. -G "Visual Studio 15 2017 Win64" -T "Intel C++ Compiler 19.0" -DDEVELOPER_MODE=OFF
cmake --build . --config Debug
cd ..
pause