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

    switch (mOpcode & 0xF000) {    
        case 0x0000:
            switch (mOpcode & 0x000F) {
                case 0x0000: // 00E0: Clears the screen
                    mGraphix.fill(0);
                    mDrawFlag = true;

                    mProgramCounter += 2;
                    break;

                case 0x000E: // 00EE: Returns from subroutine
                    mProgramCounter = mStack[mStackP--] + 2;
                    break;

                default:
                    std::cout << std::hex << "Unknown opcode: " << mOpcode << std::endl;
            }
            break;
       
        case 0x1000: // 1NNN: Jump to address NNN
            mProgramCounter = mOpcode & 0x0FFF;
            break;
             
        case 0x2000: // 2NNN: Calls subroutine at NNN 
            mStack[mStackP++ + 1] = mProgramCounter;

            mProgramCounter = mOpcode & 0x0FFF;
            break;
             
        case 0x3000: // 3XNN: Skips next instruction if VX == NN 
            if (mVReg[(mOpcode & 0x0F00) >> 8] == (mOpcode & 0x00FF)) {
                mProgramCounter += 4;
            }
            else {
                mProgramCounter += 2;
            }
            break;
             
        case 0x4000: // 3XNN: Skips next instruction if VX != NN 
            if (mVReg[(mOpcode & 0x0F00) >> 8] != (mOpcode & 0x00FF)) {
                mProgramCounter += 4;
            }
            else {
                mProgramCounter += 2;
            }
            break;
             
        case 0x5000: // 5XY0: Skips next instruction if VX == VY 
            if (mVReg[(mOpcode & 0x0F00) >> 8] == mVReg[(mOpcode & 0x00F0) >> 4]) {
                mProgramCounter += 4;
            }
            else {
                mProgramCounter += 2;
            }
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

                case 0x0001: // 8XY1: Sets VX to VX OR VY (bitwise OR)
                    mVReg[(mOpcode & 0x0F00) >> 8] |= mVReg[(mOpcode & 0x00F0) >> 4];
                    
                    mProgramCounter += 2;
                    break;

                case 0x0002: // 8XY2: Sets VX to VX AND VY (bitwise AND)
                    mVReg[(mOpcode & 0x0F00) >> 8] &= mVReg[(mOpcode & 0x00F0) >> 4];

                    mProgramCounter += 2;
                    break;

                case 0x0003: // 8XY3: Sets VX to VX XOR VY (bitwise XOR)
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
                mProgramCounter += 4;
            }
            else {
                mProgramCounter += 2;
            }
            break;

        case 0xA000: // ANNN: Sets index registry to the address NNN
            mIndexRegistry = mOpcode & 0x0FFF;
            mProgramCounter += 2;
            break;
             
        case 0xB000: // XXXX: description
            // TODO
            break;
             
        case 0xC000: // XXXX: description
            // TODO
            break;
             
        case 0xD000: // XXXX: description
            // TODO
            break;
             
        case 0xE000: // XXXX: description
            // TODO
            break;
             
        case 0xF000: // XXXX: description
            // TODO
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

void Chip8::setKeys(const std::array<Byte, 16>& keyState) {
    mKeys = keyState; 
}

bool Chip8::getDrawFlag() const {
    return mDrawFlag;
}
