// SPDX-License-Identifier: MIT
#include "FileParsing.h"
#include <Arduino.h>
#include "LittleFS.h"
#include "MatrixStateRP2040.h"
#include "SafeString.h"
#include "ArduinoJson.h"
#include "NetManager.h"
#include "JumperlessDefinesRP2040.h"
#include "LEDs.h"
#include <EEPROM.h>
#include "MachineCommands.h"
#include "Probing.h"

bool debugFP = EEPROM.read(DEBUG_FILEPARSINGADDRESS);
bool debugFPtime = EEPROM.read(TIME_FILEPARSINGADDRESS);

createSafeString(nodeFileString, 1200);

int numConnsJson = 0;
createSafeString(specialFunctionsString, 800);

char inputBuffer[INPUTBUFFERLENGTH] = {0};

ArduinoJson::StaticJsonDocument<8000> wokwiJson;

String connectionsW[MAX_BRIDGES][5];

File nodeFile;

File wokwiFile;

unsigned long timeToFP = 0;

void savePreformattedNodeFile(int source)
{
    LittleFS.remove("nodeFile.txt");

    nodeFile = LittleFS.open("nodeFile.txt", "w+");
    if (debugFP)
    {
        Serial.print("source = ");
        Serial.println(source);
    }
    if (source == 0)
    {
        while (Serial.available() == 0)
        {
        }

        while (Serial.available() > 0)
        {
            nodeFile.write(Serial.read());
            delayMicroseconds(800);
        }
    }
    if (source == 1)
    {
        nodeFile.print("f 117-D1, 116-D0,");
        while (Serial1.available() == 0)
        {
        }
        delayMicroseconds(900);
        // Serial.println("waiting for Arduino to send file");
        while (Serial1.available() > 0)
        {
            nodeFile.write(Serial1.read());
            delayMicroseconds(800);
            // Serial.println(Serial1.available());
        }

        while (Serial1.available() > 0)
        {
            Serial1.read();
            delay(1);
        }
    }
    // nodeFile.write("\n\r");

    //     nodeFile.seek(0);
    //     nodeFileString.read(nodeFile);
    //    Serial.println(nodeFileString);

    nodeFile.close();
}

void printNodeFile(void)
{
    nodeFile = LittleFS.open("nodeFile.txt", "r");
    if (!nodeFile)
    {
        // if (debugFP)
        // Serial.println("Failed to open nodeFile");
        return;
    }
    else
    {
        // if (debugFP)
        // Serial.println("\n\ropened nodeFile.txt\n\n\rloading bridges from file\n\r");
    }
    nodeFileString.clear();

    nodeFileString.read(nodeFile);
    nodeFile.close();
    // Serial.print(nodeFileString);

    //     int newLines = 0;
    // Serial.println(nodeFileString.indexOf(","));
    // Serial.println(nodeFileString.charAt(nodeFileString.indexOf(",")+1));
    // Serial.println(nodeFileString.indexOf(","));

    if (nodeFileString.charAt(nodeFileString.indexOf(",") + 1) != '\n')
    {
        nodeFileString.replace(",", ",\n\r");
        nodeFileString.replace("{", "{\n\r");
    }

    // int nodeFileStringLength = nodeFileString.indexOf("}");
    // if (nodeFileStringLength != -1)
    // {
    // //nodeFileString.remove(nodeFileStringLength + 1, -1);
    // }

    // if (nodeFileString.indexOf(",\n\r") == -1)
    // {
    //     nodeFileString.replace(",", ",\n\r");
    //     nodeFileString.replace("{", "{\n\r");
    // }

    // nodeFile.close();

    int nodeFileStringLength = nodeFileString.indexOf("}");
    if (nodeFileStringLength != -1)
    {
        if (nodeFileString.charAt(nodeFileStringLength + 1) != '\n')
        {
            nodeFileString.replace("}", "}\n\r");
        }
        nodeFileString.remove(nodeFileStringLength + 2, -1);
    }

    // nodeFileString.readUntilToken(nodeFileString, "{");
    // nodeFileString.removeLast(9);

    Serial.print(nodeFileString);
    // Serial.print('*');
    nodeFileString.clear();
}

