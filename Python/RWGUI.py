#FILE: RWGUI.py
#AUTHOR: Akshay Nagendra <akshaynag@gatech.edu>
#DESCRIPTION: Python file intended to generate a directed graph from an input file that is specified by the cmdline, or the default will be used
#See "input_graph_lecture.dmp" for an example


import turtle 
import re
import math
import random
import Tkinter as tkinter
import time
import sys

#PARAMETERS THAT CAN BE CONFIGURED BY USER
#INPUT_GRAPH_FILENAME = "input_graph_lecture.dmp" #DEFAULT FILE
INPUT_GRAPH_FILENAME = "input_graph.dmp"
WINDOW_WIDTH = 1820
WINDOW_HEIGHT = 980
FONT_SIZE = 16  #6 for native 720p, 12 for 720p x11, 12 for native 1080p, 16 for 1080p x11
DEGREE_FOR_CONFLICT = 5

if len(sys.argv) >= 2:
    if sys.argv[1] == "720p":
        WINDOW_HEIGHT = 620
        WINDOW_WIDTH = 1180
        FONT_SIZE = 12
        if len(sys.argv) > 2:
            if sys.argv[2] == "--native":
                FONT_SIZE = 6
            elif sys.argv[2] == "--x11":
                FONT_SIZE = 12
            else:
                try:
                    FONT_SIZE = int(sys.argv[2])
                except ValueError:
                    print "RWGUI.py ERROR: Cannot convert value into an integer; Try again..."
                    exit()
            
    elif sys.argv[1] == "1080p":
        WINDOW_HEIGHT = 980
        WINDOW_WIDTH = 1820
        FONT_SIZE = 16
        if len(sys.argv) > 2:
            if sys.argv[2] == "--native":
                FONT_SIZE = 12
            elif sys.argv[2] == "--x11":
                FONT_SIZE = 16
            else:
                try:
                    FONT_SIZE = int(sys.argv[2])
                except ValueError:
                    print "RWGUI.py ERROR: Cannot convert value into an integer; Try again..."
                    exit()
    else:
        print "RWGUI.py ERROR: Unsupported resolution"
        exit()

CURSOR_SIZE = 50
GLOBAL_SKEW_X = 1.5*CURSOR_SIZE
GLOBAL_SKEW_Y = CURSOR_SIZE
CANVAS_DISPLAY_W = 0.9 * WINDOW_HEIGHT
CANVAS_DISPLAY_H = 0.9 * WINDOW_HEIGHT
CANVAS_WIDTH = 0.7 * WINDOW_HEIGHT
CANVAS_HEIGHT = 0.7 * WINDOW_HEIGHT
WINDOW_DIM = str(WINDOW_WIDTH) + "x" + str(WINDOW_HEIGHT)
SCREEN_WIDTH = 2000
SCREEN_HEIGHT = 2000
TEXT_BUFFER = 10
SBUFFER = 20
EBUFFER = 40
Y_STEP = 2.5*CURSOR_SIZE
SCROLLBAR_WIDTH = FONT_SIZE
NUM_CONSOLE_BUTTONS = 4
CONSOLE_WIDTH = CANVAS_DISPLAY_W + SCROLLBAR_WIDTH
CONSOLE_BUTTON_HEIGHT = WINDOW_HEIGHT - (CANVAS_DISPLAY_H + SCROLLBAR_WIDTH)
LABEL_WIDTH = int(1.25*FONT_SIZE)
COLUMN_HEADERS = ["NODE","SIGNAL NAME","NODE DELAY","LABEL","CLUSTER"]
CLUSTER_COLUMN_HEADERS = ["ROOT","ROOT NAME","CLUSTER SIZE","MEMBERS"]
NUM_COLUMNS_TABLE = len(COLUMN_HEADERS)
NUM_CLUSTER_COLUMN_HEADERS = len(CLUSTER_COLUMN_HEADERS)
TABLE_ELEMENTS = []
CLUSTER_TABLE_ELEMENTS = []
TABLE_WIDTH = WINDOW_WIDTH - (CANVAS_DISPLAY_W + SCROLLBAR_WIDTH)
CURRENT_MODE = ""

#CLUSTERING DATA
LSET = []
LSET_FONT = FONT_SIZE + 4
CURRENT_CLUSTER_INDEX = -1
LSET_LABEL = 0
LSET_INDEX = 0
CLUSTERS_DRAWN = []

