/**
 * @file ops.h 
 * @brief function pointers for every opcode
*/

#pragma once 
#include <cpu/instruction.h>
#include <common.h>

/// @brief Crashes
ExecStatus instrUnimplemented(void);

/**
 * @brief Load register to register 
 * Loads the value of register r' into register r 
 * It takes 1 M-cycle
*/
ExecStatus instrLoadRegReg(void);

/**
 * @brief LD r, n (2 M-cycles)
 * M1: Opcode fetch (Dispatcher)
 * M2: Fetch immediate byte and load into register r.
*/
ExecStatus instrLoadReg8bitImm(void);

/**
 * @brief LD r, (HL) (2 M-cycles)
 * M1: Opcode fetch (Dispatcher)
 * M2: Read from memory at address [HL] into register r.
*/
 ExecStatus instrLoadRegHL(void);

/**
 * @brief LD (HL), r (2 M-cycles)
 * M1: Opcode fetch (Dispatcher)
 * M2: Write register r into memory at address [HL].
*/
ExecStatus instrLoadHLReg(void);

/**
 * @brief LD (HL), n8 (3 M-cycles)
 * M1: Opcode fetch (Dispatcher)
 * M2: Fetch immediate byte n into internal latch.
 * M3: Write latched byte into memory at [HL].
*/
ExecStatus instrLoadHL8bitImm(void);

/**
 * @brief LD A, (rr) (2 M cycles)
 * rr is BC or DE
*/
ExecStatus instrLoadAReg16(void);

/**
 * @brief LD (rr), A (2 M cycles)
 * rr is BC or DE
*/
ExecStatus instrLoadReg16A(void);

/**
 * @brief LD (nn), A (4 M-cycles)
 * M1: Opcode fetch (Dispatcher)
 * M2: Fetch LSB of address into latch.
 * M3: Fetch MSB of address into latch.
 * M4: Write A to the latched 16-bit address.
*/
ExecStatus instrLoad16BitImmA(void);

ExecStatus instrLoadA16BitImm(void);

/**
 * @brief LDH A, (C) (2 M-cycles)
 * Address = 0xFF00 + C
*/
ExecStatus instrLoadHighAC(void);

/**
 * @brief LDH (C), A (2 M-cycles)
 * Address = 0xFF00 + C
*/
ExecStatus instrLoadHighCA(void);

/**
 * @brief LDH A, (n) (3 M-cycles)
 * M1: Fetch Opcode (Dispatcher)
 * M2: Fetch immediate offset 'n' into latch Z.
 * M3: Read from [0xFF00 + Z] into A.
*/
ExecStatus instrLoadHighA8BitImm(void);

/**
 * @brief LDH (n), A (3 M-cycles)
 * M1: Fetch Opcode (Dispatcher)
 * M2: Fetch immediate offset 'n' into latch Z.
 * M3: Write A into [0xFF00 + Z].
*/
ExecStatus instrLoadHigh8BitImmA(void);

/**
 * @brief LD A, (HL+/-) (2 M-cycles)
 * M1: Fetch (Dispatcher)
 * M2: Read [HL] into A, then adjust HL.
*/
ExecStatus instrLoadAHLIncDec(void);

/**
 * @brief LD (HL+/-), A (2 M-cycles)
 * M1: Fetch (Dispatcher)
 * M2: Write A into [HL], then adjust HL.
*/
ExecStatus instrLoadHLIncDecA(void);

/**
 * @brief LD rr, nn (3 M-cycles)
 * M1: Fetch Opcode (Dispatcher)
 * M2: Fetch LSB of nn into latch.
 * M3: Fetch MSB of nn, then commit full 16-bit value to rr.
*/
ExecStatus instrLoad16BitReg16BitImm(void);

/**
 * @brief LD (nn), SP (5 M-cycles)
 * M1: Fetch Opcode (Dispatcher)
 * M2: Fetch LSB of nn into latch Z.
 * M3: Fetch MSB of nn into latch W.
 * M4: Write LSB of SP to address WZ, then WZ++.
 * M5: Write MSB of SP to address WZ.
*/
ExecStatus instrLoad16BitImmSP(void);

