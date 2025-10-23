# VM
Instructions have this format:

| `76543210` |
| ---------- |
| `IOOOOOOO` |

- `I` means the 32 bit word after the instruction is pushed before running the instruction
- `O` is the opcode

## Instructions
- `0x00` - NOP
- `0x01` - ADD
- `0x02` - SUB
- `0x03` - MUL
- `0x04` - DIV
- `0x05` - MOD
- `0x06` - R2D
- `0x07` - D2R
- `0x08` - DUP
- `0x09` - OVER
- `0x0A` - DROP
- `0x0B` - ROT
- `0x0C` - AREA
- `0x0D` - READ
- `0x0E` - WRITE
- `0x0F` - JUMP
- `0x10` - JNZ
- `0x11` - HALT
- `0x12` - ECALL
- `0x13` - REG
- `0x14` - WREG

## System calls
- `0x00` - Print character: `( ch -- )`
- `0x01` - Print string: `( string len -- )`
