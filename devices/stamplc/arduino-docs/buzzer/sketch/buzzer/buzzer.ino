/*
 *SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 *SPDX-License-Identifier: MIT
 */
#include <Arduino.h>
#include <M5StamPLC.h>

void setup()
{
    /* Init M5StamPLC */
    M5StamPLC.begin();

    /* Play tone */
    // M5StamPLC.tone(frequency, duration);

    /* Stop playing tone */
    // M5StamPLC.noTone();
}

void loop()
{
    M5StamPLC.tone(523, 50);
    delay(1000);
    M5StamPLC.tone(659, 50);
    delay(1000);
    M5StamPLC.tone(880, 50);
    delay(1000);
}