/**
 * @brief LD SP, HL (2 M-cycles)
 * M1: Fetch (Dispatcher)
 * M2: Internal transfer of HL value to SP.
*/
ExecStatus instrLoadSpHl(void);

/// @brief PUSH rr (4 M-cycles)
ExecStatus instrPush(void);

/// @brief POP rr (3 M-cycles)
ExecStatus instrPop(void);

/**
 * @brief LD HL, SP+e8 (3 M-cycles)
 * M1: Fetch (Dispatcher)
 * M2: Fetch signed immediate e8.
 * M3: Calculate SP + e8, set flags, and load into HL.
*/
ExecStatus instrLoadHlSpE8(void);

/**
 * @brief ADD A, r (1 M-cycle)
 * Adds register r to A and updates flags Z, N(0), H, C.
*/
ExecStatus instrAddAReg(void);

/**
 * @brief ADD A, (HL) (2 M-cycles)
 * M1: Fetch (Dispatcher)
 * M2: Read [HL], perform addition, update A and flags.
*/
ExecStatus instrAddAHL(void);

/**
 * @brief ADD A, n8 (2 M-cycles)
 * M1: Fetch (Dispatcher)
 * M2: Fetch immediate byte, perform addition, update A and flags.
*/
ExecStatus instrAddA8BitImm(void);

/**
 * @brief ADC, r (1 M-cycle )
 * M1: Adds to the 8 bit A register, the carry flag and the 8 bit reg
*/
ExecStatus instrAdcReg(void);

/**
 * @brief ADC (HL) (2 M-cycles)
 * Adds to 8 bit A, the carry flag and the data from the addr specified by 16 bit HL,
 * sroes back into A reg 
*/
ExecStatus instrAdcHL(void);

/**
 * @brief ADC n (2 M-cycles)
 * Adds to tA, the carry flag and the imm data n, and stores back into A
*/
ExecStatus instrAdc8BitImm(void);

/**
 * @brief SUB A, r (1 M-cycle)
 * Subs register r to A and updates flags
*/
ExecStatus instrSubReg(void);

/**
 * @brief SUB HL (2 M-cycle)
 * Subs addr at HL from A 
*/
ExecStatus instrSubHL(void);

/**
 * @brief SUB n (2 M-cycle)
 * Subs 8 bit imm from A 
*/
ExecStatus instrSub8BitImm(void);

/**
 * @brief SBC r (1 M-cycles)
 * Substracts from A, the carry flag and r, store back in A 
*/
ExecStatus instrSbcReg(void);

/**
 * @brief SBC (HL) (2 M-cycles)
 * Subtract from A, the carry flag and data at the addr stored in HL
*/
ExecStatus instrSbcHL(void);

/**
 * @brief SBC n (2 M-cycles)
 * Subtracts from A, carry flag and the imm and store back in A
*/
ExecStatus instrSbc8BitImm(void);

/**
 * @brief CP r (1 M-cycle)
 * Subtracts from A, r and updates the flag based on result. 
 * Difference from Sub is that it doesnt update A 
*/
ExecStatus instrCompareReg(void);

/**
 * @brief CP (HL): (2 M-cycles)
 * Subtract from A, data at the addr stored at HL, updates flag
 * Difference from SUB HL is that it does not update A
*/
ExecStatus instrCompareHL(void);

/**
 * @brief CP n (2 M-cycles)
 * Subtracts from the 8 bit A register, the immediate data n, and updates flags based on the result.
 * Difference from SUB n, is that it does not update the A reg
*/
ExecStatus instrCompare8BitImm(void);

/// @brief INC r, (1 M-cycle)
ExecStatus instrIncrementReg(void);

/// @brief DEC r, (1 M-cycle)
ExecStatus instrDecrementReg(void);

/// @brief INC (HL) (2 M-cycles)
ExecStatus instrIncrementHL(void);

/// @brief DEC (HL) (3 M-cycles)
ExecStatus instrDecrementHL(void);

/// @brief AND r : Bitwis AND (1 M-cycle)
ExecStatus instrAndReg(void);

/// @brief AND (HL) : Bitwise AND with HL, (2 M-cycles)
ExecStatus instrAndHL(void);

/// @brief AND n, Bitwise AND with imm, (2 M-cycles)
ExecStatus instrAnd8BitImm(void);