#OTHER GLOBAL DATA
MAX_IO_DELAY = 0

#GUI setup
root = tkinter.Tk()
root.title("RW Clustering GUI")
root.geometry(WINDOW_DIM)
drawingFrame = tkinter.Frame(root)

drawingFrame.place(x = 0, y = 0, width = CANVAS_DISPLAY_W+SCROLLBAR_WIDTH, height = CANVAS_DISPLAY_H+SCROLLBAR_WIDTH)
v = tkinter.Scrollbar(drawingFrame, orient=tkinter.VERTICAL)
h = tkinter.Scrollbar(drawingFrame,orient=tkinter.HORIZONTAL)

v.place(x = CANVAS_DISPLAY_W, y =0, width = SCROLLBAR_WIDTH, height = CANVAS_DISPLAY_H + SCROLLBAR_WIDTH)
h.place(x=0, y = CANVAS_DISPLAY_H, width = CANVAS_DISPLAY_W + SCROLLBAR_WIDTH, height = SCROLLBAR_WIDTH)
cv = tkinter.Canvas(drawingFrame,width=SCREEN_WIDTH,height=SCREEN_HEIGHT,yscrollcommand=v.set,xscrollcommand=h.set)
cv.place(x = 0, y = 0, width = CANVAS_DISPLAY_W, height = CANVAS_DISPLAY_H)

tableFrame = tkinter.Frame(root)
tableFrame.place(x = CANVAS_DISPLAY_W + SCROLLBAR_WIDTH, y = 0, width = TABLE_WIDTH, height = WINDOW_HEIGHT)

v.config(command = cv.yview)
h.config(command = cv.xview)
cv.config(scrollregion=cv.bbox(tkinter.ALL))


p = turtle.RawTurtle(cv)
pScreen = p.getscreen()
p.speed(0)
pScreen.register_shape("Circle.gif")
pScreen.register_shape("Cluster.gif")
pScreen.screensize(SCREEN_WIDTH,SCREEN_HEIGHT)

LEFT_X = -(CANVAS_WIDTH/2)+CURSOR_SIZE
RIGHT_X = (CANVAS_WIDTH/2)-CURSOR_SIZE
TOP_Y = (CANVAS_HEIGHT/2)-CURSOR_SIZE
DOWN_Y = -(CANVAS_HEIGHT/2)+CURSOR_SIZE

#Center canvas
cv.yview_scroll(-5,"units")
cv.xview_scroll(-5,"units")

class Node:
    def __init__(self):
        self.prevn = []
        self.nextn = []
        self.nodeID = -1
        self.delay = 1
        self.circuitNet = ""
        self.X = -1
        self.Y = -1
        self.cluster = []
        self.label = 0
        self.members = []
    def printNodeInfo(self):
        print "NODE ", self.nodeID
        print "Nets Contained: ", self.circuitNet
        print "Ancestors: ", self.prevn
        print "Successors: ", self.nextn
        print "Cluster(", self.nodeID, "): ", self.cluster
        print "Label: ", self.label
        print "Delay: ", self.delay
        if self.X != -1 and self.Y != -1:
            print "X: ",self.X
            print "Y: ",self.Y
    def printSummaryInfo(self):
        print "NODE ", self.nodeID

class Edge:
    """Class to track edge objects between nodes"""
    def __init__(self):
        self.srcNode = None
        self.dstNode = None
        self.startX = -1
        self.startY = -1
        self.endX = -1
        self.endY = -1
        self.angle = -1
    def printEdgeInfo(self):
        print "EDGE FROM NODE ", self.srcNode.nodeID, " TO NODE ", self.dstNode.nodeID
        print "Starting Coordinate: (", self.startX, ", ", self.startY, ")"
        print "Ending Coordinate: (", self.endX, ", ", self.endY, ")"
        print "Heading Angle: ",self.angle

		
def retrieveNodeByID(nList,ID):
    """Helper function for retreiving a node by ID"""
    for i in range(0,len(nList)):
        if nList[i].nodeID == ID:
            return nList[i]
			
def edgeGenerator(srcNode,dstNode):
    """Helper function for creating a list of edges for all nodes"""
    e = Edge()
    e.srcNode = srcNode
    e.dstNode = dstNode
    e.startX = srcNode.X 
    e.startY = srcNode.Y
    e.endX = dstNode.X
    e.endY = dstNode.Y
    angle = math.atan2(e.endY-e.startY,e.endX-e.startX)
    e.angle = math.degrees(angle)
    return e
			
