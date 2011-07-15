import sys
import argparse
import math

parser = argparse.ArgumentParser()
parser.add_argument('map')
parser.add_argument('-ap', dest='activationPatternFilename', default=None,
                    help='Activation pattern data file to plot')
parser.add_argument('-models', dest='plotModels', action='store_true',
                    help='Plot models (map contents)')
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

    print >>out, "set arrow from 0,0,0 to %f,%f,0 linewidth 2" % (
        math.cos(angle), math.sin(angle))
    print >>out, "splot '%s' with lines title ''" % plotDataFilename

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

elif args.plotModels:
    raise Exception("unimplemented")

out.close()
