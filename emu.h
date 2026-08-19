#include "cpu.h"
#include "./SDL2/include/SDL.h"

class Emulator {

    private:
        CPU cpu;
        PPU ppu;
        //PPU ppu;
        //APU apu;
        std::fstream romFile;
        //Size of PRG ROM in 16KB units
        int prg_rom;
        //Size CHR ROM in 8KB units; 0 indicates use of CHR RAM
        int chr_rom;
        //iNES header flags used for determining which memory mapper to use
        int flag6;
        int flag7;
        bool running;

        // The CPU runs at 1.79 MHz on NTSC systems - clock speed is in nanoseconds
        double const clock_speed = 1000000000 / 1790000;
        double render_speed = 1000 / 60;

        // Used for rendering
        const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 320, LOGICAL_WIDTH = 256, LOGICAL_HEIGHT = 240;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;
    public:
        //Emulator(const char * filename);
        Emulator();
        void nes_test();
        void run(const char * filename);
};