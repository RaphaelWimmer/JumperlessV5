// SPDX-License-Identifier: MIT

#include "Peripherals.h"

#include "Adafruit_INA219.h"

#include "LEDs.h"
#include "MatrixStateRP2040.h"
#include "NetManager.h"

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <Arduino.h>
#include <stdio.h>

#include "MCP_DAC.h"
#include "mcp4725.hpp"
// #include "AdcUsb.h"

#include <Adafruit_MCP4728.h>

#include <SPI.h>
 #include "MCP23S17.h"
//  #include "SPI.h"

#include "PersistentStuff.h"
#include <Adafruit_MCP23X17.h>
#include <Wire.h>
#include "FileParsing.h"

#include "CH446Q.h"
#include "Probing.h"
#include "hardware/adc.h"
#include "Commands.h"
#define CS_PIN 17

// MCP23S17 MCPIO(17, 16, 19, 18, 0x27); //  SW SPI address 0x00

 MCP23S17 MCPIO(17, 7,  &SPI); //  HW SPI address 0x00 //USE HW SPI
//Adafruit_MCP23X17 MCPIO;

// MCP23S17 MCPIO(17, 16, 19, 18, 0x7); //  SW SPI address 0x
#define CSI Serial.write("\x1B\x5B");
// #define CSI Serial.write("\033");

#define DAC_RESOLUTION 9

float adcRange[4][2] = {{0, 5}, {0, 5}, {0, 5}, {-8, 8}};
float dacOutput[2] = {0, 0};
float railVoltage[2] = {0, 0};
uint8_t gpioState[10] = {
    2, 2, 2, 2, 2,
    2, 2, 2, 2, 2}; // 0 = output low, 1 = output high, 2 = input, 3 = input
                    // pullup, 4 = input pulldown
uint8_t gpioReading[10] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3}; // 0 = low, 1 = high 2 = floating 3 = unknown

