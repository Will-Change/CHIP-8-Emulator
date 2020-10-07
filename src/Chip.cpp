//i was having some errors so i just included everything like an insane person
#include "Chip.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <vector>
#include "time.h"


//rom is gonna be expecting this in the memory, each character is an example of sprites, each sprite is five bytes(im talking about the fontset)
unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


Chip8::Chip8() {}
Chip8::~Chip8() {}

// Initialise and basically reset everything in a way, i should have used a constructor instead
void Chip8::init() {
    pc      = 0x200;    // setting our program counter to 0x200, 0x200 == 512
    //we just reset these guys
    
    opcode  = 0;
    I     = 0;
    sp      = 0;

    // Clear the display
    for (int i = 0; i < 2048; ++i) {
        gfx[i] = 0;
    }

    // Clear the stack, keypad, and registers
    for (int i = 0; i < 16; ++i) {
        stack[i]    = 0;
        key[i]      = 0;
        V[i]        = 0;
    }

    // Clear memory
    for (int i = 0; i < 4096; ++i) {
        memory[i] = 0;
    }

    // Load font set into memory
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;

    // Seed rng
    srand (time(NULL)); //used to set the starting value (seed) for generating a sequence of pseudo-random integer values.  The srand(x) function sets the seed of the random number generator algorithm used by the function rand( ).  A seed value of 1 is the default setting yielding the same sequence of values as if srand(x) were not used.  Any other value for the seed produces a different sequence.
}

// Initialise and load ROM into memory, we could have used a constructor instead but.....aaaaah i guess this is how we do it then
bool Chip8::loadROM(const int argc, const char *file_name)
{
    // Initialise
    init();
    
    std::string romName;
    if(argc < 2)//check to see if program was opened via command line with an argument (or drag and drop) or via GUI/File Explorer.
       {
           std::cout << "Enter the complete ROM file name(please include the extension as well): \n";
           getline(std::cin, romName);
       }
       else if(argc == 2)
       {
           romName = file_name;
       }
       else
       {
           std::cerr << "Something went terribly wrong when collecting rom name. CHIP8.CPP LN: 369" << std::endl;
           return false;
       }
    
    printf("Loading ROM: %s\n", file_name);

    //open rom file using name passed from argv or cin.
    std::ifstream romfile(romName.data(), std::ios::binary);
    if(!romfile)
    {
        std::cerr << "Error opening "<< file_name <<". Please check the name of your file and try again." << std::endl;
        return false;
    }

    //get rom file size
    romfile.seekg(0L, std::ios::end);
    const int unsigned fsize = romfile.tellg();
    romfile.seekg(0L, std::ios::beg);
    std::cout << romName << " is " << fsize << " bytes." << std::endl;

    //create buffer using rom size
    std::vector<char> romBuffer(fsize);
    if(romBuffer.empty())
    {
        std::cerr << "Error creating memory buffer." << std::endl;
        return false;
    }

    //write file to buffer
    romfile.read(romBuffer.data(), fsize);
    romfile.close();
    if(romBuffer.size() != fsize)
    {
        std::cerr << "Error writing file to buffer." << std::endl;
        return false;
    }

    //write buffer to memory
    if((4096-512) > fsize)
    {
        for(unsigned int pos = 0; pos < fsize; pos++)
        {
            memory[0x200 + pos] = romBuffer[pos];
        }
    }
    else
    {
        std::cerr << "ERROR: ROM file size is too large for memory!" << std::endl;
        return false;
    }
    return true;
}