void parseWokwiFileToNodeFile(void)
{

    // delay(3000);
    LittleFS.begin();
    timeToFP = millis();

    wokwiFile = LittleFS.open("wokwi.txt", "w+");
    if (!wokwiFile)
    {
        if (debugFP)
            Serial.println("Failed to open wokwi.txt");
        return;
    }
    else
    {
        if (debugFP)
        {
            Serial.println("\n\ropened wokwi.txt\n\r");
        }
        else
        {
            // Serial.println("\n\r");
        }
    }

    Serial.println("paste Wokwi diagram.json here\n\r");
    while (Serial.available() == 0)
    {
    }

    int numCharsRead = 0;

    char firstChar = Serial.read();

    if (firstChar != '{') // in case you just paste a wokwi file in from the menu, the opening brace will have already been read
    {
        inputBuffer[numCharsRead] = '{';
        numCharsRead++;
    }
    else
    {
        inputBuffer[numCharsRead] = firstChar;
        numCharsRead++;
    }
    /*
        Serial.println(firstChar);
      Serial.println(firstChar);
        Serial.println(firstChar);
       Serial.println(firstChar);
       Serial.println(firstChar);
       Serial.print(firstChar);
    */
    delay(1);
    while (Serial.available() > 0)
    {
        char c = Serial.read();
        inputBuffer[numCharsRead] = c;

        numCharsRead++;

        delayMicroseconds(1000);
    }

    createSafeStringFromCharArray(wokwiFileString, inputBuffer);
    delay(3);
    wokwiFile.write(inputBuffer, numCharsRead);

    delay(3);

    wokwiFile.seek(0);

    if (debugFP)
        Serial.println("\n\n\rwokwiFile\n\n\r");

    /* for (int i = 0; i < numCharsRead; i++)
     {
         Serial.print((char)wokwiFile.read());
     }*/
    if (debugFP)
    {
        Serial.print(wokwiFileString);

        Serial.println("\n\n\rnumCharsRead = ");

        Serial.print(numCharsRead);

        Serial.println("\n\n\r");
    }
    wokwiFile.close();

    deserializeJson(wokwiJson, inputBuffer);

    if (debugFP)
    {

        Serial.println("\n\n\rwokwiJson\n\n\r");

        Serial.println("\n\n\rconnectionsW\n\n\r");
    }

    numConnsJson = wokwiJson["connections"].size();

    copyArray(wokwiJson["connections"], connectionsW);

    // deserializeJson(connectionsW, Serial);
    if (debugFP)
    {
        Serial.println(wokwiJson["connections"].size());

        for (int i = 0; i < MAX_BRIDGES; i++)
        {
            // Serial.println(wokwiJson["connections"].size());
            if (connectionsW[i][0] == "")
            {
                break;
            }
            Serial.print(connectionsW[i][0]);
            Serial.print(",   \t ");

            Serial.print(connectionsW[i][1]);
            Serial.print(",   \t ");

            Serial.print(connectionsW[i][2]);
            Serial.print(",   \t ");

            Serial.print(connectionsW[i][3]);
            Serial.print(",   \t ");

            Serial.print(connectionsW[i][4]);
            Serial.print(",   \t ");

            Serial.println();
        }

        Serial.println("\n\n\rRedefining\n\n\r");
    }

    changeWokwiDefinesToJumperless();

    writeToNodeFile();
    // while(1);
    openNodeFile();
}

void changeWokwiDefinesToJumperless(void)
{

    String connString1 = "                               ";
    String connString2 = "                               ";
    String connStringColor = "                               ";
    String bb = "bb1:";

    int nodeNumber;

    for (int i = 0; i < numConnsJson; i++)
    {
        if (debugFP)
        {
            Serial.println(' ');
        }
        for (int j = 0; j < 2; j++)
        {
            nodeNumber = -1;
            connString1 = connectionsW[i][j];
            if (debugFP)
            {
                Serial.print(connString1);
                Serial.print("   \t\t  ");
            }
            if (connString1.startsWith("bb1:") || connString1.startsWith("bb2:"))
            {
                // Serial.print("bb1 or bb2  ");

                int periodIndex = connString1.indexOf(".");
                connString1 = connString1.substring(4, periodIndex);

                if (connString1.endsWith("b"))
                {
                    nodeNumber = 30;
                    // Serial.println("bottom");
                    connString1.substring(0, connString1.length() - 1);
                    nodeNumber += connString1.toInt();
                }
                else if (connString1.endsWith("t"))
                {
                    nodeNumber = 0;
                    // Serial.println("top");
                    connString1.substring(0, connString1.length() - 1);
                    nodeNumber += connString1.toInt();
                }
                else if (connString1.endsWith("n"))
                {
                    nodeNumber = GND;
                }
                else if (connString1.endsWith("p"))
                {
                    nodeNumber = SUPPLY_5V;
                }
            }
            else if (connString1.startsWith("nano:"))
            {
                // Serial.print("nano\t");
                int periodIndex = connString1.indexOf(".");
                connString1 = connString1.substring(5, periodIndex);

                nodeNumber = NANO_D0;

                if (isDigit(connString1[connString1.length() - 1]))
                {

                    nodeNumber += connString1.toInt();
                }
                else if (connString1.equals("5V"))
                {
                    nodeNumber = SUPPLY_5V;
                }
                else if (connString1.equalsIgnoreCase("AREF"))
                {

                    nodeNumber = NANO_AREF;
                }
                else if (connString1.equalsIgnoreCase("GND"))
                {
                    nodeNumber = GND;
                }
                else if (connString1.equalsIgnoreCase("RESET"))
                {

                    nodeNumber = NANO_RESET;
                }
                else if (connString1.equalsIgnoreCase("3.3V"))
                {
                    nodeNumber = SUPPLY_3V3;
                }
                else if (connString1.startsWith("A"))
                {
                    nodeNumber = NANO_A0;
                    nodeNumber += connString1.toInt();
                }
            }
            else if (connString1.startsWith("vcc1:"))
            {
                // Serial.print("vcc1\t");
                nodeNumber = SUPPLY_5V;
            }
            else if (connString1.startsWith("vcc2:"))
            {
                // Serial.print("vcc2\t");
                nodeNumber = SUPPLY_3V3;
            }
            else if (connString1.startsWith("gnd1:"))
            {
                // Serial.print("gnd1\t");
                nodeNumber = GND;
            }
            else if (connString1.startsWith("gnd2:"))
            {
                // Serial.print("gnd2\t");
                nodeNumber = GND;
            }
            else if (connString1.startsWith("gnd3:"))
            {
                nodeNumber = GND;
            }
            else if (connString1.startsWith("pot1:"))
            {
                nodeNumber = DAC0;
            }
            else
            {

                connectionsW[i][j] = -1;
            }

            // nodeNumber += connString1.toInt();

            connectionsW[i][j] = nodeNumber;
            if (debugFP)
            {
                Serial.print(connectionsW[i][j]);

                Serial.print("   \t ");
            }
        }
    }
}
void clearNodeFile(void)
{
    LittleFS.remove("nodeFile.txt");
    nodeFile = LittleFS.open("nodeFile.txt", "w+");
    nodeFile.print("!");
    nodeFile.close();
}

