#include <iostream>
#include <cmath>
#include <random>
#include <tuple>
#include <omp.h>
#include <iomanip>

const double PI = 3.14159265358979323846;

// Generates a random point on a sphere of distance D centered at (x, y, z)
std::tuple<double, double, double> random_point_on_sphere(double x, double y, double z, double D, std::minstd_rand &mt) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    double theta = 2 * PI * dist(mt);  // azimuthal angle
    double phi = acos(2 * dist(mt) - 1);  // polar angle

    double dx = D * sin(phi) * cos(theta);
    double dy = D * sin(phi) * sin(theta);
    double dz = D * cos(phi);

    return {x + dx, y + dy, z + dz};
}

// Checks if the point (x, y, z) is inside the defined cube
bool is_inside_neighboring_cube(double x, double y, double z) {
    return (1 <= x && x < 2) && (0 <= y && y < 1) && (0 <= z && z < 1);
}

// Performs Monte Carlo integration to estimate probability
double monte_carlo_integration(double D, int num_points = 500000) {
    int count_inside_cube = 0;

    #pragma omp parallel for reduction(+:count_inside_cube)
    for (int i = 0; i < num_points; i++) {
        std::minstd_rand mt(i);  
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        auto [x, y, z] = random_point_on_sphere(dist(mt), dist(mt), dist(mt), D, mt);
        count_inside_cube += is_inside_neighboring_cube(x, y, z) ? 1 : 0;
    }

    return static_cast<double>(6 * count_inside_cube) / num_points;
}

int main() {
    double D = 0.750; // Sphere radius
    std::cout << std::fixed << std::setprecision(12) << "Result: " << monte_carlo_integration(D) << std::endl;
    return 0;
}