def arrowGenerator(cursor,e,sBuffer,eBuffer):
    """Helper function to generate directional arrows between ancestors and successors"""
    cursor.penup()
    cursor.goto(e.startX,e.startY - sBuffer)
    cursor.pendown()
    distance = math.hypot(e.endX - e.startX,e.endY - e.startY) - eBuffer
    cursor.seth(e.angle)
    cursor.forward(distance)
    cursor.stamp()

def nodePlacement(srcNodeList,nodeMasterList):
    """Helper function to generate placement of all nodes"""
    X_SRC_STEP = (RIGHT_X-LEFT_X)/len(srcNodeList)
	
    nodesToBePlaced = nodeMasterList[:]
    nodesPlaced = []

	
    #Place Source Nodes
    for i in range(0,len(srcNodeList)):
        SRCX = 0;
        if (i%2 == 0):
            SRCX = LEFT_X+X_SRC_STEP*(math.floor(i/2))
        else:
            SRCX = RIGHT_X-X_SRC_STEP*(math.floor(i/2))
        nodesToBePlaced.pop(nodesToBePlaced.index(srcNodeList[i]))
        srcNodeList[i].X = SRCX
        srcNodeList[i].Y = TOP_Y
        nodesPlaced.append(srcNodeList[i])
	
    CUR_Y = TOP_Y
    #Place all next nodes
    while nodesToBePlaced:
        toBePlaced = []
        for n in nodesToBePlaced:
            readyToPlace = True
            for pr in n.prevn:
                np = retrieveNodeByID(nodeMasterList,pr)
                if np not in nodesPlaced:
                    readyToPlace = False
            if readyToPlace:
                toBePlaced.append(n)
	
        #Begin placement
        CUR_Y -= Y_STEP
        if len(toBePlaced) > 0:
            X_STEP = (RIGHT_X-LEFT_X)/len(toBePlaced)
        for i in range(0,len(toBePlaced)):
            SRCX = 0
            if (i%2==0):
                SRCX = LEFT_X+X_SRC_STEP*(math.floor(i/2))
            else:
                SRCX = RIGHT_X-X_SRC_STEP*(math.floor(i/2))

            if (X_SRC_STEP > GLOBAL_SKEW_X):
                SRCX = SRCX + random.randint(-GLOBAL_SKEW_X, GLOBAL_SKEW_X)
            if (Y_STEP > GLOBAL_SKEW_Y):
                CUR_Y += random.randint(-GLOBAL_SKEW_Y,GLOBAL_SKEW_Y)
            nodesToBePlaced.pop(nodesToBePlaced.index(toBePlaced[i]))
            toBePlaced[i].X = SRCX
            toBePlaced[i].Y = CUR_Y
            nodesPlaced.append(toBePlaced[i])

	
    return nodesPlaced
	
def drawNodes(cursor,nList):
    """Helper function to draw the nodes onto the screen"""
    for n in nList:
        cursor.penup()
        cursor.goto(n.X,n.Y)
        cursor.stamp()
        cursor.penup()
        cursor.goto(n.X,n.Y-TEXT_BUFFER)
        cursor.write(n.nodeID,True,align="center",font=("Times New Roman",FONT_SIZE,"normal"))
	
def verifyPlacement(nList,eList):
    """Helper function designed to verify placement of nodes onto screen for a directed acyclic graph (DAG)"""
    for e in eList:
        for n in nList:
            if n.nodeID == e.srcNode.nodeID or n.nodeID == e.dstNode.nodeID:
                continue
            elif n.Y > CURSOR_SIZE + e.srcNode.Y or n.Y < e.dstNode.Y - CURSOR_SIZE or n.X > CURSOR_SIZE + max(e.srcNode.X,e.dstNode.X) or n.X < min(e.srcNode.X,e.dstNode.X) - CURSOR_SIZE:
                continue
            headAngle = math.atan2(n.Y - e.srcNode.Y, n.X - e.srcNode.X)
            headAngle = math.degrees(headAngle)
            if headAngle - DEGREE_FOR_CONFLICT <= e.angle <= headAngle + DEGREE_FOR_CONFLICT:
                #This means that the node would be hitting the edge
                #print "Node ", n.nodeID, " is in the edge from Node ", e.srcNode.nodeID, " to Node ", e.dstNode.nodeID
                #print "Angle Violation:\nEdge Angle: ", e.angle, "\nAngle to Violating Node: ", headAngle
                return True
    return False