int gpioNet[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

int revisionNumber = 0;

int showReadings = 0;

int showADCreadings[4] = {1, 1, 1, 1};

int inaConnected = 0;
int showINA0[3] = {1, 1, 1}; // 0 = current, 1 = voltage, 2 = power
int showINA1[3] = {0, 0, 0}; // 0 = current, 1 = voltage, 2 = power

int showDAC0 = 0;
int showDAC1 = 0;

int adcCalibration[4][3] = {{0, 0, 0},
                            {0, 0, 0},
                            {0, 0, 0},
                            {0, 0, 0}}; // 0 = min, 1 = middle, 2 = max,
int dacCalibration[4][3] = {{0, 0, 0},
                            {0, 0, 0},
                            {0, 0, 0},
                            {0, 0, 0}}; // 0 = min, 1 = middle, 2 = max,

float freq[3] = {1, 1, 0};
uint32_t period[3] = {0, 0, 0};
uint32_t halvePeriod[3] = {0, 0, 0};

// q = square
// s = sinus
// w = sawtooth
// t = stair
// r = random
char mode[3] = {'z', 'z', 'z'};

int dacOn[3] = {0, 0, 0};
int amplitude[3] = {4095, 3763, 0};
int offset[3] = {2047, 2380, 2047};
int calib[3] = {-10, 0, 0};

Adafruit_MCP4728 mcp;

// MCP4725_PICO dac0_5V(5.0);
// MCP4725_PICO dac1_8V(18.0);

// MCP4822 dac_rev3; // A is dac0  B is dac1

INA219 INA0(0x40);
INA219 INA1(0x41);

uint16_t count;
uint32_t lastTime = 0;

// LOOKUP TABLE SINE
uint16_t sine0[360];
uint16_t sine1[360];

void initGPIOex(void) {

  // pinMode(17, OUTPUT);
  // pinMode(16, OUTPUT);
  // pinMode(19, OUTPUT);
  // pinMode(18, OUTPUT);
delay(30);
  SPI.setRX(16);
  SPI.setTX(19);
  SPI.setSCK(18);
  SPI.setCS(17);


  

  SPI.begin();

  // if (MCPIO.begin_SPI(CS_PIN, &SPI, 0b111) == false) {
  //   delay(3000);
  //   Serial.println("MCP23S17 not found");
  // }
  
    MCPIO.setSPIspeed(8000000);
MCPIO.begin(false);
delay(20);


  if (MCPIO.isConnected() == false) {
    delay(3000);
    Serial.println("MCP23S17 not found");
    
  }


  Serial.println(MCPIO.getSPIspeed());
  Serial.println(MCPIO.usesHWSPI());
//MCPIO.isConnected();
  //   while (testConnection(MCPIO) != 0)
  // {
  //   Serial.println("MCP23S17 not found");
  //   delay(10);
  // }

  // MCPIO.enableAddrPins();
  // MCPIO.enableAddrPins();

MCPIO.pinMode16(0b0000000000000000);

  // MCPIO.pinMode1(0, OUTPUT);
  // MCPIO.pinMode1(1, OUTPUT);
  // MCPIO.pinMode1(2, OUTPUT);
  // MCPIO.pinMode1(3, OUTPUT);

  // MCPIO.pinMode1(4, INPUT);
  // MCPIO.pinMode1(5, INPUT);
  // MCPIO.pinMode1(6, INPUT);
  // MCPIO.pinMode1(7, INPUT);
  // MCPIO.pinMode1(8, OUTPUT);
  // MCPIO.pinMode1(9, OUTPUT);
  // MCPIO.pinMode1(10, OUTPUT);
  // MCPIO.pinMode1(11, OUTPUT);
  // MCPIO.pinMode1(12, OUTPUT);
  // MCPIO.pinMode1(13, OUTPUT);
  // MCPIO.pinMode1(14, OUTPUT);
  // MCPIO.pinMode1(15, OUTPUT);
MCPIO.write16(0b0000000000000000);
  // MCPIO.writeGPIOAB(0x0000);

  // for (int i = 0; i < 106; i++)
  // {
  //   writeGPIOex(i%2, i);
  //   delay(10);
  // }

  // MCPIO.pinMode8(1, 0x00);
}

int reads2[30][2];
int readIndex2 = 0;

int foundRolloff = 0;

int lastReading = 0;
int dacSetting = 0;
int reads[30][2];
int readIndex = 0;

void initADC(void) {

  pinMode(ADC0_PIN, INPUT);
  pinMode(ADC1_PIN, INPUT);
  pinMode(ADC2_PIN, INPUT);
  pinMode(ADC3_PIN, INPUT);

  analogReadResolution(12);
}

void printCalibration(void) {
  // delay(1000);
  return;
  digitalWrite(RESETPIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(RESETPIN, LOW);
  delayMicroseconds(100);

  Serial.println("ADC 1 (0-5V)");

  // sendXYraw(CHIP_K, 7, 0, 1);
  // sendXYraw(CHIP_K, 9, 0, 1);

  // sendXYraw(CHIP_K, 0, 0, 1);

  sendXYraw(CHIP_K, 7, 0, 1);
  sendXYraw(CHIP_K, 9, 0, 1);

  sendXYraw(CHIP_K, 0, 0, 1);

  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_A, 0);
  digitalWrite(8, LOW);

  for (int i = 0; i <= 32; i++) {
    digitalWrite(8, HIGH);
    mcp.setChannelValue(MCP4728_CHANNEL_A, i * 127);
    digitalWrite(8, LOW);
    delay(10);

    Serial.print(INA1.getBusVoltage());
    Serial.print(" ");
    delay(10);

    Serial.println(readAdc(1, 16));
    delay(20);
  }
  sendXYraw(CHIP_K, 7, 0, 0);
  sendXYraw(CHIP_K, 9, 0, 0);

  sendXYraw(CHIP_K, 0, 0, 0);
  delay(100);

  digitalWrite(RESETPIN, HIGH);
  delayMicroseconds(800);
  digitalWrite(RESETPIN, LOW);
  delayMicroseconds(10000);

  Serial.println("ADC 2 +-8V");

  // sendXYraw(CHIP_K, 7, 0, 1);
  // sendXYraw(CHIP_K, 9, 0, 1);

  // sendXYraw(CHIP_K, 0, 0, 1);

  sendXYraw(CHIP_K, 6, 0, 1);
  sendXYraw(CHIP_K, 10, 0, 1);

  sendXYraw(CHIP_K, 0, 0, 1);

  // digitalWrite(8, HIGH);
  // mcp.setChannelValue(MCP4728_CHANNEL_B, 0);
  // digitalWrite(8, LOW);

  for (int i = 0; i <= 32; i++) {

    Serial.print(i * 127);
    Serial.print(" ");
    digitalWrite(8, HIGH);
    mcp.setChannelValue(MCP4728_CHANNEL_B, i * 127);
    digitalWrite(8, LOW);
    delay(50);

    // Serial.print(INA0.getBusVoltage());
    //   Serial.print(" ");
    //   delay(100);

    Serial.println(readAdc(2, 16));
    delay(10);
  }

  digitalWrite(RESETPIN, HIGH);
  delayMicroseconds(800);
  digitalWrite(RESETPIN, LOW);
  delayMicroseconds(10000);

  sendXYraw(CHIP_K, 6, 0, 1);
  sendXYraw(CHIP_K, 13, 0, 1);

  sendXYraw(CHIP_K, 0, 0, 1);

  sendXYraw(CHIP_I, 15, 5, 1);
  sendXYraw(CHIP_I, 0, 5, 1);

  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_B, 4095);
  digitalWrite(8, LOW);
  Serial.print("INA 0 ");
  Serial.print("Bus Voltage: ");

  Serial.println(INA0.getBusVoltage());
  Serial.print("INA 1 ");
  Serial.print("Bus Voltage: ");

  Serial.println(INA1.getBusVoltage());

  // for (int i = 0; i < readIndex; i++) {
  //   Serial.print("[");
  //   Serial.print(reads[i][0]);
  //   Serial.print(" ");
  //   Serial.print(reads[i][1]);
  //   Serial.println("]");
  // }

  //   for (int i = 0; i < readIndex2; i++) {
  //   Serial.print("[");
  //   Serial.print(reads2[i][0]);
  //   Serial.print(" ");
  //   Serial.print(reads2[i][1]);
  //   Serial.println("]");
  // }

  for (int i = 0; i < 3; i++) {

    Serial.print("ADC1 ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(adcCalibration[1][i]);
  }

  for (int i = 0; i < 3; i++) {

    Serial.print("ADC2 ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(adcCalibration[2][i]);
  }

  for (int i = 0; i < 3; i++) {

    Serial.print("ADC3 ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(adcCalibration[3][i]);
  }

  digitalWrite(RESETPIN, HIGH);
  delayMicroseconds(1000);
  digitalWrite(RESETPIN, LOW);
  delayMicroseconds(100);
}
void initDAC(void) {

  Wire.setSDA(4);
  Wire.setSCL(5);

  Wire.begin();
  delay(5);
  // Try to initialize!
  if (!mcp.begin()) {
    delay(3000);
    Serial.println("Failed to find MCP4728 chip");
    // while (1)
    // {
    //   delay(10);
    // }
  }
  // delay(1000);
  Serial.println("MCP4728 Found!");
  /*
   * @param channel The channel to update
   * @param new_value The new value to assign
   * @param new_vref Optional vref setting - Defaults to `MCP4728_VREF_VDD`
   * @param new_gain Optional gain setting - Defaults to `MCP4728_GAIN_1X`
   * @param new_pd_mode Optional power down mode setting - Defaults to
   * `MCP4728_PD_MOOE_NORMAL`
   * @param udac Optional UDAC setting - Defaults to `false`, latching (nearly).
   * Set to `true` to latch when the UDAC pin is pulled low
   *
   */
  pinMode(8, OUTPUT);

  // saveVoltages(railVoltage[0], railVoltage[1], dacOutput[0], dacOutput[1]);
  //  digitalWrite(8, HIGH);

  // // Vref = MCP_VREF_VDD, value = 0, 0V
  // mcp.setChannelValue(MCP4728_CHANNEL_A, 0);
  // mcp.setChannelValue(MCP4728_CHANNEL_B, 1650);
  // mcp.setChannelValue(MCP4728_CHANNEL_C, 1660);
  // mcp.setChannelValue(MCP4728_CHANNEL_D, 1641); // 1650 is roughly 0V
  // digitalWrite(8, LOW);

  mcp.saveToEEPROM();
}

void initI2C(void) {
  return;
  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  Wire1.setSDA(22);
  Wire1.setSCL(23);
  Wire1.begin();
}



int i2cScan(int sdaRow, int sclRow)
{
  //return 0;
initI2C();
removeBridgeFromNodeFile(RP_GPIO_22, -1, netSlot);
removeBridgeFromNodeFile(RP_GPIO_23, -1, netSlot);

addBridgeToNodeFile(RP_GPIO_22, sdaRow, netSlot);
addBridgeToNodeFile(RP_GPIO_23, sclRow, netSlot);
  clearAllNTCC();
  digitalWrite(RESETPIN, HIGH);
  openNodeFile(netSlot);

  getNodesToConnect();

  bridgesToPaths();
digitalWrite(RESETPIN, LOW);
  assignNetColors();

  showLEDsCore2 = 1;

  delay(5);
  sendAllPathsCore2 = 1;
delay(200);
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire1.beginTransmission(address);
    error = Wire1.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
  removeBridgeFromNodeFile(RP_GPIO_22, sdaRow, netSlot);
  removeBridgeFromNodeFile(RP_GPIO_23, sclRow, netSlot);



  return count;
  







}







void setGPIO(void) {
//     // Serial.print(19+i);
//     // Serial.print(" ");
//     // Serial.println(gpioState[i]);
//     Serial.println("\n\n\n\rsetGPIO\n\n\n\n\n\r");
// return;
  for (int i = 0; i < 4; i++) {
    switch (gpioState[i]) {
    case 0:
      pinMode(19 + i, OUTPUT);
      digitalWrite(19 + i, LOW);
      break;
    case 1:
      pinMode(19 + i, OUTPUT);
      digitalWrite(19 + i, HIGH);

      break;
    case 2:
      pinMode(19 + i, INPUT);
      break;
    case 3:
      pinMode(19 + i, INPUT_PULLUP);
      break;
    case 4:
      pinMode(19 + i, INPUT_PULLDOWN);
      break;
    }

  }

  // for (int i = 4; i <= 8; i++) {
  //   switch (gpioState[i]) {
  //   case 0:
  //     MCPIO.pinMode(i, OUTPUT);
  //     MCPIO.digitalWrite(i, LOW);
  //     break;
  //   case 1:
  //     MCPIO.pinMode(i, OUTPUT);
  //     MCPIO.digitalWrite(i, HIGH);
  //     break;
  //   case 2:
  //     MCPIO.pinMode(i, INPUT);
  //     break;
  //   case 3:
  //     MCPIO.pinMode(i, INPUT_PULLUP);

  //     break;
  //   }
  //}
  rgbColor onColor = {254, 40, 35};
  rgbColor offColor = {0, 254, 48};

  for (int i = 1; i <= 8; i++) {
    if (gpioNet[i] != -1 && gpioState[i] != 2 && gpioState[i] != 3 &&
        gpioState[i] != 4) {

      net[gpioNet[i]].color = (gpioState[i] == 1 ? onColor : offColor);
      net[gpioNet[i]].machine = 1;
      //net[gpioNet[i]].color = (gpioState[i] == 1 ? onColor : offColor);
      lightUpNet(gpioNet[i], -1, 1, 05, 0, 0,
                 packRgb(net[gpioNet[i]].color.r, net[gpioNet[i]].color.g,
                         net[gpioNet[i]].color.b));
    }
  }
}

void readGPIO(void) {
      // Serial.println("\n\n\n\rreadGPIO\n\n\n\n\n\r");
      // return;
  for (int i = 0; i < 4; i++) {
    if (gpioNet[i] != -1 &&
        (gpioState[i] == 2 || gpioState[i] == 3 || gpioState[i] == 4)) {
      int reading = 0;//readFloatingOrState(20 + i);
      switch (reading) {

      case 0:
        gpioReading[i] = 2;
        break;
      case 1:

        gpioReading[i] = 1;
        break;
      case 2:

        gpioReading[i] = 0;
        break;
      }
    } else {
      gpioReading[i] = 3;
    }
    // gpioReading[i] = digitalRead(20 + i);
  }

  for (int i = 0; i < 4; i++) {

    if (gpioNet[i + 4] != -1 &&
        (gpioState[i + 4] == 2 || gpioState[i + 4] == 3 ||
         gpioState[i + 4] == 4)) {
      int reading = 0;//readFloatingOrStateMCP(i);
      switch (reading) {

      case 0:
        gpioReading[i + 4] = 2;
        break;
      case 1:

        gpioReading[i + 4] = 1;
        break;
      case 2:

        gpioReading[i + 4] = 0;
        break;
      }
    } else {
      gpioReading[i + 4] = 3;
    }
  }

  for (int i = 0; i < 8; i++) {

    if (gpioNet[i] != -1) {
      if (gpioReading[i] == 0) {
        lightUpNet(gpioNet[i], -1, 1, 5, 0, 0, 0x000f05);

      } else if (gpioReading[i] == 1) {
        lightUpNet(gpioNet[i], -1, 1, 22, 0, 0, 0x220005);
      } else {
        lightUpNet(gpioNet[i]);
      }
    }
    // Serial.print(gpioReading[i]);
    // Serial.print(" ");
  }
  showLEDsCore2 = 2;
  // Serial.println();
}

int readFloatingOrStateMCP(int pin) {
  return 0;
  pin = pin + 3;
  enum measuredState state = unknownState;
  // enum measuredState state2 = floating;

  // int readingPullup = 0;
  // int readingPullup2 = 0;
  // int readingPullup3 = 0;

  // int readingPulldown = 0;
  // int readingPulldown2 = 0;
  // int readingPulldown3 = 0;

  // MCPIO.pinMode(pin, INPUT_PULLUP);

  // delayMicroseconds(10);

  // readingPullup = MCPIO.digitalRead(pin);

  // MCPIO.pinMode(pin, OUTPUT);
  // MCPIO.digitalWrite(pin, LOW);
  // MCPIO.pinMode(pin, INPUT);

  // delayMicroseconds(10);

  // readingPulldown = MCPIO.digitalRead(pin);

  // if (readingPullup == 1 && readingPulldown == 0) {

  //   state = floating;
  // } else if (readingPullup == 1 && readingPulldown == 1) {

  //   state = high;
  // } else if (readingPullup == 0 && readingPulldown == 0) {

  //   state = low;
  // } else if (readingPullup == 0 && readingPulldown == 1) {
  //   // Serial.print("shorted");
  // }

  // return state;
}

void setRailsAndDACs(void) {
  setTopRail(railVoltage[0], 0);
  setBotRail(railVoltage[1], 0);
  setDac0_5Vvoltage(dacOutput[0]);
  setDac1_8Vvoltage(dacOutput[1]);
}
void setTopRail(float value, int save) {

  int dacValue = (value * 4095 / 19.8) + 1660;

  // if (value < 0)
  // {
  //   dacValue -= 2048;
  // } else {
  //   dacValue += 2048;
  // }

  if (dacValue > 4095) {
    dacValue = 4095;
  } else if (dacValue < 0) {
    dacValue = 0;
  }

  //     if (value < -6)
  // {
  //   dacValue = 0;
  // }
  railVoltage[0] = value;
  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_C, dacValue);
  digitalWrite(8, LOW);
  if (save) {
    saveVoltages(railVoltage[0], railVoltage[1], dacOutput[0], dacOutput[1]);
  }
}

void setBotRail(float value, int save) {

  int dacValue = (value * 4095 / 19.8) + 1641;

  if (dacValue > 4095) {
    dacValue = 4095;
  } else if (dacValue < 0) {
    dacValue = 0;
  }

  railVoltage[1] = value;
  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_D, dacValue);
  digitalWrite(8, LOW);
  if (save) {
    saveVoltages(railVoltage[0], railVoltage[1], dacOutput[0], dacOutput[1]);
  }
}

void setTopRail(int value, int save) {
  if (value > 4095) {
    value = 4095;
  }
  mcp.setChannelValue(MCP4728_CHANNEL_C, value);
}
void setBotRail(int value, int save) {
  if (value > 4095) {
    value = 4095;
  }
  mcp.setChannelValue(MCP4728_CHANNEL_D, value);
}
uint16_t lastInputCode0 = 0;
uint16_t lastInputCode1 = offset[1] + calib[1];

void setDac0_5Vvoltage(float voltage) {
  int dacValue = (voltage * 4095 / 5.0);

  Serial.println(voltage);
  if (dacValue > 4095) {
    dacValue = 4095;
  }
  dacOutput[0] = voltage;

  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_A, dacValue);
  digitalWrite(8, LOW);
  saveVoltages(railVoltage[0], railVoltage[1], dacOutput[0], dacOutput[1]);
}

