#!/usr/bin/env sh

set -e

today="$( date +"%Y%m%d" )"
number=0
fname=
printf -v fname -- 'test/%s-%02d.targ.log' "$today" "$(( ++number ))"
while [ -e "$fname" ]; do
    printf -v fname -- 'test/%s-%02d.targ.log' "$today" "$(( ++number ))"
done

stty -F /dev/ttyACM0 speed 115200
openocd -f openocd.cfg -c "init; reset; exit"
echo "targ" > /dev/ttyACM0
ts -s %.S < /dev/ttyACM0 | tee $fname