def displayTable(frameTable,nList,cList):
    #Populate table of information
    nodeTitle = tkinter.Label(frameTable,relief=tkinter.RAISED,text="NODE INFORMATION",font=("Times New Roman", LSET_FONT, "bold"))
    nodeTitle.grid(row=0,columnspan=5,column=0,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)
    for i in range(len(nList)+1): #Rows
        for j in range(NUM_COLUMNS_TABLE): #Columns
            b = tkinter.Label(frameTable,relief=tkinter.RAISED)
            msg = ""
            fontStyle = ""
            if not i:
                fontStyle = "bold"
                msg = COLUMN_HEADERS.pop(0)
            else:
                b.config(bg="gray")
                if j == 0:
                    msg = nList[i-1].nodeID
                if j == 1:
                    msg = nList[i - 1].circuitNet
                if j == 2:
                    msg = nList[i-1].delay
                if j == 3:
                    msg = nList[i-1].label
                if j == 4:
                    msg = "{"
                    for c in nList[i-1].cluster:
                        msg += c + ","
                    msg = msg[0:len(msg)-1] + "}"
                fontStyle = "normal"
            b.config(font=("Times New Roman", FONT_SIZE, fontStyle))
            b.config(text=msg)
            b.grid(row=i+1, column=j,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)
            TABLE_ELEMENTS.append(b)
    clusterTitle = tkinter.Label(frameTable,relief=tkinter.RAISED,text="CLUSTER INFORMATION",font=("Times New Roman", LSET_FONT, "bold"))
    clusterTitle.grid(row=len(nList)+2,column=0,columnspan=5,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)
    for i in range(len(cList)+1): #Rows
        for j in range(NUM_CLUSTER_COLUMN_HEADERS): #Columns
            b = tkinter.Label(frameTable,relief=tkinter.RAISED,bg="gray")
            msg = ""
            numCol = 1
            col = j
            fontStyle = ""
            if not i:
                fontStyle = "bold"
                if j == 2:
                    numCol = 2
                if j == 3:
                    col = 4
                msg = CLUSTER_COLUMN_HEADERS.pop(0)
            else:
                if j == 0:
                    msg = cList[i-1].nodeID
                if j == 1:
                    msg = retrieveNodeByID(nList,cList[i - 1].nodeID).circuitNet
                if j == 2:
                    msg = len(cList[i-1].members)
                    numCol = 2
                if j == 3:
                    msg = "{"
                    for c in cList[i-1].members:
                        msg += c + ","
                    msg = msg[0:len(msg)-1] + "}"
                    col = 4
                fontStyle = "normal"
            b.config(font=("Times New Roman", FONT_SIZE, fontStyle))
            b.config(text=msg)
            b.grid(row=i + len(nList) + 3, column=col,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W,columnspan=numCol)
            CLUSTER_TABLE_ELEMENTS.append(b)


    lsetTitle = tkinter.Label(frameTable,relief=tkinter.RAISED,text="CURRENT L SET",font=("Times New Roman", LSET_FONT, "bold"))
    lsetTitle.grid(row=len(nList)+len(cList)+4,column=0,columnspan=5,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)
    lsetMsg = "{"
    if LSET[LSET_INDEX]:
        for lVal in LSET[LSET_INDEX]:
            lsetMsg += lVal + ","
        lsetMsg = lsetMsg[0:len(lsetMsg)-1] + "}"
    else:
        lsetMsg += "EMPTY}"
    lsetLabel = tkinter.Label(frameTable, relief=tkinter.RAISED,text=lsetMsg,font=("Times New Roman", LSET_FONT, "normal"))
    lsetLabel.grid(row=len(nList)+len(cList)+5,column=0,columnspan=5,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)

    delayTitleLabel = tkinter.Label(frameTable,relief=tkinter.RAISED,text="MAX IO PATH DELAY",font=("Times New Roman", LSET_FONT, "bold"))
    delayTitleLabel.grid(row=len(nList)+len(cList)+6,column=0,columnspan=5,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)
    delayLabel = tkinter.Label(frameTable,relief=tkinter.RAISED,text=MAX_IO_DELAY,font=("Times New Roman", LSET_FONT, "normal"))
    delayLabel.grid(row=len(nList)+len(cList)+7,column=0,columnspan=5,sticky=tkinter.N+tkinter.E+tkinter.S+tkinter.W)
    return lsetLabel

    
