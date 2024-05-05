#ifndef __WAV_H__
#define __WAV_H__

#include "audio-buffer.h"
#include "buffer.h"

AudioBuffer *wav_open(Buffer *buffer);
AudioBuffer *wav_open(const char *fname);

#endif
