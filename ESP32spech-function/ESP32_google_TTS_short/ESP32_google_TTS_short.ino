#include "WiFi.h"
#include "HTTPClient.h"
#include "Audio.h"

// WiFi credentials
const char* ssid = "bera1";
const char* password = "xxxxxxxxxx";

// I2S Pin Definitions
#define I2S_DOUT  25  // Data Out Pin
#define I2S_BCLK  33 //27  // Bit Clock Pin
#define I2S_LRC   32 //26  // Left Right Clock Pin

Audio audio;

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  // Text to be spoken
  String text = "Hello Bera How are you? This code should now compile successfully and play the text-to-speech (TTS) message. However, it should be capable of playing a much longer text, so we will need to split the text into smaller chunks to ensure that the TTS API can handle it.";
//  String text1 = "The compilation error is occurring because the min() function is being used with arguments of different types: an int and an unsigned int. To resolve this, you can cast both arguments to the same type before passing them to min(). In this case, you should cast text.length() to an int to match the type of startPos + maxChunkSize.";
  // Play the text in chunks
 playLongText(text);
 playLongText(text1);
  //audio.loop();
}

void loop() {
  audio.loop();
}

void playLongText(String text) {
  int maxChunkSize = 200; // Google TTS max length
  int startPos = 0;

  while (startPos < text.length()) {
    int endPos = min(startPos + maxChunkSize, (int)text.length()); // Cast text.length() to int
    String chunk = text.substring(startPos, endPos);

    // URL encode the text chunk
    chunk.replace(" ", "%20");

    // Construct the URL for Google TTS
    String tts_url = "http://translate.google.com/translate_tts?ie=UTF-8&q=" + chunk + "&tl=en&client=tw-ob";
    // Play the audio
    audio.connecttohost(tts_url.c_str());
    
    // Wait for the chunk to finish playing
    while (audio.isRunning()) {
      audio.loop();
    }

    startPos = endPos;
    delay(500); // Small delay between chunks
  }
}