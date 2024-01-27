## Whisper.cpp build instruction

### For NVIDIA GPU on Linux

- CUDA Toolkit 10.2, with nvcc in PATH

```bash
mkdir build && cd build
cmake -DLLAMA_CUBLAS=ON -DWHISPER_CUBLAS=ON ..
make -j$(nproc)
```

### For x86 CPU on Linux

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### For Mac Silicon with CoreML support

```
# Download model in `.bin` and `.mlmodelc` in order to use on Mac Silicon
cmake -B build -DWHISPER_COREML=1
cmake --build build -j --config Release
```

### For Windows with CUDA

```
mkdir -p build
cd build
cmake .. -DLLAMA_CUBLAS=ON -DBUILD_SHARED_LIBS=ON -DWHISPER_CUBLAS=ON -DWHISPER_SDL2=ON
cmake --build . --config Release
```

Then copy llama.dll, whisper.dll and zlib.dll

## Sample test command

- Download `ggml-base.en.bin` with [whisper.cpp/models/download-ggml-model.sh](whisper.cpp/models/download-ggml-model.sh)
- Load model

```bash
curl 127.0.0.1:3928/v1/audio/load_model \
-X POST -H "Content-Type: application/json" \
-d '{
    "model_id":"ggml-base.en.bin",
    "model_path":"/abs/path/to/whisper.cpp/models/ggml-base.en.bin"
    "warm_up_audio_path":"/abs/path/to/samples.wav"
}'
```

`warm_up_audio_path` is optional

If we enable CoreML on Mac silicon, we need to include `ggml-base.mlmodelc` file in the same folder as `ggml-base.en.bin`

- List model:

```bash
curl 127.0.0.1:3928/v1/audio/list_model
```

- Download sample audio file from:

```bash
wget https://github.com/ggerganov/whisper.cpp/raw/master/samples/jfk.wav
```

- The input needs to be converted to expected one

```
ffmpeg -i INPUT.MP3 -ar 16000 -ac 1 -c:a pcm_s16le OUTPUT.WAV
```

- Sample transcription:

```bash
curl -X POST 127.0.0.1:3928/v1/audio/transcriptions \
-H "Content-Type: multipart/form-data" \
-F file="@/abs/path/to/jfk.wav" \
-F model_id="ggml-base.en" \
-F temperature="0.0" \
-F response_format="verbose_json" # \
# -F prompt="The transcript is about OpenAI which makes technology like DALL·E, GPT-3, and ChatGPT with the hope of one day building an AGI system that benefits all of humanity. The president is trying to raly people to support the cause."
```
