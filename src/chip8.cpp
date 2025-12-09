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

	LoadFontset();

	// Initialize random number generator
	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

	SetupFunctionPointerTable();
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

void Chip8::LoadFontset() // Load fonts into memory
{
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}
}

void Chip8::Cycle()
{
	// Fetch
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	// Increment PC
	pc += 2;

	// Decode and execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement delay timer if set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement sound timer if set
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}

void Chip8::SetupFunctionPointerTable()
{
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &Chip8::OP_NULL;
		table8[i] = &Chip8::OP_NULL;
		tableE[i] = &Chip8::OP_NULL;
	}

	// Table 0 function pointers
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	// Table 8 function pointers
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	// Table E function pointers
	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	// Table F function pointers
	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &Chip8::OP_NULL;
	}
	tableE[0x07] = &Chip8::OP_Fx07;
	tableE[0x0A] = &Chip8::OP_Fx0A;
	tableE[0x15] = &Chip8::OP_Fx15;
	tableE[0x18] = &Chip8::OP_Fx18;
	tableE[0x1E] = &Chip8::OP_Fx1E;
	tableE[0x29] = &Chip8::OP_Fx29;
	tableE[0x33] = &Chip8::OP_Fx33;
	tableE[0x55] = &Chip8::OP_Fx55;
	tableE[0x65] = &Chip8::OP_Fx65;
}

// The first three digits are $00E but the fourth digit is unique
void Chip8::Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

// The first digit 8 repeats but the last digit is unique
void Chip8::Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

// The first digit E repeats but the last two digits are unique
void Chip8::TableE()
{
	((*this).*(tableE[opcode & 0x000Fu]))();
}

// The first digit F repeats but the last two digits are unique
void Chip8::TableF()
{
	((*this).*(tableF[opcode & 0x00FFu]))();
}

// Opcodes
void Chip8::OP_NULL()
{
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

void Chip8::OP_8xyE() // Set Vx = Vx SHL 1
{
	// If MSB of Vx is 1, then VF = 1, otherwise 0. Then Vx * 2
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
	registers[Vx] <<= 1; // Shift left by 1
}

void Chip8::OP_9xy0() // Skip next instruction is Vx != Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_Annn() // Set I = nnn
{
	uint16_t address = opcode & 0x0FFFu;
	index = address;
}

void Chip8::OP_Bnnn() // Jump to location nnn + V0
{
	uint16_t address = opcode & 0x0FFFu;
	pc = registers[0] + address;
}

void Chip8::OP_Cxkk() // Set Vx = random byte AND kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn() // Display n-byte sprite starting at memory location I,
{					  // at (Vx, Vy), set VF = collision
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;
	// Reset register VF to 0, used as a collision flag
	registers[0xF] = 0;

	// Iterate through all rows for specified height n
	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row];
		// Iterate through columns
		for (unsigned int col = 0; col < 8; ++col)
		{
			// 0x80 is binary 10000000. Shifting it right by col places the 1 at
			// the desired bit position. The bitwise AND operation (&) isolates
			// that single sprite pixel.
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			// Calculates the exact memory address in the video buffer (your SDL
			// pixel buffer) that corresponds to the screen pixel being drawn to.
			uint32_t *screenPixelPointer = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Check if sprite pixel is on
			if (spritePixel)
			{
				// Check if screen pixel is also on - collision
				if (*screenPixelPointer == 0xFFFFFFFF)
				{
					registers[0xF] = 1; // Set collision flag to 1
				}
				// XOR with sprite pixel
				*screenPixelPointer ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_Ex9E() // Skip next instruction if key with value of Vx is pressed
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];
	if (keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_ExA1() // Skip next instruction if key with the value of Vx is not pressed
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];
	if (!keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_Fx07() // Set Vx = delay timer value
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A() // Wait for a key press, store value of key in Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	bool keyPressDetected = false;
	for (int i = 0; i < 16; i++)
	{
		if (keypad[i])
		{
			registers[Vx] = i;
			keyPressDetected = true;
			break; // Exit the loop immediately once a key is found
		}
	}
	if (!keyPressDetected)
	{
		// No key pressed this cycle, so execute the wait by decrementing the PC.
		// This ensures the same instruction is fetched and executed again next cycle.
		pc -= 2;
	}
}

void Chip8::OP_Fx15() // Set delay timer = Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	delayTimer = registers[Vx];
}

void Chip8::OP_Fx15() // Set sound timer = Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E() // Set I = I + Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	index += registers[Vx];
}

void Chip8::OP_Fx29() // Set I = location of sprite for digit Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];
	// Can get address of the first byte of any font character by taking an offset
	// from the start address
	index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33() // Store BCD representation of Vx in memory locations
{					  // I, I + 1, I + 2
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t number = registers[Vx];
	// The interpreter takes the decimal value of Vx, and places the hundreds
	// digit in memory at location in I, the tens digit at location I+1, and
	// the ones digit at location I+2.

	// Ones place
	memory[index + 2] = number % 10;
	number /= 10;
	// Tens place
	memory[index + 1] = number % 10;
	number /= 10;
	// Hundreds place
	memory[index] = number % 10;
	// By extracting the digit at the specific position and storing in the
	// memory location, we directly end up storing the digits in BCD as a result
}

void Chip8::OP_Fx55() // Store registers V0 through Vx in memory, starting at location I
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::OP_Fx65() // Read registers V0 through Vx in memory, starting at location I
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}