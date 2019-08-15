#!/bin/sh

openocd -f openocd.cfg -c "program bin/csdc5-$1.elf verify reset exit"
