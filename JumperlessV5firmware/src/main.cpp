// SPDX-License-Identifier: MIT

/*
Kevin Santo Cappuccio
Architeuthis Flux

KevinC@ppucc.io

5/28/2024

*/
#include <Arduino.h>
#define USE_TINYUSB 1
// #define LED LED_BUILTIN
// #ifdef USE_TINYUSB
// #include
// "../include/Adafruit_TinyUSB_Arduino_changed/Adafruit_TinyUSB_changed.h"
// #include
// "../lib/Adafruit_TinyUSB_Arduino_changed/src/Adafruit_TinyUSB_changed.h"
// #endif
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
// #ifdef CFG_TUSB_CONFIG_FILE
// #include CFG_TUSB_CONFIG_FILE
// #else
// #include "tusb_config.h"
// #endif
#include "ArduinoStuff.h"
#include "CH446Q.h"
#include "Commands.h"
#include "FileParsing.h"
#include "Graphics.h"
#include "JumperlessDefinesRP2040.h"
#include "LEDs.h"
#include "LittleFS.h"
#include "MachineCommands.h"
#include "MatrixStateRP2040.h"
#include "Menus.h"
#include "NetManager.h"
#include "NetsToChipConnections.h"
#include "Peripherals.h"
#include "PersistentStuff.h"
#include "Probing.h"
#include "RotaryEncoder.h"
#include <Adafruit_TinyUSB.h>

// #include "AdcUsb.h"
// #include "logic_analyzer.h"
// LogicAnalyzer logicAnalyzer;
// Capture capture(MAX_FREQ, MAX_FREQ_THRESHOLD);

// #include "pico/multicore.h"
// #include "hardware/watchdog.h"

// using namespace logic_analyzer;

bread b;

Adafruit_USBD_CDC USBSer1;

int supplySwitchPosition = 0;
volatile bool core1busy = false;
volatile bool core2busy = false;

void machineMode(void);
// void lastNetConfirm(int forceLastNet = 0);
void rotaryEncoderStuff(void);

volatile uint8_t pauseCore2 = 0;

volatile int loadingFile = 0;

unsigned long lastNetConfirmTimer = 0;
// int machineMode = 0;

int rotEncInit = 0;
// https://wokwi.com/projects/367384677537829889

int core2initFinished = 0;

void setup() {
  pinMode(RESETPIN, OUTPUT_12MA);

  digitalWrite(RESETPIN, HIGH);
  ///multicore_lockout_victim_init();
  delayMicroseconds(8000);
  // Serial.setTimeout(8000);
  //  USB_PID = 0xACAB;
  //  USB_VID = 0x1D50;
  //  USB_MANUFACTURER = "Architeuthis Flux";
  //  USB_PRODUCT = "Jumperless";
  //  USBSetup

  USBDevice.setProductDescriptor("Jumperless");
  USBDevice.setManufacturerDescriptor("Architeuthis Flux");
  USBDevice.setSerialDescriptor("0");
  USBDevice.setID(0x1D50, 0xACAB);
  USBDevice.addStringDescriptor("Jumperless");
  USBDevice.addStringDescriptor("Architeuthis Flux");

  delay(20);
   Serial.setTimeout(500);
  Serial.begin(115200);

  delay(20);
  // pinMode(buttonPin, INPUT_PULLDOWN);

  pinMode(probePin, OUTPUT_8MA);
  pinMode(10, OUTPUT_8MA);

  digitalWrite(10, HIGH);

  // USBSer1.setStringDescriptor("Jumperless USB Serial");

  // USBSer1.begin(115200);

  EEPROM.begin(256);

  debugFlagInit();

  delay(10);

  // delay(1);

  initINA219();

  initArduino();

  delay(4);

  LittleFS.begin();

  // setDac0_5Vvoltage(0.0);
  // setDac1_8Vvoltage(1.9);

  // createSlots(-1, 0);
  delay(10);
  clearAllNTCC();

  digitalWrite(RESETPIN, LOW);

  pinMode(probePin, OUTPUT_12MA);
  // pinMode(buttonPin, INPUT_PULLDOWN);
  digitalWrite(probePin, HIGH);

  initRotaryEncoder();

  delay(10);

  while (core2initFinished == 0) {
  }
  delay(100);
  initMenu();
  initADC();
  initDAC(); // also sets revisionNumber
  setupSwirlColors();
  // setRailsAndDACs();
  //  showLEDsCore2 = 1;
  //
 // multicore_lockout_victim_init();
}

