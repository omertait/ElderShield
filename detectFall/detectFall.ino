#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_NeoPixel.h>
#include "SPIFFS.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           ""
#define BLYNK_TEMPLATE_NAME         "ElderShield"
#define BLYNK_AUTH_TOKEN            ""

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";


// MPU6050 Setup
MPU6050 accelgyro; // I2C address 0x68 by default
int16_t ax, ay, az, gx, gy, gz; // Variables to hold accelerometer and gyroscope data

// Fall Detection Parameters
const int axisThreshold = 32000; // Threshold for fall detection on any axis
const int staticThreshold = 20000; // Threshold for detecting static condition
const unsigned long staticPeriod = 2000; // Time in ms to confirm static condition
const unsigned long checkuptimout = 30000; // Time in ms to confirm checkup condition

bool respondOk = false;
bool checkupTrigDetected = false;
bool potentialFallDetected = false;
bool isStatic = false;
unsigned long fallTime = 0;
unsigned long checkupTrigTime = 0;


unsigned long staticStartTime = 0; // Time when static condition starts
const int LED_PIN = 13; // Built-in LED for fall indication

// humidity 
bool humidityAboveThreshold = false; // Tracks if humidity is above 90%
unsigned long humidityAboveTime = 0; // Time when humidity first went above 90%
const unsigned long humidityThresholdPeriod = 2000; // Threshold period in milliseconds (1 minutes is 60000)

// user checkup
int checkup = 0;

// DHT22, joystick and NeoPixel Setup
#define JOYSTICK_SW_PIN 2
#define DHTPIN 15
#define DHTTYPE DHT22
#define NEOPIXEL_PIN 16
#define NUM_LEDS 12
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }

  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}

// This function will be called every time Slider Widget
// in Blynk app writes values to the Virtual Pin V1
BLYNK_WRITE(V0)
{
  checkup = param.asInt(); // assigning incoming value from pin V1 to a variable

  // process received value
}


void setup() {
    Serial.begin(115200);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Wire.begin(21, 23); // SDA, SCL pins for ESP32
    accelgyro.initialize();

    if (accelgyro.testConnection()) {
        Serial.println("MPU6050 connection successful");
    } else {
        Serial.println("MPU6050 connection failed");
    }

    pinMode(LED_PIN, OUTPUT);
    pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);
    dht.begin();
    pixels.begin();
    pixels.show(); // Initialize all pixels to 'off'

    // Initialize SPIFFS for audio playback
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Initialize the audio components for MP3 playback
    file = new AudioFileSourceSPIFFS("/detect_fall.mp3");
    id3 = new AudioFileSourceID3(file);
    out = new AudioOutputI2S();
    mp3 = new AudioGeneratorMP3();
}


void loop() {
    // Read accelerometer and gyroscope data
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float absX = sqrt(ax * ax);
    float absY = sqrt(ay * ay);
    float absZ = sqrt(az * az);

    
    Blynk.run();

    // Print acceleration magnitude for each axis
    Serial.print("X: ");
    Serial.print(absX);
    Serial.print("Y: ");
    Serial.print(absY);
    Serial.print("Z: ");
    Serial.println(absZ);

    // Check for fall based on individual axis magnitudes
    if ((absX > axisThreshold || absY > axisThreshold || absZ > axisThreshold) && !potentialFallDetected) {
        potentialFallDetected = true;
        fallTime = millis();
        Serial.println("Potential fall detected.");
    }

    // Check if the device becomes static after a potential fall
    if (potentialFallDetected && (millis() - fallTime > staticPeriod)) {
        if (absX < staticThreshold && absY < staticThreshold && absZ < staticThreshold) {
            if (!isStatic) {
                isStatic = true;
                staticStartTime = millis(); // Time when static condition starts
            }
        } else {
            // Device moved again, reset conditions
            potentialFallDetected = false;
            isStatic = false;
        }
    }

    // Confirm the fall if the device remains static for longer than the static period
    if (isStatic && (millis() - staticStartTime > staticPeriod)) {
        Serial.println("Fall confirmed.");          
        Blynk.logEvent("fall_detected");   
        isStatic = false; // Reset static condition
        potentialFallDetected = false; // Reset fall detection flag

        // Actions after fall confirmation
        if (!mp3->isRunning()) {
            file = new AudioFileSourceSPIFFS("/detect_fall.mp3"); // Reload the file
            mp3->begin(file, out);
            digitalWrite(LED_PIN, HIGH); // Turn on the LED
        }
    }

    // Process MP3 playback
    if (mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            digitalWrite(LED_PIN, LOW); // Turn off the LED after playback stops
        }
    }

    // Humidity and temperature reading routine
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    if (!isnan(event.relative_humidity)) {
        Serial.print("Humidity: ");
        Serial.println(event.relative_humidity);
    
        // Check if humidity is above 90%
        if (event.relative_humidity > 90) {
            // If this is the first time humidity is detected above 90%, record the time
            if (!humidityAboveThreshold) {
                humidityAboveThreshold = true;
                humidityAboveTime = millis();
            }
            // If humidity has been above 90% for more than the threshold period, set the color to red
            else if (millis() - humidityAboveTime > humidityThresholdPeriod) {
                for(int i = 0; i < NUM_LEDS; i++) {
                    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Red
                }
                pixels.show();
                Blynk.logEvent("shower_alert");
                
            }
        } else {
            // If humidity goes below 90% before reaching the threshold, or if it was already below, set the color to green
            if (humidityAboveThreshold) {
                humidityAboveThreshold = false;
            }
            for(int i = 0; i < NUM_LEDS; i++) {
                pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green
            }
            pixels.show();
        }
    }

   

    // SOS button

    // check triger from Blynk virtual pin
    if (checkup == 1){
      if (!checkupTrigDetected){
        checkupTrigDetected = true;
        checkupTrigTime = millis();
      }
      if (checkupTrigDetected){
        // visualize
        while(millis() - checkupTrigTime < checkuptimout){
           for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
              pixels.rainbow(firstPixelHue);
           
        
              Serial.print("time: ");
              Serial.println(millis() - checkupTrigTime);
              
              
      
              // check for ok input from user if not timeout
              int sw = digitalRead(JOYSTICK_SW_PIN);
              Serial.print("sw: ");
              Serial.println(sw);
              if (sw == 0 ){
                 Blynk.logEvent("ok_response");
                 checkupTrigDetected = false;
                 respondOk = true;
                 break;
              }
              pixels.show(); // Update strip with new contents
              delay(10);  // Pause for a moment
          }
          if(respondOk){
            respondOk = false;
            break;
          }
          
        }
        // if timout reached without ok input -> alert and reset triger
          if (checkupTrigDetected){
            Blynk.logEvent("timeout_response");   
            checkupTrigDetected = false;
          }
      }
    }

    delay(1); // Delay for readability and to prevent flooding the serial output
}
