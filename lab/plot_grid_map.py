import sys
import argparse
import math

parser = argparse.ArgumentParser()
parser.add_argument('map')
parser.add_argument('-ap', dest='activationPatternFilename', default=None,
                    help='Activation pattern data file to plot')
parser.add_argument('-models', dest='plotModels', action='store_true',
                    help='Plot models (map contents)')
parser.add_argument('--models_apply_log', dest='applyLog', action='store_true', default=True,
                    help='Apply log function to model values')
args = parser.parse_args()
mapFilename = args.map

if not (args.activationPatternFilename or args.plotModels):
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

    print >>out, "set border 0"
    print >>out, "unset xtics; unset ytics; unset ztics"
    print >>out, "unset pm3d"
    print >>out, "splot [%f:%f] [%f:%f] [0:1] '%s' with lines lc rgb 'black' title ''" % (
        rangeX1, rangeX2, rangeY1, rangeY2, plotDataFilename)

    for y in range(0, gridHeight):
        for x in range(0, gridWidth):
            line = dataFile.readline()
            value = float(line.rstrip("\r\n"))
            print >>plotDataFile, "%d %d %f" % (x, y, value)
        print >>plotDataFile

    plotDataFile.close()
    dataFile.close()

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
