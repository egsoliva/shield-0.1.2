import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

data = pd.read_excel('rice_data.xlsx')

distance = data['Distance']
decibel = data['dB']
medium = data['Condition']

color_map = {
    "Uncovered": "red",
    "Concrete": "green",
    "Wood": "yellow",
    "Cardboard": "blue"
}

for condition, color in color_map.items():
    subset = data[data["Condition"] == condition]
    plt.scatter(
        subset["Distance"], 
        subset["dB"], 
        label=condition, 
        color=color, 
        alpha=0.7
    )

distance_list = distance.tolist()
decibel_list = decibel.tolist()

x = np.array(distance_list)
y = np.array(decibel_list)

plt.title('Scatter Plot')
plt.xticks([0, 1, 5, 10])
plt.xlabel('Distance (m)')
plt.ylabel('dB')
plt.legend(title="Condition")
plt.show()