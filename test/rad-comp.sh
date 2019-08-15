#!/usr/bin/env sh

set -e

today="$( date +"%Y%m%d" )"
number=0
fname=
printf -v fname -- 'test/%s-%02d.comp.log' "$today" "$(( ++number ))"
while [ -e "$fname" ]; do
    printf -v fname -- 'test/%s-%02d.comp.log' "$today" "$(( ++number ))"
done

openocd -f openocd.cfg -c "init; reset; exit"
python3 test/rad-comp.py $fname
