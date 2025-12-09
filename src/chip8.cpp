#include "chip8.hpp"
#include <fstream>
#include <chrono>
#include <random>

const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_SIZE = 80; // 16 chars * 5 bytes = size 80 array
const unsigned int FONTSET_START_ADDRESS = 0x50;
uint8_t fontset[FONTSET_SIZE] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	// Initialize PC
	pc = START_ADDRESS;

	// Load fonts into memory
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	// Initialize random number generator
	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

void Chip8::LoadRom(char const *filename)
{
	// Open the file as a stream of binary, move the file pointer to end
	// std::ios::binary ensures the file is opened in binary mode
	// std::ios::ate stands for "at end"
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		// get size of file
		std::streampos file_size = file.tellg();
		// allocate buffer to hold file contents
		char *buffer = new char[file_size];

		// go back to start of file
		file.seekg(0, std::ios::beg);
		// fill buffer with file contents
		file.read(buffer, file_size);
		file.close();

		// load ROM contents into CHIP-8's memory, starting at 0x200
		for (long i = 0; i < file_size; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		// free the buffer
		delete[] buffer;
	}
}

void Chip8::OP_00E0() // Clear the display
{
	// Set entire video buffer to 0
	memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE() // Return from a subroutine
{
	--sp;
	pc = stack[sp];
}

void Chip8::OP_1nnn() // Jump to location nnn
{
	pc = opcode & 0x0FFFu;
}

void Chip8::OP_2nnn() // Call subroutine at nnn
{
	stack[sp] = pc; // Current pc holds next instruction after CALL due to pc += 2, which is correct
	++sp;
	pc = opcode & 0x0FFFu;
}

void Chip8::OP_3xkk() // Skip next instruction if Vx = kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}

void Chip8::OP_4xkk() // Skip next instruction if Vx != kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}

void Chip8::OP_5xy0() // Skip next instruction if Vx = Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_6xkk() // Set Vx = kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	registers[Vx] = byte;
}

void Chip8::OP_7xkk() // Set Vx = Vx + kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	registers[Vx] += byte;
}

void Chip8::OP_8xy0() // Set Vx = Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1() // Set Vx OR Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2() // Set Vx AND Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3() // Set Vx XOR Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4() // Set Vx = Vx + Vy, set VF = carry
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFF;
}

void Chip8::OP_8xy5() // Set Vx = Vx - Vy, set VF = NOT borrow
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	// If Vx > Vy, VF is set to 1, otherwise 0.
	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6() // Set Vx = Vx SHR 1
{
	// If LSB of Vx is 1, VF is 1, otherwise 0. Then Vx is divided by 2.
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	// Save LSB in VF
	registers[0xF] = registers[Vx] & 0x1u;
	registers[Vx] >>= 1; // Shift right by 1
}

void Chip8::OP_8xy7() // Set Vx = Vy - Vx, set VF = NOT borrow
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}
