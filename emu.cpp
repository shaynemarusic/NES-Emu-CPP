#include "emu.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <algorithm>

std::string hex(uint32_t value, int width);

Emulator::Emulator() {
    running = false;

    //Initialize PPU
    ppu = PPU();

    //Initialize APU
    //apu = APU()

    //Initialize CPU
    cpu = CPU();
    cpu.link_ppu(&ppu);

    // SDL setup
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {

        printf("Error initializing SDL: %s\n", SDL_GetError());
        throw std::runtime_error("");

    }

    window = NULL;
    renderer = NULL;

    window = SDL_CreateWindow("ShayNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (window == NULL) {

        printf("Error creating SDL window: %s\n", SDL_GetError());
        throw std::runtime_error("");

    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {

        printf("Error creating SDL renderer: %s\n", SDL_GetError());
        throw std::runtime_error("");

    }

    SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    // Create Texture
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    // Display a blank screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

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
    // ppu.manual_reset();
    read_file(filename);
    // Can now begin execution

    // Perform reset interrupt
    long long cycles = 0;
    cycles += cpu.interrupt_reset();
    for (int i = 0; i < cycles * 3; i++) ppu.tick();
    running = true;

    // Code execution -- need to add timing and some simulation of concurrency, but this should work for testing the CPU
    int cycle_delta = 0;
    auto last_time = std::chrono::high_resolution_clock::now();

    while (running) {
        //Allows user to close window
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch(event.type) {

                case SDL_QUIT:
                    running = false;
                    break;
    
            }

        }

        auto current_time = std::chrono::high_resolution_clock::now();
        //auto ld = std::chrono::duration<long double>(current_time - last_time);
        auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(current_time - last_time).count();

        if (diff > clock_speed * cycle_delta) {
            double render_diff = diff * 10e-6;
            // Check to see if we need to update the screen
            if (render_diff > render_speed) update_screen();

            last_time = current_time;

            cycle_delta = cpu.decode();

            // Check for NMI being triggered
            if (ppu.nmi_trigger) cycle_delta += cpu.interrupt_NMI();

            // Run ppu the necessary number of cycles
            for (int i = 0; i < cycle_delta * 3; i++) {
                ppu.tick();
                // Check for NMI
                if (ppu.nmi_trigger) cycle_delta += cpu.interrupt_NMI();
            }

            cycles += cycle_delta;
        }
    }

    emulator_cleanup();
}

