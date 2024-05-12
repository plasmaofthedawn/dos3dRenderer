from struct import pack
import sys

if len(sys.argv) == 1:
    print("Where's my fucking file")
    quit()

file = open(sys.argv[1])

vertices = []
edges = set()

for i in file.readlines():
    try:
        i = i[:i.index("#")]
    except ValueError:
        pass
    
    i = i.split()
    
    if len(i) == 0:
        continue

    if i[0] == "v":
        vertices.append([i[1], i[2], i[3]])
    elif i[0] == "f":
        i = [int(x) - 1 for x in i[1:]]
        for j in range(len(i)):
            edges.add(
                (i[j - 1], i[j]) if i[j] > i[j - 1] else (i[j], i[j-1])
            )
    elif i[0] == "l":
        i = [int(x) - 1 for x in i[1:]]
        for j in range(len(i) - 1):
            edges.add(
                (i[j], i[j + 1]) if i[j + 1] > i[j] else (i[j + 1], i[j])
            )


edges = list(edges)

for i in range(len(vertices)):
    
    vertices[i][0] = int(float(vertices[i][0]) * 1000)
    vertices[i][1] = int(float(vertices[i][1]) * 1000)
    vertices[i][2] = int(float(vertices[i][2]) * 1000)
    vertices[i].append(1000)


print(len(vertices))
print(len(edges))

out = open(sys.argv[1].split(".")[0] + ".pmf", "wb")


header_format = "ii"
vertex_format = "iiii"
line_format = "ii"
    
out.write(pack(header_format, len(vertices), len(edges)))

out.flush()

for i in vertices:
    out.write(pack(vertex_format, i[0], i[1], i[2], i[3]))

out.flush()

for i in edges:
    out.write(pack(line_format, *i))

out.close()

    
