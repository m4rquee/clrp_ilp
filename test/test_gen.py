from itertools import chain

# Reduce from an instance of 3DM:
n = 3
E = {(0, 0, 0), (1, 1, 0), (2, 1, 1), (2, 2, 2)}
# Represents the edges of the corresponding graph:
E_1 = set()
E_2 = set()

inter_count = [0 for i in range(n)]
for e in E:
    inter_count[e[1]] += 1
    E_1.add((e[0], e[1]))
    E_2.add((e[1], e[2]))

A = list(range(n))
B = [[] for _ in A]
B_len = 0
for i, c in enumerate(inter_count):
    if c > 1: c *= 2
    offset = n + B_len
    B[i] = list(range(offset, offset + c))
    B_len += c
offset = n + B_len
C = list(range(offset, offset + n))

# Assign an id to the tuples inside each group that intersect at B:
edge_map_1 = {}
edge_map_2 = {}
for e in E:
    inter_count[e[1]] -= 1
    edge_map_1[(e[0], e[1])] = inter_count[e[1]]
    edge_map_2[(e[1], e[2])] = inter_count[e[1]]

V = [A, B, C]  # index map
L = ([([i], 1) for i in A] +  # every v in A has degree 1
     [(b, 2) for b in B] +  # every super vertex in B has degree 2
     [([i], 1) for i in C] +  # every v in C has degree 1
     [(A, n)] +  # one edge to each element
     [(list(chain.from_iterable(B)), 2 * n)]  # two edges to each super vertex
     + [(C, n)])  # one edge to each element

# Add the non-edges cuts (AxB):
for i in range(n):
    for j in range(n):
        if (i, j) not in E_1:
            L.append(([A[i]] + B[j], 3))
        else:  # Propagate the restrictions to the super vertex inside:
            b = B[j]
            b_size = len(b)
            if b_size == 1: continue
            edge_i = edge_map_1[(i, j)]
            for v in b:
                if v != b[2 * edge_i]:
                    L.append(([A[i], v], 2))

# Add the non-edges cuts (BxC):
for i in range(n):
    for j in range(n):
        if (i, j) not in E_2:
            L.append((B[i] + [C[j]], 3))
        else:  # Propagate the restrictions to the super vertex inside:
            b = B[i]
            b_size = len(b)
            if b_size == 1: continue
            edge_i = edge_map_2[(i, j)]
            for v in b:
                if v != b[2 * edge_i + 1]:
                    L.append(([v, C[j]], 2))

# Add the intersection gadget cuts:
for b in B:
    b_size = len(b)
    if b_size == 1: continue
    left = b[::2]
    right = b[1::2]

    for v in b: L.append(([v], 1))  # each vertex in each super vertex has degree one

    # Prohibit edges between non-matching pairs in the same sides:
    for i, u in enumerate(left):
        for j, v in enumerate(left):
            if i < j:
                L.append(([u, v], 2))
    for i, u in enumerate(right):
        for j, v in enumerate(right):
            if i < j:
                L.append(([u, v], 2))

    # Prohibit edges between non-matching pairs of separate sides:
    for i, u in enumerate(left):
        for j, v in enumerate(right):
            if i != j:
                L.append(([u, v], 2))

print(2 * n + B_len, len(L))
for V_i, c_i in L:
    print(len(V_i), c_i, ' '.join(map(str, V_i)))