/// @brief Or r (1 M-cycle)
ExecStatus instrOrReg(void);

/// @brief OP HL (2 M-cycles)
ExecStatus instrOrHL(void);

/// @brief OP n8, (2 M-cycles)
ExecStatus instrOr8BitImm(void);

/// @brief XOr r (1 M-cycle)
ExecStatus instrXorReg(void);

/// @brief XOR HL (2 M-cycles)
ExecStatus instrXorHL(void);

/// @brief XOR n8, (2 M-cycles)
ExecStatus instrXor8BitImm(void);

/**
 * @brief CCF (1 M-cycle)
 * Flips the C flag, resets N and H. Z is preserved.
*/
ExecStatus instrCcf(void);

/**
 * @brief SCF (1 M-cycle)
 * Sets the C flag, resets N and H. Z is preserved.
*/
ExecStatus instrScf(void);

/**
 * @brief DAA (1 M-cycle)
 * Adjusts the Accumulator for BCD math.
*/
ExecStatus instrDaa(void);

/**
 * @brief CPL (1 M-cycle)
 * Flips all bits in A. Sets N and H.
*/
ExecStatus instrCpl(void);

/**
 * @brief INC rr (2 M-cycles)
 * Increments a 16-bit register. No flags affected.
*/
ExecStatus instrIncrementRegReg(void);

/**
 * @brief DEC rr (2 M-cycles)
 * Decrements a 16-bit register. No flags affected.
*/
ExecStatus instrDecrementRegReg(void);

/// @brief ADD HL, rr (2 M-cycles)
ExecStatus instrAddHlRegReg(void);

/// @brief ADD SP, e8 (4 M-cycles)
ExecStatus instrAddSpE8(void);

/**
 * @brief RLCA (1 M-cycle)
 * Rotates A left. Bit 7 moves to Bit 0 AND Carry.
 * Flags: Z=0, N=0, H=0, C=Bit 7
*/
ExecStatus instrRlca(void);

/**
 * @brief RRCA (1 M-cycle)
 * Rotates A right. Bit 0 moves to Bit 7 AND Carry.
 * Flags: Z=0, N=0, H=0, C=Bit 0
*/
ExecStatus instrRrca(void);

/**
 * @brief RLA (1 M-cycle)
 * Rotates A left through Carry. 
 * Old bit 7 moves to Carry. Old Carry moves to Bit 0.
 * Flags: Z=0, N=0, H=0, C=Bit 7
*/
ExecStatus instrRla(void);

/**
 * @brief RRA (1 M-cycle)
 * Rotates A right through Carry.
 * Old bit 0 moves to Carry. Old Carry moves to Bit 7.
 * Flags: Z=0, N=0, H=0, C=Bit 0
*/
ExecStatus instrRra(void);

/**
 * @brief CB: RLC r (2 M-cycles)
 * Circular Rotate Left: Bit 7 moves to Bit 0 AND Carry.
 * Flags: Z=Result, N=0, H=0, C=Bit 7
*/
ExecStatus instrCbRlcReg(void);

/**
 * @brief CB: RLC (HL) (4 M-cycles total)
 * Circular Rotate Left on memory address at HL.
 * Cycles: M1(CB), M2(0x06), M3(Read), M4(Write)
*/
ExecStatus instrCbRlcHL(void);

/**
 * @brief CB: RRC r (2 M-cycles)
 * Circular Rotate Right: Bit 0 moves to Bit 7 AND Carry.
 * Flags: Z=Result, N=0, H=0, C=Bit 0
*/
ExecStatus instrCbRrcReg(void);

/**
 * @brief CB: RRC (HL) (4 M-cycles total)
 * Circular Rotate Right on memory address at HL.
 * Cycles: M1(CB), M2(0x0E), M3(Read), M4(Write)
*/
ExecStatus instrCbRrcHL(void);

/**
 * @brief CB: RL r (2 M-cycles)
 * Rotate Left through Carry: 
 * Old Carry -> Bit 0, Bit 7 -> New Carry.
 * Flags: Z=Result, N=0, H=0, C=Bit 7
*/
ExecStatus instrCbRlReg(void);

