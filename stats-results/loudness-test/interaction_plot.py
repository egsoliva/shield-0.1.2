from statsmodels.stats.multicomp import pairwise_tukeyhsd
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import statsmodels.api as sm
from statsmodels.formula.api import ols
import seaborn as sns

df = pd.read_excel("rice_data.xlsx")

model = ols('dB ~ C(Distance) + C(Condition) + C(Distance):C(Condition)', data=df).fit()
table = sm.stats.anova_lm(model) 
tukey_distance = pairwise_tukeyhsd(df['dB'], df['Distance'], alpha=0.05)
print(tukey_distance)
tukey_condition = pairwise_tukeyhsd(df['dB'], df['Condition'], alpha=0.05)
print(tukey_condition)


sns.lineplot(data=df, x='Distance', y='dB', hue='Condition', ci=95, estimator='mean')
plt.title("Interaction Effect: Distance : Condition on dB")
plt.show()