#!python
import sys
import re
import os
import math


def getParam(line,p):
    DIGITS="0123456789.,"
    out=""
    i=0
    while line[i]!=p:
        i+=1
    i+=1
    while i<len(line) and line[i] in DIGITS:
        out+=line[i]
        i+=1
    return float(out)

def parseG1(line,X,Y,F):
    Xnew=X
    Ynew=Y
    Fnew=F
    if "X" in line:
        Xnew=getParam(line,"X")
    if "Y" in line:
        Ynew=getParam(line,"Y")
    if "F" in line:
        Fnew=getParam(line,"F")/60
    time=math.sqrt((X-Xnew)**2+(Y-Ynew)**2)/Fnew
    return time,Xnew,Ynew,Fnew

    


delay=float(sys.argv[1])
sourceFile=sys.argv[2]

# Read the ENTIRE g-code file into memory
with open(sourceFile, "r") as f:
    lines = f.readlines()

if (sourceFile.endswith('.gcode')):
    destFile = re.sub('\.gcode$','',sourceFile)
    try:
        os.rename(sourceFile, destFile+".fan.bak")
    except FileExistsError:
        os.remove(destFile+".fan.bak")
        os.rename(sourceFile, destFile+".fan.bak")
    destFile = re.sub('\.gcode$','',sourceFile)
    destFile = destFile + '.gcode'
else:
    destFile = sourceFile
    os.remove(sourceFile)

fan=0
feed=10000
coordX=0
coordY=0
currentFan=0

outLines=[]
times=[]
with open(destFile, "w") as of:
    of.write('; FanStartUp postproccess\n')
    for lIndex in range(len(lines)):
        oline = lines[lIndex]
        outLines.append(oline)
        # Parse gcode line
        time=0
        if oline.startswith('G0') or oline.startswith('G1'):
            time,coordX,coordY,feed=parseG1(oline,coordX,coordY,feed)
        if lIndex==0:
            times.append(time)
        else:
            times.append(times[len(times)-2]+time)
        if oline.startswith('M107'):
            currentFan=0
        if oline.startswith('M106') and getParam(oline,"S")>currentFan:
            currentFan=getParam(oline,"S")
            target=times[len(times)-1]-delay
            del outLines[len(outLines)-1]
            del times[len(times)-1]
            for i in range(1,len(times)-1):
                if outLines[len(times)-1-i].startswith('M107') or outLines[len(times)-1-i].startswith('M106'):
                    outLines[len(times)-1-i]=""
                    continue
                if target==times[len(times)-1-i]:
                    times.insert(len(times)-1-i+1,times[len(times)-1-i])
                    outLines.insert(len(times)-1-i+1,oline)
                elif target>times[len(times)-1-i]:
                    times.insert(len(times)-1-i+1,times[len(times)-1-i])
                    outLines.insert(len(times)-1-i+1,oline)
                    if target-times[len(times)-1-i]>target:
                        print("Warning! Time error: ", target-times[len(times)-1-i], " s")
                        input()
                    break
            else:
                outLines.insert(0,oline)
                times.insert(0,0)
        elif oline.startswith('M106'):
            currentFan=getParam(oline,"S")
    for l in outLines:
        of.write(l)
        
of.close()
f.close()