/**
 * @brief CB: RL (HL) (4 M-cycles total)
 * Rotate Left through Carry on memory address at HL.
 * Old Carry -> Bit 0, Bit 7 -> New Carry.
 * Cycles: M1(CB), M2(0x16), M3(Read), M4(Write)
*/
ExecStatus instrCbRlHL(void);

/**
 * @brief CB: RR r (2 M-cycles)
 * Rotate Right through Carry: 
 * Old Carry -> Bit 7, Bit 0 -> New Carry.
 * Flags: Z=Result, N=0, H=0, C=Bit 0
*/
ExecStatus instrCbRrReg(void);

/**
 * @brief CB: RR (HL) (4 M-cycles total)
 * Rotate Right through Carry on memory address at HL.
 * Old Carry -> Bit 7, Bit 0 -> New Carry.
 * Cycles: M1(CB), M2(0x1E), M3(Read), M4(Write)
*/
ExecStatus instrCbRrHL(void);

/**
 * @brief CB: SLA r (2 M-cycles)
 * Shift Left Arithmetic: Bit 7 -> Carry, 0 -> Bit 0.
 * Flags: Z=Result, N=0, H=0, C=Bit 7
*/
ExecStatus instrCbSlaReg(void);

/**
 * @brief CB: SLA (HL) (4 M-cycles total)
 * Shift Left Arithmetic on memory address at HL.
 * Bit 7 -> Carry, 0 -> Bit 0.
 * Cycles: M1(CB), M2(0x26), M3(Read), M4(Write)
*/
ExecStatus instrCbSlaHL(void);

/**
 * @brief CB: SRA r (2 M-cycles)
 * Shift Right Arithmetic: Bit 0 -> Carry, Bit 7 remains unchanged.
 * Flags: Z=Result, N=0, H=0, C=Bit 0
*/
ExecStatus instrCbSraReg(void);

/**
 * @brief CB: SRA (HL) (4 M-cycles total)
 * Shift Right Arithmetic on memory address at HL.
 * Bit 0 -> Carry, Bit 7 remains unchanged.
 * Cycles: M1(CB), M2(0x2E), M3(Read), M4(Write)
*/
ExecStatus instrCbSraHL(void);

/**
 * @brief CB: SWAP r (2 M-cycles)
 * Swaps upper and lower 4 bits.
 * Flags: Z=Result, N=0, H=0, C=0
*/
ExecStatus instrCbSwapReg(void);

/**
 * @brief CB: SWAP (HL) (4 M-cycles total)
 * Swaps nibbles of memory address at HL.
 * Cycles: M1(CB), M2(0x36), M3(Read), M4(Write)
*/
ExecStatus instrCbSwapHL(void);

/**
 * @brief CB: SRL r (2 M-cycles)
 * Shift Right Logical: Bit 0 -> Carry, 0 -> Bit 7.
 * Flags: Z=Result, N=0, H=0, C=Bit 0
*/
ExecStatus instrCbSrlReg(void);

/**
 * @brief CB: SRL (HL) (4 M-cycles total)
 * Shift Right Logical on memory address at HL.
 * Bit 0 -> Carry, 0 -> Bit 7.
 * Cycles: M1(CB), M2(0x3E), M3(Read), M4(Write)
*/
ExecStatus instrCbSrlHL(void);

/**
 * @brief CB: BIT b, r (2 M-cycles)
 * Tests bit 'param' of register 'reg1'.
 * Flags: Z=!(bit), N=0, H=1, C=Keep
*/
ExecStatus instrCbBitReg(void);

/**
 * @brief CB: BIT b, (HL) (3 M-cycles total)
 * Tests bit 'param' of memory at HL.
 * Cycles: M1(CB), M2(Opcode), M3(Read)
*/
ExecStatus instrCbBitHL(void);

/**
 * @brief CB: RES b, r (2 M-cycles)
 * Resets bit 'param' of register 'reg1'.
 * Flags: Unaffected.
*/
ExecStatus instrCbResReg(void);

/**
 * @brief CB: RES b, (HL) (4 M-cycles total)
 * Resets bit 'param' of memory at HL.
 * Cycles: M1(CB), M2(Opcode), M3(Read), M4(Write)
*/
ExecStatus instrCbResHL(void);

/**
 * @brief CB: SET b, r (2 M-cycles)
 * Sets bit 'param' of register 'reg1'.
 * Flags: Unaffected.
*/
ExecStatus instrCbSetReg(void);

