import math,sys
T={}
for l in open('teero_track.txt'):
    p=l.split()
    if len(p)>=3:
        try: T[int(p[0])]=(float(p[1]),float(p[2]))
        except: pass
def inv(d):
    lo,hi=0,90
    for _ in range(60):
        m=(lo+hi)/2
        f=m*(1.4**(-(50*m-550)/2000)) if m>11 else m
        if f<d: lo=m
        else: hi=m
    return lo
def fit(ks,vals):
    import copy
    A=[[sum(k**(i+j) for k in ks) for j in range(3)] for i in range(3)]
    B=[sum(v*k**i for k,v in zip(ks,vals)) for i in range(3)]
    for c in range(3):
        p=max(range(c,3),key=lambda r:abs(A[r][c])); A[c],A[p]=A[p],A[c]; B[c],B[p]=B[p],B[c]
        for r in range(3):
            if r!=c:
                f=A[r][c]/A[c][c]
                for cc in range(3): A[r][cc]-=f*A[c][cc]
                B[r]-=f*B[c]
    return [B[i]/A[i][i] for i in range(3)]
a,b,st=int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3])
W=4
for k in range(a,b,st):
    ks=[j for j in range(k-W,k+W+1) if j in T]
    cx=fit([j-k for j in ks],[T[j][0] for j in ks]); cy=fit([j-k for j in ks],[T[j][1] for j in ks])
    dx,dy=cx[1],cy[1]; d=math.hypot(dx,dy); v=inv(d)
    print(f"k={k:5d} pos ({cx[0]:7.1f},{cy[0]:6.1f}) disp ({dx:6.2f},{dy:6.2f}) |v|~{v:5.1f} E~{v*v-cy[0]:7.0f}")
