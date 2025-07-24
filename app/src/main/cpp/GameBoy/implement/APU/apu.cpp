#include "../../include/apu.h"
#include "../../../GameBoyCore.h"

static APUcontext apuCtx;

APUcontext* apuGetContext()
{ return &apuCtx; }

void apuInit()
{
  memset(&apuCtx, 0, sizeof(apuCtx));

  apuCtx.bufferPtr              = 0;
  apuCtx.sampleCounter          = 0;
  apuCtx.frameSequencerCounter  = 0;
  apuCtx.frameSequencerStep     = 0;

  channelPulseInit(&apuCtx.channel1);
  channelPulseInit(&apuCtx.channel2);
}

void apuUpdate(i32 CYCLES)
{
  apuCtx.sampleCounter          -= CYCLES;
  apuCtx.frameSequencerCounter  -= CYCLES;
  
  channelPulseTickSampleGenerator(&apuCtx.channel1, CYCLES);
  channelPulseTickSampleGenerator(&apuCtx.channel2, CYCLES);

  if (apuCtx.frameSequencerCounter <= 0)
  {
    apuCtx.frameSequencerCounter += 8192;

    if ((apuCtx.frameSequencerStep & 1) == 0)
    {
      channelPulseTickLength(&apuCtx.channel1);
      channelPulseTickLength(&apuCtx.channel2);
    }
    if (apuCtx.frameSequencerStep == 2 || apuCtx.frameSequencerStep == 6)
    {
      channelPulseTickSweep(&apuCtx.channel1);
    }
    if (apuCtx.frameSequencerStep == 7)
    {
      channelPulseTickEnvelope(&apuCtx.channel1);
      channelPulseTickEnvelope(&apuCtx.channel2);
    }
    apuCtx.frameSequencerStep = (apuCtx.frameSequencerStep + 1) & 7;
  }

  if (apuCtx.sampleCounter <= 0)
  {
    apuCtx.sampleCounter += 95;
    if (!apuCtx.isEnabled) return;

    i32 ch1L = apuCtx.channel1Left  ? apuCtx.channel1.sample : 0;
    i32 ch1R = apuCtx.channel1Right ? apuCtx.channel1.sample : 0;
    i32 ch2L = apuCtx.channel2Left  ? apuCtx.channel2.sample : 0;
    i32 ch2R = apuCtx.channel2Right ? apuCtx.channel2.sample : 0;

    u8 mixedL = (u8) (ch1L + ch2L + 128);
    u8 mixedR = (u8) (ch1R + ch2R + 128);

    apuCtx.sampleBuffer[apuCtx.bufferPtr++] = mixedL;
    apuCtx.sampleBuffer[apuCtx.bufferPtr++] = mixedR;

    if (apuCtx.bufferPtr >= APU_BUFFER_SIZE)
    {
      playAudio(); 
      apuCtx.bufferPtr = 0;
    }
  }
}

void apuWrite(u16 ADDRESS, u8 VALUE)
{
  ADDRESS -= 0xFF00;
  if (!apuCtx.isEnabled && ADDRESS < 0x26) return;

  switch (ADDRESS)
  {
    // - - - Channel 1 
    case 0x10 : { channelPulseSweep(&apuCtx.channel1, VALUE);                       break; }
    case 0x11 : { channelPulseSetNRx1LengthTimerDutyCycle(&apuCtx.channel1, VALUE); break; }
    case 0x12 : { channelPulseSetNRx2EnvelopeVolume(&apuCtx.channel1, VALUE);       break; }
    case 0x13 : { channelPulseSetNRx3PeriodLow(&apuCtx.channel1, VALUE);            break; }
    case 0x14 : { channelPulseSetNRx4PeriodHiControl(&apuCtx.channel1, VALUE);      break; }
    
    // - - - Channel 2
    case 0x16 : { channelPulseSetNRx1LengthTimerDutyCycle(&apuCtx.channel2, VALUE); break; }
    case 0x17 : { channelPulseSetNRx2EnvelopeVolume(&apuCtx.channel2, VALUE);       break; }
    case 0x18 : { channelPulseSetNRx3PeriodLow(&apuCtx.channel2, VALUE);            break; }
    case 0x19 : { channelPulseSetNRx4PeriodHiControl(&apuCtx.channel2, VALUE);      break; }

    case 0x24 : 
      {
        apuCtx.NR50                 = VALUE;
        apuCtx.vinRight             = (VALUE >> 3) & 1;
        apuCtx.vinLeft              = (VALUE >> 7) & 1;
        apuCtx.masterVolumeRight    = VALUE & 7;
        apuCtx.masterVolumeLeft     = (VALUE >> 4) & 7;
        break;
      }
    case 0x25 :
      {
        apuCtx.NR51          = VALUE;
        apuCtx.channel1Left  = (VALUE >> 4) & 1;
        apuCtx.channel1Left  = (VALUE >> 5) & 1;
        apuCtx.channel1Right = VALUE & 1;
        apuCtx.channel2Right = (VALUE >> 1) & 1;
        break;
      }
    case 0x26 : 
      {
        apuCtx.NR52         = (VALUE | 0x70) & ~0x0F;
        bool was            = apuCtx.isEnabled;
        apuCtx.isEnabled    = (VALUE & 0x80) != 0;

        if (was && !apuCtx.isEnabled) apuInit();
        break;
      }
    default : break;
  }
}

u8 apuRead(u16 ADDRESS)
{
  ADDRESS -= 0xFF00;
  switch (ADDRESS)
  {
    // - - - channel 1
    case 0x10 : return apuCtx.channel1.NR10;
    case 0x11 : return apuCtx.channel1.NRX1;
    case 0x12 : return apuCtx.channel1.NRX2;
    case 0x13 : return 0xFF;
    case 0x14 : return apuCtx.channel1.NRX4;

    // - - - channel 2
    case 0x16 : return apuCtx.channel2.NRX1;
    case 0x17 : return apuCtx.channel2.NRX2;
    case 0x18 : return 0xFF;
    case 0x19 : return apuCtx.channel2.NRX4;

    // - - - master 
    case 0x24 : return apuCtx.NR50;
    case 0x25 : return apuCtx.NR51;
    case 0x26 : 
      {
        u8 ch = 0;
        ch |= apuCtx.channel1.isEnabled ? 0x1 : 0;
        return apuCtx.NR52 | ch;
      }

    default : return 0xFF;
  }
}