void setDac0_5VinputCode(uint16_t inputCode) {
  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_A, inputCode);
  digitalWrite(8, LOW);
}

void setDac1_8Vvoltage(float voltage) {

  int dacValue = (voltage * 4095 / 19.8) + 1641;

  if (dacValue > 4095) {
    dacValue = 4095;
  }
  if (dacValue < 0) {
    dacValue = 0;
  }
  dacOutput[1] = voltage;
  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_B, dacValue);
  digitalWrite(8, LOW);
  saveVoltages(railVoltage[0], railVoltage[1], dacOutput[0], dacOutput[1]);
}

void setDac1_8VinputCode(uint16_t inputCode) {
  digitalWrite(8, HIGH);
  mcp.setChannelValue(MCP4728_CHANNEL_B, inputCode);
  digitalWrite(8, LOW);
}

uint8_t csToPin[16] = {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7};
//uint8_t csToPin[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
uint16_t chipMask[16] = {
  0b0000000000000001, 0b0000000000000010, 0b0000000000000100, 0b0000000000001000,
  0b0000000000010000, 0b0000000000100000, 0b0000000001000000, 0b0000000010000000,
  0b0000000100000000, 0b0000001000000000, 0b0000010000000000, 0b0000100000000000,
  0b0001000000000000, 0b0010000000000000, 0b0100000000000000, 0b1000000000000000};


