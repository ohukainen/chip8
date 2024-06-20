# emulator-lib-chip8
This project will build an emulator for the Chip-8. This is done as a preparation before creating a Game Boy emulator. 

# Author
Johannes Källstad [email](johannes.kallstad@gmail.com) [github](https://github.com/Ohukainen)

# Dependencies
- [CMake](https://cmake.org/)

# Build
This project uses CMake. 

Setup build with (optional in parentheses):
```
cmake -S . -B build (-G <Generator>) 
```

Build with:
```
cmake --build build
```

Install with:
```
cmake --install build
```

# Sources 
- [Chip-8 wikipedia page](https://en.wikipedia.org/wiki/CHIP-8)
- [Writing a Chip-8 emulator](https://aymanbagabas.com/blog/2018/09/17/chip-8-emulator.html) by Ayman Bagabas
- [How to write an emulator](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) by Laurence Muller