def DAGPLACER(srcNList,nList):
    count = 0
    needToPlace = True
    while (needToPlace):
        #do placement
        #print "Attempting Placement Iteration ", count
        nListPlaced = nodePlacement(srcNList,nList)
        #nodePlacement(srcNList,nList)
        edgeList = []
        for srcNode in nListPlaced:
            for n in srcNode.prevn:
                prevNode = retrieveNodeByID(nListPlaced,n)
                e = edgeGenerator(prevNode,srcNode)
                edgeList.append(e)
        #verify placement
        needToPlace = verifyPlacement(nListPlaced,edgeList)
        #if (needToPlace):
            #print "Placement Iteration ", count, " Failed"
        count += 1
        #cont = raw_input("Enter any key to continue")
    return (edgeList,count-1)

def drawDAG():
    #WARNING: GLOBAL FUNCTION!!!!!
    #Drawing time
    p.ht()
    p.penup()
    p.shape("Circle.gif")
    drawNodes(p, nodeList)
    p.shape("classic")
    p.st()
    for e in edgeList:
        arrowGenerator(p, e, SBUFFER, EBUFFER)

def drawClusteredDAG():
    #WARNING: GLOBAL FUNCTION!!!!!
    #Drawing time
    p.ht()
    p.penup()
    p.shape("Cluster.gif")
    drawNodes(p, CLUSTERS_DRAWN)
    for e in edgeClusterList:
    	if e.srcNode in CLUSTERS_DRAWN and e.dstNode in CLUSTERS_DRAWN:
    		p.shape("classic")
    		p.st()
        	arrowGenerator(p, e, SBUFFER, EBUFFER)
        	p.ht()

def prevCallback():
    global LSET_INDEX,CURRENT_CLUSTER_INDEX,CLUSTERS_DRAWN,CURRENT_MODE
    statusLabel.config(text="DRAWING CLUSTERS...",bg="yellow")
    CURRENT_MODE = "CLUSTER"
    cv.delete("all")
    CURRENT_CLUSTER_INDEX -= 1
    LSET_INDEX -= 1
    if LSET_INDEX < 0:
        LSET_INDEX = 0
    if CURRENT_CLUSTER_INDEX < -1:
        CURRENT_CLUSTER_INDEX = -1
    lsetMsg = "{"
    if LSET[LSET_INDEX]:
        for lVal in LSET[LSET_INDEX]:
            lsetMsg += lVal + ","
        lsetMsg = lsetMsg[0:len(lsetMsg)-1] + "}"
    else:
        lsetMsg += "EMPTY}"
    LSET_LABEL.config(text=lsetMsg)
    for i in range(CURRENT_CLUSTER_INDEX+2):
        for j in range(NUM_CLUSTER_COLUMN_HEADERS):
            CLUSTER_TABLE_ELEMENTS[(i+1)*NUM_CLUSTER_COLUMN_HEADERS + j].config(bg="gray")
    for i in range(len(nodeList)):
        for j in range(NUM_COLUMNS_TABLE):
            TABLE_ELEMENTS[(i+1)*NUM_COLUMNS_TABLE + j].config(bg="gray")
    for i in range(CURRENT_CLUSTER_INDEX+1):
        bgcolor = "yellow"
        if i == CURRENT_CLUSTER_INDEX:
            bgcolor = "green"
        for j in range(NUM_CLUSTER_COLUMN_HEADERS):
            CLUSTER_TABLE_ELEMENTS[(i+1)*NUM_CLUSTER_COLUMN_HEADERS + j].config(bg=bgcolor)
    if (CURRENT_CLUSTER_INDEX == -1):
        cv.create_text(0,0,text="NO CLUSTERS FORMED")
    else:
        CLUSTERS_DRAWN = clusterList[0:CURRENT_CLUSTER_INDEX+1]
        for cl in CLUSTERS_DRAWN[0:-1]:
            for c in cl.members:
                for j in range(NUM_COLUMNS_TABLE):
                    TABLE_ELEMENTS[int(c)*NUM_COLUMNS_TABLE + j].config(bg="yellow")
        for c in CLUSTERS_DRAWN[-1].members:
            for j in range(NUM_COLUMNS_TABLE):
                TABLE_ELEMENTS[int(c)*NUM_COLUMNS_TABLE + j].config(bg="green")
        drawClusteredDAG()
    statusLabel.config(text="READY",bg="green")


