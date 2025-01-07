import numpy as np
from scipy.interpolate import splprep, splev
import matplotlib.pyplot as plt

# Sample data: n 2D points
x = np.linspace(0, 2 * np.pi, 20)
y = np.sin(x)
points = np.column_stack((x, y))

# Parameters
tolerance = 0.016  # Set your error tolerance
max_segments = 10  # Maximum number of segments to try


# Function to calculate the mean squared error
def calculate_error(points, fit_points):
    return np.mean(np.sqrt(np.sum((points - fit_points) ** 2, axis=1)))


# Initial fit
segments = 4
tck, u = splprep([points[:, 0], points[:, 1]], s=segments)

# Evaluate the B-spline fit at the original points
fit_points = np.array(splev(u, tck)).T

# Calculate initial error
error = calculate_error(points, fit_points)

# Dynamically add more segments until error is below tolerance
while error > tolerance and segments < max_segments:
    segments += 1

    t = np.linspace(0, 1, segments + 1)
    tck, u = splprep([points[:, 0], points[:, 1]], t=t, k=2, task=-1)
    print(tck)
    fit_points = np.array(splev(u, tck)).T
    error = calculate_error(points, fit_points)
    print(f"Segments: {segments}, Error: {error}")

# Evaluate the final B-spline fit for visualization
unew = np.linspace(0, 1.0, 1000)
out = splev(unew, tck)

# Plot the original points and the final B-spline fit
plt.plot(points[:, 0], points[:, 1], "o", label="Original points")
plt.plot(out[0], out[1], "-", label="B-spline fit")
plt.legend()
plt.show()
