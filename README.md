# Nitron
32-bit virtual machine and operating system (although it doesn't run on bare metal yet)

## Build
### Create ROM FS
- Create a folder named `romfs`.
- Copy the `programs` folder into it
- Create a file in `romfs` named `autostart.txt`
- Write `:rom/programs/playground.asm` in `autostart.txt`

## Build ROM FS
Build the ark tool from <https://github.com/mesyeti/ArkasEngine> and install it.

Run the `make_romfs.sh` script

## Build Nitron
Run `make`
