# Vocal-Eyes AI-Powered Smart Glasses

An ESP32-based smart glasses system that captures images, sends them to an AI server for scene description, and reads the description aloud using text-to-speech.

## Project Structure

```
Vocal-Eyes-AI-Powered-Smart-Glasses/
├── CameraWebServer/           # ESP32-CAM web server & image capture
│   ├── CameraWebServer.ino    # Main Arduino sketch
│   ├── camera_pins.h          # Pin definitions for various camera models
│   ├── board_config.h         # Board configuration (AI-Thinker ESP32-CAM)
│   ├── app_httpd.cpp          # HTTP server with streaming & control endpoints
│   ├── camera_index.h         # Embedded web UI (gzipped HTML)
│   ├── partitions.csv         # Flash partition table
│   └── ci.yml                 # CI build configurations
│
└── ESP32spech-function/       # ESP32 Text-to-Speech (Google TTS)
    ├── ESP32_google_TTS_short/    # Pre-defined text TTS demo
    └── Google_TTS_from_Terminal/  # Interactive serial TTS
```

---

## Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Camera Module** | ESP32-CAM (AI-Thinker) with OV2640 |
| **Audio Output** | I2S DAC (MAX98357A or similar) |
| **Microphone** | Optional - for voice commands |
| **Power** | 5V/1A minimum (USB or LiPo) |
| **Connectivity** | WiFi 2.4 GHz |

### Pin Configuration (AI-Thinker ESP32-CAM)

| Function | GPIO |
|----------|------|
| Camera D0-D7 | 5, 18, 19, 21, 36, 39, 34, 35 |
| XCLK | 0 |
| PCLK | 22 |
| VSYNC | 25 |
| HREF | 23 |
| SCCB SDA/SCL | 26/27 |
| PWDN | 32 |
| LED/Flash | 4 |
| I2S BCLK | 33 |
| I2S LRC | 32 |
| I2S DOUT | 25 |

---

## Software Architecture

### 1. CameraWebServer (ESP32-CAM)

**Purpose**: Capture images, serve web UI, stream MJPEG, send frames to AI server.

**Key Features**:
- HTTP server on port 80 (control) + 81 (stream)
- MJPEG streaming at ~30-60 fps (QVGA 320x240)
- REST endpoints: `/capture`, `/stream`, `/control`, `/status`, `/bmp`
- Auto-connects to WiFi on boot
- Captures image every 10 seconds → sends to `https://imeesam-vocal-eyes-updated.hf.space/predict`
- Receives JSON response with scene description

**Main Loop** (`CameraWebServer.ino:105-110`):
```cpp
void loop() {
  if (millis() - lastSend > 10000) {
    lastSend = millis();
    sendImage();  // POST multipart/form-data to AI server
  }
}
```

### 2. ESP32 TTS Function (Text-to-Speech)

**Purpose**: Convert AI-generated descriptions to speech via Google Translate TTS.

**Two Modes**:
1. **ESP32_google_TTS_short** - Plays hardcoded text on boot
2. **Google_TTS_from_Terminal** - Reads text from Serial Monitor, speaks it

**Audio Pipeline**:
```
Text → Chunk (200 chars) → URL encode → Google TTS HTTP → I2S → MAX98357A → Speaker
```

**Key Code** (`ESP32_google_TTS_short.ino:43-67`):
```cpp
void playLongText(String text) {
  int maxChunkSize = 200;
  while (startPos < text.length()) {
    String chunk = text.substring(startPos, endPos);
    chunk.replace(" ", "%20");
    String tts_url = "http://translate.google.com/translate_tts?ie=UTF-8&q=" + chunk + "&tl=en&client=tw-ob";
    audio.connecttohost(tts_url.c_str());
    while (audio.isRunning()) audio.loop();
    startPos = endPos;
    delay(500);
  }
}
```

---

## Data Flow

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐     ┌──────────┐
│ ESP32-CAM   │────▶│ AI Server    │────▶│ ESP32-TTS   │────▶│ Speaker  │
│ (Capture)   │     │ (HuggingFace)│     │ (Google TTS)│     │ (Audio)  │
└─────────────┘     └──────────────┘     └─────────────┘     └──────────┘
     │                    │                    │
  10 sec            POST /predict         Chunked HTTP
 interval           multipart/form-data    to Google TTS
