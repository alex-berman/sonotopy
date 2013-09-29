#!/usr/bin/python

from argparse import ArgumentParser
import subprocess
import os

def call_avconv():
    global args, export_dir
    cmd = ["avconv",
           "-ss", str(args.sync),
           "-f", "image2",
           "-r", "44100/1024",
           "-i", "%s/frame%%07d.ppm" % export_dir,
           "-map", "0",
           "-i", args.audio,
           "-r", str(args.fps)]

    if args.force:
        cmd.append("-y")

    if args.fade_out:
        num_frames = len(os.listdir(export_dir))
        num_fade_frames = int(args.fade_out * args.fps)
        start_frame = num_frames - num_fade_frames
        cmd += [
            "-vf",
            "fade=out:%d:%d" % (start_frame, num_fade_frames)]

    cmd += ["-vcodec", "libx264",
            "-pre:v", "veryslow",
            "-crf", "0",
            args.output]

    cmd += ["-acodec", "libvo_aacenc",
            "-ab", "320k"]

    print " ".join(cmd)
    subprocess.call(cmd)

parser = ArgumentParser()
parser.add_argument("-fps", type=float, default=30)
parser.add_argument("-fade-out", type=float)
parser.add_argument("-f", "--force", action="store_true")
parser.add_argument("-audio", type=str)
parser.add_argument("-o", "--output", type=str, default="export.mp4")
parser.add_argument("-sync", type=float)
args = parser.parse_args()

export_dir = "export"
call_avconv()
