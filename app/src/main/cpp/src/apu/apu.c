#include "apu/channels/noise.h"
#include "apu/channels/pulse.h"
#include "apu/channels/wave.h"
#include "apu/internal.h"
#include <platform.h>
#include <apu/apu.h>

static ApuContext ctx;

ApuContext* apuGetContext(void)
{ return &ctx; }

void apuInit(void)
{ 
  memset(&ctx, 0, sizeof(ApuContext)); 

  ctx.channel1.hasSweepHardware = true;
  ctx.channel2.hasSweepHardware = false;

  ctx.speedMultiplier = 1.0f;
}

void apuSetSpeed(f32 MULTIPLIER)
{
  if (MULTIPLIER < 0.01f) MULTIPLIER = 0.01f;
  ctx.speedMultiplier = MULTIPLIER;
}

void apuTick(void) 
{
  PlatformContext* platform = platformGetContext();
  if (!platform || !platform->audio.pushSamples) return;

  if (!ctx.audioEnabled) return;

  // - - - 1. Advance the 512 Hz Frame Sequencer
  ctx.frameSequencerTimer++;
  if (ctx.frameSequencerTimer >= APU_CYCLES_PER_FRAME_SEQ) 
  {
    ctx.frameSequencerTimer = 0;

    // - - - Step 2 and 6: Clock frequency Sweep (128 Hz)
    if (ctx.frameSequencerStep == 2 || ctx.frameSequencerStep == 6)
    {
      pulseClockSweep(&ctx.channel1);
    }

    // - - - Step 0, 2, 4, 6: Clock Length Timers
    if ((ctx.frameSequencerStep & FRAME_SEQ_LEN_MASK) == 0) 
    {
      pulseClockLength(&ctx.channel1);
      pulseClockLength(&ctx.channel2);
      waveClockLength(&ctx.channel3);
      noiseClockLength(&ctx.channel4);
    }

    // - - - Step 7: Clock Volume Envelopes
    if (ctx.frameSequencerStep == FRAME_SEQ_ENV_STEP) 
    {
      pulseClockEnvelope(&ctx.channel1);
      pulseClockEnvelope(&ctx.channel2);
    }

    ctx.frameSequencerStep = (ctx.frameSequencerStep + 1) % FRAME_SEQ_MAX_STEPS;
  }

  // - - - 2. Command channels to generate their next digital slice
  pulseStepTimer(&ctx.channel1);
  pulseStepTimer(&ctx.channel2);
  waveStepTimer (&ctx.channel3);
  noiseStepTimer(&ctx.channel4);

  // - - - 3. Resample and push to OS (44100 Hz target)
  f32 clockPaced = (f32) APU_CLOCK_SPEED * ctx.speedMultiplier;
  ctx.sampleAccumulator += ((f32)AUDIO_SAMPLE_RATE / clockPaced);
  if (ctx.sampleAccumulator >= 1.0f) 
  {
    ctx.sampleAccumulator -= 1.0f;

    f32 sampleL = 0.0f;
    f32 sampleR = 0.0f;

    // - - - Mix channel 1 
    if (ctx.channel1.dacEnabled)
    {
      f32 analog = (ctx.channel1.outputVolume / DAC_NEUTRAL_POINT) - 1.0f;
      if (ctx.panningMap & NR51_CH1_LEFT_MASK)  sampleL += analog;
      if (ctx.panningMap & NR51_CH1_RIGHT_MASK) sampleR += analog;
    }

    // - - - Mix channel 2 
    if (ctx.channel2.dacEnabled) 
    {
      f32 analog = (ctx.channel2.outputVolume / DAC_NEUTRAL_POINT) - 1.0f;
      if (ctx.panningMap & NR51_CH2_LEFT_MASK)  sampleL += analog; 
      if (ctx.panningMap & NR51_CH2_RIGHT_MASK) sampleR += analog; 
    }

    // - - - Mix CH3
    if (ctx.channel3.dacEnabled) 
    {
      f32 analog = (ctx.channel3.outputVolume / DAC_NEUTRAL_POINT) - 1.0f;
      if (ctx.panningMap & NR51_CH3_LEFT_MASK)  sampleL += analog; 
      if (ctx.panningMap & NR51_CH3_RIGHT_MASK) sampleR += analog; 
    }

    // - - - Mix CH4
    if (ctx.channel4.dacEnabled) 
    {
      f32 analog = (ctx.channel4.outputVolume / DAC_NEUTRAL_POINT) - 1.0f;
      if (ctx.panningMap & NR51_CH4_LEFT_MASK)  sampleL += analog; 
      if (ctx.panningMap & NR51_CH4_RIGHT_MASK) sampleR += analog; 
    }

    // - - - Apply NR50 Master Volume and global scale to prevent blowing out speakers
    sampleL = (sampleL * ((ctx.masterVolumeLeft  + 1) / MIXER_MAX_VOLUME_STEPS)) * AUDIO_MASTER_OUTPUT_SCALE;
    sampleR = (sampleR * ((ctx.masterVolumeRight + 1) / MIXER_MAX_VOLUME_STEPS)) * AUDIO_MASTER_OUTPUT_SCALE;

    ctx.sampleBuffer[ctx.bufferIndex++] = sampleL;
    ctx.sampleBuffer[ctx.bufferIndex++] = sampleR;

    // - - - Flush
    if (ctx.bufferIndex >= AUDIO_BUFFER_SIZE) 
    {
      platform->audio.pushSamples(ctx.sampleBuffer, AUDIO_BUFFER_SIZE);
      ctx.bufferIndex = 0;
    }
  }
}