def nextCallback():
    global LSET_INDEX,CURRENT_CLUSTER_INDEX,CLUSTERS_DRAWN,CURRENT_MODE
    statusLabel.config(text="DRAWING CLUSTERS...",bg="yellow")
    CURRENT_MODE = "CLUSTER"
    cv.delete("all")
    LSET_INDEX += 1
    allClustersPlaced = False
    CURRENT_CLUSTER_INDEX += 1
    if LSET_INDEX >= len(LSET):
        LSET_INDEX = len(LSET)-1
        allClustersPlaced = True
    if CURRENT_CLUSTER_INDEX >= len(clusterList):
        CURRENT_CLUSTER_INDEX = len(clusterList)-1
    lsetMsg = "{"
    if LSET[LSET_INDEX]:
        for lVal in LSET[LSET_INDEX]:
            lsetMsg += lVal + ","
        lsetMsg = lsetMsg[0:len(lsetMsg)-1] + "}"
    else:
        lsetMsg += "EMPTY}"
    LSET_LABEL.config(text=lsetMsg)
    for i in range(CURRENT_CLUSTER_INDEX+1):
        bgcolor = "yellow"
        if i == CURRENT_CLUSTER_INDEX:
            bgcolor = "green"
        if allClustersPlaced:
            bgcolor = "dark green"
        for j in range(NUM_CLUSTER_COLUMN_HEADERS):
            CLUSTER_TABLE_ELEMENTS[(i+1)*NUM_CLUSTER_COLUMN_HEADERS + j].config(bg=bgcolor)
    CLUSTERS_DRAWN = clusterList[0:CURRENT_CLUSTER_INDEX+1]

    clusterBgColor = "yellow"
    if allClustersPlaced:
        clusterBgColor = "dark green"
    for cl in CLUSTERS_DRAWN[0:-1]:
        for c in cl.members:
            for j in range(NUM_COLUMNS_TABLE):
                TABLE_ELEMENTS[int(c)*NUM_COLUMNS_TABLE + j].config(bg=clusterBgColor)
    clusterBgColor = "green"
    if allClustersPlaced:
        clusterBgColor = "dark green"
    for c in CLUSTERS_DRAWN[-1].members:
        for j in range(NUM_COLUMNS_TABLE):
            TABLE_ELEMENTS[int(c)*NUM_COLUMNS_TABLE + j].config(bg=clusterBgColor)
    drawClusteredDAG()
    statusLabel.config(text="READY",bg="green")

def dagCallback():
    global LSET_INDEX,CURRENT_CLUSTER_INDEX,CURRENT_MODE
    statusLabel.config(text="DRAWING DAG...",bg="yellow")
    CURRENT_MODE = "DAG"
    cv.delete("all")
    LSET_INDEX = 0
    CURRENT_CLUSTER_INDEX = -1
    lsetMsg = "{"
    if LSET[LSET_INDEX]:
        for lVal in LSET[LSET_INDEX]:
            lsetMsg += lVal + ","
        lsetMsg = lsetMsg[0:len(lsetMsg)-1] + "}"
    else:
        lsetMsg += "EMPTY}"
    LSET_LABEL.config(text=lsetMsg)
    for i in range(len(clusterList)):
        for j in range(NUM_CLUSTER_COLUMN_HEADERS):
            CLUSTER_TABLE_ELEMENTS[(i+1)*NUM_CLUSTER_COLUMN_HEADERS + j].config(bg="gray")
    for i in range(len(nodeList)):
        for j in range(NUM_COLUMNS_TABLE):
            TABLE_ELEMENTS[(i+1)*NUM_COLUMNS_TABLE + j].config(bg="gray")
    drawDAG()
    statusLabel.config(text="READY",bg="green")

def redrawCallback():
    statusLabel.config(text="REDRAWING...",bg="yellow")
    cv.delete("all")
    if CURRENT_MODE == "DAG":
        drawDAG()
    elif CURRENT_MODE == "CLUSTER":
        if CURRENT_CLUSTER_INDEX == -1:
            cv.create_text(0,0,text="NO CLUSTERS FORMED")
        else:
            drawClusteredDAG()
    statusLabel.config(text="READY",bg="green")
    
#END OF FUNCTIONS    
    
print "Parsing graph.txt"
graphFile = open(INPUT_GRAPH_FILENAME,"r")
nodeList = []
edgeList = []
clusterList = []
clusterOrder = []
FLAG = ""

