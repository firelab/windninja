
import sys

if __name__ == '__main__':

    if len(sys.argv) < 3:
        sys.exit(1)

    fin = open(sys.argv[1])
    if not fin:
        sys.exit(1)
    key = sys.argv[2]
    val = None
    for line in fin.readlines():
        line = line.strip()
        if not line:
            continue
        if line[0] == '#':
            continue
        tokens = [t.strip() for t in line.split('=')]
        if len(tokens) != 2:
            break
        if key == tokens[0]:
            val = tokens[1]
            break
    fin.close()
    if not val:
        sys.exit(1)
    sys.stdout.write(val)
    sys.exit(0)






