# emulator-lib-chip8
This project builds a Chip-8 emulator. The emulator compiles to a library that a game class (representing the screen and buttons) uses to emulate games. 

# Author
Johannes Källstad [email](johannes.kallstad@gmail.com) [github](https://github.com/ohukainen)

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
- [David Winter's CHIP-8 emulation page](http://www.pong-story.com/chip8/) (Contains some games)
