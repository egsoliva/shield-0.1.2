from sklearn.metrics import mean_squared_error
import numpy as np
import pandas as pd
import math

data = pd.read_excel('gps_data.xlsx')
lat = data['Latitude']
lon = data['Longitude']
true_lat = data['True Latitude']
true_lon = data['True Longitude']
true_dist, true_dist1, true_dist2 = [], [], []
dist1, dist2 = [], []
mse1, mse2 = [], []
loc_rmse = []

dlat = np.deg2rad(lat - true_lat)
dlon = np.deg2rad(lon - true_lon)
lat = np.deg2rad(lat)
lon = np.deg2rad(lon)
true_lat = np.deg2rad(true_lat)
true_lon = np.deg2rad(true_lon)

for i in lat:
    true_dist_value = 0
    true_dist.append(true_dist_value)

# Haversine Formula
rEarth = 6371000
a = (np.sin(dlat / 2) ** 2) + np.cos(true_lat) * np.cos(lat) * ((np.sin(dlon / 2) / 2) ** 2)
c = 2 * np.arcsin(np.sqrt(a))
dist = rEarth * c

dist1 = dist[0:30]
true_dist1 = true_dist[0:30]
mse1 = mean_squared_error(dist1, true_dist1)
np.sqrt(mse1)
loc_rmse.append(np.sqrt(mse1))

dist2 = dist[30:60]
true_dist2 = true_dist[30:60]
mse2 = mean_squared_error(dist2, true_dist2)
np.sqrt(mse2)
loc_rmse.append(np.sqrt(mse2))