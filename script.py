#!python
import sys
import re
import os
from copy import copy
from dataclasses import dataclass


@dataclass
class Pos:
    X: float
    Y: float
    Z: float
    E: float
    XYZabs: bool = True
    Eabs: bool = False


@dataclass
class GcodeLine:
    line: str or int
    pos: Pos
    time: float


def getParam(line: str, p: str) -> float or None:
    DIGITS = "-0123456789.,"
    out = ""
    i = line.find(p)
    if i == -1:
        return None
    i += len(p)
    while i < len(line) and line[i] in DIGITS:
        out += line[i]
        i += 1
    return float(out)


def rmComment(line: str) -> str:
    i = 0
    while i < len(line):
        if line[i] == ";":
            break
        i += 1
    return line[:i]


def parseLine(line: str, prev: Pos) -> Pos:
    cur: Pos = copy(prev)
    if not line.startswith(("G", "M")):
        return cur
    if line.startswith(("G1 ", "G0 ")):
        lineRmCom = rmComment(line)
        if cur.XYZabs:
            X = getParam(lineRmCom, "X")
            if X is not None:
                cur.X = X
            Y = getParam(lineRmCom, "Y")
            if Y is not None:
                cur.Y = Y
            Z = getParam(lineRmCom, "Z")
            if Z is not None:
                cur.Z = Z
        else:
            X = getParam(lineRmCom, "X")
            if X is not None:
                cur.X += X
            Y = getParam(lineRmCom, "Y")
            if Y is not None:
                cur.Y += Y
            Z = getParam(lineRmCom, "Z")
            if Z is not None:
                cur.Z += Z
        if cur.Eabs:
            E = getParam(lineRmCom, "E")
            if E is not None:
                cur.E = E
        else:
            E = getParam(lineRmCom, "E")
            if E is not None:
                cur.E += E
    elif line.startswith("G90"):
        cur.XYZabs = True
    elif line.startswith("G91"):
        cur.XYZabs = False
    elif line.startswith("M82"):
        cur.Eabs = True
    elif line.startswith("M83"):
        cur.Eabs = False
    return cur


def removeGXYZE(line: str) -> str:
    i = 0
    while i < len(line):
        if line[i] in "GXYZE":
            x = i
            for j in range(i + 1, len(line)):
                if line[j] == " " or line[j] == ";":
                    x = j
                    break
            else:
                x = len(line)
            line = line[:i] + line[x:]
            i -= 1
        i += 1
    while "  " in line:
        line = line.replace("  ", " ")
    if len(line) > 0 and line[0] != " ":
        line = " " + line
    return line


def splitLine(
    cur: GcodeLine, bef: GcodeLine, d: float
) -> tuple[GcodeLine, GcodeLine]:
    all = cur.time - bef.time
    if not (cur.line.startswith("G0 ") or cur.line.startswith("G1 ")):
        if not (cur.line.startswith("G10") or cur.line.startswith("G11")):
            print("SplitLine(): error. G0/G1 not found.",
                  "Check klipper_estimator parameters")
            print(cur.line)
            input("Press any key to continue")
            return GcodeLine(";FanRampUp: split error",
                             cur.pos, cur.time), copy(cur)
        return GcodeLine(";FanRampUp: can't split G10/G11",
                         cur.pos, cur.time), copy(cur)
    lineRmCom = rmComment(cur.line)
    prefix = cur.line[0:2]
    suffix = removeGXYZE(cur.line)
    s1 = prefix
    s2 = prefix
    pos1 = copy(bef.pos)
    pos2 = copy(cur.pos)
    if cur.pos.XYZabs:
        if "X" in lineRmCom:
            pos1.X = bef.pos.X + (cur.pos.X - bef.pos.X) * d / all
            s1 += " X" + str(pos1.X)
            s2 += " X" + str(pos2.X)
        if "Y" in lineRmCom:
            pos1.Y = bef.pos.Y + (cur.pos.Y - bef.pos.Y) * d / all
            s1 += " Y" + str(pos1.Y)
            s2 += " Y" + str(pos2.Y)
        if "Z" in lineRmCom:
            pos1.Z = bef.pos.Z + (cur.pos.Z - bef.pos.Z) * d / all
            s1 += " Z" + str(pos1.Z)
            s2 += " Z" + str(pos2.Z)
    else:
        if "X" in lineRmCom:
            X = getParam(lineRmCom, "X")
            pos1.X += X * d / all
            s1 += " X" + str(X * d / all)
            s2 += " X" + str(X * (all - d) / all)
        if "Y" in lineRmCom:
            Y = getParam(lineRmCom, "Y")
            pos1.Y += Y * d / all
            s1 += " Y" + str(Y * d / all)
            s2 += " Y" + str(Y * (all - d) / all)
        if "Z" in lineRmCom:
            Z = getParam(lineRmCom, "Z")
            pos1.Z += Z * d / all
            s1 += " Z" + str(Z * d / all)
            s2 += " Z" + str(Z * (all - d) / all)
    if "E" in lineRmCom:
        E = getParam(lineRmCom, "E")
        if cur.pos.Eabs:
            pos1.E = bef.pos.E + (cur.pos.E - bef.pos.E) * d / all
            s1 += " E" + str(pos1.E)
            s2 += " E" + str(pos2.E)
        else:
            E = getParam(lineRmCom, "E")
            pos1.E += E * d / all
            s1 += " E" + str(E * d / all)
            s2 += " E" + str(E * (all - d) / all)
    s1 += suffix
    s2 += suffix
    time1 = bef.time + (cur.time - bef.time) * d / all
    time2 = cur.time
    return GcodeLine(s1, copy(pos1), time1), GcodeLine(s2, copy(pos2), time2)


