import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols

data = pd.read_excel("rice_data.xlsx")

model = ols('dB ~ C(Distance) + C(Condition) + C(Distance):C(Condition)', data=data).fit()
table = sm.stats.anova_lm(model) 
print(table)