# VM
Instructions have this format:

| `76543210` |
| ---------- |
| `IOOOOOOO` |

- `I` means the 32 bit word after the instruction is pushed before running the instruction
- `O` is the opcode

## Registers
Registers:
- 0 - General purpose
- 1 - General purpose
- 2 - General purpose
- 3 - General purpose
- 4 - General purpose
- 5 - General purpose
- 6 - General purpose
- 7 - General purpose

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
- `0x27` - AND
- `0x28` - XOR
- `0x29` - OR
- `0x2A` - NOT
- `0x2B` - SWAP
- `0x2C` - CALL
- `0x2D` - RET
- `0x2E` - FARCALL

## System calls
The higher 16 bits is the section, the lower 16 bits is the call.

### Section `0x0000` - User calls
This section has some space for user defined calls

### Section `0x0001` - IO
- `0x00` - Print character: `( ch -- )`
- `0x01` - Print string: `( string len -- )`
- `0x02` - Print hex: `( num -- )`
- `0x03` - Input char: `( -- ch )`
- `0x04` - Print null terminated string `( string -- )`
- `0x05` - Input line: `( string* maxLength -- )`
- `0x06` - Print decimal: `( num -- )`

### Section `0x0002` - Memory
- `0x00` - Alloc: `( size -- ptr )`
- `0x01` - Realloc `( ptr size -- ptr )`
- `0x02` - Free `( ptr -- )`

### Section `0x0003` - VM
- `0x00` - Dump: `( -- )`
- `0x01` - Get user calls amount `( -- size )`
- `0x02` - Run on new instance `( code codeSize areaSize stackSize areaPtr -- )`
- `0x03` - Error to string `( error -- str* )`
- `0x04` - Dump memory: `( addr size -- )`
- `0x05` - Get memory usage: `( -- used total )`
- `0x06` - Set area pointer: `( ptr -- )`
- `0x07` - Get area pointer: `( -- ptr )`
- `0x08` - Nothing `( -- )`
- `0x09` - Run file `( path -- success )`

### Section `0x0004` - Assembler
- `0x00` - Assemble `( completion? source assembler* -- size success )`
	- Source must be null terminated
- `0x01` - New assembler `( -- asm*)`
- `0x02` - Free assembler `( asm* -- )`
- `0x03` - Get assembler binary pointer `( asm* -- bin* )`
- `0x04` - Set assembler area `( area size asm* -- )`
- `0x05` - Get assembler data pointer `( asm* -- data* )`
- `0x06` - Set assembler data pointer `( data* asm* -- )`
- `0x07` - Set assembler binary `( bin* size asm* -- )`
- `0x08` - Free incomplete assembler references `( asm* -- )`
- `0x09` - Reset assembler binary length `( asm* -- )`
- `0x0a` - Set assembler binary pointer `( bin* asm* -- )`
- `0x0b` - Extend assembler binary `( size asm* -- success )`
- `0x0c` - Set assembler mode `( code=0,data=1 asm* -- )`

### Section `0x0005` - Filesystem
- `0x00` - Read file `( path -- size contents* success )`
- `0x01` - Read text file `( path -- contents* success )`
- `0x02` - Write file `( path size data -- success )`
- `0x03` - Write text file `( path str -- success )`
