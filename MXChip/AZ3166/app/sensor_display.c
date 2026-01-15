/**
 ******************************************************************************
 * @file    sensor_display.c
 * @author  Microsoft Corporation & Contributors
 * @version V1.0.0
 * @date    2026-01-15
 * @brief   Implementation of sensor display logic.
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

#include "sensor_display.h"
#include "screen.h"
#include <stdio.h>
#include <stdlib.h>

void Display_Pressure_Screen(float pressure, float temp)
{
    char buffer[32];

    // P: 1013.25 hPa
    int p_int  = (int)pressure;
    int p_frac = (int)((pressure - p_int) * 100);
    snprintf(buffer, sizeof(buffer), "P: %d.%02d hPa", p_int, p_frac < 0 ? -p_frac : p_frac);
    screen_print(buffer, L0);

    // T: 25.50 C
    int t_int  = (int)temp;
    int t_frac = (int)((temp - t_int) * 100);
    snprintf(buffer, sizeof(buffer), "T: %d.%02d C", t_int, t_frac < 0 ? -t_frac : t_frac);
    screen_print(buffer, L1);

    screen_print("Sensor: LPS22HB", L3);

    // Also print to console
    printf("[Pressure] P: %d.%02d hPa | T: %d.%02d C\r\n",
        p_int,
        p_frac < 0 ? -p_frac : p_frac,
        t_int,
        t_frac < 0 ? -t_frac : t_frac);
}

void Display_Humidity_Screen(float humidity, float temp)
{
    char buffer[32];

    int h_int  = (int)humidity;
    int h_frac = (int)((humidity - h_int) * 100);
    snprintf(buffer, sizeof(buffer), "H: %d.%02d %%", h_int, h_frac < 0 ? -h_frac : h_frac);
    screen_print(buffer, L0);

    int t_int  = (int)temp;
    int t_frac = (int)((temp - t_int) * 100);
    snprintf(buffer, sizeof(buffer), "T: %d.%02d C", t_int, t_frac < 0 ? -t_frac : t_frac);
    screen_print(buffer, L1);

    screen_print("Sensor: HTS221", L3);

    printf("[Humidity] H: %d.%02d %% | T: %d.%02d C\r\n",
        h_int,
        h_frac < 0 ? -h_frac : h_frac,
        t_int,
        t_frac < 0 ? -t_frac : t_frac);
}

void Display_AccelGyro_Screen(float ax, float ay, float az, float gx, float gy, float gz)
{
    char buffer[32];

    // Explicitly format A values
    int a0i = (int)ax;
    int a0f = (int)((ax - a0i) * 10);
    if (a0f < 0)
        a0f = -a0f;

    int a1i = (int)ay;
    int a1f = (int)((ay - a1i) * 10);
    if (a1f < 0)
        a1f = -a1f;

    snprintf(buffer, sizeof(buffer), "A:%d.%d %d.%d", a0i, a0f, a1i, a1f);
    screen_print(buffer, L0);

    // Explicitly format G values
    int g0i = (int)gx;
    int g0f = (int)((gx - g0i) * 10);
    if (g0f < 0)
        g0f = -g0f;

    int g1i = (int)gy;
    int g1f = (int)((gy - g1i) * 10);
    if (g1f < 0)
        g1f = -g1f;

    snprintf(buffer, sizeof(buffer), "G:%d.%d %d.%d", g0i, g0f, g1i, g1f);
    screen_print(buffer, L1);

    screen_print("Sensor: LSM6DSL", L3);

    // Print full precision to console
    printf("[IMU] A: %d.%d %d.%d %d.%d | G: %d.%d %d.%d %d.%d\r\n",
        (int)ax,
        abs((int)((ax - (int)ax) * 100)),
        (int)ay,
        abs((int)((ay - (int)ay) * 100)),
        (int)az,
        abs((int)((az - (int)az) * 100)),
        (int)gx,
        abs((int)((gx - (int)gx) * 100)),
        (int)gy,
        abs((int)((gy - (int)gy) * 100)),
        (int)gz,
        abs((int)((gz - (int)gz) * 100)));
}

void Display_Mag_Screen(float mx, float my, float mz, float temp)
{
    char buffer[32];

    int m0i = (int)mx, m0f = (int)((mx - m0i) * 10);
    if (m0f < 0)
        m0f = -m0f;
    int m1i = (int)my, m1f = (int)((my - m1i) * 10);
    if (m1f < 0)
        m1f = -m1f;

    snprintf(buffer, sizeof(buffer), "M:%d.%d %d.%d", m0i, m0f, m1i, m1f);
    screen_print(buffer, L0);

    int t_int  = (int)temp;
    int t_frac = (int)((temp - t_int) * 100);
    snprintf(buffer, sizeof(buffer), "T: %d.%02d C", t_int, t_frac < 0 ? -t_frac : t_frac);
    screen_print(buffer, L1);

    screen_print("Sensor: LIS2MDL", L3);
    printf("[Magnetometer] M: %d.%d %d.%d %d.%d | T: %d.%02d\r\n",
        m0i,
        m0f,
        m1i,
        m1f,
        (int)mz,
        abs((int)((mz - (int)mz) * 10)),
        t_int,
        t_frac < 0 ? -t_frac : t_frac);
}

void Display_MQTT_Stats(int pub_count, int sub_count)
{
    char buffer[32];

    screen_print("MQTT Status", L0);
    screen_print("   Connected   ", L1);

    snprintf(buffer, sizeof(buffer), "P: %d   S: %d", pub_count, sub_count);
    screen_print(buffer, L2);

    screen_print("               ", L3);

    printf("[MQTT Stats] Pub: %d | Sub: %d\r\n", pub_count, sub_count);
}