void setCSex(int chip, int value) {



 //delayMicroseconds(300);

  if (value > 0) {
    // MCPIO.digitalWrite(csToPin[chip], HIGH);
   //MCPIO.write16(0b1111111111111111);
    MCPIO.write16(chipMask[chip]);
    // Serial.println(chipMask[chip]);
    // Serial.println(" ");
    delayMicroseconds(200);
  } else {
    MCPIO.write16(0b0000000000000000);
    delayMicroseconds(100);
   ///MCPIO.digitalWrite(csToPin[chip], LOW);
  }
// } else {
//  //MCPIO.digitalWrite(csToPin[i], LOW);
// }
// }
     //delayMicroseconds(300);

}

void writeGPIOex(int value, uint8_t pin) {
//MCPIO.begin(false);
  //MCPIO.write1(pin, value);
  switch (value) {
  case 0:
    MCPIO.write16(0b0000000000000000);
    break;
  case 1:
    MCPIO.write16(0b1111111111111111);
    break;
  }

  // switch (pin) {
  // case 1:
  // case 5:
  //   if (value == 0) {
  //     MCPIO.digitalWrite(0, LOW);
  //   } else {
  //     MCPIO.digitalWrite(0, HIGH);
  //   }
  //   break;
  // case 2:
  // case 6:
  //   if (value == 0) {
  //     MCPIO.digitalWrite(1, LOW);
  //   } else {
  //     MCPIO.digitalWrite(1, HIGH);
  //   }
  //   break;
  // case 3:
  // case 7:
  //   if (value == 0) {
  //     MCPIO.digitalWrite(2, LOW);
  //   } else {
  //     MCPIO.digitalWrite(2, HIGH);
  //   }
  //   break;
  // case 4:
  // case 8:
  //   if (value == 0) {
  //     MCPIO.digitalWrite(3, LOW);
  //   } else {
  //     MCPIO.digitalWrite(3, HIGH);
  //   }
  // }
}

void initINA219(void) {

  // delay(3000);
  // Serial.println(__FILE__);
  // Serial.print("INA219_LIB_VERSION: ");
  // Serial.println(INA219_LIB_VERSION);

  // Wire.begin();
  // Wire.setClock(1000000);
  Wire.begin();

  if (!INA0.begin() || !INA1.begin()) {
    Serial.println("Failed to find INA219 chip");
  }

  INA0.setMaxCurrentShunt(1, 2.0);
  INA1.setMaxCurrentShunt(1, 2.0);

  Serial.println(INA0.setBusVoltageRange(16));
  Serial.println(INA1.setBusVoltageRange(16));
}

void dacSine(int resolution) {
  uint16_t i;
  switch (resolution) {
  case 0 ... 5:
    for (i = 0; i < 32; i++) {
      setDac1_8VinputCode(DACLookup_FullSine_5Bit[i]);
    }
    break;
  case 6:
    for (i = 0; i < 64; i++) {
      setDac1_8VinputCode(DACLookup_FullSine_6Bit[i]);
    }
    break;
  case 7:
    for (i = 0; i < 128; i++) {
      setDac1_8VinputCode(DACLookup_FullSine_7Bit[i]);
    }
    break;

  case 8:
    for (i = 0; i < 256; i++) {
      setDac1_8VinputCode(DACLookup_FullSine_8Bit[i]);
    }
    break;

  case 9 ... 12:
    for (i = 0; i < 512; i++) {
      setDac1_8VinputCode(DACLookup_FullSine_9Bit[i]);
    }
    break;
  }
}

void chooseShownReadings(void) {
  showADCreadings[0] = 0;
  showADCreadings[1] = 0;
  showADCreadings[2] = 0;
  showADCreadings[3] = 0;

  showINA0[0] = 0;
  showINA0[1] = 0;
  showINA0[2] = 0;

  inaConnected = 0;

  for (int i = 0; i <= newBridgeIndex; i++) {

    if (path[i].node1 == ADC0 || path[i].node2 == ADC0) {
      showADCreadings[0] = path[i].net;
    }

    if (path[i].node1 == ADC1 || path[i].node2 == ADC1) {
      showADCreadings[1] = path[i].net;
    }

    if (path[i].node1 == ADC2 || path[i].node2 == ADC2) {
      showADCreadings[2] = path[i].net;
    }

    if (path[i].node1 == ADC3 || path[i].node2 == ADC3) {
      showADCreadings[3] = path[i].net;
    }
    // Serial.println(newBridgeIndex);
    //     for (int j = 0; j < 4; j++)
    //     {
    //       Serial.print("ADC");
    //       Serial.print(j);
    //       Serial.print(": ");
    //       Serial.print(showADCreadings[j]);
    //       Serial.print(" ");

    //     }

    //     Serial.println(" ");

    if (path[i].node1 == ISENSE_PLUS || path[i].node1 == ISENSE_PLUS ||
        path[i].node2 == ISENSE_MINUS || path[i].node2 == ISENSE_MINUS) {
      // Serial.println(showReadings);

      inaConnected = 1;

      switch (showReadings) {
      case 0:
        break;

      case 1:
        showINA0[0] = 1;
        showINA0[1] = 0;
        showINA0[2] = 0;
        break;
      case 2:
        showINA0[0] = 1;
        showINA0[1] = 1;
        showINA0[2] = 0;
        break;
      case 3:

        showINA0[0] = 1;
        showINA0[1] = 1;
        showINA0[2] = 1;
        break;
      }

      // Serial.println(showINA0[0]);
      // Serial.println(showINA0[1]);
      // Serial.println(showINA0[2]);
      // Serial.println(" ");
    }
  }
  if (inaConnected == 0) {
    showINA0[0] = 0;
    showINA0[1] = 0;
    showINA0[2] = 0;
    // showReadings = 3;
  }
}