void removeBridgeFromNodeFile(int node1, int node2)
{

    nodeFile = LittleFS.open("nodeFile.txt", "r+");
    if (!nodeFile)
    {
        if (debugFP)
            Serial.println("Failed to open nodeFile");
        return;
    }
    else
    {
        if (debugFP)
            Serial.println("\n\ropened nodeFile.txt\n\n\rloading bridges from file\n\r");
    }

    nodeFileString.clear();

    nodeFileString.read(nodeFile);
    nodeFile.close();

    char nodeAsChar[20];
    itoa(node1, nodeAsChar, 10);
    char paddedChar[21];

    paddedChar[0] = ' ';
    for (int i = 1; i < 20; i++)
    {
        if (nodeAsChar[i - 1] == '\0')
        {
            paddedChar[i] = ' ';
            paddedChar[i + 1] = '\0';
            break;
        }
        paddedChar[i] = nodeAsChar[i - 1];
    }

    int numberOfLines = 0;
    // Serial.print(paddedChar);
    // Serial.println("*");

    char lines[100][20];

    int lineIndex = 0;
    int charIndex = 0;

    for (int i = 0; i < 100; i++)
    {

        if (nodeFileString[charIndex] == '\0')
        {
            numberOfLines = i;
            break;
        }
        lines[i][0] = ' ';
        for (int j = 1; j < 20; j++)
        {
            if (nodeFileString[charIndex] == ',')
            {
                lines[i][j] = ' ';
                lines[i][j + 1] = ',';
                // lines[i][j + 2] = '\n';
                // lines[i][j + 3] = '\r';
                lines[i][j + 2] = '\0';

                charIndex++;
                break;
            }
            else if (nodeFileString[charIndex] == '-')
            {
                lines[i][j] = ' ';
                lines[i][j + 1] = '-';
                lines[i][j + 2] = ' ';
                j += 2;
                charIndex++;
                // break;
            }
            else if (nodeFileString[charIndex] == '\n' || nodeFileString[charIndex] == '\r' || nodeFileString[charIndex] == '{' || nodeFileString[charIndex] == '}')
            {
                lines[i][j] = ' ';
                charIndex++;
            }
            else
            {
                lines[i][j] = nodeFileString[charIndex];
                charIndex++;
            }
        }
    }

    // Serial.println("\n\r********");

    for (int i = 0; i < numberOfLines; i++)
    {
        if (lines[i][0] == '\0')
        {
            // break;
        }
        if (strstr(lines[i], paddedChar) != NULL)
        {
            // Serial.println(lines[i]);
            // delay(1);

            for (int j = 0; j < 18; j++)
            {
                lines[i][j] = ' ';
            }
            // lines[i][18] = '\n';
            // lines[i][19] = '\r';
            lines[i][0] = '\0';
        }
    }

    nodeFileString.clear();
    nodeFileString.concat("{");
    for (int i = 0; i < numberOfLines; i++)
    {
        if (lines[i][0] == '\0')
        {
            continue;
        }
        // Serial.println(i);
        // delay(1);
        for (int j = 0; j < 20; j++)
        {
            if (lines[i][j] == '\0')
            {
                break;
            }
            if (lines[i][j] == ' ')
            {
                continue;
            }
            nodeFileString.concat(lines[i][j]);
            // Serial.print(lines[i][j]);
            // delay(1);
        }
    }
    nodeFileString.concat("}\n\r");

    nodeFile.close();
    nodeFile = LittleFS.open("nodeFile.txt", "w+");
    nodeFile.write(nodeFileString.c_str());

    nodeFile.close();
}

