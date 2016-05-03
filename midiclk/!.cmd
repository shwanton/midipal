@call \DIY\SetVars.cmd
 make -f midiclk\makefile %1
 if errorlevel 1 goto EOF
 if {%1} == {clean} goto EOF
 avr-objdump -h -S build\midiclk\midiclk.elf > build\midiclk\midiclk.lst
 set path=%path%;C:\Program Files (x86)\Atmel\AVR Tools\STK500
 stk500 -cUSB -dATmega328p -I4Mhz -e -ifbuild\midiclk\midiclk.hex -pf -vf
:EOF
