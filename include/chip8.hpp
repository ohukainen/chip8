#pragma once

#include <cstdint>
#include <array>
#include <string>

using Byte = uint8_t;
using Word = uint16_t;

class Chip8 {
public:
    void initialize();   
    void loadGame(const std::string& gameFilepath);

    void emulateCycle();
    void setKeys(const std::array<bool, 16>& keyState);
   

    bool getDrawFlag() const;
private:
    Byte randomNumber();

    std::array<Byte, 4096> mMemory;     // Memory 
    std::array<Byte, 64 * 32> mGraphix; // Pixels on the screen 

    Word mOpcode;                       // Operation code 
    Word mProgramCounter;               // Program counter 
    Word mIndexRegistry;                // Index registry
    std::array<Byte, 16> mVReg;         // V registry

    Byte mDelayTimer;                   // Timer for delay
    Byte mSoundTimer;                   // Timer for sound      
                            
    std::array<Word, 16> mStack;        // Stack 
    Word mStackP;                       // Stack pointer

    std::array<bool, 16> mKeys;         // Keypad representation

    bool mDrawFlag;                     // Flag for refreshing screen
};
