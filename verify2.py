import sys

f1, f2 = open(sys.argv[1]), open(sys.argv[2])
f2lines = f2.readlines()
diff = []
for line in f1.readlines():
    if line not in f2lines:
        v1, v2, w = list(map(int, line[:-1].split()))
        line = f"{v2} {v1} {w}\n"
        if line not in f2lines:
            diff.append(line)
f1.close()
f2.close()

if len(diff) == 0:
    print("Ok")
else:
    for str in diff:
        print(str, end='')