void showLEDmeasurements(void) {
  int samples = 8;

  int adc0ReadingUnscaled;
  float adc0Reading;

  int adc1ReadingUnscaled;
  float adc1Reading;

  int adc2ReadingUnscaled;
  float adc2Reading;

  int adc3ReadingUnscaled;
  float adc3Reading;
  int bs = 0;

  if (showADCreadings[0] != 0) {

    adc0ReadingUnscaled = readAdc(0, samples);

    adc0Reading = (adc0ReadingUnscaled) * (5.0 / 4095);
    int mappedAdc0Reading = map(adc0ReadingUnscaled, 0, 4095, 4, 30);
    uint32_t color =
        logoColors8vSelect[(map(adc0ReadingUnscaled, 0, 4095, 0, 30) + 26) %
                           59];

    lightUpNet(showADCreadings[0], -1, 1, mappedAdc0Reading, 0, 0, color);

    // showLEDsCore2 = 2;
  }

  if (showADCreadings[1] != 0) {

    adc1ReadingUnscaled = readAdc(1, samples);
    adc1Reading = (adc1ReadingUnscaled) * (5.0 / 4095);
    int mappedAdc1Reading = map(adc1ReadingUnscaled, 0, 4095, 4, 45);

    uint32_t color =
        logoColors8vSelect[(map(adc1ReadingUnscaled, 50, 4095, 0, 30) + 26) %
                           59];

    lightUpNet(showADCreadings[1], -1, 1, mappedAdc1Reading, 0, 0, color);
    // showLEDsCore2 = 2;
  }

  if (showADCreadings[2] != 0) {

    adc2ReadingUnscaled = readAdc(2, samples);
    adc2Reading = (adc2ReadingUnscaled) * (5.0 / 4095);

    int mappedAdc2Reading = map(adc2ReadingUnscaled, 0, 4095, 4, 40);
    uint32_t color =
        logoColors8vSelect[map(adc2ReadingUnscaled, 0, 4095, 0, 30) + 28];

    lightUpNet(showADCreadings[2], -1, 1, mappedAdc2Reading, 0, 0, color);
    // showLEDsCore2 = 2;
  }

  if (showADCreadings[3] != 0) {

    adc3ReadingUnscaled = readAdc(3, samples);
    adc3Reading = (adc3ReadingUnscaled) * (16.0 / 4010);
    adc3Reading -= 8.1; // offset
    int mappedAdc3Reading = map(adc3ReadingUnscaled, 0, 4095, -40, 40);
    int hueShift = 0;

    if (mappedAdc3Reading < 0) {
      hueShift = map(mappedAdc3Reading, -40, 0, 0, 60);
      mappedAdc3Reading = abs(mappedAdc3Reading) + 3;
    } else {
      hueShift = map(mappedAdc3Reading, 0, 40, 0, 60);
      mappedAdc3Reading = abs(mappedAdc3Reading) + 3;
    }
    uint32_t color =
        logoColors8vSelect[map(adc3ReadingUnscaled, 1000, 3000, 0, 59)];

    lightUpNet(showADCreadings[3], -1, 1, mappedAdc3Reading, 0, 0, color);
    // showLEDsCore2 = 2;
  }
}

void showMeasurements(int samples, int printOrBB) {
  unsigned long startMillis = millis();
  int printInterval = 250;
  while (Serial.available() == 0 && Serial1.available() == 0 &&
         checkProbeButton() == 0)

  {

    Serial.print("\r                                                           "
                 "            \r");
    int adc0ReadingUnscaled;
    float adc0Reading;

    int adc1ReadingUnscaled;
    float adc1Reading;

    int adc2ReadingUnscaled;
    float adc2Reading;

    int adc3ReadingUnscaled;
    float adc3Reading;
    int bs = 0;

    if (showADCreadings[0] != 0) {
      //       if (readFloatingOrState(ADC0_PIN, -1) == floating)//this doesn't
      //       work because it's buffered
      // {

      //  bs += Serial.print("Floating  ");
      // }
      adc0ReadingUnscaled = readAdc(0, samples);
      // Serial.print("D0: ");
      // Serial.println(adc0ReadingUnscaled);
      // Serial.print("V\n\r");
      adc0Reading = (adc0ReadingUnscaled) * (5.0 / 4095);
      // adc0Reading -= 0.1; // offset
      bs += Serial.print("D0: ");
      bs += Serial.print(adc0Reading);
      bs += Serial.print("V\t");
    }

    if (showADCreadings[1] != 0) {

      adc1ReadingUnscaled = readAdc(1, samples);
      adc1Reading = (adc1ReadingUnscaled) * (5.0 / 4095);
      // adc1Reading -= 0.1; // offset
      bs += Serial.print("D1: ");
      bs += Serial.print(adc1Reading);
      bs += Serial.print("V\t");
    }

    if (showADCreadings[2] != 0) {

      adc2ReadingUnscaled = readAdc(2, samples);
      adc2Reading = (adc2ReadingUnscaled) * (5.0 / 4095);
      // adc2Reading -= 0.1; // offset

      bs += Serial.print("D2: ");
      bs += Serial.print(adc2Reading);
      bs += Serial.print("V\t");
    }

    if (showADCreadings[3] != 0) {

      adc3ReadingUnscaled = readAdc(3, samples);
      adc3Reading = (adc3ReadingUnscaled) * (16.0 / 4010);
      adc3Reading -= 8.1; // offset
      bs += Serial.print("D3: ");
      bs += Serial.print(adc3Reading);
      bs += Serial.print("V\t");
      int mappedAdc3Reading = map(adc3ReadingUnscaled, 0, 4095, -40, 40);
      int hueShift = 0;

      if (mappedAdc3Reading < 0) {
        hueShift = map(mappedAdc3Reading, -40, 0, 0, 200);
        mappedAdc3Reading = abs(mappedAdc3Reading);
      }
    }

    if (showINA0[0] == 1 || showINA0[1] == 1 || showINA0[2] == 1) {
      bs += Serial.print("   INA219: ");
    }

    if (showINA0[0] == 1) {
      bs += Serial.print("I: ");
      bs += Serial.print(INA0.getCurrent_mA());
      bs += Serial.print("mA\t");
    }
    if (showINA0[1] == 1) {
      bs += Serial.print(" V: ");
      bs += Serial.print(INA0.getBusVoltage());
      bs += Serial.print("V\t");
    }
    if (showINA0[2] == 1) {
      bs += Serial.print("P: ");
      bs += Serial.print(INA0.getPower_mW());
      bs += Serial.print("mW\t");
    }
    Serial.print(digitalRead(buttonPin));
    bs += Serial.print("      \r");
    rotaryEncoderStuff();
    if (encoderButtonState != IDLE) {
      // showReadings = 0;
      return;
    }
    while (millis() - startMillis < printInterval &&
           (Serial.available() == 0 && Serial1.available() == 0 &&
            checkProbeButton() == 0)) {

      showLEDmeasurements();
      delayMicroseconds(5000);
    }
    startMillis = millis();
  }
}

