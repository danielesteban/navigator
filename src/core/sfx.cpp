#include "sfx.hpp"
#include <glm/gtc/type_ptr.hpp>

GLuint SFX::sfxId = 1;

SFX::SFX(HTTPRequest* request):
  id(sfxId++),
  buffer(0),
  request(request)
{

}

SFX::~SFX() {
  if (request != nullptr) {
    delete request;
  }
  for (const auto& source : sources) {
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
      alSourceStop(source);
    }
    alDeleteSources(1, &source);
  }
   if (buffer) {
    alDeleteBuffers(1, &buffer);
  }
}

bool SFX::isReady() {
  return buffer != 0;
}

void SFX::play(const glm::vec3& position) {
  if (buffer == 0) {
    return;
  }
  ALuint source;
  alGenSources(1, &source);
  if (alGetError() != AL_NO_ERROR) {
    return;
  }
  alSourcef(source, AL_GAIN, 1.0);
  alSourcef(source, AL_PITCH, 1.0);
  alSourcei(source, AL_LOOPING, AL_FALSE);
  alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
  alSource3f(source, AL_DIRECTION, 0.0, 0.0, 0.0);
  alSource3f(source, AL_VELOCITY, 0.0, 0.0, 0.0);
  alSourcei(source, AL_BUFFER, (ALint) buffer);

  alSourcefv(source, AL_POSITION, glm::value_ptr(position));
  ALfloat angle = glm::atan(position.y, position.x);
  ALfloat angles[2] = { (ALfloat)(glm::pi<ALfloat>()/6.0 - angle), (ALfloat)(-glm::pi<ALfloat>()/6.0 - angle) };
  alSourcefv(source, AL_STEREO_ANGLES, angles);

  if (alGetError() != AL_NO_ERROR) {
    alDeleteSources(1, &source);
    return;
  }
  sources.push_back(source);
  alSourcePlay(source);
}

void SFX::update() {
  if (request != nullptr && request->isReady) {
    if (request->response.status >= 200 && request->response.status < 400 && request->response.size > 0) {
      buffer = LoadSound(request->response.data, request->response.size);
    }
    delete request;
    request = nullptr;
  }
  if (!sources.empty()) {
    std::erase_if(sources, [](const ALuint& source) {
      ALenum state;
      alGetSourcei(source, AL_SOURCE_STATE, &state);
      if (state == AL_PLAYING) {
        return false;
      }
      alDeleteSources(1, &source);
      return true;
    });
  }
}

struct MemoryFile {
  const unsigned char *buffer;
  sf_count_t length;
  sf_count_t curpos;
};

enum FormatType {
  Int16,
  Float,
  IMA4,
  MSADPCM
};

