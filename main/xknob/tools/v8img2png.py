#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
v8img2png.py —— 把 LVGL v8 由 lv_img_conv 生成的图片 .c 数组还原成 PNG。

背景
----
X-Knob / X-TRACK 的图标资源是 LVGL v8 格式的 .c 数组：
    const lv_img_dsc_t img_src_xxx = {
      .header.w = 42, .header.h = 42,
      .data_size = 1764 * LV_IMG_PX_SIZE_ALPHA_BYTE,
      .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
      .data = img_src_xxx_map,
    };
其中 LV_IMG_CF_TRUE_COLOR_ALPHA 在 16 位色深（LV_COLOR_DEPTH==16 且 LV_COLOR_16_SWAP==0）下，
像素为「RGB565 小端 2 字节 + Alpha 1 字节」逐像素交错，共 3 字节/像素。

LVGL v9 的资源二进制格式已变（lv_image_dsc_t 头部新增 magic/stride，cf 改用
lv_color_format_t，v8 的 LV_IMG_CF_* 全部作废），且 v9 的 RGB565A8 采用平面布局
（RGB 数组后跟 A 数组），与本格式不兼容。因此本脚本做的是「还原成 PNG」，
之后由 LVGL v9 的 lodepng 解码器在运行时按 PNG 头解码（lv_conf.h: LV_USE_LODEPNG=1）。

字节序说明
----------
本脚本解析 .c 中 `#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0` 这一分支
（该分支在 X-Knob 的 41 张图中普遍存在），按小端解释 RGB565：
    val = b0 | (b1 << 8)
已用 img_src_dot_blue.c 实测校验：字节 0x39,0x53 → val=0x5339 → rgb(82,101,205)。

用法
----
    python3 v8img2png.py <in.c> <out.png> [<in2.c> <out2.png> ...]

仅依赖 Python 标准库（zlib / struct / re），无需 Pillow。
"""

import os
import re
import struct
import sys
import zlib


def _extract_u16_branch(text: str) -> bytes:
    """取出 `#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0` 分支内的所有字节。"""
    lines = text.split("\n")
    start = None
    for i, line in enumerate(lines):
        if line.startswith("#if") and "== 16" in line and "SWAP == 0" in line:
            start = i
            break
    if start is None:
        raise ValueError("未找到 `#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0` 分支")

    end = None
    for j in range(start + 1, len(lines)):
        if lines[j].startswith("#endif"):
            end = j
            break
    if end is None:
        raise ValueError("该分支缺少配对的 #endif")

    body = "\n".join(lines[start + 1:end])
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)   # 去掉 /* ... */ 注释
    return bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", body))


def _parse_size(text: str):
    """从 lv_img_dsc_t 初始化块里读 w / h。"""
    def grab(field):
        m = re.search(r"\.header\.%s\s*=\s*(\d+)" % field, text)
        if not m:
            raise ValueError("未找到 .header.%s" % field)
        return int(m.group(1))
    return grab("w"), grab("h")


def _rgb565_to_rgb888(v: int):
    r5 = (v >> 11) & 0x1F
    g6 = (v >> 5) & 0x3F
    b5 = v & 0x1F
    # 位扩展：把 5/6 位拉满到 8 位（(v*255 + max//2) // max）
    return (r5 * 255 + 15) // 31, (g6 * 255 + 31) // 63, (b5 * 255 + 15) // 31


def _write_png(path: str, width: int, height: int, rgba: bytearray) -> None:
    """写 8 位 RGBA PNG（color type 6），只用标准库。"""
    def chunk(tag: bytes, data: bytes) -> bytes:
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)  # 8bit, RGBA, 无隔行

    # 每行前置 1 字节滤波器类型（0 = None）
    raw = bytearray()
    stride = width * 4
    for y in range(height):
        raw.append(0)
        raw += rgba[y * stride:(y + 1) * stride]

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", zlib.compress(bytes(raw), 9)))
        f.write(chunk(b"IEND", b""))


def convert(in_c: str, out_png: str) -> None:
    with open(in_c, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    width, height = _parse_size(text)
    data = _extract_u16_branch(text)

    expect = width * height * 3
    if len(data) != expect:
        raise ValueError("%s: 字节数 %d != w*h*3 = %d（%dx%d）"
                         % (os.path.basename(in_c), len(data), expect, width, height))

    rgba = bytearray(width * height * 4)
    for n in range(width * height):
        b0 = data[n * 3]
        b1 = data[n * 3 + 1]
        alpha = data[n * 3 + 2]
        r, g, b = _rgb565_to_rgb888(b0 | (b1 << 8))
        o = n * 4
        rgba[o] = r
        rgba[o + 1] = g
        rgba[o + 2] = b
        rgba[o + 3] = alpha

    _write_png(out_png, width, height, rgba)
    print("OK  %-28s -> %-22s %dx%d" % (os.path.basename(in_c), os.path.basename(out_png),
                                        width, height))


def main(argv):
    pairs = argv[1:]
    if not pairs or len(pairs) % 2 != 0:
        print(__doc__)
        return 2
    for i in range(0, len(pairs), 2):
        convert(pairs[i], pairs[i + 1])
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
