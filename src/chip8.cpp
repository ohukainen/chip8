#include "chip8.hpp"
#include "font.hpp"

#include <ios>
#include <iostream>
#include <fstream>

void Chip8::initialize() {
    std::cout << "Initializing Chip-8 emulator" << std::endl;
    mProgramCounter = 0x200; 
    mOpcode = 0;    
    mIndexRegistry = 0;      
    mStackP = 0;      
 
    // TODO: clear display	
    // TODO: clear stack
    // TODO: clear registers V0-VF
    // TODO: clear memory
 
    int start = 0x50;
    std::cout << "Loading font into memory, starting at adress: " << start << std::endl;
    for (const auto & i : fontset) {
      mMemory[start++] = fontset[i];		
    }

    // TODO: reset timers
}

void Chip8::loadGame(const std::string& gameFilepath) {
    std::fstream fs(gameFilepath, std::ios::binary);
    
    char data;
    int i = 0;
    while (fs.get(data)) {
        mMemory[512 + i++] = static_cast<Byte>(data);
    }
}

void Chip8::emulateCycle() {
    mOpcode = mMemory[mProgramCounter] << 8 | mMemory[mProgramCounter+ 1];

    switch (mOpcode & 0xF000)
    {    
        case 0xA000: // ANNN: Sets index registry to the address NNN
            mIndexRegistry = mOpcode & 0x0FFF;
            mProgramCounter += 2;
            break;

            //TODO: add cases for valid operation codes

        default:
            std::cout << std::hex << "Unknown opcode: " << mOpcode << std::endl;
    }  

    if (mDelayTimer > 0) {
        --mDelayTimer;
    }

    if (mSoundTimer > 0) {
        if (mSoundTimer == 1) {
            std::cout << "BEEP!" << std::endl;
        }
        --mSoundTimer;
    }  
}

void Chip8::setKeys(const std::array<Byte, 16>& keyState) {
    mKeys = keyState; 
}

bool Chip8::getDrawFlag() const {
    return mDrawFlag;
}
