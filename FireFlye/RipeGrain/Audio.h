#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <memory>
#include <filesystem>

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

namespace RipeGrain
{
    class Audio
    {
    private:
        std::unique_ptr<BYTE[]> audio_data;
        XAUDIO2_BUFFER buffer = { 0 };
        WAVEFORMATEXTENSIBLE wfx = { 0 };
    public:
        Audio(std::filesystem::path path)
        {
            HANDLE hFile = CreateFile(
                path.string().c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
            if (INVALID_HANDLE_VALUE == hFile)
                throw HRESULT_FROM_WIN32(GetLastError());

            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
                throw HRESULT_FROM_WIN32(GetLastError());

            DWORD dwChunkSize;
            DWORD dwChunkPosition;
            //check the file type, should be fourccWAVE or 'XWMA'
            FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
            DWORD filetype;
            ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
            if (filetype != fourccWAVE)
                throw S_FALSE;

            FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
            ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

            //fill out the audio data buffer with the contents of the fourccDATA chunk
            FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
            audio_data = std::make_unique<BYTE[]>(dwChunkSize);
            ReadChunkData(hFile, audio_data.get(), dwChunkSize, dwChunkPosition);

            buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
            buffer.pAudioData = audio_data.get();  //buffer containing audio data
            buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

            //system.engine->CreateSourceVoice(&voice, getFormat(), 0, XAUDIO2_DEFAULT_FREQ_RATIO, this, NULL, NULL);
        }
    public:
        const XAUDIO2_BUFFER* getBuffer() const
        {
            return &buffer;
        }
        WAVEFORMATEX* getFormat() const
        {
            return (WAVEFORMATEX*)&wfx;
        }
    private:
        static HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
        {
            HRESULT hr = S_OK;
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
                return HRESULT_FROM_WIN32(GetLastError());

            DWORD dwChunkType;
            DWORD dwChunkDataSize;
            DWORD dwRIFFDataSize = 0;
            DWORD dwFileType;
            DWORD bytesRead = 0;
            DWORD dwOffset = 0;

            while (hr == S_OK)
            {
                DWORD dwRead;
                if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());

                if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());

                switch (dwChunkType)
                {
                case fourccRIFF:
                    dwRIFFDataSize = dwChunkDataSize;
                    dwChunkDataSize = 4;
                    if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    break;

                default:
                    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                        return HRESULT_FROM_WIN32(GetLastError());
                }

                dwOffset += sizeof(DWORD) * 2;

                if (dwChunkType == fourcc)
                {
                    dwChunkSize = dwChunkDataSize;
                    dwChunkDataPosition = dwOffset;
                    return S_OK;
                }

                dwOffset += dwChunkDataSize;

                if (bytesRead >= dwRIFFDataSize) return S_FALSE;

            }

            return S_OK;
        }
        static HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
        {
            HRESULT hr = S_OK;
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
                return HRESULT_FROM_WIN32(GetLastError());
            DWORD dwRead;
            if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
            return hr;
        }
    public:
        void SetLoop(int loop_count = XAUDIO2_LOOP_INFINITE)
        {
            buffer.LoopBegin = 0;
            buffer.LoopLength = 0;
            buffer.LoopCount = loop_count;
        }
    };

    class PlayBackHandle : private IXAudio2VoiceCallback
    {
        friend class AudioSystem;
    private:
        bool isStopped = true;
    private:
        IXAudio2SourceVoice* voice = nullptr;
    public:
        PlayBackHandle(const PlayBackHandle&) = delete;
        PlayBackHandle& operator= (const PlayBackHandle&) = delete;
        PlayBackHandle(PlayBackHandle&& other) noexcept
        {
            *this = std::move(other);
        }
        PlayBackHandle& operator= (PlayBackHandle&& other) noexcept
        {
            isStopped = other.isStopped;
            voice = other.voice;

            isStopped = true;
            voice = nullptr;
        }
    public:
        PlayBackHandle() = default;
        ~PlayBackHandle()
        {
            Destroy();
        }
    private:
        void OnStreamEnd() override
        {
            isStopped = true;
        }
        void OnVoiceProcessingPassEnd() override {}
        void OnVoiceProcessingPassStart(UINT32 SamplesRequired) override {}
        void OnBufferEnd(void* pBufferContext) override {}
        void OnBufferStart(void* pBufferContext) override {}
        void OnLoopEnd(void* pBufferContext) override {}
        void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
    public:
        void Stop()
        {
            isStopped = true;
            voice->Stop();
        }
        void Play(Audio& audio)
        {
            isStopped = false;
            //voice->FlushSourceBuffers();
            voice->SubmitSourceBuffer(audio.getBuffer());
            voice->Start();
        }
        void SetSpeed(float freq)
        {
            voice->SetFrequencyRatio(freq);
        }
    public:
        bool IsStopped() const
        {
            return isStopped;
        }
        bool HasActiveVoice() const
        {
            return voice != nullptr;
        }
        void Destroy()
        {
            if (voice)
            {
                voice->Stop();
                voice->DestroyVoice();
                voice = nullptr;
            }
        }
    };

    struct EventPlayAudio
    {
        const Audio& audio;
        bool PLAY_IMMEDIATE = true;
        PlayBackHandle* handle = nullptr;
        float PLAYBACK_SPEED = 1.0f;
    };

    std::unique_ptr<Event> CreatePlayBackEvent(EventPlayAudio event)
    {
        return CreateEventObject<EventPlayAudio>(event);
    }

    std::unique_ptr<Event> CreatePlayBackEvent(const Audio& audio ,bool play_immediate = true , PlayBackHandle* handle = nullptr , float speed = 1.0f)
    {
        return CreatePlayBackEvent(EventPlayAudio{audio, play_immediate , handle , speed});
    }
}