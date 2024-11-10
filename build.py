import sys
sys.path.append('../my-build-tools/')

from builder import builder
tools = builder.tools

# get inputs
flag = "#include"
inputs = []
with open("./test.h", 'r') as file:
    lines = file.readlines()
for line in lines:
    if line.startswith("//") or "files" not in line:
        continue
    elif flag in line:
        line = line.replace(flag, "")
        line = line.replace('"', "")
        line = line.strip()
        inputs.append(line)

# on macro
def _onMacro():
    def onMacro(code, command, argument = None):
        return code
    return onMacro

# on line
def _onLine():
    def onLine(line):
        return line
    return onLine

# merge
bldr1 = builder.code()
bldr1.setInput(inputs)
bldr1.setComment("//", "{0}")
bldr1.setOutput("tools.h")
bldr1.onMacro(_onMacro())
bldr1.onLine(_onLine())
bldr1.start()

# test
bldr2 = builder.c()
bldr2.setDebug(True)
bldr2.setInput("./test.c")
bldr2.setOutput('test')
bldr2.start()
bldr2.run()