void addBridgeToNodeFile(int node1, int node2)
{
    nodeFile = LittleFS.open("nodeFile.txt", "r+");
    if (!nodeFile)
    {
        if (debugFP)
            Serial.println("Failed to open nodeFile");
        return;
    }
    else
    {
        if (debugFP)
            Serial.println("\n\ropened nodeFile.txt\n\n\rloading bridges from file\n\r");
    }

    int nodeFileBraceIndex = 0;

    while (nodeFile.available())
    {
        char c = nodeFile.read();
        // Serial.print(c);

        if (c == '}')
        {
            break;
        }
        else
        {
            nodeFileBraceIndex++;
        }

        if (c == '!')
        {
            nodeFile.seek(0);
            nodeFile.print("{\n\r");
            nodeFile.print(node1);
            nodeFile.print("-");
            nodeFile.print(node2);
            nodeFile.print(",\n\r}\n\r");

            // if (1)
            // {
            //     Serial.println("wrote to nodeFile.txt");

            //     Serial.println("nodeFile.txt contents:\n\r");
            //     nodeFile.seek(0);

            //     while (nodeFile.available())
            //     {
            //         Serial.write(nodeFile.read());
            //     }
            //     Serial.println("\n\r");
            // }
            // Serial.print (nodeFile);
            nodeFile.close();
            return;
        }
    }
    // Serial.println(nodeFileBraceIndex);
    nodeFile.seek(nodeFileBraceIndex);

    nodeFile.print(node1);
    nodeFile.print("-");
    nodeFile.print(node2);
    nodeFile.print(",}\n\r");

    if (debugFP)
    {
        Serial.println("wrote to nodeFile.txt");

        Serial.println("nodeFile.txt contents:\n\r");
        nodeFile.seek(0);

        while (nodeFile.available())
        {
            Serial.write(nodeFile.read());
        }
        Serial.println("\n\r");
    }
    nodeFile.close();
}

void writeToNodeFile(void)
{

    LittleFS.remove("nodeFile.txt");
    delayMicroseconds(100);
    nodeFile = LittleFS.open("nodeFile.txt", "w+");
    if (!nodeFile)
    {
        if (debugFP)
            Serial.println("Failed to open nodeFile");
        return;
    }
    else
    {
        if (debugFP)
            Serial.println("\n\rrecreated nodeFile.txt\n\n\rloading bridges from wokwi.txt\n\r");
    }
    nodeFile.print("{\n\r");
    for (int i = 0; i < numConnsJson; i++)
    {
        if (connectionsW[i][0] == "-1" && connectionsW[i][1] != "-1")
        {
            // lightUpNode(connectionsW[i][0].toInt());
            continue;
        }
        if (connectionsW[i][1] == "-1" && connectionsW[i][0] != "-1")
        {
            // lightUpNode(connectionsW[i][1].toInt());
            continue;
        }
        if (connectionsW[i][0] == connectionsW[i][1])
        {
            // lightUpNode(connectionsW[i][0].toInt());
            continue;
        }

        nodeFile.print(connectionsW[i][0]);
        nodeFile.print("-");
        nodeFile.print(connectionsW[i][1]);
        nodeFile.print(",\n\r");
    }
    nodeFile.print("\n\r}\n\r");

    if (debugFP)
    {
        Serial.println("wrote to nodeFile.txt");

        Serial.println("nodeFile.txt contents:");
        nodeFile.seek(0);

        while (nodeFile.available())
        {
            Serial.write(nodeFile.read());
        }
        Serial.println("\n\r");
    }
    nodeFile.close();
}

void openNodeFile()
{
    timeToFP = millis();

    nodeFile = LittleFS.open("nodeFile.txt", "r");
    if (!nodeFile)
    {
        if (debugFP)
            Serial.println("Failed to open nodeFile");
        return;
    }
    else
    {
        if (debugFP)
            Serial.println("\n\ropened nodeFile.txt\n\n\rloading bridges from file\n\r");
    }

    nodeFileString.clear();
    nodeFileString.read(nodeFile);

    nodeFile.close();
    splitStringToFields();
    // parseStringToBridges();
}

