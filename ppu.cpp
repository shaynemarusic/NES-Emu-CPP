#include "ppu.h"

// Constructors

// Default constructor
PPU::PPU() {
    memory = std::unique_ptr<uint8_t[]>(new uint8_t[65536]);
    std::fill_n(this->memory.get(), 65536, 0);
    vertical_mirroring = 0;
    memory_mapper = 0;

    // Set registers
    ppuctrl = 0;
    ppumask = 0;
    ppustatus = 0;
    oamaddr = 0;
    oamdata = 0;
    ppuscroll = 0;
    ppuaddr = 0;
    ppudata = 0;
    oamdma = 0;
}

// Setters + Getters
void PPU::set_memory_mapper(int mapper) { memory_mapper = mapper; }

int PPU::get_mapper() const { return memory_mapper; }

void PPU::set_mirroring(int vert) { vertical_mirroring = vert; }

int PPU::get_mirroring() const { return vertical_mirroring; }

void PPU::set_ppuctrl(uint8_t value) { ppuctrl = value; }

uint8_t PPU::get_ppuctrl() const { return ppuctrl; }

void PPU::set_ppumask(uint8_t value) { ppumask = value; }

uint8_t PPU::get_ppumask() const { return ppumask; }

void PPU::set_ppustatus(uint8_t value) { ppustatus = value; }

uint8_t PPU::get_ppustatus() const { return ppustatus; }

void PPU::set_oamaddr(uint8_t value) { oamaddr = value; }

uint8_t PPU::get_oamaddr() const { return oamaddr; }

void PPU::set_oamdata(uint8_t value) { oamdata = value; }

uint8_t PPU::get_oamdata() const { return oamdata; }

void PPU::set_ppuscroll(uint8_t value) { ppuscroll = value; }

uint8_t PPU::get_ppuscroll() const { return ppuscroll; }

void PPU::set_ppuaddr(uint8_t value) { ppuaddr = value; }

uint8_t PPU::get_ppuaddr() const { return ppuaddr; }

void PPU::set_ppudata(uint8_t value) { ppudata = value; }

uint8_t PPU::get_ppudata() const { return ppudata; }

void PPU::set_oamdma(uint8_t value) { oamdma = value; }

uint8_t PPU::get_oamdma() const { return oamdma; }