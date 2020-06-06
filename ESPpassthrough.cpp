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


#include "ESPpassthrough.h"

ESPpassthrough::ESPpassthrough(SoftwareSerial* uart, int reset_pin)
{
    _swser = uart;
    _clean_buffer();
    _reset_pin = reset_pin;

    pinMode(_reset_pin, OUTPUT);
}

ESPpassthrough::~ESPpassthrough()
{
}

int ESPpassthrough::connect_ap(String ssid, String password)
{
    PRINTLN("connect_ap");
    _reset();
    _empty();

    // AT
    if (!_at_command("ATE0", "OK", 10000)) {
        return NOT_OK;
    }
    delay(1000);

    // Set station mode
    if (!_at_command("AT+CWMODE=1", "OK", 10000)) {
        return CWMODE_ERROR;
    }
    delay(1000);

    // Disable autoconnection
    if (!_at_command("AT+CWAUTOCONN=0", "OK", 10000)) {
        return DISABLE_CWAUTOCONN_ERROR;
    }
    delay(1000);

    // Connect to AP
    String at_cmd = "AT+CWJAP=\"" + ssid +"\",\"" + password +"\"";
    if (!_at_command(at_cmd, "OK", 20000)) {
        return AP_JOIN_ERROR;
    }
    delay(1000);

    return OK;
}

void ESPpassthrough::disconnect_ap()
{
    _at_command("AT+CWQAP", "OK", 1000);
    return;
}


int ESPpassthrough::open(String addr, String port)
{
    String command = "AT+CIPSTART=\"TCP\",\"";
    command = command + addr + "\"," + port;

    // Connect to IP and port
    if(!_at_command(command, "OK", 10000)) {
        return CONNECT_IP_PORT_ERROR;
    }
    delay(1000);

    // Set passthrough mode
    if(!_at_command("AT+CIPMODE=1", "OK", 10000)) {
        return PASSTHROUGH_MODE_ERROR;
    }
    delay(1000);

    // Start transmission 
    if(!_at_command("AT+CIPSEND", "OK", 10000)) {
        return ENABLE_SEND_ERROR;
    }
    delay(1000);

    return OK;
}

bool ESPpassthrough::close(unsigned long inter_char_delay)
{
    // Wait for a minimu of 2 seconds in order to allow 
    // recognition of escape sequence
    delay(3000);

    // Now send escape sequence 
    // An inter character delay of 5ms has been proved to
    // be the optimal value (set as default in the function)
    for(int i=0; i<3; i++) {
        _swser->print("+");
        PRINT("+");
        delay(inter_char_delay);
    }
    delay(2000);
    _swser->println("AT+CIPCLOSE");
    PRINTLN("Tx: AT+CIPCLOSE");
    return _result("OK", 10000);
}

// Private methods
void ESPpassthrough::_clean_buffer()
{
    //PRINTLN("_clean_buffer");
    memset(_buffer, '\0', BUFF_SIZE);
    _byte_idx = 0;
    _line_ready = false;
    return;
}

void ESPpassthrough::_reset()
{
    digitalWrite(_reset_pin, LOW);
    delay(500);
    digitalWrite(_reset_pin, HIGH);
    delay(1000);
}

void ESPpassthrough::_empty(unsigned long timeout)
{
    unsigned long start_time = millis();

    //PRINTLN("_empty");
    while ( (millis() - start_time) < timeout ) {
        _swser->read();
    }
}

// SwSerial reading states
#define CLEAN_BUFFER  1
#define READING_CHARS 2
#define RETURN_BUFFER 3
#define EMPTY_STREAM  4
#define TIMEOUT       5

/**
 * Gather characters read fron software serial.
 * If TERMINATOR is received or timeout expired returns the 
 * number of characters read.
 * 
 */
int ESPpassthrough::_read_line(unsigned long timeout)
{
    int reading_state = CLEAN_BUFFER;
    unsigned long start_time = millis();
    bool run_flag = true;
    volatile int byte;

    while (run_flag) {
        switch (reading_state) {
            case EMPTY_STREAM:
                _empty();
                reading_state = CLEAN_BUFFER;
                break;

            case CLEAN_BUFFER:
                _clean_buffer();
                reading_state = READING_CHARS;
                break;

            case READING_CHARS:
                byte = _swser->read();
                if (byte != -1) {
                    // A byte has been received
                    if ((char)byte == LINE_TERMINATOR) {
                        // EOL found
                        // PRINT("EOL ");
                        // PRINTLN(_buffer);
                        reading_state = RETURN_BUFFER;
                    } else if (isPrintable(byte)) {
                        // Store only printable chars
                        if (_byte_idx < LAST_CHAR) {
                            _buffer[_byte_idx++] = byte;  
                            _buffer[_byte_idx] = '\0';
                            // Continue reading if timeout isn't expired
                            reading_state = TIMEOUT;
                        } else {
                            // Buffer is full but the EOL hasn't be found
                            PRINTLN("BUFF FULL");
                            reading_state = RETURN_BUFFER;
                        }
                    }
                } else {
                    reading_state = TIMEOUT;
                }
                break;

            case TIMEOUT:
                // Control if timeout is expired
                if ((millis() - start_time) > timeout) {
                    // Timeout expired
                    //PRINTLN("TMOUT");
                    reading_state = RETURN_BUFFER;
                }
                else {
                    // Continue reading chars
                    reading_state = READING_CHARS;
                }
                break;

            case RETURN_BUFFER:
                // Stop reading
                //PRINTLN("RETURN_BUFFER");
                run_flag = false;
                break;
        } // end switch reading_state
    } // end while

    // Return number of bytes actually read
    return _byte_idx;
}


/**
 * Read software serial line by line until timeout expires.
 * Return True if the string passed as input is found, false 
 * otherwise.
 */
bool ESPpassthrough::_result(String result, unsigned long timeout)
{
    unsigned long start_time = millis();

    while ( (millis() - start_time) < timeout ) {
        if (_read_line()) {
            PRINT("Rx: ");
            PRINTLN(_buffer);
            String rx_result(_buffer);
            if (rx_result.indexOf(result) != -1) {
                return true;
            }
        }
    }
    // Timeout return
    return false;
}

bool ESPpassthrough::_at_command(String at_cmd, String expected_result, unsigned long timeout)
{
    _swser->println(at_cmd);
    PRINT("Tx: ");
    PRINTLN(at_cmd);
    return _result(expected_result, timeout);
}