// /*
// The MIT License (MIT)

// Copyright (c) 2013 Seeed Technology Inc.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// */

// #include "SHT31.h"
// #include <math.h>

// SHT31::SHT31() {
// }

// bool SHT31::begin(uint8_t i2caddr) {
//     Wire.begin();
//     _i2caddr = i2caddr;
//     reset();
//     //return (readStatus() == 0x40);
//     return true;
// }

// //boolean S == Scale.  True == Farenheit; False == Celcius
// float SHT31::getTemperature(bool S) {
//     if (! getTempHum()) {
//         return NAN;
//     }
//     if (S == true) {
//         return convertCtoF(temp);
//     }
//     return temp;
// }

// float SHT31::convertCtoF(float c) {
//     return c * 9 / 5 + 32;
// }

// float SHT31::getHumidity(void) {
//     if (! getTempHum()) {
//         return NAN;
//     }
//     return humidity;
// }

// void SHT31::reset(void) {
//     writeCommand(SHT31_SOFTRESET);
//     delay(10);
// }

// void SHT31::heater(bool h) {
//     if (h) {
//         writeCommand(SHT31_HEATEREN);
//     } else {
//         writeCommand(SHT31_HEATERDIS);
//     }
// }

// uint8_t SHT31::crc8(const uint8_t* data, int len) {
//     const uint8_t POLYNOMIAL(0x31);
//     uint8_t crc(0xFF);

//     for (int j = len; j; --j) {
//         crc ^= *data++;

//         for (int i = 8; i; --i) {
//             crc = (crc & 0x80)
//                   ? (crc << 1) ^ POLYNOMIAL
//                   : (crc << 1);
//         }
//     }
//     return crc;
// }


// bool SHT31::getTempHum(void) {
//     uint8_t readbuffer[6];

//     writeCommand(SHT31_MEAS_HIGHREP);

//     delay(50);
//     Wire.requestFrom(_i2caddr, (uint8_t)6);
//     if (Wire.available() != 6) {
//         return false;
//     }
//     for (uint8_t i = 0; i < 6; i++) {
//         readbuffer[i] = Wire.read();
//     }
//     uint16_t ST, SRH;
//     ST = readbuffer[0];
//     ST <<= 8;
//     ST |= readbuffer[1];

//     if (readbuffer[2] != crc8(readbuffer, 2)) {
//         return false;
//     }

//     SRH = readbuffer[3];
//     SRH <<= 8;
//     SRH |= readbuffer[4];

//     if (readbuffer[5] != crc8(readbuffer + 3, 2)) {
//         return false;
//     }

//     double stemp = ST;
//     stemp *= 175;
//     stemp /= 0xffff;
//     stemp = -45 + stemp;
//     temp = stemp;

//     double shum = SRH;
//     shum *= 100;
//     shum /= 0xFFFF;

//     humidity = shum;

//     return true;
// }

// void SHT31::writeCommand(uint16_t cmd) {
//     Wire.beginTransmission(_i2caddr);
//     Wire.write(cmd >> 8);
//     Wire.write(cmd & 0xFF);
//     Wire.endTransmission();
// }
