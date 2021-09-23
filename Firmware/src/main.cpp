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
#define SERIAL_TIMEOUT 10000

#define LED_CONTROL_PIN LED_BUILTIN
#define RELAY_CONTROL_PIN 5
#define GPIO_CH0_PIN A1
#define GPIO_CH1_PIN A2
#define GPIO_CH2_PIN A3
#define GPIO_CH3_PIN A4
#define GPIO_CH4_PIN A5

#define OUTPUT_PINS                                                            \
    { LED_CONTROL_PIN, RELAY_CONTROL_PIN }
#define INPUT_PINS \
    {}
#define INPUT_PULLUP_PINS \
    { GPIO_CH0_PIN, GPIO_CH1_PIN, GPIO_CH2_PIN, GPIO_CH3_PIN, GPIO_CH4_PIN }

typedef enum _GpioCh {
    GPIO_CH0,
    GPIO_CH1,
    GPIO_CH2,
    GPIO_CH3,
    GPIO_CH4,
} GpioCh_t;

enum _RelayControl {
    RELAY_CONTROL_OFF = 0,
    RELAY_CONTROL_ON = 1,
};

typedef enum _LedControl_t {
    LED_CONTROL_OFF = 0,
    LED_CONTROL_ON = 1,
} LedControl_t;

typedef enum _GpioState {
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH = 1,
} GpioState_t;

typedef enum _CommandList_t {
    CMD_LIST = 0,
    CMD_STAT,
    CMD_ON,
    CMD_OFF,
    CMD_GET_0,
    CMD_GET_1,
    CMD_GET_2,
    CMD_GET_3,
    CMD_GET_4,
    CMD_DEV,
    CMD_VER,
    MAX_CMD
} CommandList_t;

#define COMMAND_PROC_OK "OK\n"
#define COMMAND_PROC_FAIL "FAIL\n"
#define COMMAND_PROC_NG "NG\n"
#define COMMAND_PROC_NO_CMD "NO_CMD\n"
#define COMMAND_STAT_ON "ON\n"
#define COMMAND_STAT_OFF "OFF\n"
#define COMMAND_STAT_HI "HIGH\n"
#define COMMAND_STAT_LO "LOW\n"
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
const uint8_t g_InputPins[] = INPUT_PINS;
const uint8_t g_InputPullupPins[] = INPUT_PULLUP_PINS;
const String g_Command[] = {
    "LIST",  "STAT",  "ON",    "OFF", "GET_0", "GET_1",
    "GET_2", "GET_3", "GET_4", "DEV", "VER",
};  // CommandList_tと対応
const char g_CmdTerminator = 0x0a;
const uint8_t g_GpioChPinList[] = {GPIO_CH0_PIN, GPIO_CH1_PIN, GPIO_CH2_PIN,
                                   GPIO_CH3_PIN, GPIO_CH4_PIN};

/*
 * Prototypes
 */
static int FindCommandList(String str, CommandList_t *list);
static bool ProcCmdOn(void);
static bool ProcCmdOff(void);
static bool ChangeLedState(LedControl_t ledState);
static bool IsEnableRelay(void);
static bool GetGpioState(GpioCh_t ch);

void setup() {
    // Open Serial
    Serial.begin(115200);

    // I/O Initialize
    for (size_t i = 0; i < ARRAY_LENGTH(g_OutputPins); i++) {
        pinMode(g_OutputPins[i], OUTPUT);
    }
    for (size_t i = 0; i < ARRAY_LENGTH(g_InputPins); i++) {
        pinMode(g_InputPins[i], INPUT);
    }
    for (size_t i = 0; i < ARRAY_LENGTH(g_InputPullupPins); i++) {
        pinMode(g_InputPullupPins[i], INPUT_PULLUP);
    }

    // Serial Timeout
    Serial.setTimeout(SERIAL_TIMEOUT);
}