void setup1() {
  delay(10);

  initGPIOex();

  delay(4);

  initCH446Q();

  delay(4);

  initLEDs();

  delay(4);

  core2initFinished = 1;
  // delay(4);
//multicore_lockout_victim_init();
  // lightUpRail();

  // delay(4);
  // showLEDsCore2 = 1;
}

unsigned long teardownTimer = 0;
unsigned long lastTeardown = 0;
unsigned long teardownTime = 2000;

char connectFromArduino = '\0';

char input;

int serSource = 0;
int readInNodesArduino = 0;
int baudRate = 115200;

int restoredNodeFile = 0;

const char firmwareVersion[] = "5.0.0"; //// remember to update this

int firstLoop = 1;

volatile int probeActive = 0;

int showExtraMenu = 1;
int tearDownToggle = 0;

int tinyUSB = 0;
unsigned long timer = 0;
int lastProbeButton = 0;

void loop() {
  // #ifdef USE_TINYUSB
  //   tinyUSB = 1;
  // #endif

  // while (core2initFinished == 0)
  // {
  // }
  delay(100);

  setRailsAndDACs();
  //  while (millis() < 4000)
  //  {
  //   char ddd = Serial.read();
  //  }
  // Serial.flush();
  // startupColorsV5();
  // lightUpRail();

  // showLEDsCore2 = 3;

  // while(1){
  // Serial.println(digitalRead(BUTTON_ENC));
  // delay(100);
  // }
  if (PROTOTYPE_VERSION > 0) {
    getNothingTouched();
  }
menu:
  routableBufferPower(1);
  // showLEDsCore2 = 1;
  // Serial.println(showLEDsCore2);
  //  Serial.print("Fuck you!\n\r");
  // Serial.print("netSlot = ");
  // Serial.println(netSlot);
  printMainMenu(showExtraMenu);
  //  printCalibration();

  // int logoTopSetting[2] = {-1, -1}; //{function, settingOption}
  // int logoBottomSetting[2] = {-1, -1};
  // int buildingTopSetting[2] = {-1, -1};
  // int buildingBottomSetting[2] = {-1, -1};

  // Serial.print("Logo Top      :\t");
  // Serial.print(logoTopSetting[0]);
  // Serial.print("\t");
  // Serial.println(logoTopSetting[1]);
  // Serial.print("Logo Bottom   :\t");
  // Serial.print(logoBottomSetting[0]);
  // Serial.print("\t");
  // Serial.println(logoBottomSetting[1]);
  // Serial.print("Building Top  :\t");
  // Serial.print(buildingTopSetting[0]);
  // Serial.print("\t");
  // Serial.println(buildingTopSetting[1]);
  // Serial.print("Building Bot  :\t");
  // Serial.print(buildingBottomSetting[0]);
  // Serial.print("\t");
  // Serial.println(buildingBottomSetting[1]);

  // for (int i = 1; i < 9; i++)
  // {
  //   Serial.print (gpioState[i]);
  //   Serial.print("\t");

  // }
  //   Serial.println();
  //   for (int i = 1; i < 9; i++)
  // {
  //   Serial.print (gpioNet[i]);
  //   Serial.print("\t");

  // }
  Serial.println();
  // printColorName(5);
  //  saveVoltages(railVoltage[0], railVoltage[1], dacOutput[0], dacOutput[1]);
  //  readVoltages();
  int toggg = 0;

  int chipSc = 0;
  unsigned long lastTimerrr = 0;
  // while (1) {
  //   // if (millis() - lastTimerrr > 600) {
  //   //   chipSc++;
  //   //   if (chipSc > 15) {
  //   //     toggg = !toggg;
  //   //     chipSc = 0;
  //   //     Serial.println(" ");
  //   //   }
  //   //    lastTimerrr = millis();
  //   //   // Serial.print("chipSc = ");
  //   //   // Serial.println(chipSc);
  //   //   // Serial.println();
  //   //   // delay(300);
  //   // }
  //     // Serial.print("chipSc = ");
  //     // Serial.println(chipNumToChar(chipSc));
  //     // Serial.println();
  //     // delay(100);

  //   setCSex(chipSc, 1);
  //   //delay(100);
  //   setCSex(chipSc, 0);
  //   delay(1);

  //   // writeGPIOex(toggg, 0);
  //   // delay(10);

  //   //delay(100);
  //   //  delay(1000
  // }
  // setupSwirlColors();\

  if (firstLoop == 1) {
    firstLoop = 0;
    delay(100);
    // defconDisplay = 0;

    goto loadfile;
  }

dontshowmenu:

Serial.print("core2busy = ");
Serial.println(core2busy);


  // defconDisplay = 0;
  //  readVoltages();
  //  refreshConnections();
  connectFromArduino = '\0';
  // showLEDsCore2 = 1;
  while (Serial.available() == 0 && connectFromArduino == '\0' &&
         slotChanged == 0) {
    // {  pinMode(26, INPUT);
    // pinMode(10, OUTPUT_8MA);
    // digitalWrite(10, HIGH);
    // pinMode(9, OUTPUT_8MA);
    // digitalWrite(9, HIGH);
    // Serial.println(defconDisplay);
    if (clickMenu() >= 0) {
      // defconDisplay = -1;
      goto loadfile;
    }
    // Serial.println(digitalRead(11));
    // delay(300);

    if (showReadings >= 1) {
      showMeasurements();
    }
    // Serial.println(digitalRead(buttonPin));

    if ((millis() % 100) < 1) {

      int probeButton = checkProbeButton();

      if (probeButton != lastProbeButton) {
        lastProbeButton = probeButton;

        if (probeButton > 0) { //&& inPadMenu == 0) {
                               // Serial.print("probeButton = ");
                               // Serial.println(probeButton);
          // calibrateProbe();
          // int longShort = longShortPress(1000);
          // defconDisplay = -1;
          if (probeButton == 2) {

            input = 'p';
            probingTimer = millis();
            goto skipinput;
          } else if (probeButton == 1) {
            // getNothingTouched();
            input = 'c';
            probingTimer = millis();
            // Serial.println("probing\n\r");

            goto skipinput;
          }
        }
      }

      // pinMode(19, INPUT);
    } else if ((millis() % 150) < 1) {
      if (PROTOTYPE_VERSION > 0) {
        // Serial.print("Prototype Version ");
        // Serial.print(PROTOTYPE_VERSION);
        // Serial.print("\n\r");
        checkPads();
      }
      //
    }
  }

  if (slotChanged == 1) {
    goto loadfile;
  }

  if (connectFromArduino != '\0') {
  } else {
    input = Serial.read();
    // Serial.print("\n\r");
    if (input == '}' || input == ' ' || input == '\n' || input == '\r') {
      goto dontshowmenu;
    }
  }

skipinput:
  switch (input) {
  case '?': {
    Serial.print("Jumperless firmware version: ");
    Serial.println(firmwareVersion);
    break;
  }
  case '$': {
    // return current slot number
    Serial.println(netSlot);
    break;
  }
  case 'i': {
    Serial.println("I2C scan\n\n\r");
    Serial.println("enter SCL row\n\r");
    Serial.print("SCL = ");
    delay(100);
    int sclRow = -1;
    int sdaRow = -1;
    while (Serial.available() == 0) {
    }
    while (sclRow == -1 || (sclRow < 1 || sclRow > 100)) {
      sclRow = Serial.parseInt();
      delay(100);
    }
    Serial.println("enter SDA row\n\r");

    while (Serial.available() == 0) {
    }
    while (sdaRow == -1 || (sdaRow < 1 || sdaRow > 100)) {
      sdaRow = Serial.parseInt();
      delay(100);
    }
    Serial.print("SCL = ");
    Serial.println(sclRow);
    Serial.print("SDA = ");
    Serial.println(sdaRow);
    Serial.println("Scanning I2C bus\n\r");

    i2cScan(sdaRow, sclRow);
    break;
  }

  case 'g': {
    // pauseCore2 = 1;
    //  while (slotChanged == 0)
    //  {
    //
    while (Serial.available() == 0 && slotChanged == 0) {
      if (slotChanged == 1) {
        // b.print("Jumperless", 0x101000, 0x020002, 0);
        // delay(100);
        goto menu;
      }
    }
    b.clear();

    char f[80] = {' '};
    int index = 0;
    // leds.clear();
    while (Serial.available() > 0) {
      if (index > 19) {
        break;
      }
      f[index] = Serial.read();
      index++;
      // b.print(f);
      // delayMicroseconds(30);
      // leds.show();
    }
    f[index] = ' ';
    f[index + 1] = ' ';
    uint32_t color = 0x100010;
    // Serial.print(index);
    defconString[0] = f[0];
    defconString[1] = f[1];
    defconString[2] = f[2];
    defconString[3] = f[3];
    defconString[4] = f[4];
    defconString[5] = f[5];
    defconString[6] = f[6];
    defconString[7] = f[7];
    defconString[8] = f[8];
    defconString[9] = f[9];
    defconString[10] = f[10];
    defconString[11] = f[11];
    defconString[12] = f[12];
    defconString[13] = f[13];
    defconString[14] = f[14];
    defconString[15] = f[15];
    defconDisplay = 0;
    // b.print(f, color);

    break;
  }
  case 'e': {
    if (showExtraMenu == 0) {
      showExtraMenu = 1;
    } else {
      showExtraMenu = 0;
    }
    break;
  }

  case 's': {
    printSlots(-1);

    break;
  }
  case 'v':

    if (showReadings >= 3 || (inaConnected == 0 && showReadings >= 1)) {
      showReadings = 0;
      break;
    } else {
      showReadings++;

      chooseShownReadings();
      // Serial.println(showReadings);

      goto dontshowmenu;
      // break;
    }
  case 'p': {
    probeActive = 1;

    delayMicroseconds(1500);
    probeMode(10, 1);
    delayMicroseconds(2500);
    probeActive = 0;
    // clearLEDs();
    // assignNetColors();
    // showNets();
    // showLEDsCore2 = 1;
    break;
  }
  case 'c': {
    // removeBridgeFromNodeFile(19, 1);
    probeActive = 1;
    delayMicroseconds(1500);
    probeMode(19, 0);
    delayMicroseconds(2500);
    probeActive = 0;
    // clearLEDs();
    // assignNetColors();
    // showNets();
    // showLEDsCore2 = 1;
    break;
  }
  case 'n':
    couldntFindPath(1);
    Serial.print("\n\n\rnetlist\n\n\r");
    listSpecialNets();
    listNets();

    break;
  case 'b':
    couldntFindPath(1);
    Serial.print("\n\n\rBridge Array\n\r");
    printBridgeArray();
    Serial.print("\n\n\n\rPaths\n\r");
    printPathsCompact();
    Serial.print("\n\n\rChip Status\n\r");
    printChipStatus();
    Serial.print("\n\n\r");
    Serial.print("Revision ");
    Serial.print(revisionNumber);
    Serial.print("\n\n\r");
    break;

  case 'm':

    break;

  case '!':
    printNodeFile();
    break;

  case 'w':

    if (waveGen() == 1) {
      break;
    }
  case 'o': {
    // probeActive = 1;
    inputNodeFileList(rotaryEncoderMode);
    showSavedColors(netSlot);
    // input = ' ';
    showLEDsCore2 = 1;
    // probeActive = 0;
    goto loadfile;
    // goto dontshowmenu;
    break;
  }

  case 'x': {

    if (netSlot == NUM_SLOTS - 1) {
      netSlot = 0;
    } else {
      netSlot++;
    }

    Serial.print("\r                                         \r");
    Serial.print("Slot ");
    Serial.print(netSlot);
    slotPreview = netSlot;
    goto loadfile;
  }
  case 'z': {

    if (netSlot == 0) {
      netSlot = NUM_SLOTS - 1;
    } else {
      netSlot--;
    }
    Serial.print("\r                                         \r");
    Serial.print("Slot ");
    Serial.print(netSlot);
    slotPreview = netSlot;
    goto loadfile;
  }
  case 'y': {
  loadfile:
    loadingFile = 1;

    // digitalWrite(RESETPIN, HIGH);

    // delayMicroseconds(2);

    // digitalWrite(RESETPIN, LOW);

    // showSavedColors(netSlot);
    //  drawWires();

    // showLEDsCore2 = 1;
    slotChanged = 0;
    loadingFile = 0;
    refreshConnections();
    chooseShownReadings();
    break;
  }
  case 'f':

    probeActive = 1;
    readInNodesArduino = 1;
    // clearAllNTCC();

    // sendAllPathsCore2 = 1;
    // timer = millis();

    // clearNodeFile(netSlot);

    if (connectFromArduino != '\0') {
      serSource = 1;
    } else {
      serSource = 0;
    }
    savePreformattedNodeFile(serSource, netSlot, rotaryEncoderMode);

    refreshConnections();

    if (debugNMtime) {
      Serial.print("\n\n\r");
      Serial.print("took ");
      Serial.print(millis() - timer);
      Serial.print("ms");
    }
    input = ' ';

    probeActive = 0;
    if (connectFromArduino != '\0') {
      connectFromArduino = '\0';
      // Serial.print("connectFromArduino\n\r");
      //  delay(2000);
      input = ' ';
      readInNodesArduino = 0;

      goto dontshowmenu;
    }
    // chooseShownReadings();

    connectFromArduino = '\0';
    readInNodesArduino = 0;
    break;

  case '\n':
    goto menu;
    break;

  case 't':
#ifdef FSSTUFF
    clearNodeFile();
#endif

#ifdef EEPROMSTUFF
    lastCommandWrite(input);

    runCommandAfterReset('t');
#endif

#ifdef FSSTUFF
    openNodeFile();
    getNodesToConnect();
#endif
    Serial.println("\n\n\rnetlist\n\n\r");

    bridgesToPaths();

    listSpecialNets();
    listNets();
    printBridgeArray();
    Serial.print("\n\n\r");
    Serial.print(numberOfNets);

    Serial.print("\n\n\r");
    Serial.print(numberOfPaths);

    assignNetColors();
#ifdef PIOSTUFF
    sendAllPaths();
#endif

    break;

  case 'l':
    if (LEDbrightnessMenu() == '!') {
      clearLEDs();
      delayMicroseconds(9200);
      sendAllPathsCore2 = 1;
    }
    break;

  case 'r':

    if (rotaryEncoderMode == 1) {
      // unInitRotaryEncoder();

      rotaryEncoderMode = 0;
      // createSlots(-1, rotaryEncoderMode);
      //  showSavedColors(netSlot);
      // assignNetColors();

      // showNets();
      lightUpRail();

      showLEDsCore2 = 1;
      debugFlagSet(10); // encoderModeOff
      goto menu;
    } else {
      rotaryEncoderMode = 1;
      if (rotEncInit == 0) // only do this once
      {
        createSlots(-1, rotaryEncoderMode);
        // initRotaryEncoder();
        rotEncInit = 1;
        // Serial.print("\n\n\r (you should unplug an)");
      }
      printRotaryEncoderHelp();
      delay(100);
      // initRotaryEncoder();
      // refreshSavedColors();
      showSavedColors(netSlot);
      showLEDsCore2 = 1;
      debugFlagSet(11); // encoderModeOn

      //   delay(700);
      //   Serial.flush();
      //   Serial.end();

      //   delay(700);
      //       watchdog_enable(1, 1);
      // while(1);
      //*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
    }
    goto dontshowmenu;

    break;

  case 'u':
    Serial.print("\n\r");
    Serial.print("enter baud rate\n\r");
    while (Serial.available() == 0)
      ;
    baudRate = Serial.parseInt();
    Serial.print("\n\r");
    Serial.print("setting baud rate to ");
    Serial.print(baudRate);
    Serial.print("\n\r");

    setBaudRate(baudRate);
    break;

  case 'd': {
    debugFlagInit();

  debugFlags:

    Serial.print("\n\r0.   all off");
    Serial.print("\n\r9.   all on");
    Serial.print("\n\ra-z. exit\n\r");

    Serial.print("\n\r1. file parsing           =    ");
    Serial.print(debugFP);
    Serial.print("\n\r2. net manager            =    ");
    Serial.print(debugNM);
    Serial.print("\n\r3. chip connections       =    ");
    Serial.print(debugNTCC);
    Serial.print("\n\r4. chip conns alt paths   =    ");
    Serial.print(debugNTCC2);
    Serial.print("\n\r5. LEDs                   =    ");
    Serial.print(debugLEDs);
    Serial.print("\n\n\r6. swap probe pin         =    ");
    if (probeSwap == 0) {
      Serial.print("19");
    } else {
      Serial.print("18");
    }

    Serial.print("\n\n\n\r");

    while (Serial.available() == 0)
      ;

    int toggleDebug = Serial.read();
    Serial.write(toggleDebug);
    toggleDebug -= '0';

    if (toggleDebug >= 0 && toggleDebug <= 9) {

      debugFlagSet(toggleDebug);

      delay(10);

      goto debugFlags;
    } else {
      break;
    }
  }

  case ':':

    if (Serial.read() == ':') {
      // Serial.print("\n\r");
      // Serial.print("entering machine mode\n\r");
      machineMode();
      showLEDsCore2 = 1;
      goto dontshowmenu;
      break;
    } else {
      break;
    }

  default:
    while (Serial.available() > 0) {
      int f = Serial.read();
      delayMicroseconds(30);
    }

    break;
  }

  goto menu;
}

