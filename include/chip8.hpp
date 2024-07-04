#pragma once

#include <cstdint>
#include <array>
#include <string>

using Byte = uint8_t;
using Word = uint16_t;

class Chip8 {
public:
    void initialize();   
    bool loadGame(const std::string& gameFilepath);
    
    Word fetchNextOperationCode();
    void executeOperationCode(Word operationCode);
    void setKeys(const std::array<bool, 16>& keyState);
    
    const std::array<Byte, 4096>& getMemory() const;
    const std::array<Byte, 64 * 32>& getGraphix() const;

    Word getProgramCounter() const;
    Word getIndexRegistry() const;

    const std::array<Byte, 16>& getVReg() const;
    Byte getDelayTimer() const;
    Byte getSoundTimer() const;

    const std::array<Word, 16>& getStack() const;
    Word getStackP() const;

    const std::array<bool, 16>& getKeys() const;         

    bool getDrawFlag() const;

private:
    std::array<Byte, 4096> mMemory;     // Memory 
    std::array<Byte, 64 * 32> mGraphix; // Pixels on the screen 

    Word mProgramCounter;               // Program counter 
    Word mIndexRegistry;                // Index registry
    std::array<Byte, 16> mV;            // V registry

    Byte mDelayTimer;                   // Timer for delay
    Byte mSoundTimer;                   // Timer for sound      
                            
    std::array<Word, 16> mStack;        // Stack 
    Word mStackP;                       // Stack pointer

    std::array<bool, 16> mKeys;         // Keypad representation

    bool mDrawFlag;                     // Flag for refreshing screen
};
