#include "WiFi.h"
#include "HTTPClient.h"
#include "Audio.h"

// WiFi credentials
const char* ssid[] = {"bera1", "bera2", "bera3"};  //fill up with your ID
const char* password[] = {"xxxxxxxxx", "yyyyyyyyyyy", "zzzzzzzzzzz"};  // fill up with your passswords
const int numNetworks = 3;   //if you have only one network just make it 1

// I2S Pin Definitions
#define I2S_DOUT  25  // Data Out Pin
#define I2S_BCLK  33  // Bit Clock Pin
#define I2S_LRC   32  // Left Right Clock Pin

Audio audio;

void setup() {
  Serial.begin(115200);

  // Attempt to connect to one of the available Wi-Fi networks
  for (int i = 0; i < numNetworks; i++) {
    WiFi.begin(ssid[i], password[i]);
    Serial.print("Connecting to ");
    Serial.print(ssid[i]);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi");
      break;
    }
  }

  // Initialize I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21);  // Set volume level

  Serial.println("Enter text to speak: ");
}

void loop() {
  static String text = "";
  
  // Read input from serial
  if (Serial.available() > 0) {
    char received = Serial.read();
    
    if (received == '\n') {
      playLongText(text);
      text = "";  // Clear the text for the next input
      Serial.println("\nEnter text to speak: ");
    } else {
      text += received;
      Serial.print(received);
    }
  }

  audio.loop();
}

void playLongText(String text) {
  int maxChunkSize = 200; // Google TTS max length
  int startPos = 0;

  while (startPos < text.length()) {
    int endPos = min(startPos + maxChunkSize, (int)text.length());
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