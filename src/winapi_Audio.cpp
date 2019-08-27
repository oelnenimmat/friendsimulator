struct WinApiAudio
{
	IMMDeviceEnumerator *	pEnumerator;
	IMMDevice *				pDevice;
	IAudioClient *			pClient;
	IAudioRenderClient * 	pRenderClient;

	WAVEFORMATEX * 			pFormat;
	uint32					bufferFrameCount;

    bool32                  isPlaying;
};

// Note(Leo): REFERENCE_TIME are in units of [100 nanoseconds], use these for conversions
constexpr int64 REFERENCE_TIMES_PER_SECOND = 10000000;     // seconds to REFERENCE_TIME
constexpr int64 REFERENCE_TIMES_PER_MILLISECOND = 10000;   // milliseconds to REFERENCE_TIME

namespace winapi
{
	internal WinApiAudio
	CreateAudio();			
	
	internal void
	ReleaseAudio (WinApiAudio * audio);

    internal void
    StartPlaying(WinApiAudio * audio);

    internal void
    StopPlaying(WinApiAudio * audio);

    internal void
    GetAudioBuffer(WinApiAudio * audio, int * sampleCount, game::StereoSoundSample ** samples);

    internal void
    ReleaseAudioBuffer(WinApiAudio * audio, int sampleCount);
}

/* Todo(Leo): copied from web, forgot where. Extracts actual format tag if
it is hidden behind WAVEFORMATEXTENSIBLE struct */
inline uint32_t
UNKNOWN_GetFormatTag( const WAVEFORMATEX* wfx )
{
    if ( wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE )
    {
        if ( wfx->cbSize < ( sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX) ) )
            return 0;

        static const GUID s_wfexBase = {0x00000000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};

        auto wfex = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>( wfx );

        if ( memcmp( reinterpret_cast<const BYTE*>(&wfex->SubFormat) + sizeof(DWORD),
                     reinterpret_cast<const BYTE*>(&s_wfexBase) + sizeof(DWORD), sizeof(GUID) - sizeof(DWORD) ) != 0 )
        {
            return 0;
        }

        return wfex->SubFormat.Data1;
    }
    else
    {
        return wfx->wFormatTag;
    }
}

internal WinApiAudio
winapi::CreateAudio ()
{
    const CLSID CLSID_MMDeviceEnumerator =   __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator =      __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient =             __uuidof(IAudioClient);
    const IID IID_IAudioRenderClient =       __uuidof(IAudioRenderClient);

    WinApiAudio audio = {};    

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    WinApiLog(  "Create audio device enumerator",
                CoCreateInstance(   CLSID_MMDeviceEnumerator, nullptr,
                                        CLSCTX_ALL, IID_IMMDeviceEnumerator ,
                                        (void**)&audio.pEnumerator));
    WinApiLog(  "Get audio endpoint device",
                audio.pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audio.pDevice));

    WinApiLog(  "Activate device to get audio client",
                audio.pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&audio.pClient));

    WinApiLog("Get audio format", audio.pClient->GetMixFormat(&audio.pFormat));

    #if 1
    std::cout
        << "|======================================|\n"
        << "Selected device format:\n"
        << "\tformat: "             << WinApiWaveFormatTagString(UNKNOWN_GetFormatTag(audio.pFormat)) << "\n"
        << "\tchannels: "           << audio.pFormat->nChannels << "\n"
        << "\tsamples per sec: "    << audio.pFormat->nSamplesPerSec << "\n"
        << "\tavg bytes per sec: "  << audio.pFormat->nAvgBytesPerSec << "\n"
        << "\tblock align: "        << audio.pFormat->nBlockAlign << "\n"
        << "\tbits per sample: "    << audio.pFormat->wBitsPerSample << "\n"
        << "|======================================|\n";
    #endif

    REFERENCE_TIME requestedDuration = 1 * REFERENCE_TIMES_PER_SECOND;
    WinApiLog(  "Initialize audio client",
                audio.pClient->Initialize(  AUDCLNT_SHAREMODE_SHARED, 0,
                                            requestedDuration, 0, audio.pFormat, nullptr));

    WinApiLog("Get buffer size", audio.pClient->GetBufferSize(&audio.bufferFrameCount));
    WinApiLog("Get audio renderer", audio.pClient->GetService(IID_IAudioRenderClient, (void**)&audio.pRenderClient));

    return audio;
}

internal void
winapi::ReleaseAudio(WinApiAudio * audio)
{
    CoTaskMemFree(audio->pFormat);

    audio->pEnumerator->Release();
    audio->pEnumerator = nullptr;

    audio->pDevice->Release();
    audio->pEnumerator = nullptr;

    audio->pClient->Release();
    audio->pClient = nullptr;

    audio->pRenderClient->Release();
    audio->pRenderClient = nullptr;

    CoUninitialize();
}

internal void
winapi::StartPlaying(WinApiAudio * audio)
{
    WinApiLog("Start playing audio", audio->pClient->Start());
}

internal void
winapi::StopPlaying(WinApiAudio * audio)
{
    WinApiLog("Stop playing audio", audio->pClient->Stop());
}

internal void
winapi::GetAudioBuffer(WinApiAudio * audio, int * frameCount, game::StereoSoundSample ** samples)
{
    uint32 currentPadding;
    WinApiLog("Get audio padding", audio->pClient->GetCurrentPadding(&currentPadding));
    uint32 framesAvailable = audio->bufferFrameCount - currentPadding;

    uint8 ** data = reinterpret_cast<uint8 **>(samples);
    WinApiLog("Get audio buffer", audio->pRenderClient->GetBuffer(framesAvailable, data));

    *frameCount = framesAvailable;
}

internal void
winapi::ReleaseAudioBuffer(WinApiAudio * audio, int32 frameCount)
{
    WinApiLog("Release audio buffer", audio->pRenderClient->ReleaseBuffer(frameCount, 0));

}