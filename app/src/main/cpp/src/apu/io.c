#include <apu/apu.h>
#include <bus.h>

void apuWrite(u16 ADDR, u8 VALUE)
{
  ApuContext* ctx = apuGetContext();
  if (!ctx->audioEnabled && ADDR != REG_NR52) return;

  switch (ADDR)
  {
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
        memset(&ctx->channel2, 0, sizeof(PulseChannel));
        ctx->panningMap = 0;
      }
      break;
  }
}

u8 apuRead(u16 ADDR)
{
  ApuContext* ctx = apuGetContext();
  if (ADDR == REG_NR52)
  {
    u8 val = ctx->audioEnabled ? NR52_AUDIO_ENABLE_MASK : 0x00;
    if (ctx->channel2.enabled) val |= NR52_CH2_ACTIVE_MASK;
    return val | NR52_UNUSED_BITS_MASK;
  }
  return OPEN_BUS_VALUE;
}
