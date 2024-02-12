#include <iostream>
#include <random>
#include <cmath>
#include <omp.h>

double compute_condition_parallel(long long n) {
    std::mt19937_64 gen(std::random_device{}()); // Use 64-bit Mersenne Twister for better large range support
    std::uniform_real_distribution<double> distr(0.0, 1.0);

    long long condition_met = 0;

    // Parallelize the loop with OpenMP
    #pragma omp parallel for reduction(+:condition_met) private(gen)
    for (long long i = 0; i < n; ++i) {
        // Thread-local random number generation to avoid contention
        double x1 = distr(gen);
        double x2 = distr(gen);
        double y1 = distr(gen);
        double y2 = distr(gen);

        double distance = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
        double min_x = std::min(x1 + x2, 2 - (x1 + x2));
        double min_y = std::min(y1 + y2, 2 - (y1 + y2));
        double min_condition = std::min(min_x, min_y);

        if (distance > min_condition) {
            condition_met++;
        }
    }

    return static_cast<double>(condition_met) / n;
}

int main() {
    while (true) {
        long long n;
        std::cout << "Enter the number of simulations (n): ";
        std::cin >> n;

        if (n <= 0) {
            std::cout << "Number of simulations must be greater than 0." << std::endl;
            continue;
        }

        double estimate = compute_condition_parallel(n);
        std::cout << "Estimated value: " << estimate << std::endl;

        char choice;
        std::cout << "Run another simulation? (y/n): ";
        std::cin >> choice;
        if (choice != 'y' && choice != 'Y') {
            break;
        }
    }

    return 0;
}
