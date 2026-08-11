
// This class represents the PPU (duh, again). The NES used a 2C02
class PPU {

    private:
        // Memory
        /*
        Like the CPU, the PPU can address 64KB of memory, though it only actually has 16 (addresses past 3FFF are wrapped around)
        The situation is actually similar for the CPU, however, I was unaware when desigining that class; may need to adjust later
        Layout:
            Pattern Tables: 0000 - 0x1FFF
                Pattern tables store the 8x8 pixel tiles which can be drawn
                Many games store pattern tables in CHR-ROM, but those that don't use RAM and fill them during execution
                Each of the two pattern tables contain one of the least significant bits of a color that is combined with info from
                an attribute table to determine the color of a given pixel. For example if 0x0000[0] = 1 and 0x0008[0] = 0, then the
                lower order bits of that associated pixel's color are 0b01
                - Pattern Table 1: 0000 - 0x0FFF
                - Pattern Table 2: 0x1000 - 0x1FFF
            Name Tables: 0x2000 - 0x3EFF
                A Name table is a table of 32x30 tiles where each tile points to one of the tiles in the pattern tables (meaning each
                table covers a region of 256x240 pixels). Attribute tables contain the upper two bits of the color info for the
                associated tiles
                - Name Table 0: 0x2000 - 0x23BF
                - Attribute Table 0: 0x23C0 - 23FF
                - Name Table 1: 0x2400 - 0x27BF
                - Attribute Table 1: 0x27C0 - 0x27FF
                - Name Table 2: 0x2800 - 0x2BBF
                - Attribute Table 2: 0x2BC0 - 0x2BFF
                - Name Table 3: 0x2C00 - 0x2FBF
                - Attribute Table 3: 0x2FC0 - 0x2FFF
                - Mirrors of 0x2000 - 0x2EFF: 0x3000 - 0x3EFF - can probably be ignored
            Palettes: 0x3F00 - 0x3F1F
                Each palette contains 16 entries. Each entry is not a color itself, but rather an index to a color in the system
                palette. Bits 6 and 7 of each entry are ignorable
                - Image Palette: 0x3F00 - 0x3F0F
                    The colors usable for the background
                    0x3F00 is The Background Color (used when a sprite is transparent) and it is mirrored every four bytes
                - Sprite Palette: 0x3F10 - 0x3F1F
                    The colors usable for the sprites
                - Mirrors of 0x3F00 - 0x3F1F: 0x3F20 - 0x3FFF
            Mirrors: 0x4000 - 0x10000
        */
    public:
        PPU();
};