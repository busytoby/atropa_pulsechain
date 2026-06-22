# Lucas the Spider: YULC-Powered Scurry & Eye Glow Tutorial

This tutorial details how to use a **YULC (USB-C ESP32-S3 LED Controller)** hardware board inside your Lucas the Spider plush to handle power management, motor control, wireless communication, and decorative LED lighting accents (making his eyes glow or pulse when he moves).

---

## 📐 Why YULC?
The **YULC** board provides a few massive advantages for our robotic plush:
* **USB-C Power Delivery (PD):** You can plug a standard USB-C power bank directly into Lucas's belly to run him. YULC negotiates the power so you don't need a complex custom battery charger.
* **Onboard ESP32-S3:** Gives us built-in Bluetooth/Wi-Fi. The synthesizer can connect directly over ESP-NOW or Bluetooth (no clunky IR sensors needed).
* **Built-in LED Channels:** YULC has dedicated logic-level-shifted data pins designed to drive addressable RGB LEDs (WS2812B), which we can place behind his eyes for cute visual feedback!

---

## 🛠 Required Components

### 1. Inside the Spider Plush
* **Main Board:** 1x **YULC** (ESP32-S3 USB-C LED Controller).
* **Motors:** 2x Coreless Vibration Motors (left and right scurrying engines) or 2x Micro Geared Motors (for wheel steering).
* **Motor Drivers:** 2x N-Channel MOSFETs (e.g., IRLZ44N) connected to the YULC auxiliary GPIO headers (IO4, IO5) to switch motor power.
* **LED Accents:** 2x WS2812B NeoPixel single-pixel LEDs (placed behind transparent 30mm plastic eyes).
* **Power Source:** A standard slim USB-C power bank tucked inside the spider's abdomen plush.

### 2. Synthesizer Controller (The Keyboard)
* **Main Board:** 1x ESP32-S3 Development Board (to match wireless protocols).
* **Keys:** 5x Tactile pushbuttons.
* **Sound:** 1x Piezo buzzer.
* **Power:** 3x AAA batteries or USB.

---

## 🔌 Hardware Wiring Diagram (Inside the Spider)

```
                            +--------------------------+
                            |     YULC Controller      |
                            |                          |
                            |   [USB-C Power Input]    | <--- USB-C Power Bank
                            |                          |
                            |   [LED Data Out]         |-----( WS2812B Eye LEDs Data In )
                            |   [5V / GND Out]         |-----( WS2812B VCC/GND )
                            |                          |
                            |   [Aux GPIO Pins]        |
                            |    IO4 (Left Motor Ctrl) |----[ Gate ]
                            |    IO5 (Right Motor Ctrl)|----[ Gate ]--[ MOSFETs (IRLZ44N) ]
                            +--------------------------+                     |
                                                                             |
                                                                   ( Drives Left/Right Motors )
```

---

## 💻 ESP-NOW Wireless Control Code

We use **ESP-NOW**, a low-latency, connectionless wireless protocol built into the ESP32, to link the Synthesizer and the YULC controller.

### 1. Synthesizer Transmitter Code (ESP32)
```cpp
#include <esp_now.h>
#include <WiFi.h>

// Synthesizer keys
const int keys[] = {12, 13, 14, 27, 33}; 
const int buzzerPin = 25;
const int notes[] = {262, 294, 330, 349, 392}; // C, D, E, F, G

// MAC Address of the YULC board in the Spider (Replace with your actual board MAC)
uint8_t spiderAddress[] = {0x7C, 0xDF, 0xA1, 0xXX, 0xXX, 0xXX};

typedef struct struct_message {
  int command; // 0: Stop, 1: Slow, 2: Left, 3: Right, 4: Fast, 5: Jump
} struct_message;

struct_message myData;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, spiderAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  for(int i=0; i<5; i++) {
    pinMode(keys[i], INPUT_PULLUP);
  }
}

void loop() {
  int pressedKey = -1;
  for(int i=0; i<5; i++) {
    if(digitalRead(keys[i]) == LOW) {
      pressedKey = i;
      break;
    }
  }

  if (pressedKey != -1) {
    tone(buzzerPin, notes[pressedKey]);
    myData.command = pressedKey + 1; // Commands 1 through 5
  } else {
    noTone(buzzerPin);
    myData.command = 0; // Stop
  }

  esp_now_send(spiderAddress, (uint8_t *) &myData, sizeof(myData));
  delay(50);
}
```

### 2. YULC Spider Receiver Code (ESP32-S3)
```cpp
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN        18  // YULC default LED data pin
#define NUMPIXELS       2  // Two eye LEDs
#define LEFT_MOTOR_PIN  4  // YULC Auxiliary IO
#define RIGHT_MOTOR_PIN 5  // YULC Auxiliary IO

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef struct struct_message {
  int command;
} struct_message;

struct_message myData;

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  switch(myData.command) {
    case 0: // Stop / Turn off eyes
      analogWrite(LEFT_MOTOR_PIN, 0);
      analogWrite(RIGHT_MOTOR_PIN, 0);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.setPixelColor(1, pixels.Color(0, 0, 0));
      break;
      
    case 1: // Slow / Dim Warm White Eyes
      analogWrite(LEFT_MOTOR_PIN, 100);
      analogWrite(RIGHT_MOTOR_PIN, 100);
      pixels.setPixelColor(0, pixels.Color(50, 30, 10)); 
      pixels.setPixelColor(1, pixels.Color(50, 30, 10));
      break;
      
    case 2: // Spin Left / Blue Eyes
      analogWrite(LEFT_MOTOR_PIN, 0);
      analogWrite(RIGHT_MOTOR_PIN, 180);
      pixels.setPixelColor(0, pixels.Color(0, 0, 200));
      pixels.setPixelColor(1, pixels.Color(0, 0, 200));
      break;
      
    case 3: // Spin Right / Blue Eyes
      analogWrite(LEFT_MOTOR_PIN, 180);
      analogWrite(RIGHT_MOTOR_PIN, 0);
      pixels.setPixelColor(0, pixels.Color(0, 0, 200));
      pixels.setPixelColor(1, pixels.Color(0, 0, 200));
      break;
      
    case 4: // Fast Scurry / Bright Green Eyes
      analogWrite(LEFT_MOTOR_PIN, 255);
      analogWrite(RIGHT_MOTOR_PIN, 255);
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.setPixelColor(1, pixels.Color(0, 255, 0));
      break;
      
    case 5: // Startle Hop / Pulsing Red Eyes
      analogWrite(LEFT_MOTOR_PIN, 255);
      analogWrite(RIGHT_MOTOR_PIN, 255);
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.setPixelColor(1, pixels.Color(255, 0, 0));
      pixels.show();
      delay(80);
      analogWrite(LEFT_MOTOR_PIN, 0);
      analogWrite(RIGHT_MOTOR_PIN, 0);
      break;
  }
  pixels.show();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LEFT_MOTOR_PIN, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN, OUTPUT);
  
  pixels.begin();
  pixels.clear();
  pixels.show();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // Main execution handled asynchronously in onDataRecv callback
}
```