void splitStringToFields()
{
    int openBraceIndex = 0;
    int closeBraceIndex = 0;

    if (debugFP)
        Serial.println("\n\rraw input file\n\r");
    if (debugFP)
        Serial.println(nodeFileString);
    if (debugFP)
        Serial.println("\n\rsplitting and cleaning up string\n\r");
    if (debugFP)
        Serial.println("_");

    openBraceIndex = nodeFileString.indexOf("{");
    closeBraceIndex = nodeFileString.indexOf("}");
    int fIndex = nodeFileString.indexOf("f");

    int foundOpenBrace = -1;
    int foundCloseBrace = -1;
    int foundF = -1;

    if (openBraceIndex != -1)
    {
        foundOpenBrace = 1;
    }
    if (closeBraceIndex != -1)
    {
        foundCloseBrace = 1;
    }
    if (fIndex != -1)
    {
        foundF = 1;
    }

    // Serial.println(openBraceIndex);
    // Serial.println(closeBraceIndex);
    // Serial.println(fIndex);

    if (foundF == 1)
    {
        nodeFileString.substring(nodeFileString, fIndex + 1, nodeFileString.length());
    }

    if (foundOpenBrace == 1 && foundCloseBrace == 1)
    {

        nodeFileString.substring(specialFunctionsString, openBraceIndex + 1, closeBraceIndex);
    }
    else
    {
        nodeFileString.substring(specialFunctionsString, 0, -1); // nodeFileString.length());
    }
    specialFunctionsString.trim();

    if (debugFP)
        Serial.println(specialFunctionsString);

    if (debugFP)
        Serial.println("^\n\r");
    /*
        nodeFileString.remove(0, closeBraceIndex + 1);
        nodeFileString.trim();

        openBraceIndex = nodeFileString.indexOf("{");
        closeBraceIndex = nodeFileString.indexOf("}");
        //nodeFileString.substring(specialFunctionsString, openBraceIndex + 1, closeBraceIndex);
        specialFunctionsString.trim();
        if(debugFP)Serial.println("_");
        if(debugFP)Serial.println(specialFunctionsString);
        if(debugFP)Serial.println("^\n\r");
        */
    replaceSFNamesWithDefinedInts();
    replaceNanoNamesWithDefinedInts();
    parseStringToBridges();
}

void replaceSFNamesWithDefinedInts(void)
{
    specialFunctionsString.toUpperCase();
    if (debugFP)
    {
        Serial.println("replacing special function names with defined ints\n\r");
        Serial.println(specialFunctionsString);
    }

    specialFunctionsString.replace("GND", "100");
    specialFunctionsString.replace("GROUND", "100");
    specialFunctionsString.replace("SUPPLY_5V", "105");
    specialFunctionsString.replace("SUPPLY_3V3", "103");

    specialFunctionsString.replace("DAC0_5V", "106");
    specialFunctionsString.replace("DAC1_8V", "107");
    specialFunctionsString.replace("DAC0", "106");
    specialFunctionsString.replace("DAC1", "107");

    specialFunctionsString.replace("INA_N", "109");
    specialFunctionsString.replace("INA_P", "108");
    specialFunctionsString.replace("I_N", "109");
    specialFunctionsString.replace("I_P", "108");
    specialFunctionsString.replace("CURRENT_SENSE_MINUS", "109");
    specialFunctionsString.replace("CURRENT_SENSE_PLUS", "108");
    specialFunctionsString.replace("ISENSE_MINUS", "109");
    specialFunctionsString.replace("ISENSE_PLUS", "108");

    specialFunctionsString.replace("EMPTY_NET", "127");

    specialFunctionsString.replace("ADC0_5V", "110");
    specialFunctionsString.replace("ADC1_5V", "111");
    specialFunctionsString.replace("ADC2_5V", "112");
    specialFunctionsString.replace("ADC3_8V", "113");
    specialFunctionsString.replace("ADC0", "110");
    specialFunctionsString.replace("ADC1", "111");
    specialFunctionsString.replace("ADC2", "112");
    specialFunctionsString.replace("ADC3", "113");

    specialFunctionsString.replace("+5V", "105");
    specialFunctionsString.replace("5V", "105");
    specialFunctionsString.replace("3.3V", "103");
    specialFunctionsString.replace("3V3", "103");

    specialFunctionsString.replace("RP_GPIO_0", "114");
    specialFunctionsString.replace("RP_UART_TX", "116");
    specialFunctionsString.replace("RP_UART_RX", "117");
    specialFunctionsString.replace("GPIO_0", "114");
    specialFunctionsString.replace("UART_TX", "116");
    specialFunctionsString.replace("UART_RX", "117");
}

