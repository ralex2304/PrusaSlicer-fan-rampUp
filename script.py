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
    line: str
    pos: Pos
    time: float = 0


def getParam(line: str, p: str) -> float:
    DIGITS = "-0123456789.,"
    out = ""
    i = 0
    while i + len(p) <= len(line) and line[i:i + len(p)] != p:
        i += 1
    if i + len(p) > len(line):
        print("getParam(): Can't find parameter: ", p, " in line: ", line)
        input()
        return 0
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
    if line.startswith("G90"):
        cur.XYZabs = True
    elif line.startswith("G91"):
        cur.XYZabs = False
    elif line.startswith("M82"):
        cur.Eabs = True
    elif line.startswith("M83"):
        cur.Eabs = False
    elif line.startswith("G1 ") or line.startswith("G0 "):
        lineRmCom = rmComment(line)
        if cur.XYZabs:
            if "X" in lineRmCom:
                cur.X = getParam(lineRmCom, "X")
            if "Y" in lineRmCom:
                cur.Y = getParam(lineRmCom, "Y")
            if "Z" in lineRmCom:
                cur.Z = getParam(lineRmCom, "Z")
        else:
            if "X" in lineRmCom:
                cur.X += getParam(lineRmCom, "X")
            if "Y" in lineRmCom:
                cur.Y += getParam(lineRmCom, "Y")
            if "Z" in lineRmCom:
                cur.Z += getParam(lineRmCom, "Z")
        if cur.Eabs:
            if "E" in lineRmCom:
                cur.E = getParam(lineRmCom, "E")
        else:
            if "E" in lineRmCom:
                cur.E += getParam(lineRmCom, "E")
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
    all = cur.time
    if not (cur.line.startswith("G0 ") or cur.line.startswith("G1 ")):
        if not (cur.line.startswith("G10") or cur.line.startswith("G11")):
            print(
                "SplitLine(): error. G0/G1 not found.\
                 Check klipper_estimator parameters"
            )
            input("Press any key to continue")
            return cur.line, ";FanRampUp: split error"
        return cur.line, ";FanRampUp: can't split G10/G11"
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
    return GcodeLine(s1, copy(pos1), 0), GcodeLine(s2, copy(pos2), 0)


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

last_percent: int = -5


def update_progress(current, full):
    global last_percent
    if current * 100 // full - last_percent >= 5:
        last_percent = current * 100 // full
        print_progress_bar(last_percent)


def print_progress_bar(x: int) -> None:
    clear_screen()
    print("|", end="")
    eq = x // 10
    for i in range(eq):
        print("=", end="")
    for i in range(10 - eq):
        print(" ", end="")
    print("| " + str(x) + "%")


def clear_screen() -> None:
    if os.name == "nt":
        os.system("cls")
    else:
        os.system("clear")


currentFan: float = 0
outLines: list[GcodeLine] = []
outLines.append(GcodeLine(";FanStartUp postprocess", Pos(0, 0, 0, 0)))
with open(destFile, "w") as of:
    times_begin: int = 0
    while times_begin < len(lines):
        if lines[times_begin].startswith(";TIME_ESTIMATE_POSTPROCESSING"):
            times_begin += 1
            break
        times_begin += 1
    if times_begin == 0:
        print(
            "Error. No estimate lines found.\
                Add klipper_estimator fork to postprocessors"
        )
        input("Press any key")
        exit(1)
    for i in range(len(lines)):
        lines[i] = lines[i].replace("\n", "")
        update_progress(i, len(lines) // 2)
        if lines[i].startswith(";TIME_ESTIMATE_POSTPROCESSING"):
            break
        time = 0.0
        if len(lines[times_begin + i]) >= 2:
            time = float(
                lines[times_begin + i][0:len(lines[times_begin + i]) - 1]
            )
        outLines.append(
            GcodeLine(
                lines[i],
                parseLine(lines[i], copy(outLines[len(outLines) - 1].pos)),
                time
            )
        )
        if lines[i].startswith("M107"):
            currentFan = 0
        elif (lines[i].startswith("M106")
                and (getParam(lines[i], "S") > currentFan)):
            lines[i] += ";FanPrevS" + str(currentFan)
            currentFan = getParam(lines[i], "S")
            target = delay
            del outLines[len(outLines) - 1]
            for j in range(0, len(outLines) - 1):
                if outLines[len(outLines) - 1 - j].line.startswith("M107"):
                    outLines[len(outLines) - 1 - j].line = ""
                    continue
                elif outLines[len(outLines) - 1 - j].line.startswith(
                    "M106"
                ):
                    if (
                        getParam(
                            outLines[len(outLines) - 1 - j].line,
                            ";FanPrevS",
                        )
                        >= currentFan
                    ):
                        outLines[len(outLines) - 1 - j].line = lines[i]
                        break
                    else:
                        outLines[len(outLines) - 1 - j].line = ""
                        continue
                target -= outLines[len(outLines) - 1 - j].time
                if target == 0:
                    outLines.insert(
                        len(outLines) - 1 - j,
                        GcodeLine(
                            lines[i],
                            outLines[len(outLines) - 1 - j].pos,
                            0,
                        ),
                    )
                    break
                elif target < 0:
                    l1, l2 = splitLine(
                        outLines[len(outLines) - 1 - j],
                        outLines[len(outLines) - 1 - j - 1],
                        -target
                    )
                    l1.time = -target
                    l2.time = outLines[len(outLines) - 1 - j].time + target
                    outLines[len(outLines) - 1 - j] = copy(l2)
                    outLines[len(outLines) - 1 - j:len(outLines) - 1 - j] \
                        = [copy(l1), GcodeLine(lines[i], copy(l1.pos), 0)]
                    break
            else:
                outLines.insert(1, GcodeLine(lines[i], outLines[0].pos, 0))
        elif lines[i].startswith("M106"):
            outLines[len(outLines) - 1].line += ";FanPrevS" + str(
                currentFan
            )
            currentFan = getParam(lines[i], "S")
    for line in outLines:
        of.write(line.line + "\n")
    of.close()
