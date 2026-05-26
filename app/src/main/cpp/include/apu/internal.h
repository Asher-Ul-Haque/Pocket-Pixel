#pragma once

#include <common.h>

// - - - Audio Output Specification - - -
#define AUDIO_SAMPLE_RATE          44100
#define AUDIO_BUFFER_SIZE          1024 
#define AUDIO_MASTER_OUTPUT_SCALE  0.2f  // Scale final output to prevent clipping

// - - - Hardware Clocks & Timing - - -
#define APU_CLOCK_SPEED            1048576u // 1.04 MHz (CPU M-Cycles)
#define APU_FRAME_SEQUENCER_RATE   512u     // 512 Hz
#define APU_CYCLES_PER_FRAME_SEQ   (APU_CLOCK_SPEED / APU_FRAME_SEQUENCER_RATE) // 2048 cycles
#define APU_PERIOD_MAX_VALUE       2048u

// - - - Frame Sequencer Steps - - -
#define FRAME_SEQ_MAX_STEPS        8u
#define FRAME_SEQ_ENV_STEP         7u       // Envelope ticks at 64 Hz (Step 7)
#define FRAME_SEQ_LEN_MASK         1u       // Length ticks at 256 Hz (Steps 0, 2, 4, 6)

// - - - Channel Hardware Limits - - -
#define CH_PULSE_LENGTH_MAX        64u
#define CH_WAVE_LENGTH_MAX         256u
#define CH_MAX_VOLUME              15u
#define DAC_NEUTRAL_POINT          7.5f
#define MIXER_MAX_VOLUME_STEPS     8.0f
#define DUTY_CYCLE_STEPS           8u

// - - - Duty Cycle Hardware Patterns (12.5%, 25%, 50%, 75%) - - -
#define DUTY_PATTERN_0             0x01
#define DUTY_PATTERN_1             0x81
#define DUTY_PATTERN_2             0x87
#define DUTY_PATTERN_3             0x7E
#define DUTY_PATTERN_COUNT         4 

// - - - Hardware Register Addresses - - -
#define REG_NR21                   0xFF16
#define REG_NR22                   0xFF17
#define REG_NR23                   0xFF18
#define REG_NR24                   0xFF19

#define REG_NR50                   0xFF24
#define REG_NR51                   0xFF25
#define REG_NR52                   0xFF26

#define APU_IO_START               0xFF10
#define APU_IO_END                 0xFF3F

// - - - NR21: Length Timer & Duty Cycle - - -
#define NR21_DUTY_MASK             0xC0
#define NR21_DUTY_SHIFT            6
#define NR21_LENGTH_MASK           0x3F

// - - - NR22: Volume & Envelope - - -
#define NR22_VOL_MASK              0xF0
#define NR22_VOL_SHIFT             4
#define NR22_ENV_DIR_MASK          0x08
#define NR22_ENV_PACE_MASK         0x07
#define NR22_DAC_ENABLE_MASK       0xF8

// - - - NR23 & NR24: Period & Control - - -
#define NR23_PERIOD_LOW_MASK       0xFF
#define NR24_PERIOD_HIGH_MASK      0x07
#define NR24_PERIOD_HIGH_SHIFT     8
#define NR24_LEN_ENABLE_MASK       0x40
#define NR24_TRIGGER_MASK          0x80

// - - - NR50: Master Volume - - -
#define NR50_VOL_LEFT_MASK         0x70
#define NR50_VOL_LEFT_SHIFT        4
#define NR50_VOL_RIGHT_MASK        0x07

// - - - NR51: Panning Flags - - -
#define NR51_CH2_LEFT_MASK         0x20
#define NR51_CH2_RIGHT_MASK        0x02

// - - - NR52: Audio Master Control - - -
#define NR52_AUDIO_ENABLE_MASK     0x80
#define NR52_CH2_ACTIVE_MASK       0x02
#define NR52_UNUSED_BITS_MASK      0x70

// - - - NR10: Channel 1 Sweep - - -
#define NR10_PACE_MASK             0x70
#define NR10_PACE_SHIFT            4
#define NR10_DIR_MASK              0x08
#define NR10_SHIFT_MASK            0x07

// - - - Channel 1 Registers - - -
#define REG_NR10                   0xFF10
#define REG_NR11                   0xFF11
#define REG_NR12                   0xFF12
#define REG_NR13                   0xFF13
#define REG_NR14                   0xFF14

// - - - Panning Flags (Update for CH1) - - -
#define NR51_CH1_LEFT_MASK         0x10
#define NR51_CH1_RIGHT_MASK        0x01

#define NR52_CH1_ACTIVE_MASK       0x01
