
#include <directx>
#include <vector>

void PlayWave(IXAudio2 *pXaudio2, LPCWSTR szFilename) {
    std::vector<std::uint8_t> data;

    // load wav file...

    IXAudio2SourceVoice *pSourceVoice = nullptr;
    pXaudio2->CreateSourceVoice(&pSourceVoice, reinterpret_cast<WAVEFORMATEX*>(&data[0]));

    // Submit the wave sample data using an XAUDIO2_BUFFER structure
    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = waveData.startAudio;
    buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
    buffer.AudioBytes = waveData.audioBytes;

    if (waveData.loopLength > 0) {
        buffer.LoopBegin = waveData.loopStart;
        buffer.LoopLength = waveData.loopLength;
        buffer.LoopCount = 1; // We'll just assume we play the loop twice
    }

#if defined(USING_XAUDIO2_7_DIRECTX) || defined(USING_XAUDIO2_9)
    if (waveData.seek) {
        XAUDIO2_BUFFER_WMA xwmaBuffer = {};
        xwmaBuffer.pDecodedPacketCumulativeBytes = waveData.seek;
        xwmaBuffer.PacketCount = waveData.seekCount;
        if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer, &xwmaBuffer))) {
            wprintf(L"Error %#X submitting source buffer (xWMA)\n", hr);
            pSourceVoice->DestroyVoice();
            return hr;
        }
    }
#else
    if (waveData.seek) {
        wprintf(L"This platform does not support xWMA or XMA2\n");
        pSourceVoice->DestroyVoice();
        return hr;
    }
#endif
    else if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer))) {
        wprintf(L"Error %#X submitting source buffer\n", hr);
        pSourceVoice->DestroyVoice();
        return hr;
    }

    hr = pSourceVoice->Start(0);

    // Let the sound play
    BOOL isRunning = TRUE;
    while (SUCCEEDED(hr) && isRunning) {
        XAUDIO2_VOICE_STATE state;
        pSourceVoice->GetState(&state);
        isRunning = (state.BuffersQueued > 0) != 0;

        // Wait till the escape key is pressed
        if (GetAsyncKeyState(VK_ESCAPE))
            break;

        Sleep(10);
    }

    // Wait till the escape key is released
    while (GetAsyncKeyState(VK_ESCAPE))
        Sleep(10);

    pSourceVoice->DestroyVoice();

    return hr;
}

int main() {
    
}