int readAdc(int channel, int samples) {
  int adcReadingAverage = 0;
  // if (channel == 0) { // I have no fucking idea why this works

  //   pinMode(ADC1_PIN, OUTPUT);
  //   digitalWrite(ADC1_PIN, LOW);
  // }
unsigned long timeoutTimer = micros();

  for (int i = 0; i < samples; i++) {
    if (micros() - timeoutTimer > 5000)
    {
      break;
    }
    adcReadingAverage += analogRead(ADC0_PIN + channel); //(int)adc_read();
    delayMicroseconds(25);

  }

  int adcReading = adcReadingAverage / samples;
  // Serial.print(adc3Reading);

  // float adc3Voltage = (adc3Reading - 2528) / 220.0; // painstakingly measured

  if (channel == 0) {
    pinMode(ADC1_PIN, INPUT);
  }
  return adcReading;
}

int waveGen(void) {
  int loopCounter = 0;
  int c = 0;
  listSpecialNets();
  listNets();

  int activeDac = 3;

  mode[0] = 's';
  mode[1] = 's';
  mode[2] = 's';

  refillTable(amplitude[0], offset[0] + calib[0], 0);
  refillTable(amplitude[1], offset[1] + calib[1], 1);

  setDac0_5Vvoltage(0.0);

  setDac1_8VinputCode(offset[1] + calib[1]);
  // setDac1_8VinputCode(8190);
  // Serial.println(dac_rev3.getGain());

  // Serial.println(dac_rev3.maxValue());
  Serial.print("Revision = ");
  Serial.println(revisionNumber);
  Serial.println(
      "\n\r\t\t\t\t     waveGen\t\n\n\r\toptions\t\t\twaves\t\t\tadjust "
      "frequency\n\r");
  Serial.println(
      "\t5/0 = dac 0 0-5V (togg)\tq = square\t\t+ = frequency++\n\r");
  Serial.println("\t8/1 = dac 1 +-8V (togg)\ts = sine\t\t- = frequency--\n\r");
  Serial.println(
      "\ta = set amplitude (p-p)\tw = sawtooth\t\t* = frequency*2\n\r");
  Serial.println("\to = set offset\t\tt = triangle\t\t/ = frequency/2\n\r");
  Serial.println("\tv = voltage\t\tr = random\t\t \n\r");
  Serial.println("\th = show this menu\tx = exit\t\t \n\r");

  period[activeDac] = 1e6 / (freq[activeDac] / 10);
  halvePeriod[activeDac] = period[activeDac] / 2;
  int chars = 0;

  chars = 0;

  int firstCrossFreq0 = 0;
  int firstCrossFreq1 = 0;

  while (1) {
    yield();
    uint32_t now = micros();

    count++;

    // float adc3Voltage = (adc3Reading - 2528) / 220.0; //painstakingly
    // measured float adc0Voltage = ((adc0Reading) / 400.0) - 0.69; //- 0.93;
    // //painstakingly measured

    int adc0Reading = 0;
    int brightness0 = 0;
    int hueShift0 = 0;
    // firstCrossFreq0 = 1;

    if (dacOn[0] == 1 && freq[0] < 33) {
      // adc0Reading = INA1.getBusVoltage_mV();
      //  adc0Reading = dac0_5V.getInputCode();

      if (c == 'q') {
      } else {

        adc0Reading = readAdc(26, 1);
        adc0Reading = abs(adc0Reading);
        hueShift0 = map(adc0Reading, 0, 5000, -90, 0);
        brightness0 = map(adc0Reading, 0, 5000, 4, 100);

        lightUpNet(4, -1, 1, brightness0, hueShift0);
        showLEDsCore2 = 1;
        firstCrossFreq0 = 1;
      }
    } else {
      if (firstCrossFreq0 == 1) {
        lightUpNet(4);
        showLEDsCore2 = 1;
        firstCrossFreq0 = 0;
      }
    }

    int adc1Reading = 0;
    int brightness1 = 0;
    int hueShift1 = 0;

    if (dacOn[1] == 1 && freq[1] < 17) {
      adc1Reading = readAdc(29, 1);
      hueShift1 = map(adc1Reading, -2048, 2048, -50, 45);
      adc1Reading = adc1Reading - 2048;

      adc1Reading = abs(adc1Reading);

      brightness1 = map(adc1Reading, 0, 2050, 4, 100);

      lightUpNet(5, -1, 1, brightness1, hueShift1);
      showLEDsCore2 = 1;
      firstCrossFreq1 = 1;
    } else {
      if (firstCrossFreq1 == 1) {
        lightUpNet(5);
        showLEDsCore2 = 1;
        firstCrossFreq1 = 0;
      }
    }

    if (now - lastTime > 100000) {
      loopCounter++;
      //
      // int adc0Reading = analogRead(26);
      // if (activeDac == 0)
      // {
      // for (int i = 0; i < (analogRead(27)/100); i++)
      // {
      // Serial.print('.');

      // }
      // Serial.println(' ');
      // }
      // else if (activeDac == 1)
      // {

      // for (int i = 0; i < (analogRead(29)/100); i++)
      // {
      // Serial.print('.');

      // }
      // Serial.println(' ');
      // }

      lastTime = now;
      // Serial.println(count); // show # updates per 0.1 second
      count = 0;

      if (Serial.available() == 0) {
        // break;
      } else {
        c = Serial.read();
        switch (c) {
        case '+':
          if (freq[activeDac] >= 1.0) {

            freq[activeDac]++;
          } else {
            freq[activeDac] += 0.1;
          }
          break;
        case '-':

          if (freq[activeDac] > 1.0) {
            freq[activeDac]--;
          } else if (freq[activeDac] > 0.1) {
            freq[activeDac] -= 0.1;
          } else {
            freq[activeDac] = 0.0;
          }

          break;
        case '*':
          freq[activeDac] *= 2;
          break;
        case '/':
          freq[activeDac] /= 2;
          break;
        case '8':
          if (activeDac == 0) {
            setDac0_5Vvoltage(0.0);
            dacOn[0] = 0;
          }

          if (activeDac != 3)
            dacOn[1] = !dacOn[1];

          activeDac = 1;

          if (dacOn[1] == 0) {
            setDac1_8VinputCode(offset[1] + calib[1]);
          }

          break;
        case '5':
          if (activeDac == 1) {
            setDac1_8VinputCode(offset[1] + calib[1]);
            dacOn[1] = 0;
          }

          if (activeDac != 3)
            dacOn[0] = !dacOn[0];

          activeDac = 0;
          if (dacOn[activeDac] == 0) {
            setDac0_5Vvoltage(0.0);
          }
          break;
        case 'c':
          // freq[activeDac] = 0;
          break;

        case 's': {
          if (mode[2] == 'v') {
            refillTable(amplitude[activeDac], offset[activeDac] + calib[1], 1);
            mode[2] = 's';
          }

          mode[activeDac] = c;
          break;
        }

        case 'q':

        case 'w':
        case 't':
        case 'r':
        case 'z':
        case 'm':

          //
          mode[2] = mode[activeDac];
          mode[activeDac] = c;
          break;

        case 'v':
        case 'h':
        case 'a':
        case 'o':

          mode[2] = mode[activeDac];

          mode[activeDac] = c;
          break;
        case '{':
        case 'f': {
          if (mode[0] != 'v') {
            setDac0_5Vvoltage(0.0);
          }
          if (mode[1] != 'v') {
            setDac1_8VinputCode(offset[1]);
          }
          return 0;
        }

        case 'x': {
          if (mode[0] != 'v') {
            setDac0_5Vvoltage(0.0);
          }
          if (mode[1] != 'v') {

            setDac1_8VinputCode(offset[1]);
          }

          return 1;
        }
        default:
          break;
        }
        period[activeDac] = 1e6 / freq[activeDac];
        halvePeriod[activeDac] = period[activeDac] / 2;
        if (activeDac == 0) {
          Serial.print("dac 0:   ");
          Serial.print("ampl: ");
          Serial.print((float)(amplitude[activeDac]) / 819);
          Serial.print("V\t");
          Serial.print("offset: ");
          Serial.print((float)(offset[activeDac]) / 819);
          Serial.print("V\t\t");
          Serial.print("mode: ");
          Serial.print(mode[activeDac]);
          Serial.print("\t\t");
          Serial.print("freq: ");
          Serial.print(freq[activeDac]);

          // Serial.print("\t\n\r");
        } else if (activeDac == 1) {
          Serial.print("dac 1:   ");
          Serial.print("ampl: ");
          Serial.print((float)(amplitude[activeDac]) / 276);
          Serial.print("V\t");
          Serial.print("offset: ");
          Serial.print(((float)(offset[activeDac]) / 276) - 7);
          Serial.print("V\t\t");
          Serial.print("mode: ");
          Serial.print(mode[activeDac]);
          Serial.print("\t\t");
          Serial.print("freq: ");
          Serial.print(freq[activeDac]);
          // Serial.print("\t\n\r");
        }
        /*
        Serial.print("\tdacon");
        for (int i = 0; i < 3; i++)
        {
          Serial.print("\t");

          Serial.print(dacOn[i]);
        }*/
        Serial.println();
      }
    }

    uint32_t t = now % period[activeDac];
    // if (dacOn[activeDac] == 1 )
    //{
    switch (mode[activeDac]) {
    case 'q':
      if (t < halvePeriod[activeDac]) {
        if (activeDac == 0 && dacOn[activeDac] == 1) {
          setDac0_5VinputCode(amplitude[activeDac]);
          lightUpNet(4, -1, 1, DEFAULTSPECIALNETBRIGHTNESS, 12);
          showLEDsCore2 = 1;
        } else if (activeDac == 1 && dacOn[activeDac] == 1) {
          setDac1_8VinputCode(amplitude[activeDac]);

          showLEDsCore2 = 1;
        }
      } else {
        if (activeDac == 0 && dacOn[activeDac] == 1) {
          setDac0_5VinputCode(0);
          lightUpNet(4, -1, 1, 2, 12);
        }

        else if (activeDac == 1 && dacOn[activeDac] == 1) {
          setDac1_8VinputCode(offset[activeDac]);
        }
      }
      break;
    case 'w':
      if (activeDac == 0 && dacOn[activeDac] == 1)
        setDac0_5VinputCode(t * amplitude[activeDac] / period[activeDac]);
      else if (activeDac == 1 && dacOn[activeDac] == 1)
        setDac1_8VinputCode(t * amplitude[activeDac] / period[activeDac]);
      break;
    case 't':
      if (activeDac == 0 && dacOn[activeDac] == 1) {

        if (t < halvePeriod[activeDac])
          setDac0_5VinputCode(
              ((t * amplitude[activeDac]) / halvePeriod[activeDac]));
        else
          setDac0_5VinputCode(
              (((period[activeDac] - t) * (amplitude[activeDac]) /
                halvePeriod[activeDac])));
      } else if (activeDac == 1 && dacOn[activeDac] == 1) {
        if (t < halvePeriod[activeDac])
          setDac1_8VinputCode(t * amplitude[activeDac] /
                              halvePeriod[activeDac]);
        else
          setDac1_8VinputCode((period[activeDac] - t) * amplitude[activeDac] /
                              halvePeriod[activeDac]);
      }
      break;
    case 'r':
      if (activeDac == 0 && dacOn[activeDac] == 1)
        setDac0_5VinputCode(random(amplitude[activeDac]));
      else if (activeDac == 1 && dacOn[activeDac] == 1) {
        setDac1_8VinputCode(random(amplitude[activeDac]));
      }
      break;
    case 'z': // zero
      if (activeDac == 0)
        setDac0_5Vvoltage(0);
      else if (activeDac == 1)
        setDac1_8VinputCode(offset[activeDac]);
      break;
    case 'h': // high
      Serial.println(
          "\n\r\t\t\t\t     waveGen\t\n\n\r\toptions\t\t\twaves\t\t\tadjust "
          "frequency\n\r");
      Serial.println(
          "\t5/0 = dac 0 0-5V (togg)\tq = square\t\t+ = frequency++\n\r");
      Serial.println(
          "\t8/1 = dac 1 +-8V (togg)\ts = sine\t\t- = frequency--\n\r");
      Serial.println(
          "\ta = set amplitude (p-p)\tw = sawtooth\t\t* = frequency*2\n\r");
      Serial.println("\to = set offset\t\tt = triangle\t\t/ = frequency/2\n\r");
      Serial.println("\tv = voltage\t\tr = random\t\t \n\r");
      Serial.println("\th = show this menu\tx = exit\t\t \n\r");
      mode[activeDac] = mode[2];
      break;
    case 'm': // mid
      // setDac1_8VinputCode(2047);
      break;
    case 'a': {
      float newAmplitudeF = 0;
      int newAmplitude = 0;
      int input = 0;
      char aC = 0;
      int a = 0;
      if (activeDac == 0) {

        Serial.print("\n\renter amplitude (0-5): ");
        while (Serial.available() == 0)
          ;
        aC = Serial.read();
        if (aC == 'a')
          aC = Serial.read();
        a = aC;

        Serial.print(aC);

        if (a >= 48 && a <= 53) {

          input = a - 48;
          newAmplitude = input * 819;
          Serial.print(".");
          while (Serial.available() == 0)
            ;
          a = Serial.read();

          if (a == '.') {
            while (Serial.available() == 0)
              ;

            a = Serial.read();
          }

          if (a >= 48 && a <= 57) {
            Serial.print((char)a);
            input = a - 48;
            newAmplitude += input * 81.9;

            amplitude[activeDac] = newAmplitude;
            Serial.print("\tamplitude: ");
            Serial.print((float)(amplitude[activeDac]) / 819);
            Serial.println("V");
            if ((offset[activeDac] - (amplitude[activeDac] / 2)) < 10 ||
                ((amplitude[activeDac] / 2) - offset[activeDac]) < 10) {
              offset[activeDac] = (amplitude[activeDac] / 2) - 1;
            }
            { offset[activeDac] = (amplitude[activeDac] / 2) - 2; }
            refillTable(amplitude[activeDac], offset[activeDac], 0);

            mode[activeDac] = mode[2];
            mode[2] = '0';
            break;
          }
        }
      } else if (activeDac == 1) {

        Serial.print("\n\renter peak amplitude (0-7.5): ");
        while (Serial.available() == 0)
          ;
        aC = Serial.read();
        if (aC == 'o')
          aC = Serial.read();
        a = aC;

        Serial.print(aC);

        if (a >= 48 && a <= 55) {

          input = a - 48;
          newAmplitude = input * 276;
          Serial.print(".");
          while (Serial.available() == 0)
            ;
          a = Serial.read();

          if (a == '.') {
            while (Serial.available() == 0)
              ;

            a = Serial.read();
          }

          if (a >= 48 && a <= 57) {
            Serial.print((char)a);
            input = a - 48;
            newAmplitude += input * 27.6;
            newAmplitude *= 2;

            amplitude[activeDac] = newAmplitude;
            Serial.print("\tamplitude: ");
            Serial.print((amplitude[activeDac]));
            Serial.println(" ");
            Serial.print((float)(amplitude[activeDac]) / 276);
            Serial.println("V");

            refillTable(amplitude[activeDac], offset[activeDac] + calib[1], 1);

            mode[activeDac] = mode[2];
            mode[2] = '0';
            break;
          }
        }
      }
    }
    case 'o': {

      int newOffset = 0;
      int input = 0;
      char aC = 0;
      int o = 0;
      if (activeDac == 0) {

        Serial.print("\n\renter offset (0-5): ");
        while (Serial.available() == 0)
          ;
        aC = Serial.read();
        if (aC == 'o')
          aC = Serial.read();

        o = aC;

        Serial.print(aC);

        if (o >= 48 && o <= 53) {

          input = o - 48;
          newOffset = input * 819;
          Serial.print(".");
          while (Serial.available() == 0)
            ;
          o = Serial.read();

          if (o == '.') {
            while (Serial.available() == 0)
              ;

            o = Serial.read();
          }

          if (o >= 48 && o <= 57) {
            Serial.print((char)o);
            input = o - 48;
            newOffset += input * 81.9;

            offset[activeDac] = newOffset;
            Serial.print("\toffset: ");
            Serial.print((float)(offset[activeDac]) / 819);
            Serial.println("V");

            refillTable(amplitude[activeDac], offset[activeDac], 0);

            mode[activeDac] = mode[2];
            break;
          }
        }
      } else if (activeDac == 1) {
        int negative = 0;

        Serial.print("\n\rEnter offset (-7 - 7): ");
        while (Serial.available() == 0)
          ;
        aC = Serial.read();
        if (aC == '-') {
          Serial.print('-');
          negative = 1;
          while (Serial.available() == 0)
            ;
          aC = Serial.read();
        }

        o = aC;

        Serial.print(aC);

        if (o >= 48 && o <= 55) {

          input = o - 48;
          newOffset = input * 276;

          if (input == '7') {
            Serial.print(".00");
          } else {

            Serial.print(".");
            while (Serial.available() == 0)
              ;
            o = Serial.read();
            if (o == '.') {
              while (Serial.available() == 0)
                ;
              o = Serial.read();
            }

            if (o >= 48 && o <= 57) {
              Serial.print((char)o);
              input = o - 48;
              newOffset += input * 27.6;
            }
          }

          if (negative == 1)
            newOffset *= -1;

          newOffset += (7 * 276);

          offset[activeDac] = newOffset;
          Serial.print("\toffset: ");
          Serial.print(((float)(offset[activeDac]) / 276) - 7);
          Serial.print("  ");
          Serial.print(offset[activeDac]);
          Serial.println("V");

          refillTable(amplitude[activeDac], offset[activeDac] + calib[1], 1);

          mode[activeDac] = mode[2];
          break;
        }
      }
    }
    case 'v': {
      if (activeDac == 0 && mode[2] != 'v') {
        // freq[activeDac] = 0;
        setDac0_5Vvoltage(amplitude[activeDac] / 819);
        mode[2] = 'v';
      } else if (activeDac == 1 && mode[2] != 'v') {
        // freq[activeDac] = 0;
        // refillTable(0, offset[activeDac] + calib[1], 1);
        setDac1_8Vvoltage(((amplitude[activeDac] + calib[1]) / 276) -
                          ((offset[activeDac] / 276) - 7));
        mode[2] = 'v';
      } else if (mode[2] == 'v') {
        // mode[2] = 's';
      }

      break;
    }

    default:
    case 's':
      // reference
      // float f = ((PI * 2) * t)/period;
      // setDac1_8VinputCode(2047 + 2047 * sin(f));
      //
      if (mode[activeDac] != 'v') {
        int idx = (360 * t) / period[activeDac];
        if (activeDac == 0 && dacOn[activeDac] == 1)
          setDac0_5VinputCode(sine0[idx]); // lookuptable
        else if (activeDac == 1 && dacOn[activeDac] == 1)
          setDac1_8VinputCode(sine1[idx]); // lookuptable
      }
      break;
    }
  }
}

