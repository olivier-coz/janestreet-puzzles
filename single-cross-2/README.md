# Readme for Cube Lattice Line Segment Puzzle

## Problem Statement
The task is to find a length `D` in a 3D space partitioned into a grid of unit cubes which maximizes the probability that the endpoints of a random line segment of length `D` lie in orthogonally adjacent unit cubes. The segment should cross exactly one integer-coordinate plane.

## Algorithm and Reasoning
### 1. Random Point on a Sphere:
We wish to find a line segment of length `D` that crosses a plane between two orthogonally adjacent cubes. To simplify the calculation, we fix one end of the segment at a random point `(x, y, z)` inside a cube and look for the opposite endpoint on the surface of a sphere of radius `D` centered at `(x, y, z)`. This way, if the sphere of radius `D` intersects the neighboring cube, it increases the probability of finding the desired line segment.

The function `random_point_on_sphere` generates a random point on the surface of a sphere given a center `(x, y, z)` and a distance `D`. It does this by:
- Using the spherical coordinates `theta` (azimuthal angle) and `phi` (polar angle).
- Transforming the spherical coordinates into Cartesian coordinates using trigonometric functions.
  
### 2. Checking if Point is Inside Neighboring Cube:
The function `is_inside_neighboring_cube` checks if a given point `(x, y, z)` is inside the defined neighboring cube. This is done by checking if the `x` coordinate lies between 1 and 2 (exclusive of 2) and if the `y` and `z` coordinates lie between 0 and 1 (exclusive of 1).

### 3. Monte Carlo Integration:
Monte Carlo methods are used to estimate the probability of random events. In this context, we use it to approximate the probability that a random segment of length `D` crosses the integer-coordinate plane.

The function `monte_carlo_integration` performs the Monte Carlo integration by:
- Randomly generating a large number of points on the sphere of radius `D` using `random_point_on_sphere`.
- Checking if these points lie inside the neighboring cube using `is_inside_neighboring_cube`.
- Using OpenMP for parallel processing to speed up the simulation.
- Calculating the probability as the ratio of points inside the neighboring cube to the total number of generated points. We multiply this ratio by 6 since there are 6 neighboring cubes (top, bottom, left, right, front, back) for any given cube.

### 4. Main Function:
The main function initializes the length `D` (currently set to 0.750) and uses the Monte Carlo integration to calculate and display the probability that a segment of length `D` crosses the integer-coordinate plane.

## Compile and Run
To compile the program, ensure that you have OpenMP installed and use:
```bash
g++ -fopenmp -o puzzle_solution main.cpp
```

To run the compiled code:
```bash
./puzzle_solution
```

## Note:
This code approximates the solution for a single value of `D`. To find the optimal `D`, you'd need to repeat the Monte Carlo simulation across various potential values of `D` and identify which gives the highest probability.
