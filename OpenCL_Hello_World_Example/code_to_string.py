#Input: <OpenCL Code file> <File containing const char *KernelSource>

#!/usr/bin/python

import sys
import re

openCLCodeFilename = sys.argv[1]
print openCLCodeFilename

codeAsString = "const char *KernelSource = "

with open(openCLCodeFilename) as openCLFile:
	openCLLines = openCLFile.readlines()

# Strip new-lines at end
openCLLines = [x.strip() for x in openCLLines]

for codeLine in openCLLines:
	codeAsString += "\"" + codeLine + "\\n\" \\\n"

# Cut off final escape and new line
codeAsString = codeAsString[:len(codeAsString) - 3]

codeAsString += ";"


hostCodeFilename = sys.argv[2]
with open(hostCodeFilename, 'r') as hostCodeFile:
    hostCodeLines = hostCodeFile.readlines()

hostCodeFile.close()

startMarker = "// Start Kernel"
endMarker = "// End Kernel"

fullReplacement = startMarker + '\n' + codeAsString + "\n"

finalText = ""
lineIndex = 0
while lineIndex < len(hostCodeLines):
	line = hostCodeLines[lineIndex]
	if startMarker in line:
		finalText += fullReplacement
		while endMarker not in line:
			lineIndex += 1
			line = hostCodeLines[lineIndex]
	finalText += line
	lineIndex += 1

# print finalText

hostCodeOutputFile = open(hostCodeFilename, "w")

hostCodeOutputFile.write(finalText)

hostCodeOutputFile.close()

