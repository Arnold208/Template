/**
 ******************************************************************************
 * @file    simple_sensor.c
 * @author  Microsoft Corporation & Contributors
 * @version V1.0.0
 * @date    2026-01-15
 * @brief   Implementation of simplified sensor API.
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

#include "simple_sensor.h"
#include "../lib/mxchip_bsp/stm_sensor/Inc/sensor.h"

float Sensor_ReadPressure(void)
{
    lps22hb_t search_data = lps22hb_data_read();
    return search_data.pressure_hPa;
}

float Sensor_ReadTemperature(void)
{
    // Using HTS221 for temperature by default as it's paired with humidity
    hts221_data_t data = hts221_data_read();
    return data.temperature_degC;
}

float Sensor_ReadHumidity(void)
{
    hts221_data_t data = hts221_data_read();
    return data.humidity_perc;
}

void Sensor_ReadAccel(float* x, float* y, float* z)
{
    lsm6dsl_data_t data = lsm6dsl_data_read();
    if (x)
        *x = data.acceleration_mg[0];
    if (y)
        *y = data.acceleration_mg[1];
    if (z)
        *z = data.acceleration_mg[2];
}

void Sensor_ReadGyro(float* x, float* y, float* z)
{
    lsm6dsl_data_t data = lsm6dsl_data_read();
    if (x)
        *x = data.angular_rate_mdps[0];
    if (y)
        *y = data.angular_rate_mdps[1];
    if (z)
        *z = data.angular_rate_mdps[2];
}

void Sensor_ReadMag(float* x, float* y, float* z)
{
    lis2mdl_data_t data = lis2mdl_data_read();
    if (x)
        *x = data.magnetic_mG[0];
    if (y)
        *y = data.magnetic_mG[1];
    if (z)
        *z = data.magnetic_mG[2];
}
