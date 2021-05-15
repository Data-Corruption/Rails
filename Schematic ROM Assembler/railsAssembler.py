import sys
import nbtlib
import numpy as np

from nbtlib import File
from nbtlib.tag import *

_height = 15
_length = 17
_width = 35

lineTags = {}
lineNumber = 0
instructions = np.zeros((128, 2, 16), dtype=int)
block = np.zeros((_height, _length, _width), dtype=int)

def toBinaryString(x, bits):
  if 2**bits <= x:
    sys.exit('ERROR: argument out of bounds' + str(lineNumber))
  else:
    bin = int(f'{x:b}')
    return f'{bin:0{bits}d}'

def parseImmediate(x):
    if ':' in x:
        if x not in lineTags:
            sys.exit('ERROR: unknown tag on line ' + str(lineNumber))
        return toBinaryString(lineTags[x], 8)[::-1]
    return toBinaryString(int(x), 8)[::-1]

def parseRegister(s):
    s.strip()
    if 'r' in s:
        s = s.replace('r', '')
    return toBinaryString(int(s), 4)

# ========== Script Start ==========
# Get arguments # -ass
cmdargsLength = len(sys.argv)
cmdargs = sys.argv

if cmdargsLength < 2:
    sys.exit('ERROR: missing source / destination arguments')

# open source file
try:
    sourceFile = open(cmdargs[1], "r")
except:
    print('ERROR: issue reading source file')

lines = sourceFile.readlines()

# load line tags
for line in lines:
    line.strip()
    args = line.split()

    if '#' in args[0]:
        continue

    if ':' in args[0]:
        lineTags[args[0]] = lineNumber
    lineNumber += 1

# load instructions
threeOPs = [0, 1, 2, 3, 4]
immOPs = [6, 8, 10, 11, 12]
acOPs = [5, 7, 13, 14]
abOPs = [9, 15]

opCodes = {
    'ADD':  0,
    'ADDC': 1,
    'SUB':  2,
    'SWB':  3,
    'NAND': 4,
    'RSFT': 5,
    'LIMM': 6,
    'LD':   7,
    'LDIM': 8,
    'ST':   9,
    'STIM': 10,
    'BEQ':  11,
    'BGT':  12,
    'JMPL': 13,
    'IN':   14,
    'OUT':  15,
    # Pseudo Instructions
    'NOP':  0,
    'MOV':  0,
    'JMP':  11,
    'EXIT': 13,
    }

rawLines = np.zeros((256, 16), dtype=int)
lineNumber = 0
try:

    for line in lines:
        line.strip()
        args = line.split()
        i = 0

        if '#' in args[i]:
            continue
        if ':' in args[i]:
            i += 1

        opcode = opCodes[args[i]]

        cinst = []
        cinst.extend(list(toBinaryString(opcode, 4)))
    
        if args[i] == 'NOP':
            cinst.extend([0,0,0,0,0,0,0,0,0,0,0,0])
            opcode = ''
        if args[i] == 'MOV':
            cinst.extend([0, 0, 0, 0])
            cinst.extend(list(parseRegister(args[i + 1])))
            cinst.extend(list(parseRegister(args[i + 2])))
            opcode = ''
        if args[i] == 'JMP':
            cinst.extend(list(parseImmediate(args[i + 1])))
            cinst.extend([1, 1, 1, 1])
            opcode = ''
        if args[i] == 'EXIT':
            cinst.extend([0,0,0,0,0,0,0,0,0,0,0,0])
            opcode = ''
    
        if opcode in threeOPs:
            cinst.extend(list(parseRegister(args[i + 1])))
            cinst.extend(list(parseRegister(args[i + 2])))
            cinst.extend(list(parseRegister(args[i + 3])))
        if opcode in immOPs:
            cinst.extend(list(parseImmediate(args[i + 1])))
            cinst.extend(list(parseRegister(args[i + 2])))
        if opcode in acOPs:
            cinst.extend(list(parseRegister(args[i + 1])))
            cinst.extend([0, 0, 0, 0])
            cinst.extend(list(parseRegister(args[i + 2])))
        if opcode in abOPs:
            cinst.extend(list(parseRegister(args[i + 1])))
            cinst.extend(list(parseRegister(args[i + 2])))
            cinst.extend([0, 0, 0, 0])
    
        if len(cinst) < 16:
            sys.exit('ERROR: Parsing error on line ' + str(lineNumber))

        rawLines[lineNumber] = cinst
        print("Line " + str(lineNumber) + ": " + ("".join(map(str, cinst))))
        lineNumber += 1
except:
    sys.exit('ERROR: parsing line ' + str(lineNumber))


print('Sucsesfully assembled source file!')

rI = 0
for x in range(0, 256, 2):
    instructions[rI, 0] = rawLines[x]
    instructions[rI, 1] = rawLines[x + 1]
    rI += 1

# create blockarray
vertIgnore = [1, 3, 5, 7, 9, 11, 13]
lengthIgnore = [8]
widthIgnore = [8, 17, 26]
observerDown = [0, 2, 4, 6]
bID, ci = 0, 0

for y in range(_height):
    if y in vertIgnore:
        continue
    for z in range(_length):
        if z in lengthIgnore:
            continue
        x0 = 0
        hf = 0
        for x in range(_width):
            if x in widthIgnore:
                continue

            if y in observerDown:
                bID = 1
            else:
                bID = 2

            if (x0 % 2) == 0:
                if instructions[ci, 0, hf]:
                    block[y, z, x] = bID
                else:
                    block[y, z, x] = 3
            else:
                if instructions[ci, 1, hf]:
                    block[y, z, x] = bID
                else:
                    block[y, z, x] = 3
                hf += 1
            x0 += 1
        ci += 1

# encode blockarray into bytearray
blockData = bytearray()

for y in range(_height):
    for z in range(_length):
        for x in range(_width):
            blockData.append(block[y, z, x])

romSchematic = File({
    'Schematic': Compound({
        'Version': Int(2),
        'DataVersion': Int(2584),
        'Height': Short(_height),
        'Length': Short(_length),
        'Width': Short(_width),
        'PaletteMax': Int(4),
        'Palette': Compound({
            'minecraft:air': Int(0),
            'minecraft:observer[facing=up,powered=false]': Int(1),
            'minecraft:observer[facing=down,powered=false]': Int(2),
            'minecraft:blue_stained_glass': Int(3),
            }),
        'BlockData': ByteArray(blockData),
    })
},
gzipped=True
)

romSchematic.save(cmdargs[2])
print('Sucsesfully generated ROM schematic file!')