#include "driver/twai.h"
#include "ADebouncer.h"

// Define whether we're compiling for an EA00004 or an EA00022.
// Comment out the one we're compiling for
#define EA00004 1
//#define EA00022 1

// Communication Pins
#define CAN_RX_PIN 44
#define CAN_TX_PIN 43

// Inputs
#define THROTTLEA_PIN A0   // TR_WP1 - Increases as throttle is pressed
#define THROTTLEB_PIN A1   // TR_WP2 - Decreases as throttle is pressed
#define BRAKER_PIN A2      // Brake #1, Front Brake
#define BRAKEL_PIN A3      // Brake #2, Rear Brake
#define START_STOP_PIN D3  // Input

// Outputs
#define HEADLIGHT_PIN A6  // Output. PWM in future, but for now, just on/off

// Set up the rgb led names (Arduino Nano ESP32)
#define ledR 14
#define ledG 15
#define ledB 16


static bool driver_installed = false;

volatile uint16_t throttlePos = 0;
volatile bool rearLight = false;
volatile bool unlockBattery = false;
volatile bool brakeEnabled = false;
volatile bool scooterEnabled = false;
volatile int keepAliveIndex = 0;

ADebouncer StartStopDebouncer;

#define keepAliveResetIndex 5
#define keepAliveMax 40
#if EA00004
uint8_t keepaliveCodes[41][8] = {
  { 0x01, 0x00, 0xdd, 0x00, 0xcc, 0x35, 0xe0, 0x26 },
  { 0x02, 0x00, 0xdd, 0x43, 0xb3, 0x02, 0x40, 0x8b },
  { 0x03, 0x00, 0xdd, 0x43, 0x05, 0x46, 0x0f, 0x6c },
  { 0x04, 0x00, 0xdd, 0x43, 0x34, 0xe5, 0x5e, 0x8d },
  { 0x05, 0x00, 0xdd, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x06, 0x00, 0x83, 0x43, 0xb1, 0x49, 0xc5, 0xa3 },
  { 0x07, 0x00, 0x5f, 0x43, 0x66, 0xc8, 0x35, 0xf4 },
  { 0x08, 0x00, 0x59, 0x43, 0xe1, 0xfd, 0x31, 0xfa },
  { 0x09, 0x00, 0xc4, 0x43, 0x76, 0xc7, 0x18, 0xe0 },
  { 0x0a, 0x00, 0x64, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x0b, 0x00, 0x13, 0x43, 0x01, 0x2a, 0x41, 0xa7 },
  { 0x0c, 0x00, 0x44, 0x43, 0x14, 0x5b, 0x5e, 0x3a },
  { 0x0d, 0x00, 0x07, 0x43, 0xba, 0x86, 0xf7, 0x96 },
  { 0x0e, 0x00, 0xee, 0x43, 0x11, 0x3c, 0x37, 0x6a },
  { 0x0f, 0x00, 0xd0, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x10, 0x00, 0xc5, 0x43, 0xd6, 0x30, 0x20, 0x92 },
  { 0x11, 0x00, 0xb6, 0x43, 0x4c, 0xa9, 0xf2, 0x89 },
  { 0x12, 0x00, 0x70, 0x43, 0xf2, 0x1d, 0x15, 0x2a },
  { 0x13, 0x00, 0x23, 0x43, 0x05, 0x87, 0xa9, 0x46 },
  { 0x14, 0x00, 0x71, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x15, 0x00, 0x9c, 0x43, 0x79, 0x0a, 0x38, 0x91 },
  { 0x16, 0x00, 0xb6, 0x43, 0xa4, 0xb7, 0xf0, 0x76 },
  { 0x17, 0x00, 0x80, 0x43, 0xe4, 0xaf, 0x27, 0xfb },
  { 0x18, 0x00, 0x75, 0x43, 0x5c, 0x80, 0x47, 0xa3 },
  { 0x19, 0x00, 0x0f, 0x43, 0x03, 0x57, 0x43, 0x41 },
  { 0x1a, 0x00, 0x2f, 0x43, 0x8a, 0xa3, 0x4c, 0x6c },
  { 0x1b, 0x00, 0x21, 0x43, 0x36, 0xe7, 0x21, 0xc3 },
  { 0x1c, 0x00, 0x0f, 0x43, 0x79, 0xc0, 0xab, 0xba },
  { 0x1d, 0x00, 0x29, 0x43, 0xb6, 0x44, 0x3b, 0xce },
  { 0x1e, 0x00, 0x6a, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x1f, 0x00, 0xfc, 0x43, 0x66, 0x48, 0x45, 0xc7 },
  { 0x20, 0x00, 0xb5, 0x43, 0xc9, 0xfa, 0xdd, 0x9f },
  { 0x21, 0x00, 0xbf, 0x43, 0x68, 0x9b, 0x32, 0x4c },
  { 0x22, 0x00, 0x9b, 0x43, 0xeb, 0x43, 0xdf, 0x90 },
  { 0x23, 0x00, 0xf7, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x24, 0x00, 0x4c, 0x43, 0x96, 0x3c, 0x96, 0x04 },
  { 0x25, 0x00, 0xe2, 0x43, 0x48, 0x2f, 0x34, 0xb5 },
  { 0x26, 0x00, 0xdc, 0x43, 0x27, 0xfc, 0x06, 0xb0 },
  { 0x27, 0x00, 0xf5, 0x43, 0xd7, 0x21, 0x01, 0x97 },
  { 0x28, 0x00, 0xd7, 0x43, 0x28, 0x00, 0x2a, 0x00 },
  { 0x29, 0x00, 0x39, 0x43, 0xcd, 0x3c, 0x36, 0x2c }
};
#endif