// Runs the file and creates a log file containing info on instructions executed and the status of CPU and PPU registers
void Emulator::run_log(const char * filename, std::vector<Log_Settings> settings) {
    running = false;
    // Reset components to known state
    cpu.manual_reset();
    // ppu.manual_reset();
    read_file(filename);
    // Create log file
    std::string fn(filename);
    std::ofstream test_log = std::ofstream(fn.substr(0, fn.length() - 4) + " Output Log.txt");
    // Can now begin execution

    // Gather log setting info
    int setting_mask = 0;
    for (int i = 0; i < settings.size(); i++) {
        switch (settings[i]) {
            case Log_Settings::ACCUMULATOR:
                setting_mask |= 1;
                break;
            case Log_Settings::X_REG:
                setting_mask |= 2;
                break;
            case Log_Settings::Y_REG:
                setting_mask |= 4;
                break;
            case Log_Settings::STATUS:
                setting_mask |= 8;
                break;
            case Log_Settings::STACK_POINTER:
                setting_mask |= 16;
                break;
            case Log_Settings::SCANLINE:
                setting_mask |= 32;
                break;
            case Log_Settings::DOT:
                setting_mask |= 64;
                break;
            case Log_Settings::CYCLE:
                setting_mask |= 128;
                break;
        }
    }

    std::string pc, acc, hi, low, sp, p, x, y, op, ppux, ppuy;

    int lines = 0;
    // Perform reset interrupt
    long long cycles = 0;
    cycles += cpu.interrupt_reset();
    for (int i = 0; i < cycles * 3; i++) ppu.tick();
    running = true;

    // Code execution -- need to add timing and some simulation of concurrency, but this should work for testing the CPU
    int cycle_delta = 0;
    auto last_time = std::chrono::high_resolution_clock::now();

    while (running) {
        //Allows user to close window
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch(event.type) {

                case SDL_QUIT:
                    running = false;
                    break;
    
            }

        }

        auto current_time = std::chrono::high_resolution_clock::now();
        //auto ld = std::chrono::duration<long double>(current_time - last_time);
        auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(current_time - last_time).count();

        if (diff > clock_speed * cycle_delta) {
            double render_diff = diff * 10e-6;
            // Check to see if we need to update the screen
            if (render_diff > render_speed) update_screen();

            last_time = current_time;
            lines++;

            // Log state of registers, PC, etc before instruction is decoded
            uint16_t pcint = cpu.get_PC();
            pc = hex(pcint, 4);
            acc = hex(cpu.get_accumulator(), 2);
            sp = hex(cpu.get_stack(), 2);
            p = hex(cpu.get_status(), 2);
            x = hex(cpu.get_x(), 2);
            y = hex(cpu.get_y(), 2);

            // Ugly but effective
            {
                std::stringstream ss;
                ss << std::setw(3) << ppu.get_dot();
                ppux = ss.str();
            }
            {
                std::stringstream ss;
                ss << std::setw(3) << ppu.get_scanline();
                ppuy = ss.str();
            }

            // Log values of opcode and operands
            op = hex(cpu.get_next_opcode(), 2);
            hi = hex(cpu.get_next_high_nibble(), 2);
            low = hex(cpu.get_next_low_nibble(), 2);

            // Write to log file
            test_log << pc << std::setw(4);
            // Next part is opcode dependent
            test_log << op;
            int op_int = cpu.get_next_opcode();
            if (pcIncrement[op_int] == 1) {
                test_log << std::setw(11);
            }
            else if (pcIncrement[op_int] == 2) {
                test_log << " " << low << std::setw(8);
            }
            else if (pcIncrement[op_int] == 3) {
                test_log << " " << low << " " << hi << std::setw(5);
            }

            if (unofficialOpcode[op_int]) {
                
            }
            test_log << opcodeName[op_int];
            AddressingMode mode = opcodeMode[op_int];
            uint16_t exp, ind_add, exp_low, exp_high;
            switch (mode) {
                case AddressingMode::IMP:
                    test_log << std::setw(31);
                    break;
                case AddressingMode::ACC:
                    test_log << " A" << std::setw(29);
                    break;
                case AddressingMode::IMM:
                    test_log << " #$" << low << std::setw(26);
                    break;
                case AddressingMode::ZP:
                    test_log << " $" << low << " = " << hex(cpu.memory[cpu.get_next_low_nibble()], 2) << std::setw(22);
                    break;
                case AddressingMode::ZPX:
                    exp = (cpu.get_next_low_nibble() + cpu.get_x()) & 0xFF;
                    test_log << " $" << low << ",X @ " << hex(exp, 2) << " = " << hex(cpu.read(exp), 2) << std::setw(15);
                    break;
                case AddressingMode::ZPY:
                    exp = (cpu.get_next_low_nibble() + cpu.get_y()) & 0xFF;
                    test_log << " $" << low << ",Y @ " << hex(exp, 2) << " = " << hex(cpu.read(exp), 2) << std::setw(15);
                    break;
                case AddressingMode::JSR:
                    test_log << " $" << hi << low << std::setw(25);
                    break;
                case AddressingMode::ABSX:
                    exp = ((((uint16_t) cpu.get_next_high_nibble()) << 8) | cpu.get_next_low_nibble()) + cpu.get_x();
                    test_log << " $" << hi << low << ",X @ " << hex(exp, 4) << " = " << hex(cpu.read(exp), 2) << std::setw(11);
                    break;
                case AddressingMode::ABSY:
                    exp = ((((uint16_t) cpu.get_next_high_nibble()) << 8) | cpu.get_next_low_nibble()) + cpu.get_y();
                    test_log << " $" << hi << low << ",Y @ " << hex(exp, 4) << " = " << hex(cpu.read(exp), 2) << std::setw(11);
                    break;
                case AddressingMode::IND: 
                    exp_low = ((uint16_t) cpu.get_next_high_nibble() << 8) | cpu.get_next_low_nibble();
                    exp_high = ((uint16_t) cpu.get_next_high_nibble() << 8) | ((cpu.get_next_low_nibble() + 1) & 0xFF);
                    exp = ((uint16_t) cpu.read(exp_high) << 8) | (uint16_t)cpu.read(exp_low);
                    test_log << " ($" << hi << low << ") = " << hex(exp, 4) << std::setw(16);
                    break;
                case AddressingMode::INDX:
                    exp = (cpu.get_next_low_nibble() + cpu.get_x()) & 0xFF;
                    ind_add = ((uint16_t) cpu.read((exp + 1) & 0xFF) << 8) | cpu.read(exp);
                    test_log << " ($" << low << ",X) @ " << hex(exp, 2) << " = " << hex(ind_add, 4) << " = " << hex(cpu.read(ind_add), 2) << std::setw(6);
                    break;
                case AddressingMode::INDY:
                    exp = (((uint16_t) cpu.read((cpu.get_next_low_nibble() + 1) & 0xFF) << 8) | cpu.read(cpu.get_next_low_nibble()));
                    ind_add = exp + cpu.get_y();
                    test_log << " ($" << low << "),Y = " << hex(exp, 4) << " @ " << hex(ind_add, 4) << " = " << hex(cpu.read(ind_add), 2) << std::setw(4);
                    break;
                case AddressingMode::REL:
                    exp = pcint + (int8_t)cpu.get_next_low_nibble() + 2;
                    test_log << " $" << hex(exp, 4) << std::setw(25);
                    break;
                case AddressingMode::ABS:
                    exp = ((uint16_t) cpu.get_next_high_nibble() << 8) | cpu.get_next_low_nibble();
                    test_log << " $" << hi << low << " = " << hex(cpu.read(exp), 2) << std::setw(20);
                    break;
            }

            cycle_delta = cpu.decode();

            // Check for NMI being triggered
            if (ppu.nmi_trigger) cycle_delta += cpu.interrupt_NMI();

            // Run ppu the necessary number of cycles
            for (int i = 0; i < cycle_delta * 3; i++) {
                ppu.tick();
                // Check for NMI
                if (ppu.nmi_trigger) cycle_delta += cpu.interrupt_NMI();
            }

            // Write state of registers
            if ((setting_mask & 1) == 1) {
                test_log << "A:" << acc << " ";
            }
            test_log << "A:"
                     << acc 
                     << " X:" 
                     << x 
                     << " Y:" 
                     << y 
                     << " P:" 
                     << p 
                     << " SP:" 
                     << sp 
                     << " PPU:" 
                     << ppuy 
                     << "," 
                     << ppux 
                     << " CYC:" 
                     << std::to_string(cycles) 
                     << std::endl;

            cycles += cycle_delta;
            if (lines >= 10000) {
                running = false;
            }
        }
    }

    test_log.close();
    emulator_cleanup();
}

