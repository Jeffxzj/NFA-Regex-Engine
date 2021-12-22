#!/usr/bin/python3
import sys
# import time
import random

# Input
# 1. First line has two integer n, m, denoting the number of cities and the number of dependencies.
# 2. Next m lines, each line contains two integer a; b, denoting city a; b have dependencies.
# 20%, n, m <= 10
# 60%, n <= 1000, m <= 50000
# 100%, 1 <= n <= 1e5, 0 <= m <= 2e6
# Output
# A single integer, denoting the maximum stability of the new country.

SIZES = {"small": (2, 11),
         "mid"  : (11, 1001), 
         "big"  : (1001, 1e5+1)
        }

def gen_dependencies(n, m):
    generated = set()
    cities = list(range(1, n+1))
    count = 0
    while(count < m):
        a = random.randint(1, n)
        b = random.randint(1, n)
        while (b == a):
            b = random.randint(1, n)
        dep, dep_rev = (a, b), (b, a)
        if dep and dep_rev not in generated:
            generated.add(dep)
            print(a, b)
            count += 1
        

if __name__ == '__main__':
    size = sys.argv[1]
    if size not in SIZES.keys():
        print("WRONG size")
        exit(-1)
    l, r = SIZES[size][0], SIZES[size][1]
    # n: number of cities (vertices), m: number of dependencies (connected edges)
    n = random.randint(l, r)   
    if n == 2:
        m = 1
    if n > 2:
        m_max = (n * (n - 1)) // 2    
        if sys.argv[2] == "le":
            m = random.randint(l, r)
        else:
            m = random.randint(l, m_max)
    
    print(n, m)
    gen_dependencies(n, m)
        