#if EA00022
uint8_t keepaliveCodes[41][8] = {
  { 0x01, 0x00, 0xDD, 0x00, 0x61, 0x3D, 0x3F, 0x42 },
  { 0x02, 0x00, 0xDD, 0xC4, 0x26, 0xB6, 0x74, 0xC7 },
  { 0x03, 0x00, 0xDD, 0xC4, 0xE1, 0x0A, 0xA4, 0x98 },
  { 0x04, 0x00, 0xDD, 0xC4, 0xA2, 0xD1, 0x44, 0xF6 },
  { 0x05, 0x00, 0xDD, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x06, 0x00, 0x70, 0xC4, 0x91, 0xD9, 0x2E, 0x46 },
  { 0x07, 0x00, 0xCD, 0xC4, 0x86, 0xB1, 0x1A, 0xE5 },
  { 0x08, 0x00, 0x58, 0xC4, 0x3E, 0x3D, 0x93, 0x51 },
  { 0x09, 0x00, 0xC4, 0xC4, 0x51, 0x8D, 0xFC, 0x01 },
  { 0x0A, 0x00, 0xFA, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x0B, 0x00, 0x69, 0xC4, 0x4C, 0xBC, 0x75, 0xC6 },
  { 0x0C, 0x00, 0xAD, 0xC4, 0xC5, 0x11, 0x91, 0xE6 },
  { 0x0D, 0x00, 0x0C, 0xC4, 0x2B, 0x18, 0x0E, 0x3C },
  { 0x0E, 0x00, 0x7C, 0xC4, 0xEA, 0x78, 0xC5, 0x8F },
  { 0x0F, 0x00, 0x38, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x10, 0x00, 0x82, 0xC4, 0x5F, 0xDD, 0xCC, 0x92 },
  { 0x11, 0x00, 0x4B, 0xC4, 0x2C, 0x83, 0x84, 0x60 },
  { 0x12, 0x00, 0xF8, 0xC4, 0x3F, 0x9F, 0x68, 0xF0 },
  { 0x13, 0x00, 0x38, 0xC4, 0x83, 0xB5, 0xC1, 0x7E },
  { 0x14, 0x00, 0xBE, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x15, 0x00, 0x0F, 0xC4, 0x2E, 0x75, 0x66, 0x65 },
  { 0x16, 0x00, 0xB3, 0xC4, 0x27, 0x1B, 0x05, 0x82 },
  { 0x17, 0x00, 0xCE, 0xC4, 0x78, 0x5A, 0x92, 0xE4 },
  { 0x18, 0x00, 0x7A, 0xC4, 0xD9, 0x10, 0x5F, 0x71 },
  { 0x19, 0x00, 0x09, 0xC4, 0x09, 0x57, 0x43, 0x54 },
  { 0x1A, 0x00, 0x9B, 0xC4, 0xE6, 0x7F, 0xD3, 0x71 },
  { 0x1B, 0x00, 0xDA, 0xC4, 0xF1, 0xA1, 0x13, 0xC6 },
  { 0x1C, 0x00, 0xE6, 0xC4, 0x35, 0x46, 0x1B, 0xE5 },
  { 0x1D, 0x00, 0xC3, 0xC4, 0x69, 0x6E, 0x4C, 0xC8 },
  { 0x1E, 0x00, 0xED, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x1F, 0x00, 0x8D, 0xC4, 0xFA, 0x6E, 0xD8, 0x23 },
  { 0x20, 0x00, 0x15, 0xC4, 0xF8, 0xD5, 0x31, 0x94 },
  { 0x21, 0x00, 0x6E, 0xC4, 0x97, 0x47, 0x15, 0x55 },
  { 0x22, 0x00, 0x7A, 0xC4, 0x9D, 0x9F, 0xC9, 0xB3 },
  { 0x23, 0x00, 0xD3, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x24, 0x00, 0x51, 0xC4, 0x5C, 0x83, 0xA7, 0x8D },
  { 0x25, 0x00, 0x95, 0xC4, 0xA3, 0x58, 0xCB, 0x33 },
  { 0x26, 0x00, 0x8E, 0xC4, 0xDC, 0xB4, 0xE0, 0x3A },
  { 0x27, 0x00, 0x4D, 0xC4, 0x98, 0xD3, 0x77, 0xD3 },
  { 0x28, 0x00, 0x97, 0xC4, 0x2C, 0x00, 0x41, 0x00 },
  { 0x29, 0x00, 0x92, 0xC4, 0xB7, 0xBE, 0xAE, 0x1A }
};
#endif
uint8_t BMSOutputEnable[8] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t BMSOutputDisable[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t RearLightEnable[8] = { 0x01, 0x00, 0x00 };
uint8_t RearLightDisable[8] = { 0x00, 0x00, 0x00 };

volatile double TorqueOutputMax = 900L;
#define TorqueOutputReference 800L

// https://deepbluembedded.com/esp32-timers-timer-interrupt-tutorial-arduino-ide/
hw_timer_t *twoMSTimer = NULL;
volatile uint16_t timerCounter;
#define timerCounterMax 5000
#define timerIntervalMS 2

// ADC Filtering
#define FILTER_LEN 50
volatile uint8_t filterIndex = 0;

uint32_t ThrottleA_Buffer[FILTER_LEN] = { 0 };
uint32_t ThrottleB_Buffer[FILTER_LEN] = { 0 };
uint32_t BrakeR_Buffer[FILTER_LEN] = { 0 };
uint32_t BrakeL_Buffer[FILTER_LEN] = { 0 };

#define ThrottleAADCMin 1100
#define ThrottleAADCMax 3100
#define ThrottleBADCMin 1100
#define ThrottleBADCMax 3100
#define ThrottlePositionCount 1000

#define BrakeRADCMin 0
#define BrakeRADCMax 0
#define BrakeLADCMin 0
#define BrakeLADCMax 0
#define BrakePositionCount 1000

volatile uint32_t ThrottleAFiltered = 0;
volatile uint32_t ThrottleBFiltered = 0;
volatile uint32_t BrakeRFiltered = 0;
volatile uint32_t BrakeLFiltered = 0;



uint32_t readADC_Avg(uint8_t pinNumber, uint32_t buffer[], uint32_t adcMin, uint32_t adcMax, uint32_t outputMax) {
  // Read and Average
  int i = 0;
  uint32_t Sum = 0;
  uint32_t raw = analogRead(pinNumber);
  //Serial.print("Raw: ");
  //Serial.println(raw);


  buffer[filterIndex] = raw;
  for (i = 0; i < FILTER_LEN; i++) {
    Sum += buffer[i];
  }
  auto avg = Sum / FILTER_LEN;

  // If a range is specified
  if (adcMin != 0 || adcMax != 0) {
    // We have a range
    // Ensure it's within limits
    if (avg < adcMin) return 0;               // We can short-circuit here since if the average is below the minimum, the result is certain to be zero.
    else if (avg > adcMax) return outputMax;  // We can also shortcircuit here for the same reason.

    // Here's where the real calculations happen. We have a number between two numbers,
    // and we need to calculate where it is between those two numbers on a scale of 0 to `outputMax`
    return (avg - adcMin) * outputMax / (adcMax - adcMin);

  } else {
    // No range defined. Return the raw average.
    return avg;
  }
}


void loop() {
  // Read our analog inputs
  ThrottleAFiltered = readADC_Avg(THROTTLEA_PIN, ThrottleA_Buffer, ThrottleAADCMin, ThrottleAADCMax, ThrottlePositionCount);
  ThrottleBFiltered = 1000 - readADC_Avg(THROTTLEB_PIN, ThrottleB_Buffer, ThrottleBADCMin, ThrottleBADCMax, ThrottlePositionCount);
  BrakeRFiltered = readADC_Avg(BRAKER_PIN, BrakeR_Buffer, BrakeRADCMin, BrakeRADCMax, BrakePositionCount);
  BrakeLFiltered = readADC_Avg(BRAKEL_PIN, BrakeL_Buffer, BrakeLADCMin, BrakeLADCMax, BrakePositionCount);

  filterIndex++;
  if (filterIndex == FILTER_LEN) {
    filterIndex = 0;
  }
  // TODO: Based on the two values, Figure out if the throttle has borked, and do something sensible
  throttlePos = ThrottleAFiltered;
  StartStopDebouncer.debounce(digitalRead(START_STOP_PIN));
  if (StartStopDebouncer.falling()) {
    // TODO: Authentication of some sort
    scooterEnabled = !scooterEnabled;
    if (scooterEnabled) {
      updateColor(0, 255, 0);
      digitalWrite(HEADLIGHT_PIN, HIGH);
      unlockBattery = true;
      rearLight = true;
    } else {
      updateColor(255, 0, 0);
      digitalWrite(HEADLIGHT_PIN, LOW);
      unlockBattery = false;
      rearLight = false;
    }
  }
}

void updateColor(uint8_t R, uint8_t G, uint8_t B) {
  // write the RGB values to the pins
  ledcWrite(1, 255 - R);  // write red component to channel 1, etc.
  ledcWrite(2, 255 - G);
  ledcWrite(3, 255 - B);
}

void sendCANMessage(uint32_t identifier, uint8_t length, uint8_t data[]) {
  // Configure message to transmit
  twai_message_t message;
  message.extd = 0;
  message.flags = 0;
  message.rtr = 0;
  message.identifier = identifier;
  message.data_length_code = length;
  for (int i = 0; i < length; i++) {
    message.data[i] = data[i];
  }
  auto result = twai_transmit(&message, pdMS_TO_TICKS(1));
  // Queue message for transmission
  if (result != ESP_OK) {
    Serial.print("Failed to queue message for transmission: ");
    Serial.println(result);
  }
}

void sendMotorTorque() {

  union {
    uint32_t x;
    byte myData[4];  //little endian
  } data;
  if (scooterEnabled) {
    double throttleF = pow((double)throttlePos, 2.2L) / 5700L;
    throttleF = throttleF * TorqueOutputMax / TorqueOutputReference;
    data.x = lround(throttleF);
  } else {
    data.x = 0;
  }
  sendCANMessage(0x153, 4, data.myData);
}

void sendKeepAlive() {
  sendCANMessage(0x742, 8, keepaliveCodes[keepAliveIndex]);
  keepAliveIndex++;
  if (keepAliveIndex > keepAliveMax) {
    keepAliveIndex = keepAliveResetIndex;
  }
}

void sendBMSEnable() {
  if (unlockBattery) {
    sendCANMessage(0x140, 8, BMSOutputEnable);
  } else {
    sendCANMessage(0x140, 8, BMSOutputDisable);
  }
}

void sendRearLight() {
  if (rearLight) {
    sendCANMessage(0x500, 3, RearLightEnable);
  } else {
    sendCANMessage(0x500, 3, RearLightDisable);
  }
}

void sendDebugVariables() {
  Serial.print("Bottom:0 Top:4000 ThrottleA:");
  Serial.print(ThrottleAFiltered);
  Serial.print("  ThrottleB:");
  Serial.print(ThrottleBFiltered);
  Serial.print("  BrakeR:");
  Serial.print(BrakeRFiltered);
  Serial.print("  BrakeL:");
  Serial.println(BrakeLFiltered);
}

void IRAM_ATTR Timer0_ISR() {
  // This timer runs every 2ms in order to send the current motor speed.
  // There are other things we need to do, but since the ESP32 has a limited
  // number of timers, we don't wish to use them all for the various messages
  // we need to send at different intervals. Instead, we simply increment a
  // variable and check if it's divisible by the desired interval.

  sendMotorTorque();

  // Every 50 ms
  if (timerCounter % 50 == 0) {
    sendRearLight();
    sendDebugVariables();
  }

  // Every 200 ms
  if (timerCounter % 200 == 0) {
    sendBMSEnable();
  }

  // Every 1000 ms
  if (timerCounter % 1000 == 0) {
    sendKeepAlive();
  }

  timerCounter += timerIntervalMS;
  if (timerCounter > timerCounterMax) {
    timerCounter = 0;
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  // Configure the Start/Stop button
  pinMode(START_STOP_PIN, INPUT_PULLUP);
  StartStopDebouncer.mode(DELAYED, 10, false);

  // Configure the headlight pin
  pinMode(HEADLIGHT_PIN, OUTPUT);
  // Set up the RGB LED
  ledcAttachPin(ledR, 1);  // assign RGB led pins to channels
  ledcAttachPin(ledG, 2);
  ledcAttachPin(ledB, 3);

  // Initialize channels
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(1, 12000, 8);  // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);
  updateColor(255, 0, 0);

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NO_ACK);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  //Look in the api-reference for other speed sets.
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  f_config.acceptance_mask = 0;
  
  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    //return;
  }
  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    Serial.println("Driver started");
    // TWAI driver is now successfully installed and started
    driver_installed = true;
  } else {
    Serial.println("Failed to start driver");
  }


  // Setup the timer https://deepbluembedded.com/esp32-timers-timer-interrupt-tutorial-arduino-ide/
  twoMSTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(twoMSTimer, &Timer0_ISR, true);
  timerAlarmWrite(twoMSTimer, timerIntervalMS * 1000, true);
  timerAlarmEnable(twoMSTimer);
}
