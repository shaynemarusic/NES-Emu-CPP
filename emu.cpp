#include "emu.h"
#include <iostream>
#include <sstream>
#include <iomanip>

std::string hex(uint32_t value, int width);

Emulator::Emulator() {
    running = false;

    //Initialize PPU
    //ppu = PPU()

    //Initialize APU
    //apu = APU()

    //Initialize CPU
    cpu = CPU();
}

// Emulator::Emulator(const char * filename) {
//     running = false;
//     //Initialize PPU

//     //Initialize APU

//     //Load ROM
//     //Ripped from my CHIP 8 emulator
//     this->romFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);

//     if (romFile.is_open()) {

//         unsigned int i = 0;
//         int size = romFile.tellg();
//         romFile.seekg(0, std::ios::beg);

//         //Parse file header
//         //Verify file is an ines file (first 4 bytes)
//         //Will need to throw an error gracefully
//         char byte;

//         romFile.read(&byte, 1);

//         if (byte != 'N') {

//         }

//         romFile.read(&byte, 1);
        
//         if (byte != 'E') {

//         }

//         romFile.read(&byte, 1);

//         if (byte != 'S') {

//         }

//         romFile.read(&byte, 1);

//         //End-of-file character
//         if (byte != 0x1A) {

//         }

//         //Get the number of PRG-ROM banks
//         romFile.read(&byte, 1);
//         prg_rom = byte;

//         //Get the number of CHR-ROM banks
//         romFile.read(&byte, 1);
//         chr_rom = byte;

//         //Parse header flags
//         //This will be done later in more depth
//         romFile.read(&byte, 1);
//         flag6 = byte;

//         romFile.read(&byte, 1);
//         flag7 = byte;

//         //flags 8-10 are rarely used, bytes 11-15 are unused padding, so just jump to byte 16

//         //Load everything into memory based on memory mapper
//         //The mapper we use is determined by an 8 bit number whose lower nibble is the upper nibble of flag6 and whose upper nibble is the
//         //upper nibble of flag7
//         //For now, assume no mapper
//         int mapperNum = (flag7 & 0xF0) | (flag6 >> 4);

//         //Initialize CPU
//         cpu = CPU(mapperNum);
        
//         // Instead of using OOP principles to implement mappers, each mapper will have a write function stored in a table

//         // Check for trainer; low-key don't know what to do if there is one in terms of writing to memory, so will just skip the trainer
//         // if there is one for now

//         if ((flag6 & 4) == 4) {
//             romFile.seekg(528);
//         }
//         else {
//             romFile.seekg(16);
//         }

//         // Read in the initial PRG-ROM into the CPU - varies based on mapper
//         // May need to re-engineer this later but this shall do for having no mapper probably

//         // Mirror the rom bank
//         if (prg_rom == 1) {
//             for (int i = 0; i < 16000; i++) {
//                 romFile.read(&byte, 1);
//                 cpu.memory[0x8000 + i] = byte;
//                 cpu.memory[0xC000 + i] = byte;
//             }
//         }
//         // Load first two banks into memory otherwise
//         else {
//             for (int i = 0; i < 32000; i++) {
//                 romFile.read(&byte, 1);
//                 cpu.memory[0x8000 + i] = byte;
//             }
//         }

//         // Read CHR-ROM
//         // CHR-ROM is used by the PPU to fill the pattern table, hence I will just move on for now
//         // If CHR ROM is 0, CHR RAM is used
//         if (chr_rom != 0) {
//             // NEEDS TO BE REPLACED LATER
//             romFile.seekg(8192 * chr_rom, std::ios::cur);
//         }

//         // Lastly, there's PlayChoice ROM which is kinda niche, but will deal with anyway
//         // If the 1st bit of flag 7 is set, there's 8KB of additional data to read called INST ROM as well as
//         // 16 bytes of PROM Data output used to decrypt the INST and 16 bytes of PROM CounterOut output used similarly which is sometimes ignored
//         if (flag7 & 2 == 2) {
//             // NEEDS TO BE REPLACED LATER
//             romFile.seekg(8192, std::ios::cur);
//         }

//         // May be an additional bit of data at the end of file but it can be safely ignored

//         // Can now begin execution

//         // Perform reset interrupt
//         cpu.interrupt_reset();
//         running = true;

//         // Code execution -- need to add timing and some simulation of concurrency, but this should work for testing the CPU
//         while (running) {
//             cpu.decode();
//         }

//         romFile.close();
//     }
//     else {

//         std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;

//     }
// }

