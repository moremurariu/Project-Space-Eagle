import sys
from xl import *
# transplant.py NEWPREFIX CUTNEW OLDFILE CUTOLD OUT  (rt cuts)
newp, cn, oldf, co, out = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4]), sys.argv[5]
L = open(newp).readlines()[:cn + 68] + open(oldf).readlines()[co + 68:]
open(out, 'w').writelines(L)
