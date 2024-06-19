# emulator-lib-chip8
This project will build an emulator for the Chip-8. This is done as a preparation before creating a Game Boy emulator. 

# Author
Johannes Källstad [email](johannes.kallstad@gmail.com) [github](https://github.com/Ohukainen)

# Dependencies
- [CMake](https://cmake.org/)

# Build
This project uses CMake. 

To setup the build run:
```
cmake -S . -B build
```

This will setup the build source (-S) as the current directory and the build target (-B) as a folder called build.

When the build is setup the project can be built by running:
```
cmake --build build
```

And installed from the build folder by running 
```
cmake --install build
```
