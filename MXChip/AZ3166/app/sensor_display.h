/**
 ******************************************************************************
 * @file    sensor_display.h
 * @author  Microsoft Corporation & Contributors
 * @version V1.0.0
 * @date    2026-01-15
 * @brief   Header for sensor display module.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2026 Microsoft Corporation</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Microsoft Corporation nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#ifndef SENSOR_DISPLAY_H
#define SENSOR_DISPLAY_H

/**
 * @brief Displays pressure and temperature on the screen.
 * @param pressure Pressure in hPa
 * @param temp Temperature in Celsius
 */
void Display_Pressure_Screen(float pressure, float temp);

/**
 * @brief Displays humidity and temperature on the screen.
 * @param humidity Humidity in percent
 * @param temp Temperature in Celsius
 */
void Display_Humidity_Screen(float humidity, float temp);

/**
 * @brief Displays accelerometer and gyroscope data.
 * @param ax Accel X
 * @param ay Accel Y
 * @param az Accel Z
 * @param gx Gyro X
 * @param gy Gyro Y
 * @param gz Gyro Z
 */
void Display_AccelGyro_Screen(float ax, float ay, float az, float gx, float gy, float gz);

/**
 * @brief Displays magnetometer and temperature data.
 * @param mx Mag X
 * @param my Mag Y
 * @param mz Mag Z
 * @param temp Temperature in Celsius
 */
void Display_Mag_Screen(float mx, float my, float mz, float temp);

#endif