def findLineByTime(lines: list[list[GcodeLine]], target: int) -> (int, int):
    if target <= 0:
        return 0, 1
    L: int = 0
    R: int = len(lines) - 1
    M: int = (L + R) // 2
    while R-L > 1:
        if target > lines[M][-1].time:
            L = M
        else:
            R = M
        M = (L + R) // 2
    i = R
    if target >= lines[i][-1].time:
        return i + 1, 0
    for j in range(len(lines[i]) - 2, -1, -1):
        if target >= lines[i][j].time:
            return i, j + 1
    else:
        return i, 0


delay: float = float(sys.argv[1])
sourceFile: str = sys.argv[2]

# Read the ENTIRE g-code file into memory
with open(sourceFile, "r") as f:
    lines: list[str] = f.readlines()
    f.close()

if sourceFile.endswith(".gcode"):
    destFile = re.sub("\\.gcode$", "", sourceFile)
    try:
        os.rename(sourceFile, destFile + ".fan.bak")
    except FileExistsError:
        os.remove(destFile + ".fan.bak")
        os.rename(sourceFile, destFile + ".fan.bak")
    destFile = re.sub("\\.gcode$", "", sourceFile)
    destFile = destFile + ".gcode"
else:
    destFile = sourceFile
    os.remove(sourceFile)

last_percent: int = -10


def update_progress(current, full) -> None:
    global last_percent
    if current * 100 // full - last_percent >= 10:
        last_percent = current * 100 // full
        print_progress_bar(last_percent)


def print_progress_bar(x: int) -> None:
    if x == 0:
        print("Fan ramp up")
        print(" __")
    else:
        print(" ||    " + str(x) + "%")
        if x == 100:
            print(" ‾‾")


currentFan: float = 0
outLines: list[list[GcodeLine]] = []
fanCommands: list[list[int]] = []
outLines.append([GcodeLine(";FanStartUp postprocess", Pos(0, 0, 0, 0), 0)])
with open(destFile, "w") as of:
    times_begin: int = 0
    while times_begin < len(lines):
        if lines[times_begin].startswith(";TIME_ESTIMATE_POSTPROCESSING"):
            times_begin += 1
            break
        times_begin += 1
    if times_begin == 0:
        print("Error. No estimate lines found.",
              "Add klipper_estimator fork to postprocessors")
        input("Press any key")
        exit(1)
    for i in range(len(lines)):
        lines[i] = lines[i].replace("\n", "")
        update_progress(i, len(lines) // 2)
        if lines[i].startswith(";TIME_ESTIMATE_POSTPROCESSING"):
            break
        lineTime = 0.0
        if len(lines[times_begin + i]) >= 2:
            lineTime = float(lines[times_begin + i])
        outLines.append([
            GcodeLine(
                lines[i],
                parseLine(lines[i], copy(outLines[-1][-1].pos)),
                outLines[-1][-1].time + lineTime
            )
        ])
        if lines[i].startswith("M107"):
            fanCommands.append([len(outLines) - 1, 0])
            currentFan = 0
        elif (lines[i].startswith("M106")
                and (getParam(lines[i], "S") > currentFan)):
            lines[i] += ";FanPrevS" + str(currentFan)
            currentFan = getParam(lines[i], "S")
            target = outLines[-1][-1].time - delay
            outLines.pop()
            j, k = findLineByTime(outLines, target)
            br = False
            while (len(fanCommands)
                    and (fanCommands[-1][0] > j or fanCommands[-1][0] == j
                         and fanCommands[-1][1] >= k)):
                line = outLines[fanCommands[-1][0]][fanCommands[-1][1]]
                if line.line.startswith("M107"):
                    line.line = ""
                    fanCommands.pop()
                else:
                    if (
                        getParam(line.line, ";FanPrevS")
                        >= currentFan
                    ):
                        line.line = lines[i]
                        br = True
                        break
                    else:
                        line.line = ""
                        fanCommands.pop()
                if br:
                    break
            if br:
                continue
            if j == 0 and k == 1:
                outLines[0].insert(1, GcodeLine(lines[i],
                                                outLines[0][0].pos,
                                                0))
                fanCommands.append([0, 1])
                continue
            if k == 0:
                befLine = outLines[j - 1][-1]
            else:
                befLine = outLines[j][k - 1]
            if target == befLine.time:
                outLines[j].insert(
                    k,
                    GcodeLine(
                        lines[i],
                        outLines[j][k].pos,
                        befLine.time,
                    ),
                )
                fanCommands.append([j, k])
            elif target > befLine.time:
                l1, l2 = splitLine(
                    outLines[j][k],
                    befLine,
                    target - befLine.time
                )
                outLines[j][k] = copy(l2)
                outLines[j][k:k] \
                    = [copy(l1),
                        GcodeLine(lines[i], copy(l1.pos), l1.time)]
                fanCommands.append([j, k + 1])
        elif lines[i].startswith("M106"):
            fanCommands.append([len(outLines) - 1, 0])
            outLines[-1][-1].line += ";FanPrevS" + str(currentFan)
            currentFan = getParam(lines[i], "S")
    for i in outLines:
        for j in i:
            of.write(j.line + "\n")
    of.close()
