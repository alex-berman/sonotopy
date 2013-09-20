#!/bin/sh
avconv -f image2 -r 44100/1024 -i 'export/frame%05d.ppm' -map 0 -i "$1" -r 30 \
    -vcodec libx264 -pre:v veryslow \
    -acodec libvo_aacenc -ab 320k \
    export.mp4
