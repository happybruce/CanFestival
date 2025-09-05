# CanFestival Tools (sdo_read / sdo_write)

Small CLI tools to read and write CANopen SDOs using CanFestival + SocketCAN.

## Build

- cmake -S . -B build
- cmake --build build -j

Executables: `build/sdo_read`, `build/sdo_write`.

## sdo_read

Usage:

- sdo_read <can-if> <nodeId> <index-hex> <subidx-hex> [type]
- type: u8|u16|u32|i8|i16|i32|domain (default u32)

Examples:

- ./build/sdo_read can0 1 1000 00 u32   # 读取 0x1000:00 设备类型
- ./build/sdo_read can0 1 1018 01 u32   # 读取 VendorId
- ./build/sdo_read can0 1 1017 00 u16   # 读取心跳时间

Notes:
- 工具会动态配置主站的 SDO Client (0x1280) 指向目标节点 (0x600+ID / 0x580+ID)，并向目标发送 NMT Start。
- 对于 domain 类型，会显示长度与前若干字节的十六进制内容。

## sdo_write

Usage:

- sdo_write <can-if> <nodeId> <index-hex> <subidx-hex> <type> <value>
- type: u8|u16|u32|i8|i16|i32
- value: 十进制或 0x 前缀十六进制

Examples:

- ./build/sdo_write can0 1 1017 00 u16 1000   # 写入心跳时间 1000ms
- ./build/sdo_write can0 1 2000 01 u8 0x12    # 写入 0x2000:01 = 0x12
- ./build/sdo_write can0 1 3000 02 u32 0x1F4  # 写入 0x1F4

Notes:
- 某些对象需要通过 0x1010 存储或重置通信才能生效，参考设备手册。

## Troubleshooting

- 超时：检查 can0 速率与接口已启用；用 candump 观察 0x600/0x580 报文是否往返。
- SDO Abort：工具会打印 32 位 AbortCode，请查阅 CiA 301 对应含义。