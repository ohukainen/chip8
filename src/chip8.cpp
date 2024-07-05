#include "chip8.hpp"
#include "FONTSET.hpp"

#include <iostream>
#include <fstream>
#include <random>
#include <string>
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
    mV.fill(0);
    mMemory.fill(0);

    int start = 0x50;
    for (const auto & byte : FONTSET) {
        mMemory[start++] = byte;		
    }

    mDelayTimer = 0;
    mSoundTimer = 0;
}

bool Chip8::loadGame(const std::string& gameFilepath) {
    std::fstream fs(gameFilepath, std::ios::binary| std::ios::in);
    char data;
    int i = 0;
    
    while (fs.get(data)) {
        if (0x200 + i > 4096) {
            return false;
        }
        mMemory[0x200 + i] = static_cast<Byte>(data);
        i++;
    }
    return true;
}

bool Chip8::loadGame(const std::vector<Byte>& gameInstructions) {
    int i = 0;

    for (const Byte& byte : gameInstructions) {
        if (0x200 + i > 4096) {
            return false;
        }
        mMemory[0x200 + i] = byte;
        i++;
    }
    return true;
}

void Chip8::emulateCycle() {
    Word operationCode = mMemory[mProgramCounter] << 8 | mMemory[mProgramCounter + 1];
    mDrawFlag = false;
    Word N;
    Byte X;
    Byte Y;

    switch (operationCode & 0xF000) {
        case 0x0000:
            switch (operationCode & 0x00FF) {
                case 0x00E0: // 00E0: Clears the screen
                    mDrawFlag = true;
                    mGraphix.fill(0);

                    mProgramCounter += 2;
                    break;

                case 0x00EE: // 00EE: Returns from subroutine
                    mProgramCounter = mStack[--mStackP] + 2;
                    break;

                default:
                    std::cout << "Unknown opcode: " << std::to_string(operationCode) << std::endl;
            }
            break;
       
        case 0x1000: // 1NNN: Jump to address NNN
            N = operationCode & 0x0FFF;

            mProgramCounter = N;
            break;
             
        case 0x2000: // 2NNN: Calls subroutine at NNN 
            N = operationCode & 0x0FFF;

            mStack[mStackP++] = mProgramCounter;

            mProgramCounter = N;
            break;
             
        case 0x3000: // 3XNN: Skips next instruction if VX == NN 
            N = operationCode & 0x00FF;
            X = (operationCode & 0x0F00) >> 8;

            if (mV[X] == N) {
                mProgramCounter += 2;
            }

            mProgramCounter += 2;
            break;
             
        case 0x4000: // 4XNN: Skips next instruction if VX != NN 
            N = operationCode & 0x00FF;
            X = (operationCode & 0x0F00) >> 8;

            if (mV[X] != N) {
                mProgramCounter += 2;
            }

            mProgramCounter += 2;
            break;
             
        case 0x5000: // 5XY0: Skips next instruction if VX == VY 
            X = (operationCode & 0x0F00) >> 8;
            Y = (operationCode & 0x00F0) >> 4;

            if (mV[X] == mV[Y]) {
                mProgramCounter += 2;
            }
           
            mProgramCounter += 2;
            break;
             
        case 0x6000: // 6XNN: Sets VX to NN 
            N = operationCode & 0x00FF;
            X = (operationCode & 0x0F00) >> 8;

            mV[X] = N;

            mProgramCounter += 2;
            break;
             
        case 0x7000: // 7XNN: Adds NN to VX 
            N = operationCode & 0x00FF;
            X = (operationCode & 0x0F00) >> 8;

            mV[X] += N;

            mProgramCounter += 2;
            break;
             
        case 0x8000: 
            switch (operationCode & 0x000F) {
                case 0x0000: // 8XY0: Sets VX to VY
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    mV[X] = mV[Y];
                    
                    mProgramCounter += 2;
                    break;

                case 0x0001: // 8XY1: Sets VX to VX OR VY 
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    mV[X] |= mV[Y];

                    mProgramCounter += 2;
                    break;

                case 0x0002: // 8XY2: Sets VX to VX AND VY 
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    mV[X] &= mV[Y];

                    mProgramCounter += 2;
                    break;

                case 0x0003: // 8XY3: Sets VX to VX XOR VY 
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    mV[X] ^= mV[Y];

                    mProgramCounter += 2;
                    break;

                case 0x0004: // 8XY4: Adds VY to VX, Sets VF to 1 if overflow and 0 if not
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    if (Word(mV[X] + mV[Y]) > 0x00FF) {
                        mV[0xF] = 1;
                    }
                    else {
                        mV[0xF] = 0;
                    }
                    mV[X] += mV[Y];

                    mProgramCounter += 2;
                    break;

                case 0x0005: // 8XY5: Subtracts VY from VX, Sets VF to 0 if underflow and 1 if not
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    if (mV[X] >= mV[Y]) {
                        mV[0xF] = 1;
                    }
                    else {
                        mV[0xF] = 0;
                    }
                    mV[X] = mV[X] - mV[Y];

                    mProgramCounter += 2;
                    break;

                case 0x0006: // 8XY6: Stores the LSB of VX into VF before right shifting VX by 1 
                    X = (operationCode & 0x0F00) >> 8;

                    mV[0xF] = mV[X] & 0x0001;
                    mV[X] >>= 1;

                    mProgramCounter += 2;
                    break;

                case 0x0007: // 8XY7: Sets VX to VY subtracted by VX, Sets VF to 0 if overflow and 1 if not
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    if (mV[Y] >= mV[X]) {
                        mV[0xF] = 1;
                    }
                    else {
                        mV[0xF] = 0;
                    }
                    mV[X] = mV[Y] - mV[X];

                    mProgramCounter += 2;
                    break;

                case 0x000E: // 8XYE: Sets VF to 1 if VX MSB is set and 0 if not before left shifting VX by 1 
                    X = (operationCode & 0x0F00) >> 8;
                    Y = (operationCode & 0x00F0) >> 4;

                    mV[0xF] = mV[X] >> 7;
                    mV[X] <<= 1;

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << "Unknown opcode: " << std::to_string(operationCode) << std::endl;
            }
            break;
             
        case 0x9000: // 9XY0: Skips next instruction if VX != VY  
            X = (operationCode & 0x0F00) >> 8;
            Y = (operationCode & 0x00F0) >> 4;

            if (mV[X] != mV[Y]) {
                mProgramCounter += 2;
            }
           
            mProgramCounter += 2;
            break;

        case 0xA000: // ANNN: Sets index registry to the address NNN
            N = operationCode & 0x0FFF;

            mIndexRegistry = N;

            mProgramCounter += 2;
            break;
             
        case 0xB000: // BNNN: Jumps to the address NNN + V0 
            N = operationCode & 0x0FFF;

            mProgramCounter = N + mV[0];
            break;
             
        case 0xC000: // CXNN: Sets VX to NN AND random number (0-255)
            N = operationCode & 0x00FF;
            X = (operationCode & 0x0F00) >> 8;

            mV[X] = N & randomNumber(); 

            mProgramCounter += 2;
            break;
             
        case 0xD000: // DXYN: Draws sprite with height N in memory location I at position (X, Y) and sets VF to 1 on collision
            {
                N = operationCode & 0x000F;
                X = (operationCode & 0x0F00) >> 8;
                Y = (operationCode & 0x00F0) >> 4;

                mDrawFlag = true;

                mV[0xF] = 0;
                for (int row = 0; row < N; row++) {
                    for (int bit = 0; bit < 8; bit++) {
                        Word index = ((mV[X] + bit) + (mV[Y] + row) * 64) % 2048; 
                        if ((mMemory[mIndexRegistry + row] & (0b10000000 >> bit)) != 0) {
                            if (mGraphix[index] == 1) {
                                mV[0xF] = 1;
                            }
                            mGraphix[index] ^= 1;
                        }
                    }
                }
                mProgramCounter += 2;
            }
            break;
             
        case 0xE000: 
            switch (operationCode & 0x00FF) {
                case 0x009E: // EX9E: Skips next instruction if key X is pressed 
                    X = (operationCode & 0x0F00) >> 8;

                    if (mKeys[mV[X]] == true) {
                        mProgramCounter += 2;
                    }

                    mProgramCounter += 2;
                    break;

                case 0x00A1: // EXA1: Skips next instruction if key X is not pressed 
                    X = (operationCode & 0x0F00) >> 8;

                    if (mKeys[mV[X]] == false) {
                        mProgramCounter += 2;
                    }

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << "Unknown opcode: " << std::to_string(operationCode) << std::endl;
            }
            break;
             
        case 0xF000: 
            switch (operationCode & 0x00FF) {
                case 0x0007: // FX07: sets VX to the delay timers value
                    X = (operationCode & 0x0F00) >> 8;

                    mV[X] = mDelayTimer;

                    mProgramCounter += 2;
                    break;

                case 0x000A: // FX0A: Waits for input and sets VX to the pressed key
                    X = (operationCode & 0x0F00) >> 8;

                    for (uint64_t i = 0; i < mKeys.size(); i++) {
                        if (mKeys[i] == true) {

                            mV[X] = i;

                            mProgramCounter += 2;
                            break;
                        }
                    }
                    break;

                case 0x0015: // FX15: Sets delay timer to VX
                    X = (operationCode & 0x0F00) >> 8;

                    mDelayTimer = mV[X];

                    mProgramCounter += 2;
                    break;

                case 0x0018: // FX18: Sets the sound timer to VX 
                    X = (operationCode & 0x0F00) >> 8;

                    mSoundTimer = mV[X];

                    mProgramCounter += 2;
                    break;

                case 0x001E: // FX1E: Adds VX to I
                    X = (operationCode & 0x0F00) >> 8;

                    mIndexRegistry += mV[X];

                    mProgramCounter += 2;
                    break;

                case 0x0029: // FX29: Sets I to the memory address of font for character X
                    X = (operationCode & 0x0F00) >> 8;

                    mIndexRegistry = 0x50 + (5 * mV[X]);

                    mProgramCounter += 2;
                    break;

                case 0x0033: // FX33: Stores BCD of VX in memory addresses I to I + 2 
                    X = (operationCode & 0x0F00) >> 8;

                    mMemory[mIndexRegistry] = ( mV[X] / 100);
                    mMemory[mIndexRegistry + 1] = ( (mV[X] / 10) % 10);
                    mMemory[mIndexRegistry + 2] = ( (mV[X] % 100) % 10);

                    mProgramCounter += 2;
                    break;

                case 0x0055: // FX55: Stores from V0 to VX into memory starting at address I 
                    X = (operationCode & 0x0F00) >> 8;

                    for (int i = 0; i <= X; i++) {
                        mMemory[mIndexRegistry + i] = mV[i];
                    }

                    mProgramCounter += 2;
                    break;

                case 0x0065: // FX65: Fills from V0 to VX from memory starting at address I 
                    X = (operationCode & 0x0F00) >> 8;

                    for (int i = 0; i <= X; i++) {
                         mV[i] = mMemory[mIndexRegistry + i];
                    }

                    mProgramCounter += 2;
                    break;

                default:
                    std::cout << "Unknown opcode: " << std::to_string(operationCode) << std::endl;
            }
            break;

        default:
            std::cout << "Unknown opcode: " << std::to_string(operationCode) << std::endl;
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
    return mV;
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
