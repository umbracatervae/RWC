import re
import sys


INPUT_FILENAME_RAW = "s9234"
if len(sys.argv) > 1:
    INPUT_FILENAME_RAW = sys.argv[1]
INPUT_FILENAME = "../" + INPUT_FILENAME_RAW + ".blif"
blifFile = open(INPUT_FILENAME,"r")

print "CHECKING: %s" %(INPUT_FILENAME_RAW + ".blif")

IO_PINS = []
LATCHES = []
NON_IO_GATES = []
mode = ""
numInputs = 0
numOutputs = 0
numCount = 0
for line in blifFile:
    line = line.strip()
    inputSearch = re.search(r'^\.inputs\s+(.*)', line)
    outputSearch = re.search(r'^\.outputs\s+(.*)', line)
    latchSearch = re.search(r'^\.latch\s+(.*)',line)
    gateSearch = re.search(r'^\.names\s+(.*)',line)
    endSearch = re.search(r'^\.end',line)
    if inputSearch:
        #print "INPUT: %s" %(line)
        inputs = inputSearch.group(1).split()
        if inputs[-1] == "\\":
            mode = "INPUT"
            inputs.pop()
        else:
            mode = ""
        for i in inputs:
            IO_PINS.append(i)
            numInputs += 1

    elif outputSearch:
        #print "OUTPUT: %s" % (line)
        outputs = outputSearch.group(1).split()
        if outputs[-1] == "\\":
            mode = "OUTPUT"
            outputs.pop()
        else:
            mode = ""
        for o in outputs:
            IO_PINS.append(o)
            numOutputs += 1
    elif latchSearch:
        #print "LATCH: %s" % (line)
        latches = latchSearch.group(1).split()
        latches.pop()
        LATCHES.append(latches[0]+"[OL]")
        LATCHES.append(latches[1]+"[IL]")
    elif gateSearch:
        #print "GATE: %s" % (line)
        mode = "GATES"
        gateSignals = gateSearch.group(1).split()
        gate = gateSignals[-1]
        if gate not in IO_PINS:
            NON_IO_GATES.append(gateSignals[-1])
        else:
            #print "%s is an OUTPUT PIN!" %(gate)
            numCount += 1
    elif endSearch:
        break
    elif mode == "INPUT":
        #print "INPUT: %s" % (line)
        inputs = line.split()
        mode = ""
        if inputs[-1] == "\\":
            mode = "INPUT"
            inputs.pop()
        for i in inputs:
            IO_PINS.append(i)
            numInputs += 1
    elif mode == "OUTPUT":
        #print "OUTPUT: %s" % (line)
        outputs = line.split()
        mode = ""
        if outputs[-1] == "\\":
            mode = "OUTPUT"
            outputs.pop()
        for o in outputs:
            IO_PINS.append(o)
            numOutputs += 1


#print "NUM OF IO PINS: %d" %(len(IO_PINS))
#print "NUM OF LATCHES: %d" %(len(LATCHES))
#print "NUM OF NON-IO PIN GATES: %d" %(len(NON_IO_GATES))

totalNodes = []
for io in IO_PINS:
    totalNodes.append(io)
for io in LATCHES:
    totalNodes.append(io)
for g in NON_IO_GATES:
    totalNodes.append(g)

#print "-----------------------"
#print "PYTHON TOTAL NODE COUNT: %d" %(len(totalNodes))

INPUT_V_FILENAME = "../" + "output_" + INPUT_FILENAME_RAW + "_verbose.txt"
verboseFile = open(INPUT_V_FILENAME,"r")

totalVNodes = []

for line in verboseFile:
    nodeSearch = re.search(r'^NODE\s(.*):',line)
    if nodeSearch:
        #print "NODE |%s| found" %(nodeSearch.group(1))
        totalVNodes.append(nodeSearch.group(1))

if len(totalVNodes) == len(totalNodes):
    print "PASS"
else:
    largerSet = []
    smallerSet = []
    largerSet_str = ""
    smallerSet_str = ""
    if len(totalVNodes) >= len(totalNodes):
        largerSet = totalVNodes
        largerSet_str = "CLion"
        smallerSet = totalNodes
        smallerSet_str = "PYTHON"
    else:
        largerSet = totalNodes
        largerSet_str = "PYTHON"
        smallerSet = totalVNodes
        smallerSet_str = "CLion"
    for elem in largerSet:
        if elem not in smallerSet:
            print "|%s| in %s, not in %s" %(elem,largerSet_str,smallerSet_str)
    for elem in smallerSet:
        if elem not in largerSet:
            print "ERROR: %s has an element not in %s" %(smallerSet_str,largerSet_str)
            exit(-1)