void replaceNanoNamesWithDefinedInts(void) // for dome reason Arduino's String wasn't replacing like 1 or 2 of the names, so I'm using SafeString now and it works
{
    if (debugFP)
        Serial.println("replacing special function names with defined ints\n\r");

    char nanoName[5];

    itoa(NANO_D10, nanoName, 10);
    specialFunctionsString.replace("D10", nanoName);

    itoa(NANO_D11, nanoName, 10);
    specialFunctionsString.replace("D11", nanoName);

    itoa(NANO_D12, nanoName, 10);
    specialFunctionsString.replace("D12", nanoName);

    itoa(NANO_D13, nanoName, 10);
    specialFunctionsString.replace("D13", nanoName);

    itoa(NANO_D0, nanoName, 10);
    specialFunctionsString.replace("D0", nanoName);

    itoa(NANO_D1, nanoName, 10);
    specialFunctionsString.replace("D1", nanoName);

    itoa(NANO_D2, nanoName, 10);
    specialFunctionsString.replace("D2", nanoName);

    itoa(NANO_D3, nanoName, 10);
    specialFunctionsString.replace("D3", nanoName);

    itoa(NANO_D4, nanoName, 10);
    specialFunctionsString.replace("D4", nanoName);

    itoa(NANO_D5, nanoName, 10);
    specialFunctionsString.replace("D5", nanoName);

    itoa(NANO_D6, nanoName, 10);
    specialFunctionsString.replace("D6", nanoName);

    itoa(NANO_D7, nanoName, 10);
    specialFunctionsString.replace("D7", nanoName);

    itoa(NANO_D8, nanoName, 10);
    specialFunctionsString.replace("D8", nanoName);

    itoa(NANO_D9, nanoName, 10);
    specialFunctionsString.replace("D9", nanoName);

    itoa(NANO_RESET, nanoName, 10);
    specialFunctionsString.replace("RESET", nanoName);

    itoa(NANO_AREF, nanoName, 10);
    specialFunctionsString.replace("AREF", nanoName);

    itoa(NANO_A0, nanoName, 10);
    specialFunctionsString.replace("A0", nanoName);

    itoa(NANO_A1, nanoName, 10);
    specialFunctionsString.replace("A1", nanoName);

    itoa(NANO_A2, nanoName, 10);
    specialFunctionsString.replace("A2", nanoName);

    itoa(NANO_A3, nanoName, 10);
    specialFunctionsString.replace("A3", nanoName);

    itoa(NANO_A4, nanoName, 10);
    specialFunctionsString.replace("A4", nanoName);

    itoa(NANO_A5, nanoName, 10);
    specialFunctionsString.replace("A5", nanoName);

    itoa(NANO_A6, nanoName, 10);
    specialFunctionsString.replace("A6", nanoName);

    itoa(NANO_A7, nanoName, 10);
    specialFunctionsString.replace("A7", nanoName);

    // if(debugFP)Serial.println(bridgeString);
    if (debugFP)
        Serial.println(specialFunctionsString);
    if (debugFP)
        Serial.println("\n\n\r");
}

void parseStringToBridges(void)
{

    // int bridgeStringLength = bridgeString.length() - 1;

    int specialFunctionsStringLength = specialFunctionsString.length() - 1;

    int readLength = 0;

    newBridgeLength = 0;
    newBridgeIndex = 0;

    if (debugFP)
    {
        Serial.println("parsing bridges into array\n\r");
    }
    int stringIndex = 0;
    char delimitersCh[] = "[,- \n\r";

    createSafeString(buffer, 10);
    createSafeStringFromCharArray(delimiters, delimitersCh);
    int doneReading = 0;

    for (int i = 0; i <= specialFunctionsStringLength; i++)
    {

        stringIndex = specialFunctionsString.stoken(buffer, stringIndex, delimiters);

        // Serial.print("buffer = ");
        // Serial.println(buffer);

        // Serial.print("stringIndex = ");
        // Serial.println(stringIndex);

        buffer.toInt(path[newBridgeIndex].node1);

        // Serial.print("path[newBridgeIndex].node1 = ");
        // Serial.println(path[newBridgeIndex].node1);

        if (debugFP)
        {
            Serial.print("node1 = ");
            Serial.println(path[newBridgeIndex].node1);
        }

        stringIndex = specialFunctionsString.stoken(buffer, stringIndex, delimiters);

        buffer.toInt(path[newBridgeIndex].node2);

        if (debugFP)
        {
            Serial.print("node2 = ");
            Serial.println(path[newBridgeIndex].node2);
        }

        readLength = stringIndex;

        if (readLength == -1)
        {
            doneReading = 1;
            break;
        }
        newBridgeLength++;
        newBridgeIndex++;

        if (debugFP)
        {
            Serial.print("readLength = ");
            Serial.println(readLength);
            Serial.print("specialFunctionsString.length() = ");
            Serial.println(specialFunctionsString.length());
        }

        if (debugFP)
            Serial.print(newBridgeIndex);
        if (debugFP)
            Serial.print("-");
        if (debugFP)
            Serial.println(newBridge[newBridgeIndex][1]);
    }

    newBridgeIndex = 0;
    if (debugFP)
        for (int i = 0; i < newBridgeLength; i++)
        {
            Serial.print("[");
            Serial.print(path[newBridgeIndex].node1);
            Serial.print("-");
            Serial.print(path[newBridgeIndex].node2);
            Serial.print("],");
            newBridgeIndex++;
        }
    if (debugFP)
        Serial.print("\n\rbridge pairs = ");
    if (debugFP)
        Serial.println(newBridgeLength);

    nodeFileString.clear();

    // if(debugFP)Serial.println(nodeFileString);
    timeToFP = millis() - timeToFP;
    if (debugFPtime)
        Serial.print("\n\rtook ");

    if (debugFPtime)
        Serial.print(timeToFP);
    if (debugFPtime)
        Serial.println("ms to open and parse file\n\r");
}

