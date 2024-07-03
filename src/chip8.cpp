#include "chip8.hpp"
#include "font.hpp"

#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include <chrono>

static Byte randomNumber() {
    std::random_device randomDevice;    
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<> distribution(0,255);

    return distribution(generator);
}

void Chip8::initialize() {
    mProgramCounter = 0x200; 
    mIndexRegistry = 0;      
    mStackP = 0;      
    
    mKeys.fill(false);
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


Word Chip8::fetchNextOperationCode() {
    return mMemory[mProgramCounter] << 8 | mMemory[mProgramCounter + 1];
}

void Chip8::executeOperationCode(Word operationCode) {
    mDrawFlag = false;
    // std::cout << std::hex << operationCode << std::endl;
    switch (operationCode & 0xF000) {    
        case 0x0000:
            switch (operationCode & 0x000F) {
                case 0x0000: // 00E0: Clears the screen
                    mDrawFlag = true;
                    mGraphix.fill(0);

                    mProgramCounter += 2;
                    break;

                case 0x000E: // 00EE: Returns from subroutine
                    mProgramCounter = mStack[--mStackP] + 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << operationCode << std::endl;
            }
            break;
       
        case 0x1000: // 1NNN: Jump to address NNN
            mProgramCounter = operationCode & 0x0FFF;
            break;
             
        case 0x2000: // 2NNN: Calls subroutine at NNN 
            mStack[mStackP++] = mProgramCounter;

            mProgramCounter = operationCode & 0x0FFF;
            break;
             
        case 0x3000: // 3XNN: Skips next instruction if VX == NN 
            if (mVReg[(operationCode & 0x0F00) >> 8] == (operationCode & 0x00FF)) {
                mProgramCounter += 2;
            }

            mProgramCounter += 2;
            break;
             
        case 0x4000: // 4XNN: Skips next instruction if VX != NN 
            if (mVReg[(operationCode & 0x0F00) >> 8] != (operationCode & 0x00FF)) {
                mProgramCounter += 2;
            }

            mProgramCounter += 2;
            break;
             
        case 0x5000: // 5XY0: Skips next instruction if VX == VY 
            if (mVReg[(operationCode & 0x0F00) >> 8] == mVReg[(operationCode & 0x00F0) >> 4]) {
                mProgramCounter += 2;
            }
           
            mProgramCounter += 2;
            break;
             
        case 0x6000: // 6XNN: Sets VX to NN 
            mVReg[(operationCode & 0x0F00) >> 8] = operationCode & 0x00FF;

            mProgramCounter += 2;
            break;
             
        case 0x7000: // 7XNN: Adds NN to VX 
            mVReg[(operationCode & 0x0F00) >> 8] += operationCode & 0x00FF;

            mProgramCounter += 2;
            break;
             
        case 0x8000: 
            switch (operationCode & 0x000F) {
                case 0x0000: // 8XY0: Sets VX to VY
                    mVReg[(operationCode & 0x0F00) >> 8] = mVReg[(operationCode & 0x00F0) >> 4];
                    
                    mProgramCounter += 2;
                    break;

                case 0x0001: // 8XY1: Sets VX to VX OR VY 
                    mVReg[(operationCode & 0x0F00) >> 8] |= mVReg[(operationCode & 0x00F0) >> 4];
                    
                    mProgramCounter += 2;
                    break;

                case 0x0002: // 8XY2: Sets VX to VX AND VY 
                    mVReg[(operationCode & 0x0F00) >> 8] &= mVReg[(operationCode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0003: // 8XY3: Sets VX to VX XOR VY 
                    mVReg[(operationCode & 0x0F00) >> 8] ^= mVReg[(operationCode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0004: // 8XY4: Adds VY to VX, Sets VF to 1 if overflow and 0 if not
                    if ((Word((mVReg[(operationCode & 0x0F00) >> 8] + mVReg[(operationCode & 0x00F0) >> 4])) & 0xFF00) >> 8 != 0) {
                        mVReg[15] = 0;
                    }
                    else {
                        mVReg[15] = 1;
                    }
                    mVReg[(operationCode & 0x0F00) >> 8] += mVReg[(operationCode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0005: // 8XY5: Subtracts VY from VX, Sets VF to 0 if overflow and 1 if not
                    if (mVReg[(operationCode & 0x0F00) >> 8] >= mVReg[(operationCode & 0x00F0) >> 4]) {
                        mVReg[15] = 1;
                    }
                    else {
                        mVReg[15] = 0;
                    }
                    mVReg[(operationCode & 0x0F00) >> 8] -= mVReg[(operationCode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0006: // 8XY6: Stores the LSB of VX into VF before right shifting VX by 1 
                    mVReg[15] = mVReg[(operationCode & 0x0F00) >> 8] & 0x0001;
                    mVReg[(operationCode & 0x0F00) >> 8] >>= 1;

                    mProgramCounter += 2;
                    break;

                case 0x0007: // 8XY7: Sets VX to VY subtracted by VX, Sets VF to 0 if overflow and 1 if not
                    if (mVReg[(operationCode & 0x00F0) >> 4] >= mVReg[(operationCode & 0x0F00) >> 8]) {
                        mVReg[15] = 1;
                    }
                    else {
                        mVReg[15] = 0;
                    }
                    mVReg[(operationCode & 0x0F00) >> 8] = mVReg[(operationCode & 0x00F0) >> 4] - mVReg[(operationCode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x000E: // 8XYE: Sets VF to 1 if VX MSB is set and 0 if not before left shifting VX by 1 
                    mVReg[15] = (mVReg[(operationCode & 0x0F00) >> 8] & 0x0080) >> 7;
                    mVReg[(operationCode & 0x0F00) >> 8] <<= 1;

                    mProgramCounter += 2;
                    break;
                default:
                    std::cout << std::hex << "Unknown opcode: " << operationCode << std::endl;
            }
            break;
             
        case 0x9000: // 9XY0: Skips next instruction if VX != VY  
            if (mVReg[(operationCode & 0x0F00) >> 8] != mVReg[(operationCode & 0x00F0) >> 4]) {
                mProgramCounter += 2;
            }
           
            mProgramCounter += 2;
            break;

        case 0xA000: // ANNN: Sets index registry to the address NNN
            mIndexRegistry = operationCode & 0x0FFF;

            mProgramCounter += 2;
            break;
             
        case 0xB000: // BNNN: Jumps to the address NNN + V0 
            mProgramCounter = (operationCode & 0x0FFF) + mVReg[0];
            break;
             
        case 0xC000: // CXNN: Sets VX to NN AND random number (0-255)
            mVReg[(operationCode & 0x0F00) >> 8] = (operationCode & 0x00FF) & randomNumber(); 

            mProgramCounter += 2;
            break;
             
        case 0xD000: // DXYN: Draws sprite with height N in memory location I at position (X, Y) and sets VF to 1 on collision
            mDrawFlag = true;

            mVReg[15] = 0;   
            for (int row = 0; row < (operationCode & 0x000F); row++) {
                for (int bit = 0; bit < 8; bit++) {
                    if ((mMemory[mIndexRegistry + row] & (0x0080 >> bit)) != 0) {
                        if (mGraphix[mVReg[(operationCode & 0x0F00) >> 8] + bit + (((mVReg[operationCode & 0x00F0] >> 4) + row) * 64)] == 1) {
                            mVReg[15] = 1;
                        }
                        mGraphix[mVReg[(operationCode & 0x0F00) >> 8] + bit + (((mVReg[operationCode & 0x00F0] >> 4) + row) * 64)] ^= 1;
                    }
                }
            }

            mProgramCounter += 2;
            break;
             
        case 0xE000: 
            switch (operationCode & 0x000F) {
                case 0x000E: // EX9E: Skips next instruction if key X is pressed 
                    if (mKeys[(operationCode & 0x0F00) >> 8] == true) {
                        mProgramCounter += 2;
                    }

                    mProgramCounter += 2;
                    break;

                case 0x0001: // EXA1: Skips next instruction if key X is not pressed 
                    if (mKeys[(operationCode & 0x0F00) >> 8] == false) {
                        mProgramCounter += 2;
                    }

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << operationCode << std::endl;
            }
            break;
             
        case 0xF000: 
            switch (operationCode & 0x00FF) {
                case 0x0007: // FX07: sets VX to the delay timers value
                    mVReg[(operationCode & 0x0F00) >> 8] = mDelayTimer;

                    mProgramCounter += 2;
                    break;

                case 0x000A: // FX0A: Waits for input and sets VX to the pressed key
                    for (uint64_t i = 0; i < mKeys.size(); i++) {
                        if (mKeys[i] == true) {
                            mVReg[(operationCode & 0x0F00) >> 8] = i;

                            mProgramCounter += 2;
                            break;
                        }
                    }
                    break;

                case 0x0015: // FX15: Sets delay timer to VX
                    mDelayTimer = mVReg[(operationCode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x0018: // FX18: Sets the sound tumer to VX 
                    mSoundTimer = mVReg[(operationCode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x001E: // FX1E: Adds VX to I
                    mIndexRegistry += mVReg[(operationCode & 0x0F00) >> 8];

                    mProgramCounter += 2;
                    break;

                case 0x0029: // FX29: Sets I to the memory address of font for character X
                    mIndexRegistry = 0x50 + (5 * mVReg[(operationCode & 0x0F00) >> 8]);

                    mProgramCounter += 2;
                    break;

                case 0x0033: // FX33: Stores BCD of VX in memory addresses I to I + 2 
                    mMemory[mIndexRegistry] = mVReg[(operationCode & 0x0F00) >> 8] / 100;
                    mMemory[mIndexRegistry + 1] = (mVReg[(operationCode & 0x0F00) >> 8] - (100 * mMemory[mIndexRegistry])) / 10;
                    mMemory[mIndexRegistry + 2] = mVReg[(operationCode & 0x0F00) >> 8] - ((100 * mMemory[mIndexRegistry]) + (10 * mMemory[mIndexRegistry + 1]));

                    mProgramCounter += 2;
                    break;

                case 0x0055: // FX55: Stores from V0 to VX into memory starting at address I 
                    for (int i = 0; i <= mVReg[(operationCode & 0x0F00) >> 8]; i++) {
                        mMemory[mIndexRegistry + i] = mVReg[i];
                    }

                    mProgramCounter += 2;
                    break;

                case 0x0065: // FX65: Fills from V0 to VX from memory starting at address I 
                    for (int i = 0; i <= (operationCode & 0x0F00) >> 8; i++) {
                         mVReg[i] = mMemory[mIndexRegistry + i];
                    }

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << operationCode << std::endl;
            }
            break;

        default:
            std::cout << std::hex << "Unknown opcode: " << operationCode << std::endl;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
}

void Chip8::setKeys(const std::array<bool, 16>& keyState) {
    mKeys = keyState; 
}

const std::array<Byte, 4096>& Chip8::getMemory() const {
    return mMemory;
}

const std::array<Byte, 64 * 32>& Chip8::getGraphix() const {
    return mGraphix;
}

Word Chip8::getProgramCounter() const {
    return mProgramCounter;
}

Word Chip8::getIndexRegistry() const {
    return mIndexRegistry;
}

const std::array<Byte, 16>& Chip8::getVReg() const {
    return mVReg;
}

Byte Chip8::getDelayTimer() const {
    return mDelayTimer;
}

Byte Chip8::getSoundTimer() const {
    return mSoundTimer;
}

const std::array<Word, 16>& Chip8::getStack() const {
    return mStack;
}
    
Word Chip8::getStackP() const {
    return mStackP;
}

const std::array<bool, 16>& Chip8::getKeys() const {
    return mKeys;
}

    
bool Chip8::getDrawFlag() const {
    return mDrawFlag;
}
