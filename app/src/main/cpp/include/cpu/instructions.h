#pragma once 
#include <common.h>

typedef enum 
{
  AM_IMP,     ///< Implied addressing mode, no operand
  AM_R_D16,   ///< Register and 16-bit immediate value
  AM_R_R,     ///< Register to register
  AM_MR_R,    ///< Memory address pointed to by a register
  AM_R,       ///< Register only
  AM_R_D8,    ///< Register and 8-bit immediate value
  AM_R_MR,    ///< Register and memory address pointed to by a register
  AM_R_HLI,   ///< Register and memory address pointed to by HL with increment
  AM_R_HLD,   ///< Register and memory address pointed to by HL with decrement
  AM_HLI_R,   ///< Memory address pointed to by HL with increment and register
  AM_HLD_R,   ///< Memory address pointed to by HL with decrement and register
  AM_R_A8,    ///< Register and 8-bit immediate value as memory address
  AM_A8_R,    ///< 8-bit immediate value as memory address and register
  AM_HL_SPR,  ///< Memory address pointed to by HL and stack pointer relative addressing
  AM_D16,     ///< 16-bit immediate value only
  AM_D8,      ///< 8-bit immediate value only
  AM_D16_R,   ///< 16-bit immediate value and register
  AM_MR_D8,   ///< Memory address pointed to by a register and 8-bit immediate value
  AM_MR,      ///< Memory address pointed to by a register only
  AM_A16_R,   ///< 16-bit immediate value as memory address and register
  AM_R_A16    ///< Register and 16-bit immediate value as memory address
} AddressMode;

typedef enum 
{
  RT_NONE, ///< No register
  RT_A,    ///< Accumulator register
  RT_B,    ///< General purpose register B
  RT_C,    ///< General purpose register C
  RT_D,    ///< General purpose register D
  RT_E,    ///< General purpose register E
  RT_H,    ///< General purpose register H
  RT_L,    ///< General purpose register L
  RT_AF,   ///< Combined register AF (A and F)
  RT_BC,   ///< Combined register BC (B and C)
  RT_DE,   ///< Combined register DE (D and E)
  RT_HL,   ///< Combined register HL (H and L)
  RT_SP,   ///< Stack Pointer register
  RT_PC    ///< Program Counter register
} RegType;

typedef enum
{
  CT_NONE, ///< No condition, used for unconditional instructions
  CT_NZ,   ///< Not Zero condition, used for conditional jumps and calls when the Zero Flag is not set
  CT_Z,    ///< Zero condition, used for conditional jumps and calls when the Zero Flag is set
  CT_NC,   ///< Not Carry condition, used for conditional jumps and calls when the Carry Flag is not set
  CT_C     ///< Carry condition, used for conditional jumps and calls when the Carry Flag is set
} ConditionType;