void Emulator::run(const char * filename) {
    running = false;
    // Reset components to known state
    cpu.manual_reset();

    //Load ROM
    //Ripped from my CHIP 8 emulator
    this->romFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);

    if (romFile.is_open()) {

        unsigned int i = 0;
        int size = romFile.tellg();
        romFile.seekg(0, std::ios::beg);

        //Parse file header
        //Verify file is an ines file (first 4 bytes)
        //Will need to throw an error gracefully
        char byte;

        romFile.read(&byte, 1);

        if (byte != 'N') {

        }

        romFile.read(&byte, 1);
        
        if (byte != 'E') {

        }

        romFile.read(&byte, 1);

        if (byte != 'S') {

        }

        romFile.read(&byte, 1);

        //End-of-file character
        if (byte != 0x1A) {

        }

        //Get the number of PRG-ROM banks
        romFile.read(&byte, 1);
        prg_rom = byte;

        //Get the number of CHR-ROM banks
        romFile.read(&byte, 1);
        chr_rom = byte;

        //Parse header flags
        //This will be done later in more depth
        romFile.read(&byte, 1);
        flag6 = byte;

        romFile.read(&byte, 1);
        flag7 = byte;

        //flags 8-10 are rarely used, bytes 11-15 are unused padding, so just jump to byte 16

        //Load everything into memory based on memory mapper
        //The mapper we use is determined by an 8 bit number whose lower nibble is the upper nibble of flag6 and whose upper nibble is the
        //upper nibble of flag7
        //For now, assume no mapper
        int mapperNum = (flag7 & 0xF0) | (flag6 >> 4);

        //Set CPU memory mapper
        cpu.set_memMap(mapperNum);
        
        // Instead of using OOP principles to implement mappers, each mapper will have a write function stored in a table

        // Check for trainer; low-key don't know what to do if there is one in terms of writing to memory, so will just skip the trainer
        // if there is one for now

        if ((flag6 & 4) == 4) {
            romFile.seekg(528);
        }
        else {
            romFile.seekg(16);
        }

        // Read in the initial PRG-ROM into the CPU - varies based on mapper
        // May need to re-engineer this later but this shall do for having no mapper probably

        // Mirror the rom bank
        if (prg_rom == 1) {
            for (int i = 0; i < 16000; i++) {
                romFile.read(&byte, 1);
                cpu.memory[0x8000 + i] = byte;
                cpu.memory[0xC000 + i] = byte;
            }
        }
        // Load first two banks into memory otherwise
        else {
            for (int i = 0; i < 32000; i++) {
                romFile.read(&byte, 1);
                cpu.memory[0x8000 + i] = byte;
            }
        }

        // Read CHR-ROM
        // CHR-ROM is used by the PPU to fill the pattern table, hence I will just move on for now
        // If CHR ROM is 0, CHR RAM is used
        if (chr_rom != 0) {
            // NEEDS TO BE REPLACED LATER
            romFile.seekg(8192 * chr_rom, std::ios::cur);
        }

        // Lastly, there's PlayChoice ROM which is kinda niche, but will deal with anyway
        // If the 1st bit of flag 7 is set, there's 8KB of additional data to read called INST ROM as well as
        // 16 bytes of PROM Data output used to decrypt the INST and 16 bytes of PROM CounterOut output used similarly which is sometimes ignored
        if (flag7 & 2 == 2) {
            // NEEDS TO BE REPLACED LATER
            romFile.seekg(8192, std::ios::cur);
        }

        // May be an additional bit of data at the end of file but it can be safely ignored

        // Can now begin execution

        // Perform reset interrupt
        cpu.interrupt_reset();
        running = true;

        // Code execution -- need to add timing and some simulation of concurrency, but this should work for testing the CPU
        while (running) {
            cpu.decode();
        }

        romFile.close();
    }
    else {

        std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;

    }
}

// Function that just runs Kevin Horton's nestest in automation mode and creates a log file
void Emulator::nes_test() {
    running = false;
    cpu.manual_reset();

    this->romFile.open("nestest.nes", std::ios::in | std::ios::binary | std::ios::ate);
    
    // Set the program counter to 0xC000 to run in automation mode
    cpu.set_PC(0xC000);

    // Load the test ROM
    // Skip header for now
    romFile.seekg(0x10);
    char byte;

    // Load the test into ROM
    for (int i = 0; i < 16000; i++) {
        romFile.read(&byte, 1);
        cpu.memory[0x8000 + i] = byte;
        cpu.memory[0xC000 + i] = byte;
    }

    // Create log file
    std::ofstream test_log = std::ofstream("nestest log.txt");

    std::string pc, acc, hi, low, sp, p, x, y, op;

    running = true;
    while (running) {
        // Log state of registers, PC, etc before instruction is decoded
        pc  = hex(cpu.get_PC(), 4);
        acc = hex(static_cast<uint8_t>(cpu.get_accumulator()), 2);
        sp  = hex(cpu.get_stack(), 2);
        p   = hex(cpu.get_status(), 2);
        x   = hex(static_cast<uint8_t>(cpu.get_x()), 2);
        y   = hex(static_cast<uint8_t>(cpu.get_y()), 2);
        
        // Decode and execute instruction
        cpu.decode();

        // Log values of opcode and operands
        op = hex(cpu.get_opcode(), 2);
        hi  = hex(cpu.get_high_nibble(), 2);
        low = hex(cpu.get_low_nibble(), 2);

        // Write to log file
        test_log << pc << "  ";
    }
}

// Helper function for writing log files
std::string hex(uint32_t value, int width)
{
    std::stringstream ss;
    ss << std::uppercase
       << std::hex
       << std::setw(width)
       << std::setfill('0')
       << value;
    return ss.str();
}