```

---

## Technical Plan

### Phase 1: Hardware Integration & Testing
- [ ] Verify ESP32-CAM pinout matches `board_config.h`
- [ ] Test camera capture & web server locally
- [ ] Test I2S audio output with MAX98357A
- [ ] Validate WiFi connectivity and stability

### Phase 2: AI Server Integration
- [ ] Confirm HuggingFace Space endpoint: `https://imeesam-vocal-eyes-updated.hf.space/predict`
- [ ] Test API response format (expects JSON with description field)
- [ ] Handle errors: network timeout, invalid response, rate limits
- [ ] Add retry logic with exponential backoff

### Phase 3: TTS Integration
- [ ] Merge TTS code into main CameraWebServer sketch
- [ ] On AI response received → extract description → send to `playLongText()`
- [ ] Handle long descriptions (chunking already implemented)
- [ ] Add volume control via HTTP endpoint

### Phase 4: Power Optimization
- [ ] Deep sleep between captures (if battery-powered)
- [ ] Reduce camera frame size for lower power
- [ ] Disable LED flash when not streaming
- [ ] Optimize WiFi reconnection logic

### Phase 5: UX Improvements
- [ ] Add physical button for manual capture
- [ ] Voice activation keyword detection
- [ ] Battery level monitoring & low-power warning
- [ ] OLED display for status (optional)

### Phase 6: Production Hardening
- [ ] Secure WiFi credentials (NVS/Preferences)
- [ ] OTA update support
- [ ] Watchdog timer for crash recovery
- [ ] Logging to SPIFFS for debugging

---

## Configuration

### WiFi Credentials
Edit `CameraWebServer.ino:9-10`:
```cpp
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

### AI Server URL
Edit `CameraWebServer.ino:13`:
```cpp
const char* serverUrl = "https://your-ai-server.com/predict";
```

### Camera Model
Edit `board_config.h:6`:
```cpp
#define CAMERA_MODEL_AI_THINKER  // Change for your board
```

### I2S Pins (TTS)
Edit `ESP32_google_TTS_short.ino:10-12`:
```cpp
#define I2S_DOUT  25
#define I2S_BCLK  33
#define I2S_LRC   32
```

---

## Building & Flashing

### Prerequisites
- Arduino IDE 2.x or PlatformIO
- ESP32 board package: `espressif/esp32`
- Libraries: `ESP32-audioI2S` (for TTS)

### Arduino IDE
1. Open `CameraWebServer/CameraWebServer.ino`
2. Select Board: **AI Thinker ESP32-CAM**
3. Select Partition Scheme: **Custom (partitions.csv)**
4. Upload

### PlatformIO
```bash
cd CameraWebServer
pio run -t upload
```

---

## API Endpoints (CameraWebServer)

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web UI (camera control panel) |
| `/stream` | GET | MJPEG video stream |
| `/capture` | GET | Single JPEG capture |
| `/bmp` | GET | BMP format capture |
| `/control` | GET | Camera settings (var=value) |
| `/status` | GET | JSON camera status |
| `/xclk` | GET | Set XCLK frequency |
| `/reg` | GET | Set sensor register |
| `/greg` | GET | Get sensor register |
| `/pll` | GET | Set PLL configuration |
| `/resolution` | GET | Set camera window/resolution |

---

## Known Issues / TODOs

1. **Hardcoded WiFi credentials** - Move to NVS/Preferences
2. **No error handling** for AI server failures
3. **No audio feedback** when AI request fails
4. **Blocking delays** in main loop (use FreeRTOS tasks)
5. **Single-threaded** - camera + HTTP + TTS on same core
6. **No OTA updates** support
7. **Google TTS URL** may change/expire (unofficial API)

---

## License

MIT License - See LICENSE file for details.

---

## References

- [ESP32-CAM Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/camera.html)
- [ESP32-audioI2S Library](https://github.com/schreibfaul1/ESP32-audioI2S)
- [Google Translate TTS](https://cloud.google.com/text-to-speech)
- [HuggingFace Inference API](https://huggingface.co/docs/api-inference/index)