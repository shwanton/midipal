@echo off
 set path=%path%;C:\Program Files (x86)\Atmel\AVR Tools\STK500
 stk500 -cUSB -dATmega328p -I125Khz -e -fD4FF -FD4FF -EFD -GFD
 stk500 -cUSB -dATmega328p -I4Mhz -ifmuboot.hex -pf -vf
 stk500 -cUSB -dATmega328p -I4Mhz -ifmidiseq.hex -pf -vf
:stk500 -cUSB -dATmega328p -I4Mhz -iemidiseq_eprom.hex -pe -ve
