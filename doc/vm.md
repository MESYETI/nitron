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
- `0x15` - READ8
- `0x16` - WRITE8
- `0x17` - READ16
- `0x18` - WRITE16
- `0x19` - JZ
- `0x20` - DIVMOD
- `0x21` - EQU
- `0x22` - LESS
- `0x23` - GREATER
- `0x24` - LE
- `0x25` - GE
- `0x26` - NEG

## System calls
- `0x00` - Print character: `( ch -- )`
- `0x01` - Print string: `( string len -- )`
- `0x02` - Dump: `( -- )`
- `0x03` - Alloc: `( size -- ptr )`
- `0x04` - Realloc `( ptr size -- ptr )`
- `0x05` - Free `( ptr -- )`
- `0x06` - Print hex: `( num -- )`
- `0x07` - Input char: `( -- ch )`
