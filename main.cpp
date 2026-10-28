#include "emu.h"
#include <unordered_set>

#undef main
// Log setting macros
#define STANDARD_SETTINGS std::unordered_set<Log_Settings>({Log_Settings::ACCUMULATOR, Log_Settings::CYCLE, Log_Settings::DOT, Log_Settings::SCANLINE, Log_Settings::STACK_POINTER, Log_Settings::STATUS, Log_Settings::X_REG, Log_Settings::Y_REG, Log_Settings::OP_CODE})
#define MESEN_JUST_CPU_SETTINGS std::unordered_set<Log_Settings>({Log_Settings::ACCUMULATOR, Log_Settings::STACK_POINTER, Log_Settings::STATUS, Log_Settings::X_REG, Log_Settings::Y_REG})

int main(int argc, char *argv [] ) {
    Emulator emu = Emulator();
    //emu.nes_test();
    //emu.run("Donkey Kong (World) (Rev A).nes");
    //emu.blargg();
    emu.run_log("03-immediate.nes", MESEN_JUST_CPU_SETTINGS, 10000);
    return 0;
}