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
parser.add_argument('-models', dest='plotModels', action='store_true',
                    help='Plot models (map contents)')
parser.add_argument('-traj', dest='trajectoryFilename', default=None,
                    help='Trajectory file to plot')
parser.add_argument('--models_apply_log', dest='applyLog', action='store_true', default=True,
                    help='Apply log function to model values')
args = parser.parse_args()
mapFilename = args.map

if not (args.activationPatternFilename or
        args.plotModels or
        args.trajectoryFilename):
    raise Exception("nothing to plot")

mapFile = open(mapFilename, 'r')
gridWidth = int(mapFile.readline().rstrip("\r\n"))
gridHeight = int(mapFile.readline().rstrip("\r\n"))
spectrumResolution = int(mapFile.readline().rstrip("\r\n"))

out = sys.stdout

rangeX1 = -0.5
rangeX2 = -0.5 + gridWidth
rangeY1 = -0.5
rangeY2 = -0.5 + gridHeight

def writeModelPlotData(mapFile, gridX, gridY, plotDataFilename):
    z = 0
    margin = 0.1
    x1 = -0.5 + gridX + margin
    x2 = -0.5 + gridX + 1 - margin
    y1 = -0.5 + gridY + margin
    y2 = -0.5 + gridY + 1 - margin

    with open(plotDataFilename, 'w') as f:
        for i in range(0, spectrumResolution):
            color = float(mapFile.readline().rstrip("\r\n"))
            if args.applyLog:
                color = math.log(color) + 1;
            y = y1 + (y2 - y1) * float(i) / (spectrumResolution-1)
            print >>f, "%f %f %f %f" % (x1, y, z, color)
            print >>f, "%f %f %f %f" % (x2, y, z, color)
            print >>f

if args.activationPatternFilename:
    dataFile = open(args.activationPatternFilename, 'r')
    plotDataFilename = args.activationPatternFilename.replace("_ap.dat", "_ap_plot.dat")
    plotDataFile = open(plotDataFilename, 'w')

    for y in range(0, gridHeight):
        for x in range(0, gridWidth):
            line = dataFile.readline()
            value = float(line.rstrip("\r\n"))
            print >>plotDataFile, "%d %d %f" % (x, y, value)
        print >>plotDataFile

    plotDataFile.close()
    dataFile.close()

    print >>out, "set border 0"
    print >>out, "unset xtics; unset ytics; unset ztics"
    print >>out, "unset pm3d"
    print >>out, "splot [%f:%f] [%f:%f] [0:1] '%s' with lines lc rgb 'black' title ''" % (
        rangeX1, rangeX2, rangeY1, rangeY2, plotDataFilename)

elif args.trajectoryFilename:
    dataFile = open(args.trajectoryFilename, 'r')
    plotDataFilename = args.trajectoryFilename.replace("_traj.dat", "_traj_plot.dat")
    plotDataFile = open(plotDataFilename, 'w')

    numPoints = int(dataFile.readline().rstrip("\r\n"))
    z = 0
    for i in range(0, numPoints):
        x = float(dataFile.readline().rstrip("\r\n"))
        y = float(dataFile.readline().rstrip("\r\n"))
        color = float(i + 1) / numPoints
        print >>plotDataFile, "%f %f %f %f" % (
            x, y, z, color)

    plotDataFile.close()
    dataFile.close()

    print >>out, "set palette rgbformulae -2,3,3"
    print >>out, "unset colorbox"
    print >>out, "splot [0:1] [0:1] [0:1] '%s' with lines lc palette z title ''" % plotDataFilename

elif args.plotModels:
    plots = []
    for y in range(0, gridHeight):
        for x in range(0, gridWidth):
            plotDataFilename = mapFilename.replace("_map.dat",
                                                   "_map%d_%d_plot.dat" % (x, y))
            writeModelPlotData(mapFile, x, y, plotDataFilename)
            plots.append("'%s' with pm3d title ''" % plotDataFilename)
    
    print >>out, "set border 0"
    print >>out, "unset xtics; unset ytics; unset ztics"
    print >>out, "unset colorbox"
    print >>out, "splot [%f:%f] [%f:%f] [0:1] \\" % (
        rangeX1, rangeX2, rangeY1, rangeY2)
    print >>out, "\\\n  , ".join(plots)

out.close()