int lenHelper(int x)
{
    if (x >= 1000000000)
        return 10;
    if (x >= 100000000)
        return 9;
    if (x >= 10000000)
        return 8;
    if (x >= 1000000)
        return 7;
    if (x >= 100000)
        return 6;
    if (x >= 10000)
        return 5;
    if (x >= 1000)
        return 4;
    if (x >= 100)
        return 3;
    if (x >= 10)
        return 2;
    return 1;
}

int printLen(int x)
{
    return x < 0 ? lenHelper(-x) + 1 : lenHelper(x);
}

void debugFlagInit(void)
{

    if (EEPROM.read(FIRSTSTARTUPADDRESS) == 255)
    {
        EEPROM.write(FIRSTSTARTUPADDRESS, 0);
        EEPROM.write(DEBUG_FILEPARSINGADDRESS, 0);
        EEPROM.write(TIME_FILEPARSINGADDRESS, 0);
        EEPROM.write(DEBUG_NETMANAGERADDRESS, 0);
        EEPROM.write(TIME_NETMANAGERADDRESS, 0);
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSADDRESS, 0);
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS, 0);
        EEPROM.write(DEBUG_LEDSADDRESS, 0);
        EEPROM.write(LEDBRIGHTNESSADDRESS, DEFAULTBRIGHTNESS);
        EEPROM.write(RAILBRIGHTNESSADDRESS, DEFAULTRAILBRIGHTNESS);
        EEPROM.write(SPECIALBRIGHTNESSADDRESS, DEFAULTSPECIALNETBRIGHTNESS);
        EEPROM.write(PROBESWAPADDRESS, 0);

        EEPROM.commit();
        delay(5);
    }

#ifdef EEPROMSTUFF
    debugFP = EEPROM.read(DEBUG_FILEPARSINGADDRESS);
    debugFPtime = EEPROM.read(TIME_FILEPARSINGADDRESS);

    debugNM = EEPROM.read(DEBUG_NETMANAGERADDRESS);
    debugNMtime = EEPROM.read(TIME_NETMANAGERADDRESS);

    debugNTCC = EEPROM.read(DEBUG_NETTOCHIPCONNECTIONSADDRESS);
    debugNTCC2 = EEPROM.read(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS);

    LEDbrightnessRail = EEPROM.read(RAILBRIGHTNESSADDRESS);
    LEDbrightness = EEPROM.read(LEDBRIGHTNESSADDRESS);
    LEDbrightnessSpecial = EEPROM.read(SPECIALBRIGHTNESSADDRESS);

    debugLEDs = EEPROM.read(DEBUG_LEDSADDRESS);

    probeSwap = EEPROM.read(PROBESWAPADDRESS);

#else

    debugFP = 1;
    debugFPtime = 1;

    debugNM = 1;
    debugNMtime = 1;

    debugNTCC = 1;
    debugNTCC2 = 1;

    // debugLEDs = 1;
#endif

    if (debugFP != 0 && debugFP != 1)
        EEPROM.write(DEBUG_FILEPARSINGADDRESS, 0);

    if (debugFPtime != 0 && debugFPtime != 1)
        EEPROM.write(TIME_FILEPARSINGADDRESS, 0);

    if (debugNM != 0 && debugNM != 1)
        EEPROM.write(DEBUG_NETMANAGERADDRESS, 0);

    if (debugNMtime != 0 && debugNMtime != 1)
        EEPROM.write(TIME_NETMANAGERADDRESS, 0);

    if (debugNTCC != 0 && debugNTCC != 1)
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSADDRESS, 0);

    if (debugNTCC2 != 0 && debugNTCC2 != 1)
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS, 0);

    if (debugLEDs != 0 && debugLEDs != 1)
        EEPROM.write(DEBUG_LEDSADDRESS, 0);

    if (LEDbrightnessRail < 0 || LEDbrightnessRail > 200)
    {
        EEPROM.write(RAILBRIGHTNESSADDRESS, DEFAULTRAILBRIGHTNESS);

        LEDbrightnessRail = DEFAULTRAILBRIGHTNESS;
    }
    if (LEDbrightness < 0 || LEDbrightness > 200)
    {
        EEPROM.write(LEDBRIGHTNESSADDRESS, DEFAULTBRIGHTNESS);
        LEDbrightness = DEFAULTBRIGHTNESS;
    }

    if (LEDbrightnessSpecial < 0 || LEDbrightnessSpecial > 200)
    {
        EEPROM.write(SPECIALBRIGHTNESSADDRESS, DEFAULTSPECIALNETBRIGHTNESS);
        LEDbrightnessSpecial = DEFAULTSPECIALNETBRIGHTNESS;
    }

    EEPROM.commit();
    delay(5);
}

