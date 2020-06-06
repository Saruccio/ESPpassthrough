/**
 * ESPpassthrough
 * Library that uses ESP AT serial commands to set it in Passthrough (transparent) Mode.
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


#ifndef __ESP_PASSTHROUGH_H__
#define __ESP_PASSTHROUGH_H__

#include <Arduino.h>
#include <SoftwareSerial.h>

#define TRACE_ON 1

#ifdef TRACE_ON
#define PRINT(x) Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#else
#define PRINT(x)
#define PRINTLN(x)
#endif

#define BUFF_SIZE 128
#define LAST_CHAR (BUFF_SIZE - 2) // Because _buffer contains string terminator '\0'

#define LINE_TERMINATOR '\r'

// AP connection return codes
#define OK           0
#define NOT_OK       1
#define CWMODE_ERROR 2
#define DISABLE_CWAUTOCONN_ERROR 3
#define AP_JOIN_ERROR 4
#define CONNECT_IP_PORT_ERROR 5
#define PASSTHROUGH_MODE_ERROR 6
#define ENABLE_SEND_ERROR 7

class ESPpassthrough
{
private:
    // Properties
    SoftwareSerial* _swser;
    int _reset_pin;

    char _buffer[BUFF_SIZE];
    int _byte_idx;

    bool _line_ready;

    // Methods
    int _read_line(unsigned long timeout=2000);
    void _clean_buffer();
    void _empty(unsigned long timeout=2000);
    void _reset();
    bool _result(String result, unsigned long timeout);
    bool _at_command(String at_cmd, String expected_result, unsigned long timeout);

public:
    ESPpassthrough(SoftwareSerial* uart, int reset_pin);
    ~ESPpassthrough();

    int connect_ap(String ssid, String password);
    void disconnect_ap();
    int open(String addr, String port);
    bool close(unsigned long inter_char_delay=5);

};




#endif // __ESP_PASSTHROUGH_H__