// Function that just runs Kevin Horton's nestest in automation mode and creates a log file
void Emulator::nes_test() {
    running = false;
    cpu.manual_reset();

    this->romFile.open("nestest.nes", std::ios::in | std::ios::binary | std::ios::ate);
    std::ifstream good_log;
    good_log.open("nestest good log.txt");
    
    // Set the program counter to 0xC000 to run in automation mode
    cpu.set_PC(0xC000);

    // Load the test ROM
    // Skip header for now
    romFile.seekg(0x10);
    char byte;

    // Load the test into ROM
    for (int i = 0; i < 0x4000; i++) {
        romFile.read(&byte, 1);
        cpu.memory[0x8000 + i] = byte;
        cpu.memory[0xC000 + i] = byte;
    }

    // Create log file
    std::ofstream test_log = std::ofstream("nestest log.txt");

    std::string pc, acc, hi, low, sp, p, x, y, op;

    int lines = 0;
    int cycles = 7;
    running = true;
    std::string line;
    while (running) {
        lines++;
        std::getline(good_log, line);
        // Log state of registers, PC, etc before instruction is decoded
        uint16_t pcint = cpu.get_PC();
        pc  = hex(pcint, 4);
        acc = hex(cpu.get_accumulator(), 2);
        sp  = hex(cpu.get_stack(), 2);
        p   = hex(cpu.get_status(), 2);
        x   = hex(cpu.get_x(), 2);
        y   = hex(cpu.get_y(), 2);

        // Log values of opcode and operands
        op = hex(cpu.get_next_opcode(), 2);
        hi  = hex(cpu.get_next_high_nibble(), 2);
        low = hex(cpu.get_next_low_nibble(), 2);

        // Write to log file
        test_log << pc << std::setw(4);
        // Next part is opcode dependent
        test_log << op;
        int op_int = cpu.get_next_opcode();
        if (pcIncrement[op_int] == 1) {
            test_log << std::setw(11);
        }
        else if (pcIncrement[op_int] == 2) {
            test_log << " " << low << std::setw(8);
        }
        else if (pcIncrement[op_int] == 3) {
            test_log << " " << low << " " << hi << std::setw(5);
        }

        if (unofficialOpcode[op_int]) {
            
        }
        test_log << opcodeName[op_int];
        AddressingMode mode = opcodeMode[op_int];
        uint16_t exp, ind_add, exp_low, exp_high;
        switch (mode) {
            case AddressingMode::IMP:
                test_log << std::setw(31);
                break;
            case AddressingMode::ACC:
                test_log << " A" << std::setw(29);
                break;
            case AddressingMode::IMM:
                test_log << " #$" << low << std::setw(26);
                break;
            case AddressingMode::ZP:
                test_log << " $" << low << " = " << hex(cpu.memory[cpu.get_next_low_nibble()], 2) << std::setw(22);
                break;
            case AddressingMode::ZPX:
                exp = (cpu.get_next_low_nibble() + cpu.get_x()) & 0xFF;
                test_log << " $" << low << ",X @ " << hex(exp, 2) << " = " << hex(cpu.memory[exp], 2) << std::setw(15);
                break;
            case AddressingMode::ZPY:
                exp = (cpu.get_next_low_nibble() + cpu.get_y()) & 0xFF;
                test_log << " $" << low << ",Y @ " << hex(exp, 2) << " = " << hex(cpu.memory[exp], 2) << std::setw(15);
                break;
            case AddressingMode::JSR:
                test_log << " $" << hi << low << std::setw(25);
                break;
            case AddressingMode::ABSX:
                exp = ((((uint16_t) cpu.get_next_high_nibble()) << 8) | cpu.get_next_low_nibble()) + cpu.get_x();
                test_log << " $" << hi << low << ",X @ " << hex(exp, 4) << " = " << hex(cpu.memory[exp], 2) << std::setw(11);
                break;
            case AddressingMode::ABSY:
                exp = ((((uint16_t) cpu.get_next_high_nibble()) << 8) | cpu.get_next_low_nibble()) + cpu.get_y();
                test_log << " $" << hi << low << ",Y @ " << hex(exp, 4) << " = " << hex(cpu.memory[exp], 2) << std::setw(11);
                break;
            case AddressingMode::IND: 
                exp_low = ((uint16_t) cpu.get_next_high_nibble() << 8) | cpu.get_next_low_nibble();
                exp_high = ((uint16_t) cpu.get_next_high_nibble() << 8) | ((cpu.get_next_low_nibble() + 1) & 0xFF);
                exp = ((uint16_t) cpu.memory[exp_high] << 8) | (uint16_t)cpu.memory[exp_low];
                test_log << " ($" << hi << low << ") = " << hex(exp, 4) << std::setw(16);
                break;
            case AddressingMode::INDX:
                exp = (cpu.get_next_low_nibble() + cpu.get_x()) & 0xFF;
                ind_add = ((uint16_t) cpu.memory[(exp + 1) & 0xFF] << 8) | cpu.memory[exp];
                test_log << " ($" << low << ",X) @ " << hex(exp, 2) << " = " << hex(ind_add, 4) << " = " << hex(cpu.memory[ind_add], 2) << std::setw(6);
                break;
            case AddressingMode::INDY:
                exp = (((uint16_t) cpu.memory[(cpu.get_next_low_nibble() + 1) & 0xFF] << 8) | cpu.memory[cpu.get_next_low_nibble()]);
                ind_add = exp + cpu.get_y();
                test_log << " ($" << low << "),Y = " << hex(exp, 4) << " @ " << hex(ind_add, 4) << " = " << hex(cpu.memory[ind_add], 2) << std::setw(4);
                break;
            case AddressingMode::REL:
                exp = pcint + (int8_t)cpu.get_next_low_nibble() + 2;
                test_log << " $" << hex(exp, 4) << std::setw(25);
                break;
            case AddressingMode::ABS:
                exp = ((uint16_t) cpu.get_next_high_nibble() << 8) | cpu.get_next_low_nibble();
                test_log << " $" << hi << low << " = " << hex(cpu.memory[exp], 2) << std::setw(20);
                break;
        }

        // Decode and execute instruction
        int temp = cpu.decode();

        // Write state of registers
        test_log << "A:" << acc << " X:" << x << " Y:" << y << " P:" << p << " SP:" << sp << " PPU:";

        // TODO: Add PPU and cycle info
        // Hackey bs for the sake of comparing files
        for (int i = 78; i < line.size() && line[i] != ':'; i++) {
            test_log << line[i];
        }
        test_log << ":" << std::to_string(cycles) << std::endl;
        cycles += temp;
        if (lines >= 8991) {
            running = false;
        }
    }

    romFile.close();
    good_log.close();
    test_log.close();
}

