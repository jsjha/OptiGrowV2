// 
// VWW - Flower Classification
// 
// (C) 2024 Abir Hossain <abirh@seas.upenn.edu>
// (C) 2024 Jessi Jha <jessijha@seas.upenn.edu>
//   
// Licensed under MIT License
// 
// Description:
// This is a simple flower classification program using ESP32-S3 camera.
//

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "NeuralNetwork.h"

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

#define INPUT_W 96
#define INPUT_H 96
#define LED_BUILT_IN 21
#define BUTTON_PIN D1 // Replace with the actual pin number for the button
#define PUMP_PIN D6   // GPIO pin connected to the pump relay

#define DEBUG_TFLITE 0

NeuralNetwork *g_nn;

// Pump durations (time in milliseconds)
const unsigned long pumpDurationTulip = 10000; // 10 seconds
const unsigned long pumpDurationRose = 15000; // 15 seconds
const unsigned long pumpDurationSunflower = 10000; // 10 seconds
const unsigned long pumpDurationDaisy = 8000;  // 8 seconds
const unsigned long pumpDurationDandelion = 5000; // 5 seconds

void initializeCamera();
void initializeNeuralNetwork();
void captureAndClassify();
void waterPlant(const char *plantName, unsigned long duration);
int GetImage(camera_fb_t *fb, TfLiteTensor *input);
uint32_t rgb565torgb888(uint16_t color);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);

  while (!Serial) {
    static int retries = 0;
    delay(1000);
    if (retries++ > 5) {
      break;
    }
  }

  Serial.setDebugOutput(false);

  // Initialize the LED, button, and pump
  pinMode(LED_BUILT_IN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW); // Ensure pump is off initially

  initializeCamera();
  initializeNeuralNetwork();
}

void loop() {
  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce delay
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("Button pressed. Capturing image...");
      captureAndClassify();

      // Wait for the button to be released before continuing
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

// Initializes the camera
void initializeCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_96X96;
  config.pixel_format = PIXFORMAT_RGB565;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while (1);
  }
  Serial.println("Camera initialized successfully.");
}

// Initializes the neural network
void initializeNeuralNetwork() {
  Serial.println("Initializing neural network...");
  g_nn = new NeuralNetwork();
}

// Captures an image, performs classification, and waters the plant
void captureAndClassify() {
  for (int attempt = 1; attempt <= 2; attempt++) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    if (fb->format != PIXFORMAT_JPEG) {
      GetImage(fb, g_nn->getInput());

      g_nn->predict();

      const char *class_names[] = {"daisy", "dandelion", "roses", "sunflowers", "tulips"};
      float *output_data = g_nn->getOutput()->data.f;

      float max_prob = -1.0;
      int predicted_class = -1;
      for (int i = 0; i < 5; i++) {
        if (output_data[i] > max_prob) {
          max_prob = output_data[i];
          predicted_class = i;
        }
      }

      if (attempt == 2) {
        // Output the result of the second attempt
        const char *detectedFlower = class_names[predicted_class];
        Serial.printf("Prediction: %s with probability: %.3f\n\n", detectedFlower, max_prob);

        // Water the plant based on the detected flower type
        if (predicted_class == 0) {
          waterPlant("Daisy", pumpDurationDaisy);
        } else if (predicted_class == 1) {
          waterPlant("Dandelion", pumpDurationDandelion);
        } else if (predicted_class == 2) {
          waterPlant("Rose", pumpDurationRose);
        } else if (predicted_class == 3) {
          waterPlant("Sunflower", pumpDurationSunflower);
        } else if (predicted_class == 4) {
          waterPlant("Tulip", pumpDurationTulip);
        }
      }
    }

    esp_camera_fb_return(fb);
  }
}

// Function to water a plant for a specified duration
void waterPlant(const char *plantName, unsigned long duration) {
  Serial.print("Watering ");
  Serial.print(plantName);
  Serial.print(" for ");
  Serial.print(duration / 1000);
  Serial.println(" seconds.");

  digitalWrite(PUMP_PIN, HIGH); // Turn on the pump
  delay(duration);             // Keep the pump on for the specified duration
  digitalWrite(PUMP_PIN, LOW); // Turn off the pump

  Serial.println("Done watering.");
}

// Helper functions remain unchanged

uint32_t rgb565torgb888(uint16_t color) {
  uint8_t hb, lb;
  uint32_t r, g, b;

  lb = (color >> 8) & 0xFF;
  hb = color & 0xFF;

  r = (lb & 0x1F) << 3;
  g = ((hb & 0x07) << 5) | ((lb & 0xE0) >> 3);
  b = (hb & 0xF8);

  return (r << 16) | (g << 8) | b;
}

int GetImage(camera_fb_t *fb, TfLiteTensor *input) {
  int post = 0;
  int startx = (fb->width - INPUT_W) / 2;
  int starty = (fb->height - INPUT_H);
  for (int y = 0; y < INPUT_H; y++) {
    for (int x = 0; x < INPUT_W; x++) {
      int getPos = (starty + y) * fb->width + startx + x;
      uint16_t color = ((uint16_t *)fb->buf)[getPos];
      uint32_t rgb = rgb565torgb888(color);

      float *image_data = input->data.f;

      image_data[post * 3 + 0] = ((rgb >> 16) & 0xFF);
      image_data[post * 3 + 1] = ((rgb >> 8) & 0xFF);
      image_data[post * 3 + 2] = (rgb & 0xFF);

      post++;
    }
  }
  return 0;
}