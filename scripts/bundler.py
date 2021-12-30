#!/usr/bin/env python3

project_dir = '~/code/projects/codinGame/breakthrough'
files = []
with open('scripts/sources.txt') as sources:
    for source in sources.readlines():
        files.append(f"../{format(source.strip())}")
output = 'btbundled.cpp'
optim_header = """
#undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
#pragma GCC target("movbe") // byte swap
#pragma GCC target("aes,pclmul,rdrnd") // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD
"""

with open(output, "w+") as out:
    out.write(optim_header)
    for file in files:
        with open(file) as f:
            for line in f.readlines():
                if (line.startswith('#ifndef') or
                    line.startswith('#define') or
                    line.startswith('#endif') or
                    line.startswith('#include "')):
                    continue;
                out.write(line)
            out.write('\n')