// Emulate one cycle
void Chip8::emulate_cycle() {

    /*pulls opcode by taking the program counter position
    within the array, shifting it 8 bits to the left and
    combining it with the portion of memory in the array
    directly 1 byte to the right*/
    opcode = memory[pc] << 8 | memory[pc + 1];   // Op code is two bytes

    //could have used member functions instead but we dont have that many opcodes so who gives a fuck, just gonna use switch instead
    
    switch(opcode & 0xF000){

        // 00E_
        case 0x0000:

            switch (opcode & 0x000F) {
                // 00E0 - Clear screen
                case 0x0000:
                    for (int i = 0; i < 2048; ++i) {
                        gfx[i] = 0;
                    }
                    drawFlag = true;
                    pc+=2;
                    break;

                // 00EE - Return from subroutine
                case 0x000E:
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // 1NNN - Jumps to address NNN
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;

        // 2NNN - Calls subroutine at NNN
        case 0x2000:
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;

        // 3XNN - Skips the next instruction if VX equals NN.
        case 0x3000:
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;

        // 4XNN - Skips the next instruction if VX does not equal NN.
        case 0x4000:
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;

        // 5XY0 - Skips the next instruction if VX equals VY.
        case 0x5000:
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        // 6XNN - Sets VX to NN.
        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;

        // 7XNN - Adds NN to VX.
        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;

        // 8XY_
        case 0x8000:
            switch (opcode & 0x000F) {

                // 8XY0 - Sets VX to the value of VY.
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 8XY1 - Sets VX to (VX OR VY).
                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 8XY2 - Sets VX to (VX AND VY).
                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 8XY3 - Sets VX to (VX XOR VY).
                case 0x0003:
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry,
                // and to 0 when there isn't.
                case 0x0004:
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1; //carry
                    else
                        V[0xF] = 0;
                    pc += 2;
                    break;

                // 8XY5 - VY is subtracted from VX. VF is set to 0 when
                // there's a borrow, and 1 when there isn't.
                case 0x0005:
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 0x8XY6 - Shifts VX right by one. VF is set to the value of
                // the least significant bit of VX before the shift.
                case 0x0006:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;

                // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's
                // a borrow, and 1 when there isn't.
                case 0x0007:
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])    // VY-VX
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // 0x8XYE: Shifts VX left by one. VF is set to the value of
                // the most significant bit of VX before the shift.
                case 0x000E:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // 9XY0 - Skips the next instruction if VX doesn't equal VY.
        case 0x9000:
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        // ANNN - Sets I to the address NNN.
        case 0xA000:
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        // BNNN - Jumps to the address NNN plus V0.
        case 0xB000:
            pc = (opcode & 0x0FFF) + V[0];
            break;

        // CXNN - Sets VX to a random number, masked by NN.
        case 0xC000:
            V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
            pc += 2;
            break;

        // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8
        // pixels and a height of N pixels.
        // Each row of 8 pixels is read as bit-coded starting from memory
        // location I;
        // I value doesn't change after the execution of this instruction.
        // VF is set to 1 if any screen pixels are flipped from set to unset
        // when the sprite is drawn, and to 0 if that doesn't happen.
        case 0xD000:
        {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
        }
            break;

        // EX__
        case 0xE000:

            switch (opcode & 0x00FF) {
                // EX9E - Skips the next instruction if the key stored
                // in VX is pressed.
                case 0x009E:
                    if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                        pc +=  4;
                    else
                        pc += 2;
                    break;

                // EXA1 - Skips the next instruction if the key stored
                // in VX isn't pressed.
                case 0x00A1:
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc +=  4;
                    else
                        pc += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // FX__
        case 0xF000:
            switch(opcode & 0x00FF)
            {
                // FX07 - Sets VX to the value of the delay timer
                case 0x0007:
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;

                // FX0A - A key press is awaited, and then stored in VX
                case 0x000A:
                {
                    bool key_pressed = false;

                    for(int i = 0; i < 16; ++i)
                    {
                        if(key[i] != 0)
                        {
                            V[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                        }
                    }

                    // If no key is pressed, return and try again.
                    if(!key_pressed)
                        return;

                    pc += 2;
                }
                    break;

                // FX15 - Sets the delay timer to VX
                case 0x0015:
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // FX18 - Sets the sound timer to VX
                case 0x0018:
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // FX1E - Adds VX to I
                case 0x001E:
                    // VF is set to 1 when range overflow (I+VX>0xFFF), and 0
                    // when there isn't.
                    if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // FX29 - Sets I to the location of the sprite for the
                // character in VX. Characters 0-F (in hexadecimal) are
                // represented by a 4x5 font
                case 0x0029:
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;

                // FX33 - Stores the Binary-coded decimal representation of VX
                // at the addresses I, I plus 1, and I plus 2
                case 0x0033:
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
                    pc += 2;
                    break;

                // FX55 - Stores V0 to VX in memory starting at address I
                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[I + i] = V[i];

                    // On the original interpreter, when the
                    // operation is done, I = I + X + 1.
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        V[i] = memory[I + i];

                    // On the original interpreter,
                    // when the operation is done, I = I + X + 1.
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
            }
            break;

        default:
            printf("\nUnimplemented op code: %.4X\n", opcode);
            exit(3);
    }


    // update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
        if(sound_timer == 1)
            ;
            // TODO: Implement sound
        --sound_timer;

}