void Emulator::blargg() {
    running = true;
    cpu.manual_reset();

    read_file("01-basics.nes");
    //cpu.memory[0x6000] = 0x80;

    cpu.interrupt_reset();
    int test = cpu.memory[0x6000];
    std::cout << test;
    int ops = 0;

    while (running) {
        // if (ops == 1031) {
        //     ops++;
        //     ops--;
        // }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch(event.type) {

                case SDL_QUIT:
                    running = false;
                    break;
    
            }

        }

        cpu.decode();
        //ops += 1;
        //std::cout << ops << std::endl;
        //std::cout << int(cpu.memory[0x6000]) << std::endl;
        if (cpu.memory[0x6000] < 80 && cpu.memory[0x6000] != 0) {
            running = false;
        }
    }
    test = cpu.memory[0x6000];
    int i = 0;
    std::string out = "";
    while (cpu.memory[0x6000 + i] != 0) {
        out += char(cpu.memory[0x6000 + i]);
    }
    std::cout << out << std::endl;
    emulator_cleanup();
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

// Do all of the initial file reading
void Emulator::read_file(const char * filename) {
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
            for (int i = 0; i < 0x4000; i++) {
                romFile.read(&byte, 1);
                cpu.memory[0x8000 + i] = byte;
                cpu.memory[0xC000 + i] = byte;
            }
        }
        // Load first two banks into memory otherwise
        else {
            for (int i = 0; i < 0x8000; i++) {
                romFile.read(&byte, 1);
                cpu.memory[0x8000 + i] = byte;
            }
        }

        // Read CHR-ROM
        // CHR-ROM is used by the PPU to fill the pattern table, some mappers/games handle pattern tables differently and may
        // need to account for that
        // If CHR ROM is 0, CHR RAM is used
        if (chr_rom != 0) {
            // NEEDS TO BE REPLACED LATER
            for (int i = 0; i < 0x2000; i++) {
                romFile.read(&byte, 1);
                ppu.memory[i] = byte;
            }
            //romFile.seekg(8192 * chr_rom, std::ios::cur);
        }

        // Lastly, there's PlayChoice ROM which is kinda niche, but will deal with anyway
        // If the 1st bit of flag 7 is set, there's 8KB of additional data to read called INST ROM as well as
        // 16 bytes of PROM Data output used to decrypt the INST and 16 bytes of PROM CounterOut output used similarly which is sometimes ignored
        if (flag7 & 2 == 2) {
            // NEEDS TO BE REPLACED LATER
            romFile.seekg(8192, std::ios::cur);
        }

        // May be an additional bit of data at the end of file but it can be safely ignored
    }
    else {

        throw std::runtime_error("Error: ROM could not be opened. Please make sure the file path is correct.");

    }
}

// Called after execution finishes - does not manage log files
void Emulator::emulator_cleanup() {
    romFile.close();

    // SDL Cleanup
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Emulator::update_screen() {
    uint8_t* locked_pixels = nullptr;
    int pitch = 0;
    SDL_LockTexture(texture, NULL, reinterpret_cast<void **>(&locked_pixels), &pitch);
    std::copy_n(ppu.frame_buffer, LOGICAL_WIDTH * LOGICAL_HEIGHT * 4, locked_pixels);
    SDL_UnlockTexture(texture);

    // Render the new pixel data
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}