typedef enum 
{
  IN_NONE, ///< No instruction, used for unimplemented opcodes
  IN_NOP,  ///< No Operation, does nothing and is used for timing purposes
  IN_LD,   ///< Load instruction, used to transfer data between registers and memory
  IN_INC,  ///< Increment instruction, used to add 1 to a register or memory location
  IN_DEC,  ///< Decrement instruction, used to subtract 1 from a register or memory location
  IN_RLCA, ///< Rotate Left Accumulator, rotates the bits in the A register to the left, with the bit that was rotated out being copied to the Carry Flag
  IN_ADD,  ///< Add instruction, used to add the value of a register or memory location to the A register
  IN_RRCA, ///< Rotate Right Accumulator, rotates the bits in the A register to the right, with the bit that was rotated out being copied to the Carry Flag
  IN_STOP, ///< Stop instruction, halts the CPU until a button is pressed or an interrupt occurs
  IN_RLA,  ///< Rotate Left Accumulator, rotates the bits in the A register to the left, with the bit that was rotated out being copied to the Carry Flag and the bit that was rotated in being set to 0
  IN_JR,   ///< Jump Relative, used to jump to a new location in the program by adding a signed 8-bit value to the current program counter
  IN_RRA,  ///< Rotate Right Accumulator, rotates the bits in the A register to the right, with the bit that was rotated out being copied to the Carry Flag and the bit that was rotated in being set to 0
  IN_DAA,  ///< Decimal Adjust Accumulator, used to adjust the value in the A register after a BCD addition or subtraction operation
  IN_CPL,  ///< Complement Accumulator, used to flip all the bits in the A register
  IN_SCF,  ///< Set Carry Flag, used to set the Carry Flag to 1
  IN_CCF,  ///< Complement Carry Flag, used to flip the value of the Carry Flag
  IN_HALT, ///< Halt instruction, halts the CPU until an interrupt occurs
  IN_ADC,  ///< Add with Carry, used to add the value of a register or memory location to the A register along with the value of the Carry Flag
  IN_SUB,  ///< Subtract instruction, used to subtract the value of a register or memory location from the A register
  IN_SBC,  ///< Subtract with Carry, used to subtract the value of a register or memory location from the A register along with the value of the Carry Flag
  IN_AND,  ///< Logical AND, used to perform a bitwise AND operation between the A register and a register or memory location
  IN_XOR,  ///< Logical Exclusive OR, used to perform a bitwise XOR operation between the A register and a register or memory location
  IN_OR,   ///< Logical OR, used to perform a bitwise OR operation between the A register and a register or memory location
  IN_CP,   ///< Compare instruction, used to compare the value of a register or memory location with the A register and set the flags accordingly without changing the value in the A register
  IN_POP,  ///< Pop instruction, used to pop a value from the stack into a register
  IN_JP,   ///< Jump instruction, used to jump to a new location in the program by setting the program counter to a new value
  IN_PUSH, ///< Push instruction, used to push a value from a register onto the stack
  IN_RET,  ///< Return instruction, used to return from a subroutine by popping the return address from the stack and setting the program counter to that address
  IN_CB,   ///< CB prefix instruction, used to access a secondary set of instructions that perform bit manipulation and rotation operations
  IN_CALL, ///< Call instruction, used to call a subroutine by pushing the return address onto the stack and setting the program counter to the address of the subroutine
  IN_RETI, ///< Return from Interrupt, used to return from an interrupt service routine by popping the return address from the stack and setting the program counter to that address, and re-enabling interrupts
  IN_LDH,  ///< Load High, used to load a value into the A register from a memory address in the range 0xFF00-0xFFFF, or to store a value from the A register into a memory address in that range
  IN_JPHL, ///< Jump to address in HL, used to jump to a new location in the program by setting the program counter to the value in the HL register
  IN_DI,   ///< Disable Interrupts, used to disable interrupts by clearing the Interrupt Enable Flag
  IN_EI,   ///< Enable Interrupts, used to enable interrupts by setting the Interrupt Enable Flag
  IN_RST,  ///< Restart, used to call a subroutine at a fixed address (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, or 0x38) by pushing the current program counter onto the stack and setting the program counter to that address
  IN_ERR,  ///< Error instruction, used to indicate an invalid or unimplemented opcode
  IN_RLC,  ///< Rotate Left Circular, rotates the bits in a register or memory location to the left, with the bit that was rotated out being copied to the Carry Flag and the bit that was rotated in being set to the value of the bit that was rotated out.
  IN_RRC,  ///< Rotate Right Circular, rotates the bits in a register or memory location to the right, with the bit that was rotated out being copied to the Carry Flag and the bit that was rotated in being set to the value of the bit that was rotated out.
  IN_RL,   ///< Rotate Left, rotates the bits in a register or memory location to the left, with the bit that was rotated out being copied to the Carry Flag and the bit that was rotated in being set to 0.
  IN_RR,   ///< Rotate Right, rotates the bits in a register or memory location to the right, with the bit that was rotated out being copied to the Carry Flag and the bit that was rotated in being set to 0.
  IN_SLA,  ///< Shift Left Arithmetic, shifts the bits in a register or memory location to the left, with the bit that was shifted out being copied to the Carry Flag and the bit that was shifted in being set to 0.
  IN_SRA,  ///< Shift Right Arithmetic, shifts the bits in a register or memory location to the right, with the bit that was shifted out being copied to the Carry Flag and the bit that was shifted in being set to the value of the most significant bit (the sign bit).
  IN_SWAP, ///< Swap Nibbles, swaps the upper and lower nibbles in a register or memory location, with the bits that were swapped out being copied to the Carry Flag.
  IN_SRL,  ///< Shift Right Logical, shifts the bits in a register or memory location to the right, with the bit that was shifted out being copied to the Carry Flag and the bit that was shifted in being set to 0.
  IN_BIT,  ///< Test Bit, used to test a specific bit in a register or memory location and set the flags accordingly without changing the value in the register or memory location
  IN_RES,  ///< Reset Bit, used to reset a specific bit in a register or memory location to 0
  IN_SET   ///< Set Bit, used to set a specific bit in a register or memory location to 1
} InstructionType;