unsigned long logoFlashTimer = 0;

int arduinoReset = 0;
unsigned long lastTimeReset = 0;

unsigned long lastSwirlTime = 0;

int swirlCount = 0;
int spread = 7;

int csCycle = 0;
int onOff = 0;
float topRailVoltage = 0.0;
float botRailVoltage = 0.0;

int readcounter = 0;
unsigned long schedulerTimer = 0;
unsigned long schedulerUpdateTime = 500;
int rowProbed = 0;
int swirled = 0;
int countsss = 0;

int tempDD = 0;
void loop1() // core 2 handles the LEDs and the CH446Q8
{

  if (micros() - schedulerTimer > schedulerUpdateTime || showLEDsCore2 == 3 ||
      showLEDsCore2 == 4 && core1busy == false) {

    if (((showLEDsCore2 >= 1 && loadingFile == 0) || showLEDsCore2 == 3 ||
         swirled == 1) &&
        sendAllPathsCore2 == 0) {

         
      // Serial.println(showLEDsCore2);
      int rails =
          showLEDsCore2; // 3 doesn't show nets and keeps control of the LEDs

      // if (swirled == 1) {
      //   rails = 2;
      // }
      if (rails != 3) {
        lightUpRail(-1, -1, 1);
        logoSwirl(swirlCount, spread, probeActive);
      }

      if (rails == 5 || rails == 3) {
        logoSwirl(swirlCount, spread, probeActive);
      }

      if (rails != 2 && rails != 5 && rails != 3 && inClickMenu == 0 &&
          inPadMenu == 0) {
        // if (defconDisplay >= 0 && probeActive == 0) {
        //   // defcon(swirlCount, spread, defconDisplay);
        // } else {
         //multicore_lockout_start_blocking();
         //multicore_lockout_start_timeout_us(1000);
         while(core1busy == true){
          }
          core2busy = true;
          showNets();
          //multicore_lockout_end_timeout_us(1000);
          //multicore_lockout_end_blocking();
        //}

      } else {

        // Serial.print("showLEDsCore2 = ");
        // Serial.println(showLEDsCore2);
        // Serial.print("inClickMenu = ");
        // Serial.println(inClickMenu);
        // Serial.print("inPadMenu = ");
        // Serial.println(inPadMenu);
      }
      swirled = 0;
      // delayMicroseconds(220);

      leds.show();

      // probeLEDs.clear();
      if (checkingButton == 0) {
        // Serial.print("probeActive = ");
        // Serial.println(probeActive);
        switch (showProbeLEDs) {
        case 1:
          probeLEDs.setPixelColor(0, 0x0000ff);
          // probeLEDs.show();
          break;
        case 2:
          probeLEDs.setPixelColor(0, 0xff0000);
          // probeLEDs.show();
          break;
        case 3:
          probeLEDs.setPixelColor(0, 0x00ff00);
          // probeLEDs.show();
          break;
        case 4:
          probeLEDs.setPixelColor(0, 0xffffff);
          // probeLEDs.show();
          break;

        default:
          break;
          showProbeLEDs = 0;
        }
        probeLEDs.show();

        // } else {
        //   while (checkingButton == 1) {

        //   }
        //   probeLEDs.show();
      }

      // probeLEDs.setPixelColor(0, 0x000005);

      // probeLEDs.show();
 
      if (rails != 3) {
        showLEDsCore2 = 0;
        // delayMicroseconds(3200);
      }
      if (inClickMenu == 1) {
        rotaryEncoderStuff();
      }
      core2busy = false;

    } else if (sendAllPathsCore2 == 1) {
      // leds.show();
      //multicore_lockout_start_blocking();
      while (core1busy == true) {
        Serial.println("core1busy");
        delay(1);
      } // wait for core 1 to finish
      core2busy = true;
      digitalWrite(RESETPIN, HIGH);
      delayMicroseconds(50);
      digitalWrite(RESETPIN, LOW);
      delayMicroseconds(2200);
      sendAllPaths();
      delayMicroseconds(2200);
      //multicore_lockout_end_blocking();
      // showNets();
      // leds.show();
      // delayMicroseconds(7200);
      // showLEDsCore2 = 1;
      // chooseShownReadings();
      core2busy = false;
      sendAllPathsCore2 = 0;

    } else if (millis() - lastSwirlTime > 40 && loadingFile == 0 &&
               showLEDsCore2 == 0) {
      readcounter++;

      // logoSwirl(swirlCount, spread, probeActive);

      lastSwirlTime = millis();

      if (swirlCount >= LOGO_COLOR_LENGTH - 1) {
        swirlCount = 0;

      } else {

        swirlCount++;
      }

      if (swirlCount % 10 == 0) {
        countsss++;
      }

      // defconDisplay = 0;
      if (probeActive == 0) {

        showProbeLEDs = 3;
        // Serial.print("showProbeLEDs = ");
        // Serial.println(showProbeLEDs);
      }

      // probeLEDs.setPixelColor(0, 0xffffff);
      if (showLEDsCore2 == 0) {
        swirled = 1;
      }

      // leds.show();
    } else if (inClickMenu == 0 && probeActive == 0) {

      if (((countsss > 8 && defconDisplay > 0) || countsss > 20) &&
          defconDisplay != -1) {
        countsss = 0;

        if (defconDisplay == 0) {
          tempDD++;

          if (tempDD > 6) {
            tempDD = 0;
          }
          // defconDisplay = tempDD;
        } else {
          // defconDisplay = 0;
        }
      }

      if (defconDisplay > 6) {
        // defconDisplay = 0;
      }
      if (readcounter > 20) {
        readcounter = 0;
        // setGPIO();
        // showLEDsCore2 = 1;

        // readGPIO();
      }

      // readGPIO();
//multicore_lockout_start_blocking();
      rotaryEncoderStuff();
       //multicore_lockout_end_blocking();
      if (probeActive == 0) {
        // showLEDmeasurements();
      }
    }
    schedulerTimer = micros();
  }
}