void debugFlagSet(int flag)
{
    int flagStatus;
    switch (flag)
    {
    case 1:
    {
        flagStatus = EEPROM.read(DEBUG_FILEPARSINGADDRESS);
        if (flagStatus == 0)
        {
            EEPROM.write(DEBUG_FILEPARSINGADDRESS, 1);

            debugFP = true;
        }
        else
        {
            EEPROM.write(DEBUG_FILEPARSINGADDRESS, 0);

            debugFP = false;
        }

        break;
    }

    case 2:
    {
        flagStatus = EEPROM.read(DEBUG_NETMANAGERADDRESS);

        if (flagStatus == 0)
        {
            EEPROM.write(DEBUG_NETMANAGERADDRESS, 1);

            debugNM = true;
        }
        else
        {
            EEPROM.write(DEBUG_NETMANAGERADDRESS, 0);

            debugNM = false;
        }
        break;
    }

    case 3:
    {
        flagStatus = EEPROM.read(DEBUG_NETTOCHIPCONNECTIONSADDRESS);

        if (flagStatus == 0)
        {
            EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSADDRESS, 1);

            debugNTCC = true;
        }
        else
        {
            EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSADDRESS, 0);

            debugNTCC = false;
        }

        break;
    }
    case 4:
    {
        flagStatus = EEPROM.read(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS);

        if (flagStatus == 0)
        {
            EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS, 1);

            debugNTCC2 = true;
        }
        else
        {
            EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS, 0);

            debugNTCC2 = false;
        }
        break;
    }

    case 5:
    {
        flagStatus = EEPROM.read(DEBUG_LEDSADDRESS);

        if (flagStatus == 0)
        {
            EEPROM.write(DEBUG_LEDSADDRESS, 1);

            debugLEDs = true;
        }
        else
        {
            EEPROM.write(DEBUG_LEDSADDRESS, 0);

            debugLEDs = false;
        }
        break;
    }

    case 6:
    {
        flagStatus = EEPROM.read(PROBESWAPADDRESS);

        if (flagStatus == 0)
        {
            EEPROM.write(PROBESWAPADDRESS, 1);

            probeSwap = true;
        }
        else
        {
            EEPROM.write(PROBESWAPADDRESS, 0);

            probeSwap = false;
        }
        break;
    }

    case 0:
    {
        EEPROM.write(DEBUG_FILEPARSINGADDRESS, 0);
        EEPROM.write(TIME_FILEPARSINGADDRESS, 0);
        EEPROM.write(DEBUG_NETMANAGERADDRESS, 0);
        EEPROM.write(TIME_NETMANAGERADDRESS, 0);
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSADDRESS, 0);
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS, 0);
        EEPROM.write(DEBUG_LEDSADDRESS, 0);

        debugFP = false;
        debugFPtime = false;
        debugNM = false;
        debugNMtime = false;
        debugNTCC = false;
        debugNTCC2 = false;
        debugLEDs = false;

        break;
    }

    case 9:
    {
        EEPROM.write(DEBUG_FILEPARSINGADDRESS, 1);
        EEPROM.write(TIME_FILEPARSINGADDRESS, 1);
        EEPROM.write(DEBUG_NETMANAGERADDRESS, 1);
        EEPROM.write(TIME_NETMANAGERADDRESS, 1);
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSADDRESS, 1);
        EEPROM.write(DEBUG_NETTOCHIPCONNECTIONSALTADDRESS, 1);
        EEPROM.write(DEBUG_LEDSADDRESS, 1);
        debugFP = true;
        debugFPtime = true;
        debugNM = true;
        debugNMtime = true;
        debugNTCC = true;
        debugNTCC2 = true;
        debugLEDs = true;
        break;
    }
    }
    delay(4);
    EEPROM.commit();
    delay(8);
    return;
}

void runCommandAfterReset(char command)
{
    if (EEPROM.read(CLEARBEFORECOMMANDADDRESS) == 1)
    {
        return;
    }
    else
    {

        EEPROM.write(CLEARBEFORECOMMANDADDRESS, 1);
        EEPROM.write(LASTCOMMANDADDRESS, command);
        EEPROM.commit();

        digitalWrite(RESETPIN, HIGH);
        delay(1);
        digitalWrite(RESETPIN, LOW);

        AIRCR_Register = 0x5FA0004; // hard reset
    }
}

char lastCommandRead(void)
{

    Serial.print("last command: ");

    Serial.println((char)EEPROM.read(LASTCOMMANDADDRESS));

    return EEPROM.read(LASTCOMMANDADDRESS);
}
void lastCommandWrite(char lastCommand)
{

    EEPROM.write(LASTCOMMANDADDRESS, lastCommand);
}