ALuint SFX::LoadSound(const unsigned char *data, const size_t size) {
  enum FormatType sample_format = Int16;
  ALint byteblockalign = 0;
  ALint splblockalign = 0;
  sf_count_t num_frames;
  ALenum err, format;
  ALsizei num_bytes;
  SNDFILE *sndfile;
  SF_INFO sfinfo;
  ALuint buffer;
  void *membuf;

  SF_VIRTUAL_IO io;
  io.get_filelen = vio_get_filelen;
  io.read = vio_read;
  io.seek = vio_seek;
  io.tell = vio_tell;
  io.write = vio_write;

  MemoryFile file(data, size, 0);

  sndfile = sf_open_virtual(&io, SFM_READ, &sfinfo, &file);
  if (!sndfile) {
    return 0;
  }
  if (sfinfo.frames < 1) {
    sf_close(sndfile);
    return 0;
  }

  switch ((sfinfo.format&SF_FORMAT_SUBMASK)) {
    case SF_FORMAT_PCM_24:
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_FLOAT:
    case SF_FORMAT_DOUBLE:
    case SF_FORMAT_VORBIS:
    case SF_FORMAT_OPUS:
    case SF_FORMAT_ALAC_20:
    case SF_FORMAT_ALAC_24:
    case SF_FORMAT_ALAC_32:
    case 0x0080/*SF_FORMAT_MPEG_LAYER_I*/:
    case 0x0081/*SF_FORMAT_MPEG_LAYER_II*/:
    case 0x0082/*SF_FORMAT_MPEG_LAYER_III*/:
      if (alIsExtensionPresent("AL_EXT_FLOAT32"))
        sample_format = Float;
      break;
    case SF_FORMAT_IMA_ADPCM:
      if (sfinfo.channels <= 2 && (sfinfo.format&SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
        && alIsExtensionPresent("AL_EXT_IMA4")
        && alIsExtensionPresent("AL_SOFT_block_alignment"))
        sample_format = IMA4;
      break;
    case SF_FORMAT_MS_ADPCM:
      if (sfinfo.channels <= 2 && (sfinfo.format&SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
        && alIsExtensionPresent("AL_SOFT_MSADPCM")
        && alIsExtensionPresent("AL_SOFT_block_alignment"))
        sample_format = MSADPCM;
      break;
  }

  if (sample_format == IMA4 || sample_format == MSADPCM) {
    SF_CHUNK_INFO inf = { "fmt ", 4, 0, NULL };
    SF_CHUNK_ITERATOR *iter = sf_get_chunk_iterator(sndfile, &inf);

    if(!iter || sf_get_chunk_size(iter, &inf) != SF_ERR_NO_ERROR || inf.datalen < 14)
      sample_format = Int16;
    else {
      ALubyte *fmtbuf = (ALubyte*) calloc(inf.datalen, 1);
      inf.data = fmtbuf;
      if (sf_get_chunk_data(iter, &inf) != SF_ERR_NO_ERROR)
        sample_format = Int16;
      else {
        byteblockalign = fmtbuf[12] | (fmtbuf[13]<<8);
        if (sample_format == IMA4) {
          splblockalign = (byteblockalign/sfinfo.channels - 4)/4*8 + 1;
          if (splblockalign < 1
            || ((splblockalign-1)/2 + 4)*sfinfo.channels != byteblockalign)
            sample_format = Int16;
        } else {
          splblockalign = (byteblockalign/sfinfo.channels - 7)*2 + 2;
          if (splblockalign < 2
            || ((splblockalign-2)/2 + 7)*sfinfo.channels != byteblockalign)
            sample_format = Int16;
        }
      }
      free(fmtbuf);
    }
  }

  if (sample_format == Int16) {
    splblockalign = 1;
    byteblockalign = sfinfo.channels * 2;
  } else if (sample_format == Float) {
    splblockalign = 1;
    byteblockalign = sfinfo.channels * 4;
  }

  format = AL_NONE;
  if (sfinfo.channels == 1) {
    if (sample_format == Int16)
      format = AL_FORMAT_MONO16;
    else if (sample_format == Float)
      format = AL_FORMAT_MONO_FLOAT32;
    else if (sample_format == IMA4)
      format = AL_FORMAT_MONO_IMA4;
    else if (sample_format == MSADPCM)
      format = AL_FORMAT_MONO_MSADPCM_SOFT;
  } else if (sfinfo.channels == 2) {
    if(sample_format == Int16)
      format = AL_FORMAT_STEREO16;
    else if (sample_format == Float)
      format = AL_FORMAT_STEREO_FLOAT32;
    else if (sample_format == IMA4)
      format = AL_FORMAT_STEREO_IMA4;
    else if (sample_format == MSADPCM)
      format = AL_FORMAT_STEREO_MSADPCM_SOFT;
  } else if (sfinfo.channels == 3) {
    if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT) {
      if (sample_format == Int16)
        format = AL_FORMAT_BFORMAT2D_16;
      else if (sample_format == Float)
        format = AL_FORMAT_BFORMAT2D_FLOAT32;
    }
  } else if (sfinfo.channels == 4) {
    if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT) {
      if (sample_format == Int16)
        format = AL_FORMAT_BFORMAT3D_16;
      else if(sample_format == Float)
        format = AL_FORMAT_BFORMAT3D_FLOAT32;
    }
  }
  if (!format) {
    sf_close(sndfile);
    return 0;
  }

  if (sfinfo.frames/splblockalign > (sf_count_t)(INT_MAX/byteblockalign)) {
    sf_close(sndfile);
    return 0;
  }

  membuf = malloc((size_t)(sfinfo.frames / splblockalign * byteblockalign));

  if (sample_format == Int16)
    num_frames = sf_readf_short(sndfile, (short*) membuf, sfinfo.frames);
  else if (sample_format == Float)
    num_frames = sf_readf_float(sndfile, (float*) membuf, sfinfo.frames);
  else {
    sf_count_t count = sfinfo.frames / splblockalign * byteblockalign;
    num_frames = sf_read_raw(sndfile, membuf, count);
    if (num_frames > 0)
      num_frames = num_frames / byteblockalign * splblockalign;
  }
  if (num_frames < 1) {
    free(membuf);
    sf_close(sndfile);
    return 0;
  }
  num_bytes = (ALsizei)(num_frames / splblockalign * byteblockalign);

  buffer = 0;
  alGenBuffers(1, &buffer);
  if (splblockalign > 1)
    alBufferi(buffer, AL_UNPACK_BLOCK_ALIGNMENT_SOFT, splblockalign);
  alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

  free(membuf);
  sf_close(sndfile);

  err = alGetError();
  if (err != AL_NO_ERROR) {
    if (buffer && alIsBuffer(buffer))
      alDeleteBuffers(1, &buffer);
    return 0;
  }

  return buffer;
}

sf_count_t SFX::vio_get_filelen(void *user_data) {
  MemoryFile *file = (MemoryFile*) user_data;
  return file->length;
}

sf_count_t SFX::vio_read(void *ptr, sf_count_t count, void *user_data) {
  MemoryFile *file = (MemoryFile*) user_data;
  if (count + file->curpos > file->length) {
    count = file->length - file->curpos;
  }
  if (count > 0) {
    memcpy(ptr, file->buffer + file->curpos, count);
  }
  file->curpos += count;
  return count;
}

sf_count_t SFX::vio_seek(sf_count_t offset, int whence, void *user_data) {
  MemoryFile *file = (MemoryFile*) user_data;
  sf_count_t newpos = 0;
  switch (whence) {
    case SEEK_SET:
      newpos = offset;
      break;
    case SEEK_CUR:
      newpos = file->curpos + offset;
      break;
    case SEEK_END:
      newpos = file->length - offset;
      break;
  }
  if ((newpos >= 0) && (newpos < file->length)) {
    file->curpos = newpos;
  }
  return file->curpos;
}

sf_count_t SFX::vio_tell(void *user_data) {
  MemoryFile *file = (MemoryFile*) user_data;
  return file->curpos;
}

sf_count_t SFX::vio_write(const void *ptr, sf_count_t count, void *user_data) {
  return 0;
}
