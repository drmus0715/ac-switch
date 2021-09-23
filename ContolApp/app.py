### Copyright (C) 2021 - 2021 Hiromu Ozawa  All rights reserved.
### SPDX-License-Identifier: MIT

# @file app.py
# @brief (ja)AC ON/OFF リモートスイッチ(制御側)のプログラムです。
#          動作環境
#           * AC ON/OFF Remote Switch
# @author Hiromu Ozawa (drmus0715@gmail.com)
# @date 2021/02/11

# coding:utf-8
import serial  # pyserialモジュール
from serial.tools import list_ports
import tkinter as tk  # tkモジュール
import tkinter.scrolledtext
import time

from time import sleep
from tkinter import ttk

import sys
import os

instr = None
COMPORTS = list_ports.comports()


def resourcePath(filename):
    if hasattr(sys, "_MEIPASS"):
        return os.path.join(sys._MEIPASS, filename)
    return os.path.join(filename)


class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)

        self.master = master
        self.master.title('AC ON/OFF Remote Switch')
        self.master.geometry("360x230")
        # self.pack()

        # Serial Instance
        self.inst = serial.Serial()

        # 画像ファイル読み込み
        self.image_stat = tk.PhotoImage(
            file=resourcePath("resource/disconnect.png"))

        self.CreateWidgets()
        self.ChangeStateClose()

    def CreateWidgets(self):
        ### Grid - row0
        self.label_port = tk.Label(self.master, text='COM Port')
        self.label_port.grid(row=0, column=0, sticky=tk.E, pady=10)

        self.box_port = ttk.Combobox(self.master,
                                     width=30,
                                     value=COMPORTS,
                                     state='readonly')
        self.box_port.current(0)
        self.box_port.grid(row=0, column=1)

        self.button_open = ttk.Button(self.master, text='Open', width=10)
        self.button_open.grid(row=0, column=2)
        self.button_open.bind('<Button-1>', self.ClickedOpen)

        ### Grid - row1
        self.canvas_img = tk.Canvas(self.master, width=226, height=136)
        self.canvas_img.grid(row=1, column=1)
        self.image_on_canvas = self.canvas_img.create_image(
            10, 10, image=self.image_stat, anchor=tk.NW)

        ### Grid - row2
        self.button_onoff = ttk.Button(self.master, text='ON', width=30)
        self.button_onoff.grid(row=2, column=1, pady=10)

    def ClickedOpen(self, event):
        self.OpenPort()
        while True:
            self.ChangeStateConn()
            res = self.SendAction("STAT\n")
            if res == "ON":
                self.ChangeStateOn()
                break
            elif res == "OFF":
                self.ChangeStateOff()
                break
            sleep(0.5)
        # self.ChangeStateOff()

    def ClickedClose(self, event):
        self.SendAction("OFF\n")
        sleep(0.1)
        self.ClosePort()
        self.ChangeStateClose()

    def ClickedOn(self, event):
        res = self.SendAction("ON\n")
        self.ChangeStateOn()

    def ClickedOff(self, event):
        self.SendAction("OFF\n")
        self.ChangeStateOff()

    def OpenPort(self):
        port, desc, hwid = COMPORTS[self.box_port.current()]
        self.inst.port = port
        self.inst.baudrate = 115200
        self.inst.timeout = 0.2
        self.inst.open()

    def ClosePort(self):
        self.inst.close()

    def SendAction(self, text):
        self.inst.write(text.encode("utf-8"))
        line = self.inst.readline()
        line_cl = line.strip().decode('ascii')
        print(line_cl)
        return line_cl

    def ChangeStateClose(self):
        self.image_stat = tk.PhotoImage(
            file=resourcePath("resource/disconnect.png"))
        self.canvas_img.itemconfig(self.image_on_canvas, image=self.image_stat)
        self.button_open.configure(text='Open')
        self.button_open.bind('<Button-1>', self.ClickedOpen)
        self.button_onoff['state'] = tk.DISABLED
        self.button_onoff.unbind('<Button-1>')

    def ChangeStateOn(self):
        self.image_stat = tk.PhotoImage(file=resourcePath("resource/on.png"))
        self.canvas_img.itemconfig(self.image_on_canvas, image=self.image_stat)
        self.button_open.configure(text='Close')
        self.button_open.bind('<Button-1>', self.ClickedClose)
        self.button_onoff['state'] = tk.NORMAL
        self.button_onoff.configure(text='OFF')
        self.button_onoff.bind('<Button-1>', self.ClickedOff)

    def ChangeStateOff(self):
        self.image_stat = tk.PhotoImage(file=resourcePath("resource/off.png"))
        self.canvas_img.itemconfig(self.image_on_canvas, image=self.image_stat)
        self.button_open.configure(text='Close')
        self.button_open.bind('<Button-1>', self.ClickedClose)
        self.button_onoff['state'] = tk.NORMAL
        self.button_onoff.configure(text='ON')
        self.button_onoff.bind('<Button-1>', self.ClickedOn)

    def ChangeStateConn(self):
        self.image_stat = tk.PhotoImage(
            file=resourcePath("resource/connect.png"))
        self.canvas_img.itemconfig(self.image_on_canvas, image=self.image_stat)
        self.canvas_img.update()

# アプリケーション起動
root = tk.Tk()
app = Application(master=root)
app.mainloop()