typedef struct 
{
  InstructionType   type;  ///< The type of instruction, which determines the operation to be performed
  AddressMode       mode;  ///< The addressing mode of the instruction, which determines how the operands are accessed
  RegType           reg1;  ///< The first register operand, if applicable, which specifies the register to be used in the instruction
  RegType           reg2;  ///< The second register operand, if applicable, which specifies the register to be used in the instruction
  ConditionType     cond;  ///< The condition for conditional instructions, if applicable, which specifies the condition under which the instruction should be executed
  u8                param; ///< The immediate parameter for the instruction, if applicable, which specifies a constant value to be used in the instruction
    
  u8 mCycles; ///< The number of cycles the instruction takes to execute, which is used for timing and performance analysis
  u8 mCyclesAlt; ///< The number of cycles the instruction takes to execute if a certain condition is met, which is used for timing and performance analysis of conditional instructions
} Instruction;


/**
 * @brief Retrieves a pointer to the instruction structure corresponding to the given opcode. The returned pointer can be used to access the details of the instruction associated with the provided opcode, such as its type, addressing mode, operands, and timing information.
 * @param OPCODE An 8-bit unsigned integer representing the opcode for which the instruction structure is to be retrieved.
 * @return A pointer to an Instruction structure that corresponds to the specified opcode
*/
const Instruction* instructionGetByOpcode(u8 OPCODE);

/**
 * @brief Retrieves a pointer to the instruction structure corresponding to the given CB-prefixed opcode. The returned pointer can be used to access the details of the instruction associated with the provided CB-prefixed opcode, such as its type, addressing mode, operands, and timing information.
 * @param CB_OPCODE An 8-bit unsigned integer representing the CB-prefixed opcode for which the instruction structure is to be retrieved.
 * @return A pointer to an Instruction structure that corresponds to the specified CB-prefixed opcode
*/
const Instruction* instructionGetByCBOpcode(u8 CB_OPCODE);

/**
 * @brief Retrieves the name of the instruction corresponding to the given instruction type. The returned string can be used for debugging, logging, or displaying the instruction in a human-readable format.
 * @param TYPE An enumeration value representing the type of instruction for which the name is to be retrieved.
 * @return A pointer to a null-terminated string that represents the name of the instruction corresponding to the specified instruction type.
 * @note The returned string is a constant and should not be modified by the caller.
*/
const char* instructionGetName(InstructionType TYPE);

/**
 * @brief Retrieves the name of the register corresponding to the given register type. The returned string can be used for debugging, logging, or displaying the register in a human-readable format.
 * @param REG An enumeration value representing the type of register for which the name is to be retrieved.
 * @return A pointer to a null-terminated string that represents the name of the register corresponding to the specified register type.
 * @note The returned string is a constant and should not be modified by the caller.
*/
const char* instructionGetRegName(RegType REG);