void refillTable(int amplitude, int offset, int dac) {
  // int offsetCorr = 0;
  if (dac == 0) {
    // offset = amplitude / 2;
  }

  for (int i = 0; i < 360; i++) {
    if (dac == 0) {
      sine0[i] = offset + round(amplitude / 2 * sin(i * PI / 180));
    } else if (dac == 1) {
      sine1[i] =
          offset + round((amplitude - (offset - 2047)) / 2 * sin(i * PI / 180));
    } else if (dac == 2) {
      sine0[i] = offset + round(amplitude / 2 * sin(i * PI / 180));
      sine1[i] = offset + round(amplitude / 2 * sin(i * PI / 180));
    }
  }
}
void GetAdc29Status(int i) {
  gpio_function gpio29Function = gpio_get_function(29);
  Serial.print("GPIO29 func: ");
  Serial.println(gpio29Function);

  bool pd = gpio_is_pulled_down(29);
  Serial.print("GPIO29 pd: ");
  Serial.println(pd);

  bool h = gpio_is_input_hysteresis_enabled(29);
  Serial.print("GPIO29 h: ");
  Serial.println(h);

  gpio_slew_rate slew = gpio_get_slew_rate(29);
  Serial.print("GPIO29 slew: ");
  Serial.println(slew);

  gpio_drive_strength drive = gpio_get_drive_strength(29);
  Serial.print("GPIO29 drive: ");
  Serial.println(drive);

  int irqmask = gpio_get_irq_event_mask(29);
  Serial.print("GPIO29 irqmask: ");
  Serial.println(irqmask);

  bool out = gpio_is_dir_out(29);
  Serial.print("GPIO29 out: ");
  Serial.println(out);
  Serial.printf("(%i) GPIO29 func: %i, pd: %i, h: %i, slew: %i, drive: %i, "
                "irqmask: %i, out: %i\n",
                i, gpio29Function, pd, h, slew, drive, irqmask, out);
}
