/**
 * Example sketch for ESPpassthrough library.
 * 
 * @copyright 2020 by Saruccio Culmone
 * @author Saruccio Culmone <saruccio.culmone@yahoo.it>
 * @version 2008-04-23
 * @file
 * @license MIT License
 *
 * Copyright (c) 2020 Saruccio Culmone
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <SoftwareSerial.h>
#include "ESPpassthrough.h"

// On board LED pin
#define ONBOARD_LED 13

// SWSerial Tx and Rx pin assignement
#define SWSERIAL_RX_PIN 7
#define SWSERIAL_TX_PIN 8

// ESP-01 WiFi module
#define ESP01_ENABLE_PIN 6 // Enable/disable (and so reset) ESP_01

// Serial to be remotized
SoftwareSerial swser(SWSERIAL_RX_PIN, SWSERIAL_TX_PIN);

// ESP Wifi passthrough definition
ESPpassthrough esp(&swser, ESP01_ENABLE_PIN);

// WiFi access point parameters
#define  SSID     "SSID of your WiFI"
#define  PASSWORD "Your password"

#define SERVER_IP   "Server address"
#define SERVER_PORT "Srver Port"  // Note: enter it as string

void setup()
{
   // Init Serial for tracing purposes
    Serial.begin(115200);
    Serial.println("Setup");

    // Open and configure serial port used for communication.
    // It is wired to the ESP-01 taht is already configured to 
    // the same baudrate; it is a one shot configuration
    swser.begin(9600); 

    // Configure the onboard LED for output, and set it off
    pinMode(ONBOARD_LED, OUTPUT);      
    digitalWrite(ONBOARD_LED, LOW);  

    // We aren't in a hurry
    delay(5000);
}


void loop()
{
    int counter = 0;
    int retv = -1;
    Serial.println("Start");

    // Connect to the avaialble access point
    retv = esp.connect_ap(SSID, PASSWORD);
    if (retv == OK) {
        digitalWrite(ONBOARD_LED,HIGH);
    } else {
        PRINT("AP CONNECT ERROR: reason= ");
        PRINTLN(retv);
    }
    delay(1000);
  
    retv = esp.open(SERVER_IP, SERVER_PORT);
    if (retv == OK) {
        for (int i=0; i<3; i++) {
            swser.println("Arduino is READY!");
            digitalWrite(ONBOARD_LED,LOW);
            delay(1000);
            digitalWrite(ONBOARD_LED,HIGH);
        }
    } else {
        PRINT("OPEN ERROR:reason= ");
        PRINTLN(retv);       
    }

    // Remain into the communication loop for a while
    PRINTLN("ENTERING communication loop...");
    while (counter < 10) {
        String rx = "";
        rx = swser.readStringUntil("\r");
        if (rx != "") {
          PRINT("Rx: ");
          PRINTLN(rx);
        } else {
          swser.print("Counter= ");
          swser.println(counter);
          counter++;
        }
    }

    if (!esp.close()) {
       PRINT("CLOSING ERROR:");
        PRINTLN(retv);        
    }

    delay(2000);
    esp.disconnect_ap();

    delay(20000);
}