unsigned long lastTimeNetlistLoaded = 0;
unsigned long lastTimeCommandRecieved = 0;

void machineMode(void) // read in commands in machine readable format
{
  int sequenceNumber = -1;

  lastTimeCommandRecieved = millis();

  if (millis() - lastTimeCommandRecieved > 100) {
    machineModeRespond(sequenceNumber, true);
    return;
  }
  enum machineModeInstruction receivedInstruction =
      parseMachineInstructions(&sequenceNumber);

  // Serial.print("receivedInstruction: ");
  // Serial.print(receivedInstruction);
  // Serial.print("\n\r");

  switch (receivedInstruction) {
  case netlist:
    lastTimeNetlistLoaded = millis();
    clearAllNTCC();

    // writeNodeFileFromInputBuffer();

    digitalWrite(RESETPIN, HIGH);

    machineNetlistToNetstruct();
    populateBridgesFromNodes();
    bridgesToPaths();

    clearLEDs();
    assignNetColors();
    // showNets();
    digitalWrite(RESETPIN, LOW);
    sendAllPathsCore2 = 1;
    break;

  case getnetlist:
    if (millis() - lastTimeNetlistLoaded > 300) {

      listNetsMachine();
    } else {
      machineModeRespond(0, true);
      // Serial.print ("too soon bro\n\r");
      return;
    }
    break;

  case bridgelist:
    clearAllNTCC();

    writeNodeFileFromInputBuffer();

    openNodeFile();
    getNodesToConnect();
    // Serial.print("openNF\n\r");
    digitalWrite(RESETPIN, HIGH);
    bridgesToPaths();
    clearLEDs();
    assignNetColors();
    // Serial.print("bridgesToPaths\n\r");
    digitalWrite(RESETPIN, LOW);
    // showNets();

    sendAllPathsCore2 = 1;
    break;

  case getbridgelist:
    listBridgesMachine();
    break;

  case lightnode:
    lightUpNodesFromInputBuffer();
    break;

  case lightnet:
    lightUpNetsFromInputBuffer();
    //   lightUpNet();
    // assignNetColors();
    // showLEDsCore2 = 1;
    break;

    // case getmeasurement:
    //   showMeasurements();
    //   break;

  case setsupplyswitch:

    supplySwitchPosition = setSupplySwitch();
    // printSupplySwitch(supplySwitchPosition);
    machineModeRespond(sequenceNumber, true);

    showLEDsCore2 = 1;
    break;

  case getsupplyswitch:
    // if (millis() - lastTimeNetlistLoaded > 100)
    //{

    printSupplySwitch(supplySwitchPosition);
    // machineModeRespond(sequenceNumber, true);

    // }else {
    // Serial.print ("\n\rtoo soon bro\n\r");
    // machineModeRespond(0, true);
    // return;
    // }
    break;

  case getchipstatus:
    printChipStatusMachine();
    break;

    // case gpio:
    //   break;
  case getunconnectedpaths:
    getUnconnectedPaths();
    break;

  case unknown:
    machineModeRespond(sequenceNumber, false);
    return;
  }

  machineModeRespond(sequenceNumber, true);
}