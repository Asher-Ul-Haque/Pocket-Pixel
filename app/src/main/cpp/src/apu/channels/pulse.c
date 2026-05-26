#include <apu/channels/pulse.h>
#include <apu/internal.h>

static const u8 DUTY_CYCLES[DUTY_PATTERN_COUNT] =
  {
  DUTY_PATTERN_0, ///< 12.5%
  DUTY_PATTERN_1, ///< 25%
  DUTY_PATTERN_2, ///< 50%
  DUTY_PATTERN_3  ///< 75%
};

// - - - Calculates the next frequency and performs the overflow check 
static u16 calculateSweepFrequency(PulseChannel* CHANNEL)
{
  u16 offset = CHANNEL->sweepShadow >> CHANNEL->sweepShift;
  u16 newFreq = CHANNEL->sweepDecrease ? (CHANNEL->sweepShadow - offset) : (CHANNEL->sweepShadow + offset);

  // - - - If the calculation overflows ? 2047, the channel is insta disbaled 
  if (newFreq > APU_PERIOD_MAX_VALUE - 1) CHANNEL->enabled = false;

  return newFreq;
}

void pulseTrigger(PulseChannel* CHANNEL)
{
  CHANNEL->enabled = true;

  // - - - If length timer is 0, it resets to maximum
  if (CHANNEL->lengthTimer == 0) { CHANNEL->lengthTimer = CH_PULSE_LENGTH_MAX; }

  CHANNEL->periodTimer   = APU_PERIOD_MAX_VALUE - CHANNEL->periodValue;
  CHANNEL->envelopeTimer = CHANNEL->envelopePace;
  CHANNEL->currentVolume = CHANNEL->initialVolume;

  // - - - If DAC is off, the channel instantly disables itself
  if (!CHANNEL->dacEnabled) { CHANNEL->enabled = false; }
  
  // - - - Setup Hardware Sweeep (CH1 Only) - - - 
  if (CHANNEL->hasSweepHardware)
  {
    CHANNEL->sweepShadow  = CHANNEL->periodValue;
    CHANNEL->sweepTimer   = (CHANNEL->sweepPace > 0) ? CHANNEL->sweepPace : 8;
    CHANNEL->sweepEnabled = (CHANNEL->sweepPace > 0) || (CHANNEL->sweepShift > 0);

    // - - - Overflow check is performed immediately on trigger if shift > 0 
    if (CHANNEL->sweepShift > 0) calculateSweepFrequency(CHANNEL);
  }
}

void pulseClockSweep(PulseChannel* CHANNEL)
{
  if (!CHANNEL->hasSweepHardware) return;
  if (CHANNEL->sweepTimer > 0) CHANNEL->sweepTimer--;
  if (CHANNEL->sweepTimer == 0)
  {
    CHANNEL->sweepTimer = (CHANNEL->sweepPace > 0) ? CHANNEL->sweepPace : 8;
    if (CHANNEL->sweepEnabled && CHANNEL->sweepPace > 0)
    {
      u16 newFreq = calculateSweepFrequency(CHANNEL);
      if (newFreq <= APU_PERIOD_MAX_VALUE - 1 && CHANNEL->sweepShift > 0)
      {
        CHANNEL->periodValue = newFreq;
        CHANNEL->sweepShadow = newFreq;

        // - - - overflow check is run a second time immediately, but not written back 
        calculateSweepFrequency(CHANNEL);
      }
    }
  }
}

void pulseClockLength(PulseChannel* CHANNEL)
{
  if (CHANNEL->lengthEnabled && CHANNEL->lengthTimer > 0) 
  {
    CHANNEL->lengthTimer--;
    if (CHANNEL->lengthTimer == 0) CHANNEL->enabled = false;
  }
}

void pulseClockEnvelope(PulseChannel* CHANNEL)
{
  if (CHANNEL->envelopePace != 0)
  {
    if (CHANNEL->envelopeTimer > 0) CHANNEL->envelopeTimer--;

    if (CHANNEL->envelopeTimer == 0)
    {
      CHANNEL->envelopeTimer = CHANNEL->envelopePace;

      if (CHANNEL->envelopeIncrease && CHANNEL->currentVolume < CH_MAX_VOLUME) 
      { CHANNEL->currentVolume++; }
      else if (!CHANNEL->envelopeIncrease && CHANNEL->currentVolume > 0) 
      { CHANNEL->currentVolume--; }
    }
  }
}

void pulseStepTimer(PulseChannel* CHANNEL)
{
  // - - - Default to emitting a digital 0
  CHANNEL->outputVolume = 0;

  if (!CHANNEL->enabled || !CHANNEL->dacEnabled) return;

  CHANNEL->periodTimer--;
  if (CHANNEL->periodTimer == 0)
  {
    CHANNEL->periodTimer  = APU_PERIOD_MAX_VALUE - CHANNEL->periodValue;
    CHANNEL->dutyPosition = (CHANNEL->dutyPosition + 1) % DUTY_CYCLE_STEPS;
  }

  // - - - Extract the active bit from the duty cycle pattern
  u8 bit = (DUTY_CYCLES[CHANNEL->dutyPattern] >> (7 - CHANNEL->dutyPosition)) & 1;
  if (bit) CHANNEL->outputVolume = CHANNEL->currentVolume;
}
