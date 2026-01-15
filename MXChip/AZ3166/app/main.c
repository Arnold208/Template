/**
 ******************************************************************************
 * @file    main.c
 * @author  Microsoft Corporation & Contributors
 * @version V1.0.0
 * @date    2026-01-15
 * @brief   Simplified main application for MXChip Sensor Display.
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

#include "board_init.h"
#include "screen.h"
#include "sensor_display.h"
#include "simple_sensor.h"
#include "tx_api.h"
#include <stdio.h>

/**
 * @file main.c
 * @brief Simplified main application for MXChip Sensor Display.
 *
 * This file demonstrates how to use the 'simple_sensor' and 'sensor_display'
 * modules to read and show data without complex code.
 */

volatile int current_screen     = 0;
volatile uint8_t screen_changed = 0;

// --- Button Callbacks ---
void button_a_callback()
{
    // Cycle to next screen (0 -> 1 -> 2 -> 3 -> 0)
    current_screen++;
    if (current_screen > 3)
        current_screen = 0;
    screen_changed = 1;
}

void button_b_callback()
{
    // Cycle to previous screen
    current_screen--;
    if (current_screen < 0)
        current_screen = 3;
    screen_changed = 1;
}

// --- Display Update Logic ---
void update_display()
{
    // 1. Clear the screen to prepare for new data
    screen_clear();

    // 2. Decide what to show based on the current screen number
    switch (current_screen)
    {
        case 0:
        {
            // Easy: Read float values
            float pressure = Sensor_ReadPressure();
            float temp     = Sensor_ReadTemperature();

            // Easy: Display them
            Display_Pressure_Screen(pressure, temp);
            break;
        }
        case 1:
        {
            float humidity = Sensor_ReadHumidity();
            float temp     = Sensor_ReadTemperature();

            Display_Humidity_Screen(humidity, temp);
            break;
        }
        case 2:
        {
            float ax, ay, az, gx, gy, gz;

            // Read 3-axis data into variables
            Sensor_ReadAccel(&ax, &ay, &az);
            Sensor_ReadGyro(&gx, &gy, &gz);

            Display_AccelGyro_Screen(ax, ay, az, gx, gy, gz);
            break;
        }
        case 3:
        {
            float mx, my, mz;
            float temp = Sensor_ReadTemperature();

            Sensor_ReadMag(&mx, &my, &mz);

            Display_Mag_Screen(mx, my, mz, temp);
            break;
        }
    }

    screen_changed = 0;
}

// --- Main Entry Point ---
int main(void)
{
    // 1. Initialize Board (Buttons, LEDs, I2C, Serial, etc.)
    board_init();

    printf("Starting Modular Sensor Display...\n");

    uint32_t last_tick = 0;

    // Show first screen immediately
    update_display();

    // 2. Data Loop
    while (1)
    {
        // Update if button pressed OR every 2 seconds
        if (screen_changed || (HAL_GetTick() - last_tick > 2000))
        {
            last_tick = HAL_GetTick();
            update_display();
        }
    }

    return 0;
}