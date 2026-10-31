import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import pandas as pd
from ptrade.research_utils import load_panel, evaluate, print_eval
from ptrade.strategies import signals as S

P = load_panel()
F = pd.DataFrame(0.0001/8, index=P.index, columns=P.columns)
for lb, skip, reb, rev in [(30,0,7,False),(14,0,7,False),(60,0,7,False),(90,7,14,False),(7,0,7,False),
                           (3,0,1,True),(1,0,1,True),(7,0,7,True),(5,0,3,True)]:
    w = S.xs_momentum(P, lookback_d=lb, skip_d=skip, rebalance_d=reb, reverse=rev, quantile=0.25)
    _, ev = evaluate(P, w, funding=F, label=f"XS {'REV' if rev else 'MOM'} lb={lb} skip={skip} reb={reb}")
    print_eval(ev)
for lb, reb in [(24,24),(24,6),(6,6),(72,24)]:
    w = S.intraday_reversal(P, lookback_h=lb, rebalance_h=reb)
    _, ev = evaluate(P, w, funding=F, label=f"Intraday REV lb={lb}h reb={reb}h")
    print_eval(ev)
