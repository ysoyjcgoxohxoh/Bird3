#include "driver/twai.h"
#include "ADebouncer.h"
void IRAM_ATTR Timer0_ISR(void);

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
volatile uint32_t uptime = 1;

ADebouncer StartStopDebouncer;

uint8_t BMSOutputEnable[8] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t BMSOutputDisable[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t RearLightEnable[8] = { 0x01, 0x00, 0x00 };
uint8_t RearLightDisable[8] = { 0x00, 0x00, 0x00 };
volatile uint8_t bmsAlive[4] = { 0x2C, 0x00, 0x41, 0x00 }; // Read from 0x60C message sent by BMS

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
    if (!driver_installed) {

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
    } else {
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
}

void updateColor(uint8_t R, uint8_t G, uint8_t B) {
  // write the RGB values to the pins
  ledcWrite(1, 255 - R);  // write red component to channel 1, etc.
  ledcWrite(2, 255 - G);
  ledcWrite(3, 255 - B);
}

void sendCANMessage(uint32_t identifier, uint8_t length, uint8_t data[]) {
  if (driver_installed) {
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
  /*
  sendCANMessage(0x742, 8, keepaliveCodes[keepAliveIndex]);
  keepAliveIndex++;
  if (keepAliveIndex > keepAliveMax) {
    keepAliveIndex = keepAliveResetIndex;
  }*/

  if (driver_installed) {
    byte keepAlive[8];
    keepAlive[0] = (uint8_t)(uptime & 0xFF);
    keepAlive[1] = (uint8_t)((uptime >> 8) & 0xFF);
    keepAlive[2] = (uint8_t)((uptime >> 16) & 0xFF);
    keepAlive[3] = (uint8_t)((uptime >> 24) & 0xFF);

    if (uptime % 5 == 0) {
      keepAlive[4] = (uint8_t)bmsAlive[0];
      keepAlive[5] = (uint8_t)bmsAlive[1];
      keepAlive[6] = (uint8_t)bmsAlive[2];
      keepAlive[7] = (uint8_t)bmsAlive[3];
      
      Serial.print("Keepalive: 0x");
      uint8_t i;
      for (i = 0; i < 8; i++) {
        Serial.print(keepAlive[i], HEX);
        if (i < 7)
          Serial.print(", 0x");
      }
      Serial.println();
      sendCANMessage(0x742, 8, keepAlive);
    }
    uptime++;
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
  //Serial.print("Bottom:0 Top:4000 ThrottleA:");
  //Serial.print(ThrottleAFiltered);
  //Serial.print("  ThrottleB:");
  //Serial.print(ThrottleBFiltered);
  //Serial.print("  BrakeR:");
  //Serial.print(BrakeRFiltered);
  //Serial.print("  BrakeL:");
  //Serial.println(BrakeLFiltered);
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
}
