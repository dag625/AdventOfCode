import sys
import time
from itertools import islice
from z3 import Int, Solver


start = time.time_ns()
input_file = open(sys.argv[1], "r")
input = input_file.read()

stones = []
for l in input.splitlines():
    a, b = l.split('@')
    pos = [int(s.strip()) for s in a.split(',')]
    vel = [int(s.strip()) for s in b.split(',')]
    stones.append((pos, vel))

#print(stones)

fx, fy, fz = Int("x"), Int("y"), Int("z")
fvx, fvy, fvz = Int("vx"), Int("vy"), Int("vz")
solver = Solver()
for idx, ((x,y,z),(vx, vy, vz)) in enumerate(islice(stones, 3)):
   ft = Int(f"t{idx}")
   solver.add(ft >= 0)
   solver.add(fx + fvx * ft == x + vx * ft)
   solver.add(fy + fvy * ft == y + vy * ft)
   solver.add(fz + fvz * ft == z + vz * ft)
assert str(solver.check()) == "sat"
mod = solver.model()
res = mod.eval(fx + fy + fz)
end = time.time_ns()
print(f"Part 2:  {res}")
print(f"Time:    {end - start}ns ({(end - start)/1000000000}s)")
