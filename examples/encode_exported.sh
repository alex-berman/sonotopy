#!/bin/sh
avconv -f image2 -r 44100/1024 -i 'export/frame%05d.ppm' -map 0 -i "$1" -r 25 export.mov
