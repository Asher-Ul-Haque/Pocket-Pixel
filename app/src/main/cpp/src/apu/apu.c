#include <platform.h>
#include <apu/apu.h>

static ApuContext ctx;

ApuContext* apuGetContext(void)
{ return &ctx; }

void apuInit(void)
{ memset(&ctx, 0, sizeof(ApuContext)); }

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

    // - - - Step 0, 2, 4, 6: Clock Length Timers
    if ((ctx.frameSequencerStep & FRAME_SEQ_LEN_MASK) == 0) pulseClockLength(&ctx.channel2);

    // - - - Step 7: Clock Volume Envelopes
    if (ctx.frameSequencerStep == FRAME_SEQ_ENV_STEP) pulseClockEnvelope(&ctx.channel2);

    ctx.frameSequencerStep = (ctx.frameSequencerStep + 1) % FRAME_SEQ_MAX_STEPS;
  }

  // - - - 2. Command channels to generate their next digital slice
  pulseStepTimer(&ctx.channel2);

  // - - - 3. Resample and push to OS (44100 Hz target)
  ctx.sampleAccumulator += ((f32)AUDIO_SAMPLE_RATE / (f32)APU_CLOCK_SPEED);
  if (ctx.sampleAccumulator >= 1.0f) 
  {
    ctx.sampleAccumulator -= 1.0f;

    f32 sampleL = 0.0f;
    f32 sampleR = 0.0f;

    if (ctx.channel2.dacEnabled) 
    {
      // - - - DAC translation: Map 0-15 to an analog -1.0 to 1.0 f32
      f32 analog = (ctx.channel2.outputVolume / DAC_NEUTRAL_POINT) - 1.0f;

      // - - - Mix into channels based on NR51
      if (ctx.panningMap & NR51_CH2_LEFT_MASK)  sampleL += analog; 
      if (ctx.panningMap & NR51_CH2_RIGHT_MASK) sampleR += analog; 
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
