#include "chip8.hpp"
#include "font.hpp"

#include <ios>
#include <iostream>
#include <fstream>
#include <random>

void Chip8::initialize() {
    mProgramCounter = 0x200; 
    mOpcode = 0;    
    mIndexRegistry = 0;      
    mStackP = 0;      
 
    mGraphix.fill(0);
    mStack.fill(0);
    mVReg.fill(0);
    mMemory.fill(0);

    int start = 0x50;
    for (const auto & i : fontset) {
      mMemory[start++] = fontset[i];		
    }

    mDelayTimer = 0;
    mSoundTimer = 0;
}

void Chip8::loadGame(const std::string& gameFilepath) {
    if (gameFilepath == "test") {
        return;
    }
    std::fstream fs(gameFilepath, std::ios::binary| std::ios::in);
    
    char data;
    int i = 0;
    while (fs.get(data)) {
        mMemory[512 + i++] = static_cast<Byte>(data);
    }
}

void Chip8::emulateCycle() {
    mOpcode = mMemory[mProgramCounter] << 8 | mMemory[mProgramCounter + 1];

    mDrawFlag = false;
    switch (mOpcode & 0xF000) {    
        case 0x0000:
            switch (mOpcode & 0x000F) {
                case 0x0000: // 00E0: Clears the screen
                    mDrawFlag = true;
                    mGraphix.fill(0);

                    mProgramCounter += 2;
                    break;

                case 0x000E: // 00EE: Returns from subroutine
                    mProgramCounter = mStack[--mStackP] + 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << mOpcode << std::endl;
            }
            break;
       
        case 0x1000: // 1NNN: Jump to address NNN
            mProgramCounter = mOpcode & 0x0FFF;
            break;
             
        case 0x2000: // 2NNN: Calls subroutine at NNN 
            mStack[mStackP++] = mProgramCounter;

            mProgramCounter = mOpcode & 0x0FFF;
            break;
             
        case 0x3000: // 3XNN: Skips next instruction if VX == NN 
            if (mVReg[(mOpcode & 0x0F00) >> 8] == (mOpcode & 0x00FF)) {
                mProgramCounter += 2;
            }

            mProgramCounter += 2;
            break;
             
        case 0x4000: // 4XNN: Skips next instruction if VX != NN 
            if (mVReg[(mOpcode & 0x0F00) >> 8] != (mOpcode & 0x00FF)) {
                mProgramCounter += 2;
            }

            mProgramCounter += 2;
            break;
             
        case 0x5000: // 5XY0: Skips next instruction if VX == VY 
            if (mVReg[(mOpcode & 0x0F00) >> 8] == mVReg[(mOpcode & 0x00F0) >> 4]) {
                mProgramCounter += 2;
            }
           
            mProgramCounter += 2;
            break;
             
        case 0x6000: // 6XNN: Sets VX to NN 
            mVReg[(mOpcode & 0x0F00) >> 8] = mOpcode & 0x00FF;

            mProgramCounter += 2;
            break;
             
        case 0x7000: // 7XNN: Adds NN to VX 
            mVReg[(mOpcode & 0x0F00) >> 8] += mOpcode & 0x00FF;

            mProgramCounter += 2;
            break;
             
        case 0x8000: 
            switch (mOpcode & 0x000F) {
                case 0x0000: // 8XY0: Sets VX to VY
                    mVReg[(mOpcode & 0x0F00) >> 8] = mVReg[(mOpcode & 0x00F0) >> 4];
                    
                    mProgramCounter += 2;
                    break;

                case 0x0001: // 8XY1: Sets VX to VX OR VY 
                    mVReg[(mOpcode & 0x0F00) >> 8] |= mVReg[(mOpcode & 0x00F0) >> 4];
                    
                    mProgramCounter += 2;
                    break;

                case 0x0002: // 8XY2: Sets VX to VX AND VY 
                    mVReg[(mOpcode & 0x0F00) >> 8] &= mVReg[(mOpcode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0003: // 8XY3: Sets VX to VX XOR VY 
                    mVReg[(mOpcode & 0x0F00) >> 8] ^= mVReg[(mOpcode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0004: // 8XY4: Adds VY to VX, Sets VF to 1 if overflow and 0 if not
                    if ((Word((mVReg[(mOpcode & 0x0F00) >> 8] + mVReg[(mOpcode & 0x00F0) >> 4])) & 0xFF00) >> 8 != 0) {
                        mVReg[15] = 0;
                    }
                    else {
                        mVReg[15] = 1;
                    }
                    mVReg[(mOpcode & 0x0F00) >> 8] += mVReg[(mOpcode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0005: // 8XY5: Subtracts VY from VX, Sets VF to 0 if overflow and 1 if not
                    if (mVReg[(mOpcode & 0x0F00) >> 8] >= mVReg[(mOpcode & 0x00F0) >> 4]) {
                        mVReg[15] = 1;
                    }
                    else {
                        mVReg[15] = 0;
                    }
                    mVReg[(mOpcode & 0x0F00) >> 8] -= mVReg[(mOpcode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0006: // 8XY6: Stores the LSB of VX into VF before right shifting VX by 1 
                    mVReg[15] = mVReg[(mOpcode & 0x0F00) >> 8] & 0x0001;
                    mVReg[(mOpcode & 0x0F00) >> 8] >>= 1;

                    mProgramCounter += 2;
                    break;

                case 0x0007: // 8XY7: Sets VX to VY subtracted by VX, Sets VF to 0 if overflow and 1 if not
                    if (mVReg[(mOpcode & 0x00F0) >> 4] >= mVReg[(mOpcode & 0x0F00) >> 8]) {
                        mVReg[15] = 1;
                    }
                    else {
                        mVReg[15] = 0;
                    }
                    mVReg[(mOpcode & 0x0F00) >> 8] = mVReg[(mOpcode & 0x00F0) >> 4] - mVReg[(mOpcode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x000E: // 8XYE: Sets VF to 1 if VX MSB is set and 0 if not before left shifting VX by 1 
                    mVReg[15] = (mVReg[(mOpcode & 0x0F00) >> 8] & 0x0080) >> 7;
                    mVReg[(mOpcode & 0x0F00) >> 8] <<= 1;

                    mProgramCounter += 2;
                    break;
                default:
                    std::cout << std::hex << "Unknown opcode: " << mOpcode << std::endl;
            }
            break;
             
        case 0x9000: // 9XY0: Skips next instruction if VX != VY  
            if (mVReg[(mOpcode & 0x0F00) >> 8] != mVReg[(mOpcode & 0x00F0) >> 4]) {
                mProgramCounter += 2;
            }
           
            mProgramCounter += 2;
            break;

        case 0xA000: // ANNN: Sets index registry to the address NNN
            mIndexRegistry = mOpcode & 0x0FFF;

            mProgramCounter += 2;
            break;
             
        case 0xB000: // BNNN: Jumps to the address NNN + V0 
            mProgramCounter = (mOpcode & 0x0FFF) + mVReg[0];
            break;
             
        case 0xC000: // CXNN: Sets VX to NN AND random number (0-255)
            mVReg[(mOpcode & 0x0F00) >> 8] = (mOpcode & 0x00FF) & randomNumber(); 

            mProgramCounter += 2;
            break;
             
        case 0xD000: // DXYN: Draws sprite with height N in memory location I at position (X, Y) and sets VF to 1 on collision
            mDrawFlag = true;

            mVReg[15] = 0;   
            for (int row = 0; row < (mOpcode & 0x000F); row++) {
                for (int bit = 0; bit < 8; bit++) {
                    if ((mMemory[mIndexRegistry + row] & (0x0080 >> bit)) != 0) {
                        if (mGraphix[mVReg[(mOpcode & 0x0F00) >> 8] + bit + (((mVReg[mOpcode & 0x00F0] >> 4) + row) * 64)] == 1) {
                            mVReg[15] = 1;
                        }
                        mGraphix[mVReg[(mOpcode & 0x0F00) >> 8] + bit + (((mVReg[mOpcode & 0x00F0] >> 4) + row) * 64)] ^= 1;
                    }
                }
            }

            mProgramCounter += 2;
            break;
             
        case 0xE000: 
            switch (mOpcode & 0x000F) {
                case 0x000E: // EX9E: Skips next instruction if key X is pressed 
                    if (mKeys[(mOpcode & 0x0F00) >> 8] == true) {
                        mProgramCounter += 2;
                    }

                    mProgramCounter += 2;
                    break;

                case 0x0001: // EXA1: Skips next instruction if key X is not pressed 
                    if (mKeys[(mOpcode & 0x0F00) >> 8] == false) {
                        mProgramCounter += 2;
                    }

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << mOpcode << std::endl;
            }
            break;
             
        case 0xF000: 
            switch (mOpcode & 0x00FF) {
                case 0x0007: // FX07: sets VX to the delay timers value
                    mVReg[(mOpcode & 0x0F00) >> 8] = mDelayTimer;

                    mProgramCounter += 2;
                    break;

                case 0x000A: // FX0A: Waits for input and sets VX to the pressed key
                    for (uint64_t i = 0; i < mKeys.size(); i++) {
                        if (mKeys[i] == true) {
                            mVReg[(mOpcode & 0x0F00) >> 8] = i;

                            mProgramCounter += 2;
                            break;
                        }
                    }
                    break;

                case 0x0015: // FX15: Sets delay timer to VX
                    mDelayTimer = mVReg[(mOpcode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x0018: // FX18: Sets the sound tumer to VX 
                    mSoundTimer = mVReg[(mOpcode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x001E: // FX1E: Adds VX to I
                    mIndexRegistry += mVReg[(mOpcode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x0029: // FX29: Sets I to the memory address of font for character X
                    mIndexRegistry = 0x50 + (5 * mVReg[(mOpcode & 0x0F00) >> 8]);

                    mProgramCounter += 2;
                    break;

                case 0x0033: // FX33: Stores BCD of VX in memory addresses I to I + 2 
                    mMemory[mIndexRegistry] = mVReg[(mOpcode & 0x0F00) >> 8] / 100;
                    mMemory[mIndexRegistry + 1] = (mVReg[(mOpcode & 0x0F00) >> 8] - (100 * mMemory[mIndexRegistry])) / 10;
                    mMemory[mIndexRegistry + 2] = mVReg[(mOpcode & 0x0F00) >> 8] - ((100 * mMemory[mIndexRegistry]) + (10 * mMemory[mIndexRegistry + 1]));

                    mProgramCounter += 2;
                    break;

                case 0x0055: // FX55: Stores from V0 to VX into memory starting at address I 
                    for (int i = 0; i <= mVReg[(mOpcode & 0x0F00) >> 8]; i++) {
                        mMemory[mIndexRegistry + i] = mVReg[i];
                    }

                    mProgramCounter += 2;
                    break;

                case 0x0065: // FX65: Fills from V0 to VX from memory starting at address I 
                    for (int i = 0; i <= mVReg[(mOpcode & 0x0F00) >> 8]; i++) {
                         mVReg[i] = mMemory[mIndexRegistry + i];
                    }

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << mOpcode << std::endl;
            }
            break;

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

void Chip8::setKeys(const std::array<bool, 16>& keyState) {
    mKeys = keyState; 
}

bool Chip8::getDrawFlag() const {
    return mDrawFlag;
}

const std::array<Byte, 64 * 32>& Chip8::getScreenState() const {
    return mGraphix;
}

Byte Chip8::randomNumber() {
    std::random_device randomDevice;    
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<> distribution(0,255);

    return distribution(generator);
}
