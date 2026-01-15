/**
 ******************************************************************************
 * @file    simple_sensor.h
 * @author  Microsoft Corporation & Contributors
 * @version V1.0.0
 * @date    2026-01-15
 * @brief   Header for simplified sensor API.
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

#ifndef SIMPLE_SENSOR_H
#define SIMPLE_SENSOR_H

/**
 * @brief Reads the pressure from the LPS22HB sensor.
 * @return Pressure in hPa as a float.
 */
float Sensor_ReadPressure(void);

/**
 * @brief Reads the temperature from the HTS221 sensor.
 * @return Temperature in degrees Celsius as a float.
 */
float Sensor_ReadTemperature(void);

/**
 * @brief Reads the humidity from the HTS221 sensor.
 * @return Humidity percentage as a float.
 */
float Sensor_ReadHumidity(void);

/**
 * @brief Reads the accelerometer data from the LSM6DSL sensor.
 * @param x Pointer to store X axis acceleration (mg)
 * @param y Pointer to store Y axis acceleration (mg)
 * @param z Pointer to store Z axis acceleration (mg)
 */
void Sensor_ReadAccel(float* x, float* y, float* z);

/**
 * @brief Reads the gyroscope data from the LSM6DSL sensor.
 * @param x Pointer to store X axis angular rate (mdps)
 * @param y Pointer to store Y axis angular rate (mdps)
 * @param z Pointer to store Z axis angular rate (mdps)
 */
void Sensor_ReadGyro(float* x, float* y, float* z);

/**
 * @brief Reads the magnetometer data from the LIS2MDL sensor.
 * @param x Pointer to store X axis magnetic field (mG)
 * @param y Pointer to store Y axis magnetic field (mG)
 * @param z Pointer to store Z axis magnetic field (mG)
 */
void Sensor_ReadMag(float* x, float* y, float* z);

#endif
