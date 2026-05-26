#include <math.h>
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

  // The Game Boy CPU executes 1,048,576 M-Cycles per second.
  // We need to generate 44,100 samples per second.
  // This means we advance our fractional sample tracker by this ratio every tick.
  const float samplesPerTick = 44100.0f / 1048576.0f;
  
  ctx.sampleAccumulator += samplesPerTick;

  // Once a full sample duration has elapsed, we calculate and push it.
  if (ctx.sampleAccumulator >= 1.0f) 
  {
    ctx.sampleAccumulator -= 1.0f;

    // 1. Advance the sine wave phase for a 440Hz tone

    // 2. Generate the actual wave amplitude. 
    // We scale it by 0.1f (10% volume) so we don't blow out your eardrums.
    // Inside apuTick(), replace the sinf() block with this:

    const float phaseIncrement = (440.0f * 2.0f * PI) / 44100.0f;
    ctx.testTonePhase += phaseIncrement;
    
    if (ctx.testTonePhase > 2.0f * PI) 
    {
      ctx.testTonePhase -= 2.0f * PI;
    }

    // Generate a harsh square wave (no math.h required)
    // If phase is in the first half of the cycle, go high (+0.2). If second half, go low (-0.2).
    float sample = (ctx.testTonePhase < PI) ? 0.2f : -0.2f;

    ctx.sampleBuffer[ctx.bufferIndex++] = sample; // Left
    ctx.sampleBuffer[ctx.bufferIndex++] = sample; // Right

    // 4. If our chunk buffer is full, flush it to the SDL OS layer and reset
    if (ctx.bufferIndex >= AUDIO_BUFFER_SIZE) 
    {
      platform->audio.pushSamples(ctx.sampleBuffer, AUDIO_BUFFER_SIZE);
      ctx.bufferIndex = 0;
    }
  }
}