/**
 * @brief CB: SET b, (HL) (4 M-cycles total)
 * Sets bit 'param' of memory at HL.
 * Cycles: M1(CB), M2(Opcode), M3(Read), M4(Write)
*/
ExecStatus instrCbSetHL(void);

/**
 * @brief JP nn (4 M-cycles)
 * Unconditional jump to 16-bit immediate address.
 * Cycles: M1(Fetch), M2(Read LSB), M3(Read MSB), M4(Internal/Idle)
*/
ExecStatus instrJump16BitImm(void);

/**
 * @brief JP HL (1 M-cycle)
 * Unconditional jump to the address contained in HL.
 * Note: PC becomes HL, and the next fetch happens from there.
*/
ExecStatus instrJumpHL(void);

/**
 * @brief JP cc, nn (3 or 4 M-cycles)
 * Conditional jump to 16-bit immediate address.
 * Duration: 3 M-cycles if cc=false, 4 M-cycles if cc=true.
*/
ExecStatus instrJumpConditional16BitImm(void);

/**
 * @brief JR e (3 M-cycles)
 * Unconditional relative jump using a signed 8-bit offset.
 * Cycles: M1(Fetch), M2(Read offset), M3(Internal Addition)
*/
ExecStatus instrJumpRelSigned8BitImm(void);

/**
 * @brief JR cc, e (2 or 3 M-cycles)
 * Conditional relative jump using a signed 8-bit offset.
 * Duration: 2 M-cycles if cc=false, 3 M-cycles if cc=true.
*/
ExecStatus instrJumpRelConditionalSigned8BitImm(void);

/**
 * @brief CALL nn (6 M-cycles)
 * Pushes the address of the next instruction onto the stack and jumps to nn.
 * Cycles: M1(Fetch), M2(Read LSB), M3(Read MSB), M4(Internal), M5(Push PCH), M6(Push PCL)
*/
ExecStatus instrCall16BitImm(void);

/**
 * @brief CALL cc, nn (3 or 6 M-cycles)
 * Conditional call to nn.
 * Duration: 3 M-cycles if cc=false, 6 M-cycles if cc=true.
*/
ExecStatus instrCallConditional16BitImm(void);

/**
 * @brief RET (4 M-cycles)
 * Unconditional return from subroutine.
 * Pops 16-bit address from stack into PC.
 * Cycles: M1(Fetch), M2(Pop PCL), M3(Pop PCH), M4(Internal/Update PC)
*/
ExecStatus instrReturn(void);

/**
 * @brief RET cc (2 or 5 M-cycles)
 * Conditional return from subroutine.
 * Duration: 2 M-cycles if cc=false, 5 M-cycles if cc=true.
*/
ExecStatus instrReturnConditional(void);

/**
 * @brief RETI (4 M-cycles)
 * Return from interrupt handler.
 * Pops 16-bit address into PC and sets IME = 1.
 * Cycles: M1(Fetch), M2(Pop PCL), M3(Pop PCH), M4(Internal/Update PC + IME)
*/
ExecStatus instrReturnInterrupt(void);

/**
 * @brief RST n (4 M-cycles)
 * Pushes PC onto stack and jumps to a fixed address n.
 * Cycles: M1(Fetch), M2(SP-1), M3(Push PCH), M4(Push PCL + Jump)
*/
ExecStatus instrRestart(void);

/**
 * @brief DI (1 M-cycle)
 * Disables interrupts immediately.
*/
ExecStatus instrDisableInterrupt(void);

/**
 * @brief EI (1 M-cycle)
 * Schedules interrupts to be enabled after the NEXT instruction.
*/
ExecStatus instrEnableInterrupt(void);

/**
 * @brief NOP (1 M-cycle)
 * No operation.
*/
ExecStatus instrNop(void);

/**
 * @brief STOP (1 M-cycle)
 * Enters very low power mode or prepares CGB speed switch.
*/
ExecStatus instrStop(void);

/**
 * @brief HALT (1 M-cycle)
 * Puts the CPU into a low-power state until an interrupt occurs.
 * Implements the HALT bug if interrupts are disabled but one is pending.
*/
ExecStatus instrHalt(void);
