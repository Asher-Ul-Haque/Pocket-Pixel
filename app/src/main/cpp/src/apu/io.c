#include "apu/channels/noise.h"
#include <apu/channels/wave.h>
#include <apu/internal.h>
#include <apu/apu.h>
#include <bus.h>

void apuWrite(u16 ADDR, u8 VALUE)
{
  ApuContext* ctx = apuGetContext();

  // - - - Wave Ram block 
  if (ADDR >= REG_WAVE_RAM_START && ADDR <= REG_WAVE_RAM_END) 
  {
    waveWriteRam(&ctx->channel3, ADDR, VALUE);
    return;
  }
  
  // - - - Block the rest if APU is off 
  if (!ctx->audioEnabled && ADDR != REG_NR52) return;

  switch (ADDR)
  {
    case REG_NR41: 
        ctx->channel4.lengthTimer = CH_NOISE_LENGTH_MAX - (VALUE & NR41_LENGTH_MASK);
        break;
        
    case REG_NR42: 
      ctx->channel4.initialVolume    = (VALUE & NR22_VOL_MASK) >> NR22_VOL_SHIFT;
      ctx->channel4.envelopeIncrease = (VALUE & NR22_ENV_DIR_MASK) != 0;
      ctx->channel4.envelopePace     = VALUE & NR22_ENV_PACE_MASK;
      ctx->channel4.dacEnabled       = (VALUE & NR22_DAC_ENABLE_MASK) != 0;
      if (!ctx->channel4.dacEnabled) ctx->channel4.enabled = false;
      break;
        
    case REG_NR43: 
      ctx->channel4.clockShift   = (VALUE & NR43_CLOCK_SHIFT_MASK) >> NR43_CLOCK_SHIFT_OFFSET;
      ctx->channel4.shortMode    = (VALUE & NR43_LFSR_WIDTH_MASK) != 0;
      ctx->channel4.clockDivider = VALUE & NR43_CLOCK_DIV_MASK;
      break;
        
    case REG_NR44: 
      ctx->channel4.lengthEnabled = (VALUE & NR44_LEN_ENABLE_MASK) != 0;
      if (VALUE & NR44_TRIGGER_MASK) noiseTrigger(&ctx->channel4);
      break;

    case REG_NR30: 
      ctx->channel3.dacEnabled = (VALUE & NR30_DAC_ENABLE_MASK) != 0;
      if (!ctx->channel3.dacEnabled) ctx->channel3.enabled = false;
      break;
        
    case REG_NR31: 
      ctx->channel3.lengthTimer = CH_WAVE_LENGTH_MAX - (VALUE & NR31_LENGTH_MASK);
      break;
        
    case REG_NR32: 
      ctx->channel3.volumeCode = (VALUE & NR32_VOL_MASK) >> NR32_VOL_SHIFT;
      break;
        
    case REG_NR33: 
      ctx->channel3.periodValue = (ctx->channel3.periodValue & ~NR23_PERIOD_LOW_MASK) | VALUE;
      break;
        
    case REG_NR34: 
      ctx->channel3.periodValue   = (ctx->channel3.periodValue & NR23_PERIOD_LOW_MASK) | 
                              ((VALUE & NR24_PERIOD_HIGH_MASK) << NR24_PERIOD_HIGH_SHIFT);
      ctx->channel3.lengthEnabled = (VALUE & NR34_LEN_ENABLE_MASK) != 0;
      if (VALUE & NR34_TRIGGER_MASK) waveTrigger(&ctx->channel3);
      break;

    case REG_NR10:
      ctx->channel1.sweepPace     = (VALUE & NR10_PACE_MASK) >> NR10_PACE_SHIFT;
      ctx->channel1.sweepDecrease = (VALUE & NR10_DIR_MASK) != 0;
      ctx->channel1.sweepShift    = VALUE & NR10_SHIFT_MASK;
      break;

    case REG_NR11:
      ctx->channel1.dutyPattern = (VALUE & NR21_DUTY_MASK) >> NR21_DUTY_SHIFT;
      ctx->channel1.lengthTimer = CH_PULSE_LENGTH_MAX - (VALUE & NR21_LENGTH_MASK);
      break;

    case REG_NR12: 
      ctx->channel1.initialVolume    = (VALUE & NR22_VOL_MASK) >> NR22_VOL_SHIFT;
      ctx->channel1.envelopeIncrease = (VALUE & NR22_ENV_DIR_MASK) != 0;
      ctx->channel1.envelopePace     = VALUE & NR22_ENV_PACE_MASK;
      ctx->channel1.dacEnabled       = (VALUE & NR22_DAC_ENABLE_MASK) != 0;
      if (!ctx->channel1.dacEnabled) ctx->channel1.enabled = false;
      break;

    case REG_NR13: 
      ctx->channel1.periodValue = (ctx->channel1.periodValue & ~NR23_PERIOD_LOW_MASK) | VALUE;
      break;

    case REG_NR14: 
      ctx->channel1.periodValue   = (ctx->channel1.periodValue & NR23_PERIOD_LOW_MASK) | 
                              ((VALUE & NR24_PERIOD_HIGH_MASK) << NR24_PERIOD_HIGH_SHIFT);
      ctx->channel1.lengthEnabled = (VALUE & NR24_LEN_ENABLE_MASK) != 0;
      if (VALUE & NR24_TRIGGER_MASK) pulseTrigger(&ctx->channel1);
      break;

    case REG_NR21:
      ctx->channel2.dutyPattern = (VALUE & NR21_DUTY_MASK) >> NR21_DUTY_SHIFT;
      ctx->channel2.lengthTimer = CH_PULSE_LENGTH_MAX - (VALUE & NR21_LENGTH_MASK);
      break;

    case REG_NR22:
      ctx->channel2.initialVolume    = (VALUE & NR22_VOL_MASK) >> NR22_VOL_SHIFT;
      ctx->channel2.envelopeIncrease = (VALUE & NR22_ENV_DIR_MASK) != 0;
      ctx->channel2.envelopePace     = VALUE & NR22_ENV_PACE_MASK;
      ctx->channel2.dacEnabled       = (VALUE & NR22_DAC_ENABLE_MASK) != 0;
      if (!ctx->channel2.dacEnabled) ctx->channel2.enabled = false;
      break;

    case REG_NR23:
      ctx->channel2.periodValue = (ctx->channel2.periodValue & ~NR23_PERIOD_LOW_MASK) | VALUE;
      break;

    case REG_NR24:
      ctx->channel2.periodValue   = (ctx->channel2.periodValue & NR23_PERIOD_LOW_MASK) |
                              ((VALUE & NR24_PERIOD_HIGH_MASK) << NR24_PERIOD_HIGH_SHIFT);
      ctx->channel2.lengthEnabled = (VALUE & NR24_LEN_ENABLE_MASK) != 0;
      if (VALUE & NR24_TRIGGER_MASK) pulseTrigger(&ctx->channel2);
      break;


    // - - - Global Mixer - - -

    case REG_NR50:
      ctx->masterVolumeLeft  = (VALUE & NR50_VOL_LEFT_MASK) >> NR50_VOL_LEFT_SHIFT;
      ctx->masterVolumeRight = VALUE & NR50_VOL_RIGHT_MASK;
      break;

    case REG_NR51:
      ctx->panningMap = VALUE;
      break;

    case REG_NR52:
      ctx->audioEnabled = (VALUE & NR52_AUDIO_ENABLE_MASK) != 0;
      if (!ctx->audioEnabled) 
      {
        memset(&ctx->channel1, 0, sizeof(PulseChannel));
        memset(&ctx->channel2, 0, sizeof(PulseChannel));
        memset(&ctx->channel3, 0, sizeof(WaveChannel));
        memset(&ctx->channel4, 0, sizeof(NoiseChannel));
        ctx->panningMap                 = 0;
        ctx->masterVolumeLeft           = 0;
        ctx->masterVolumeRight          = 0;
        ctx->channel1.hasSweepHardware  = true;
      }
      break;
  }
}

u8 apuRead(u16 ADDR)
{
  ApuContext* ctx = apuGetContext();

  if (ADDR >= REG_WAVE_RAM_START && ADDR <= REG_WAVE_RAM_END) 
  {
    return waveReadRam(&ctx->channel3, ADDR);
  }

  if (ADDR == REG_NR52)
  {
    u8 val = ctx->audioEnabled ? NR52_AUDIO_ENABLE_MASK : 0x00;

    if (ctx->channel1.enabled) val |= NR52_CH1_ACTIVE_MASK;
    if (ctx->channel2.enabled) val |= NR52_CH2_ACTIVE_MASK;
    if (ctx->channel3.enabled) val |= NR52_CH3_ACTIVE_MASK;
    if (ctx->channel4.enabled) val |= NR52_CH4_ACTIVE_MASK; 

    return val | NR52_UNUSED_BITS_MASK;
  }
  return OPEN_BUS_VALUE;
}
