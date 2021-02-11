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

instr = None
COMPORTS = list_ports.comports()


class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)

        self.master = master
        self.master.title('AC ON/OFF Remote Switch')
        self.master.geometry("360x210")
        # self.pack()

        # 画像ファイル読み込み
        self.init_image = tk.PhotoImage(file="./resource/disconnect.png")

        self.create_widgets()

    def create_widgets(self):
        ### Grid - row0
        self.label_port = tk.Label(self.master, text='COM Port')
        self.label_port.grid(row=0, column=0, sticky=tk.E, pady=10)

        self.box_port = ttk.Combobox(self.master, width=30, value=COMPORTS)
        # self.box_port.set(COMPORTS[1])
        self.box_port.grid(row=0, column=1)

        self.button_open = ttk.Button(self.master, text='Open', width=10)
        self.button_open.grid(row=0, column=2)

        ### Grid - row1
        self.canvas_img = tk.Canvas(self.master, width=226, height=136)
        self.canvas_img.grid(row=1, column=1)
        self.canvas_img.create_image(10,
                                     10,
                                     image=self.init_image,
                                     anchor=tk.NW)

        ### Grid - row2


# アプリケーション起動
root = tk.Tk()
app = Application(master=root)
app.mainloop()