#include <catch2/catch_test_macros.hpp>
#include <chip8.hpp>

TEST_CASE("Operation codes") {
    Chip8 chip8;
    chip8.initialize();
    
    SECTION("0x00E0") {
        chip8.executeOperationCode(0x00E0);
        
        REQUIRE(chip8.getGraphix() == std::array<Byte, 32 * 64>{0});
    }

    SECTION("0x1NNN: Jump to address NNN: Clears the screen") {
        chip8.executeOperationCode(0x1b23);
        
        REQUIRE(chip8.getProgramCounter() == 0x0b23);

        chip8.executeOperationCode(0x11ab);

        REQUIRE(chip8.getProgramCounter() == 0x01ab);
    }

    SECTION("0x2NNN: Calls subroutine at NNN") {
        Word lastPosition;

        lastPosition = chip8.getProgramCounter();
        chip8.executeOperationCode(0x2b23);

        
        REQUIRE(chip8.getProgramCounter() == 0x0b23);
        REQUIRE(chip8.getStack()[chip8.getStackP() - 1] == lastPosition);

        lastPosition = chip8.getProgramCounter();
        chip8.executeOperationCode(0x21ab);

        REQUIRE(chip8.getProgramCounter() == 0x01ab);
        REQUIRE(chip8.getStack()[chip8.getStackP() - 1] == lastPosition);
    }
    
    SECTION("3XNN: Skips next instruction if VX == NN (true)") {
        Word programCounter = chip8.getProgramCounter();
        
        chip8.executeOperationCode(0x3300);

        REQUIRE(chip8.getProgramCounter() == programCounter + 4);
    }

    SECTION("3XNN: Skips next instruction if VX == NN (false)") {
        Word programCounter = chip8.getProgramCounter();

        chip8.executeOperationCode(0x3301);

        REQUIRE(chip8.getProgramCounter() == programCounter + 2);
    }

    SECTION("4XNN: Skips next instruction if VX != NN (true)") {
        Word programCounter = chip8.getProgramCounter();

        chip8.executeOperationCode(0x4301);

        REQUIRE(chip8.getProgramCounter() == programCounter + 4);
    }

    SECTION("4XNN: Skips next instruction if VX != NN (false)") {
        Word programCounter = chip8.getProgramCounter();
        
        chip8.executeOperationCode(0x4300);

        REQUIRE(chip8.getProgramCounter() == programCounter + 2);
    }

    SECTION("5XY0: Skips next instruction if VX == VY & 6XNN: Sets VX to NN") {
        Word programCounter;
        chip8.executeOperationCode(0x6312);

        programCounter = chip8.getProgramCounter();

        REQUIRE(chip8.getVReg()[3] == 0x12);

        chip8.executeOperationCode(0x5340);

        REQUIRE(chip8.getProgramCounter() == programCounter + 2);

        chip8.executeOperationCode(0x6412);

        programCounter = chip8.getProgramCounter();

        REQUIRE(chip8.getVReg()[4] == 0x12);

        chip8.executeOperationCode(0x5340);

        REQUIRE(chip8.getProgramCounter() == programCounter + 4);
        
    }

    SECTION("7XNN: Adds NN to VX (carry flag is not changed)") {
        
    }
}