void loop() {
    String cmdBuff;
    CommandList_t list;

    // 文字列取得
    cmdBuff = Serial.readStringUntil(g_CmdTerminator);

    // 文字列検索
    int res = FindCommandList(cmdBuff, &list);
    if (1 == res) {
        switch (list) {
            case CMD_LIST:
                for (size_t c = 0; c < MAX_CMD; c++) {
                    Serial.print(g_Command[c]);
                    Serial.print(",");
                }
                Serial.println();
                break;
            case CMD_STAT:
                if (IsEnableRelay()) {
                    Serial.print(COMMAND_STAT_ON);
                } else {
                    Serial.print(COMMAND_STAT_OFF);
                }
                break;
            case CMD_ON:
                if (ProcCmdOn()) {
                    ChangeLedState(LED_CONTROL_ON);
                    Serial.print(COMMAND_PROC_OK);
                } else {
                    Serial.print(COMMAND_PROC_FAIL);
                }
                break;
            case CMD_OFF:
                if (ProcCmdOff()) {
                    ChangeLedState(LED_CONTROL_OFF);
                    Serial.print(COMMAND_PROC_OK);
                } else {
                    Serial.print(COMMAND_PROC_FAIL);
                }
                break;
            case CMD_GET_0:
                if (GetGpioState(GPIO_CH0)) {
                    Serial.print(COMMAND_STAT_HI);
                } else {
                    Serial.print(COMMAND_STAT_LO);
                }
                break;
            case CMD_GET_1:
                if (GetGpioState(GPIO_CH1)) {
                    Serial.print(COMMAND_STAT_HI);
                } else {
                    Serial.print(COMMAND_STAT_LO);
                }
                break;
            case CMD_GET_2:
                if (GetGpioState(GPIO_CH2)) {
                    Serial.print(COMMAND_STAT_HI);
                } else {
                    Serial.print(COMMAND_STAT_LO);
                }
                break;
            case CMD_GET_3:
                if (GetGpioState(GPIO_CH3)) {
                    Serial.print(COMMAND_STAT_HI);
                } else {
                    Serial.print(COMMAND_STAT_LO);
                }
                break;
            case CMD_GET_4:
                if (GetGpioState(GPIO_CH4)) {
                    Serial.print(COMMAND_STAT_HI);
                } else {
                    Serial.print(COMMAND_STAT_LO);
                }
                break;
            case CMD_DEV:
                Serial.print(COMMAND_DEV);
                break;
            case CMD_VER:
                Serial.println(COMMAND_VER);
                break;
            default:
                Serial.print(COMMAND_PROC_NG);
                break;
        }
    } else if (0 == res) {
        Serial.print(COMMAND_PROC_NG);
    }
}
/**
 * @brief コマンド検索
 * @param str 検索対象
 * @param list 検索結果
 * @retval 1 コマンドが有効
 * @retval 0 コマンドが無効
 * @retval -1 データが存在しない
 */
static int FindCommandList(String str, CommandList_t *list) {
    // check str
    if (str == NULL) {
        return -1;
    }
    // find command
    for (size_t listNum = 0; listNum < MAX_CMD; listNum++) {
        if (str.equalsIgnoreCase(g_Command[listNum])) {
            *list = (CommandList_t)listNum;
            return 1;
            break;
        }
    }
    *list = (CommandList_t)0;
    return 0;
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

/**
 * @brief リレー状態取得
 * @retval true ON状態
 * @retval false OFF状態
 */
static bool IsEnableRelay(void) {
    return RELAY_CONTROL_ON == digitalRead(RELAY_CONTROL_PIN);
}

/**
 * @brief GPIO状態取得
 * @param ch GPIO Channel
 * @retval true High状態
 * @retval false Low状態
 */
static bool GetGpioState(GpioCh_t ch) {
    return GPIO_STATE_HIGH == digitalRead(g_GpioChPinList[ch]);
}