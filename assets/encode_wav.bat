ffmpeg -i %1 -c:a pcm_s16le temp_%2
ffmpeg -i temp_%2 -ar 44100 %2
del temp_%2
