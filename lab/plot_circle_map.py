# Copyright (C) 2011 Alexander Berman
#
# Sonotopy is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys
import argparse
import math

parser = argparse.ArgumentParser()
parser.add_argument('map')
parser.add_argument('-ap', dest='activationPatternFilename', default=None,
                    help='Activation pattern data file to plot')
parser.add_argument('-d', dest='dims', choices=[2,3], default=3, type=int,
                    help='2D or 3D mode')
parser.add_argument('-models', dest='plotModels', action='store_true',
                    help='Plot models (map contents)')
parser.add_argument('--ar', dest='arrowRadius', type=float, default=1,
                    help='Arrow radius')
parser.add_argument('--filled', dest='filled', action='store_true', default=False,
                    help='Filled style')
args = parser.parse_args()
mapFilename = args.map

if not (args.activationPatternFilename or args.plotModels):
    raise Exception("nothing to plot")

mapFile = open(mapFilename, 'r')
numNodes = int(mapFile.readline().rstrip("\r\n"))
spectrumResolution = int(mapFile.readline().rstrip("\r\n"))

out = sys.stdout

if args.activationPatternFilename:
    dataFile = open(args.activationPatternFilename, 'r')
    plotDataFilename = args.activationPatternFilename.replace("_ap.dat", "_ap_plot.dat")
    plotDataFile = open(plotDataFilename, 'w')

    data = []
    angle = float(dataFile.readline().rstrip("\r\n"))
    for i in range(0, numNodes):
        line = dataFile.readline()
        value = float(line.rstrip("\r\n"))
        data.append(value)

    arrowX1 = 0
    arrowY1 = 0
    arrowX2 = args.arrowRadius * math.cos(angle)
    arrowY2 = args.arrowRadius * math.sin(angle)
    arrowZ = 0

    r = 0.7
    for i in range(0, numNodes+1):
        n = i % numNodes
        nodeAngle = float(n) / numNodes * 2 * math.pi
        z = 1 - data[n]
        print >>plotDataFile, "%f %f %f" % (
            math.cos(nodeAngle), math.sin(nodeAngle), z)
        print >>plotDataFile, "%f %f %f" % (
            r*math.cos(nodeAngle), r*math.sin(nodeAngle), z)
        print >>plotDataFile

    plotDataFile.close()
    dataFile.close()

    print >>out, "set border 0"
    print >>out, "unset xtics; unset ytics; unset ztics"
    print >>out, "set arrow front from %f,%f,%f to %f,%f,%f linewidth 2 lc rgb 'red'" % (
        arrowX1, arrowY1, arrowZ,
        arrowX2, arrowY2, arrowZ)
    if args.filled:
        print >>out, "set pm3d"
        print >>out, "set palette rgbformulae -33,-13,-10"
        print >>out, "unset colorbox"
    else:
        print >>out, "unset pm3d"
    if args.dims == 2:
        print >>out, "plot '%s' with lines lc rgb 'black' title ''" % plotDataFilename
    elif args.dims == 3:
        print >>out, "splot '%s' with lines lc rgb 'black' title ''" % plotDataFilename

elif args.plotModels:
    raise Exception("unimplemented")

out.close()
