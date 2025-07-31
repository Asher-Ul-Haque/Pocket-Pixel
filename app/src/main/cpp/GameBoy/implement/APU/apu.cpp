#include "../../include/apu.h"
#include "../../../GameBoyCore.h"

static APUcontext apuCtx;

APUcontext* apuGetContext()
{ return &apuCtx; }

void apuInit(f32* VOLUMES)
{
  memset(&apuCtx, 0, sizeof(apuCtx));

  apuCtx.bufferPtr              = 0;
  apuCtx.sampleCounter          = 0;
  apuCtx.frameSequencerCounter  = 0;
  apuCtx.frameSequencerStep     = 0;
  for (int i = 0; i < 5; ++i) apuCtx.volumes[i] = VOLUMES[i];

  channelPulseInit(&apuCtx.channel1);
  channelPulseInit(&apuCtx.channel2);
  channelWaveInit (&apuCtx.channel3);
  channelNoiseInit(&apuCtx.channel4);
}

void apuSetVolume(f32* VOLUMES)
{
  for (int i = 0; i < 5; ++i) apuCtx.volumes[i] = VOLUMES[i];
}

void apuUpdate(i32 CYCLES)
{
  apuCtx.sampleCounter          -= CYCLES;
  apuCtx.frameSequencerCounter  -= CYCLES;
  
  channelPulseTickSampleGenerator(&apuCtx.channel1, CYCLES);
  channelPulseTickSampleGenerator(&apuCtx.channel2, CYCLES);
  channelWaveTickSampleGenerator (&apuCtx.channel3, CYCLES);
  channelNoiseTickSampleGenerator(&apuCtx.channel4, CYCLES);

  if (apuCtx.frameSequencerCounter <= 0)
  {
    apuCtx.frameSequencerCounter += 8192;

    if ((apuCtx.frameSequencerStep & 1) == 0)
    {
      channelPulseTickLength(&apuCtx.channel1);
      channelPulseTickLength(&apuCtx.channel2);
      channelWaveTickLength (&apuCtx.channel3);
      channelNoiseTickLength(&apuCtx.channel4);
    }
    if (apuCtx.frameSequencerStep == 2 || apuCtx.frameSequencerStep == 6)
    {
      channelPulseTickSweep(&apuCtx.channel1);
    }
    if (apuCtx.frameSequencerStep == 7)
    {
      channelPulseTickEnvelope(&apuCtx.channel1);
      channelPulseTickEnvelope(&apuCtx.channel2);
      channelNoiseTickEnvelope(&apuCtx.channel4);
    }
    apuCtx.frameSequencerStep = (apuCtx.frameSequencerStep + 1) & 7;
  }

  if (apuCtx.sampleCounter <= 0)
  {
    apuCtx.sampleCounter += 95;
    if (!apuCtx.isEnabled) return;

    i32 ch1L = apuCtx.channel1Left  ? apuCtx.channel1.sample * apuCtx.volumes[1] : 0;
    i32 ch1R = apuCtx.channel1Right ? apuCtx.channel1.sample * apuCtx.volumes[1] : 0;
    i32 ch2L = apuCtx.channel2Left  ? apuCtx.channel2.sample * apuCtx.volumes[2] : 0;
    i32 ch2R = apuCtx.channel2Right ? apuCtx.channel2.sample * apuCtx.volumes[2] : 0;
    i32 ch3L = apuCtx.channel3Left  ? apuCtx.channel3.sample * apuCtx.volumes[3] : 0;
    i32 ch3R = apuCtx.channel3Right ? apuCtx.channel3.sample * apuCtx.volumes[3] : 0;
    i32 ch4L = apuCtx.channel4Left  ? apuCtx.channel4.sample * apuCtx.volumes[4] : 0;
    i32 ch4R = apuCtx.channel4Right ? apuCtx.channel4.sample * apuCtx.volumes[4]: 0;

    i32 left  = ch1L + ch2L + ch3L + ch4L;
    i32 right = ch1R + ch2R + ch3R + ch4R;

    left  = (left * apuCtx.masterVolumeLeft)   >> 3;
    right = (right * apuCtx.masterVolumeRight) >> 3;

    left  = (i32)(left  * apuCtx.volumes[0]); 
    right = (i32)(right * apuCtx.volumes[0]);

    left  = left < -128 ? -128 : (left > 127 ? 127 : left);
    right = right < -128 ? -128 : (right > 127 ? 127 : right);

    u8 mixedL = (u8)(left  + 128);
    u8 mixedR = (u8)(right + 128);

    if (apuCtx.bufferPtr + 2 <= APU_BUFFER_SIZE) 
    {
      apuCtx.sampleBuffer[apuCtx.bufferPtr++] = mixedL;
      apuCtx.sampleBuffer[apuCtx.bufferPtr++] = mixedR;
    }

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

    // - - - Channel 3 
    case 0x1A : { channelWaveSetNR30(&apuCtx.channel3, VALUE); break;}
    case 0x1B : { channelWaveSetNR31(&apuCtx.channel3, VALUE); break;}
    case 0x1C : { channelWaveSetNR32(&apuCtx.channel3, VALUE); break;}
    case 0x1D : { channelWaveSetNRX3(&apuCtx.channel3, VALUE); break;}
    case 0x1E : { channelWaveSetNRX4(&apuCtx.channel3, VALUE); break;}
    case 0x30 ... 0x3F :
      { 
        apuCtx.channel3.wavePatternRAM[ADDRESS - 0x30] = VALUE; 
        break;
      }

    // - - - Channel 4 
    case 0x20 : { channelNoiseSetNR41(&apuCtx.channel4, VALUE); break; }
    case 0x21 : { channelNoiseSetNR42(&apuCtx.channel4, VALUE); break; }
    case 0x22 : { channelNoiseSetNR43(&apuCtx.channel4, VALUE); break; }
    case 0x23 : { channelNoiseSetNR44(&apuCtx.channel4, VALUE); break; }

    // - - - Master 
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
        apuCtx.NR51           = VALUE;
        apuCtx.channel1Right  = (VALUE >> 0) & 1;
        apuCtx.channel2Right  = (VALUE >> 1) & 1;
        apuCtx.channel3Right  = (VALUE >> 2) & 1;
        apuCtx.channel4Right  = (VALUE >> 3) & 1;
        apuCtx.channel1Left   = (VALUE >> 4) & 1;
        apuCtx.channel2Left   = (VALUE >> 5) & 1;
        apuCtx.channel3Left   = (VALUE >> 6) & 1;
        apuCtx.channel4Left   = (VALUE >> 7) & 1;
        break;
      }
    case 0x26 : 
      {
        bool was          = apuCtx.isEnabled;
        apuCtx.isEnabled  = (VALUE & 0x80) != 0;

        if (!apuCtx.isEnabled && was) apuInit(apuCtx.volumes);

        apuCtx.NR52 = (apuCtx.NR52 & 0x0F) | 0x70;
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

    // - - - channel 3 (Wave)
    case 0x1A : return apuCtx.channel3.NR30;
    case 0x1B : return apuCtx.channel3.NR31;
    case 0x1C : return apuCtx.channel3.NR32;
    case 0x1D : return 0xFF;
    case 0x1E : return apuCtx.channel3.NRX4;
    case 0x30 ... 0x3F:
      return apuCtx.channel3.wavePatternRAM[ADDRESS - 0x30];

    // - - - master 
    case 0x24 : return apuCtx.NR50;
    case 0x25 : return apuCtx.NR51;
    case 0x26:
      {
        u8 status = 0;
        if (apuCtx.channel1.isEnabled) status |= 0x01;
        if (apuCtx.channel2.isEnabled) status |= 0x02;
        if (apuCtx.channel3.isEnabled) status |= 0x04;
        if (apuCtx.channel4.isEnabled) status |= 0x08;

        return (apuCtx.isEnabled ? 0x80 : 0x00) | 0x70 | (status & 0x0F);
      }

    default : return 0xFF;
  }
}
