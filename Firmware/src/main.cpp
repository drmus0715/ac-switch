/******************************************************************************
 * Copyright (C) 2021 - 2021 Hiromu Ozawa  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file main.cpp
 * @brief (ja)AC ON/OFF リモートスイッチ(Arduino側)のプログラムです。
 *          動作環境
 *           * Arduino Nano
 *           * Solid State Relay (Control -> D5 pin)
 *           * LED (Control -> D4 pin)
 * @author Hiromu Ozawa (drmus0715@gmail.com)
 * @date 2021/02/11
 */

#include <Arduino.h>

#ifndef DEVICE_ID
#define DEVICE_ID "empty_id"
#endif
/*
 * Definitions
 */
#define LED_CONTROL_PIN 4
#define RELAY_CONTROL_PIN 5
#define OUTPUT_PINS                                                            \
    { LED_CONTROL_PIN, RELAY_CONTROL_PIN }

enum _RelayControl {
    RELAY_CONTROL_OFF = 0,
    RELAY_CONTROL_ON = 1,
};

typedef enum _LedControl_t {
    LED_CONTROL_OFF = 0,
    LED_CONTROL_ON = 1,
} LedControl_t;

typedef enum _CommandList_t {
    CMD_STAT = 0,
    CMD_ON,
    CMD_OFF,
    CMD_DEV,
    CMD_VER,
    MAX_CMD
} CommandList_t;

#define COMMAND_PROC_OK "OK\n"
#define COMMAND_PROC_NG "NG\n"
#define COMMAND_STAT "STATUS_OK\n"
#define COMMAND_DEV "AC ON/OFF Remote Switch\n"
#define COMMAND_VER DEVICE_ID

/*
 * Macros
 */
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

/*
 * Variables
 */
const uint8_t g_OutputPins[] = OUTPUT_PINS;
const String g_Command[] = {"STAT", "ON", "OFF", "DEV",
                            "VER"}; // CommandList_tと対応
const char g_CmdTerminator = 0x0a;

/*
 * Prototypes
 */
static bool FindCommandList(String str, CommandList_t *list);
static bool ProcCmdOn(void);
static bool ProcCmdOff(void);
static bool ChangeLedState(LedControl_t ledState);

void setup() {
    // Open Serial
    Serial.begin(115200);

    // I/O Initialize
    for (size_t i = 0; i < ARRAY_LENGTH(g_OutputPins); i++) {
        pinMode(g_OutputPins[i], OUTPUT);
    }
}

void loop() {
    String cmdBuff;
    CommandList_t list;

    // 文字列取得
    cmdBuff = Serial.readStringUntil(g_CmdTerminator);

    // 文字列検索
    if (FindCommandList(cmdBuff, &list)) {
        switch (list) {
        case CMD_STAT:
            Serial.print(COMMAND_STAT);
            break;
        case CMD_ON:
            if (ProcCmdOn()) {
                ChangeLedState(LED_CONTROL_ON);
                Serial.print(COMMAND_PROC_OK);
            } else {
                Serial.print(COMMAND_PROC_NG);
            }
            break;
        case CMD_OFF:
            if (ProcCmdOff()) {
                ChangeLedState(LED_CONTROL_OFF);
                Serial.print(COMMAND_PROC_OK);
            } else {
                Serial.print(COMMAND_PROC_NG);
            }
            break;
        case CMD_DEV:
            Serial.print(COMMAND_DEV);
            break;
        case CMD_VER:
            Serial.println(COMMAND_VER);
            break;
        default:
            break;
        }
    }
}

static bool FindCommandList(String str, CommandList_t *list) {
    // find command
    for (size_t listNum = 0; listNum < MAX_CMD; listNum++) {
        if (str.equalsIgnoreCase(g_Command[listNum])) {
            *list = (CommandList_t)listNum;
            return true;
            break;
        }
    }
    *list = (CommandList_t)0;
    return false;
}

/**
 * @brief "ON"コマンドの処理
 * @param なし
 * @retval true 正常にOFF -> ONにした
 * @retval false すでにONになっている
 */
static bool ProcCmdOn(void) {
    uint8_t val = 0;
    if (RELAY_CONTROL_ON == digitalRead(RELAY_CONTROL_PIN)) {
        // すでにONなので処理しない
        return false;
    }
    digitalWrite(RELAY_CONTROL_PIN, RELAY_CONTROL_ON);
    val = digitalRead(RELAY_CONTROL_PIN); // 一応確認
    return val == RELAY_CONTROL_ON;
}

/**
 * @brief "OFF"コマンドの処理
 * @param なし
 * @retval true 正常にON -> OFFにした
 * @retval false すでにOFFになっている
 */
static bool ProcCmdOff(void) {
    uint8_t val = 0;
    if (RELAY_CONTROL_OFF == digitalRead(RELAY_CONTROL_PIN)) {
        // すでにOFFなので処理しない
        return false;
    }
    digitalWrite(RELAY_CONTROL_PIN, RELAY_CONTROL_OFF);
    val = digitalRead(RELAY_CONTROL_PIN); // 一応確認
    return val == RELAY_CONTROL_OFF;
}

/**
 * @brief LED ON/OFF
 * @param ledState LED点灯・消灯
 * @retval true 正常に切り替えた
 * @retval false 切り替えなかった
 */
static bool ChangeLedState(LedControl_t ledState) {
    LedControl_t val;
    if (ledState == (LedControl_t)digitalRead(LED_CONTROL_PIN)) {
        // 処理しない
        return false;
    }
    digitalWrite(LED_CONTROL_PIN, ledState);
    val = (LedControl_t)digitalRead(LED_CONTROL_PIN); // 一応確認
    return val == ledState;
}
