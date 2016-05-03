@call \DIY\SetVars.cmd
 make -f bootloader\makefile %1
 /WinAVR/bin/avr-size -C --mcu=atmega328 build/muboot/muboot.elf