#Parse logic
for line in graphFile:
    #parse each line 
    line = line.strip()
    sO = re.search( r'//(.*)', line)
    if sO:
        FLAG = sO.group(1)
    if (FLAG == "NODES"):
        sO2 = re.search(r'(.*):(.*);(\d+);(.*);(.*);(.*);(.*)',line)
        if sO2:
            newNode = Node()
            newNode.nodeID = sO2.group(1).strip()
            newNode.circuitNet = sO2.group(2).strip()
            newNode.delay = sO2.group(3).strip()
            newNode.prevn = sO2.group(4).split()
            newNode.nextn = sO2.group(5).split()
            newNode.label = sO2.group(6).split()
            newNode.cluster = sO2.group(7).split()
            newNode.cluster.sort()
            nodeList.append(newNode)
    if (FLAG == "CLUSTERS"):
        sO3 = re.search(r'^(.*):(.*);LSET:(.*);ISET:(.*)$',line)
        if sO3:
            c = Node()
            c.nodeID = sO3.group(1)
            c.members = sO3.group(2).split()
            LSET.append(sO3.group(3).split())
            c.prevn = sO3.group(4).split()
            clusterList.append(c)
            clusterOrder.append(sO3.group(1))
        else:
            sO4 = re.search(r'^LSET:(.*)$',line)
            if sO4:
                LSET.append(sO4.group(1).split())
    if (FLAG == "MAXDELAY"):
        sO5 = re.search(r'(\d+)',line)
        if sO5:
            MAX_IO_DELAY = sO5.group(1)



sourceNodeList = []	
for n in nodeList:
    if not n.prevn:
        sourceNodeList.append(n)

sourceClusterList = []
for c in clusterList:
    if not c.prevn:
        sourceClusterList.append(c)

#Display logic
p.ht()
statusLabel = tkinter.Label(root,text="CONSTRUCTING TABLE AND DAG",bg="yellow",font=("Times New Roman", FONT_SIZE, "bold"))
statusLabel.place(x=4*CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,y=CANVAS_DISPLAY_H+SCROLLBAR_WIDTH,width=CONSOLE_WIDTH/(NUM_CONSOLE_BUTTONS-1),height = CONSOLE_BUTTON_HEIGHT)
LSET_LABEL = displayTable(tableFrame,nodeList,clusterList)
statusLabel.config(text="CONSTRUCTING DAG")

start = time.time()

(edgeList,count) = DAGPLACER(sourceNodeList,nodeList)

end = time.time()
statusLabel.config(text="CONSTRUCTING CLUSTERS")
print "SUMMARY:\nIterations Required For DAG Placement: ", count, "\nElapsed Time (ms): ", str((end - start )*1000.0)

start = time.time()

(edgeClusterList,count) = DAGPLACER(sourceClusterList,clusterList)
 

end = time.time()
statusLabel.config(text="READY",bg="green")
print "SUMMARY:\nIterations Required For Clustered Placement: ", count, "\nElapsed Time (Includes Sorting) (ms): ", str((end - start )*1000.0)

#console buttons
dagButton = tkinter.Button(root, text="DAG", command=dagCallback)
dagButton.place(x=0*CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,y=CANVAS_DISPLAY_H+SCROLLBAR_WIDTH,width = CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,height = CONSOLE_BUTTON_HEIGHT)
prevButton = tkinter.Button(root,text="PREV",command=prevCallback)
prevButton.place(x=1*CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,y=CANVAS_DISPLAY_H+SCROLLBAR_WIDTH,width = CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,height = CONSOLE_BUTTON_HEIGHT)
nextButton = tkinter.Button(root,text="NEXT",command=nextCallback)
nextButton.place(x=2*CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,y=CANVAS_DISPLAY_H+SCROLLBAR_WIDTH,width=CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,height = CONSOLE_BUTTON_HEIGHT)
redrawButton = tkinter.Button(root,text="REDRAW",command=redrawCallback)
redrawButton.place(x=3*CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,y=CANVAS_DISPLAY_H+SCROLLBAR_WIDTH,width=CONSOLE_WIDTH/NUM_CONSOLE_BUTTONS,height = CONSOLE_BUTTON_HEIGHT)



#cv.create_oval(0,0,100,100)



#drawDAG(p,nodeList,edgeList)

#b = Button(master, text="DAG", command=drawDAG)

turtle.done()

root.mainloop()
