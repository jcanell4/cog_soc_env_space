#include "Utilities.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {

std::mt19937& thread_engine() {
    thread_local std::mt19937 eng{std::random_device{}()};
    return eng;
}

} // namespace

namespace utilities {

void seedRng(std::uint64_t seed) {
    std::mt19937& eng = thread_engine();
    if (seed == 0) {
        std::random_device rd;
        std::seed_seq seq{rd(), rd(), rd(), rd()};
        eng.seed(seq);
    } else {
        std::seed_seq seq{static_cast<std::uint32_t>(seed >> 32u),
                          static_cast<std::uint32_t>(seed & 0xffffffffu)};
        eng.seed(seq);
    }
}

std::mt19937& rng() {
    return thread_engine();
}

double randomUniform01() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(thread_engine());
}

double randomUniform(double min, double max) {
    if (min > max) {
        std::swap(min, max);
    }
    std::uniform_real_distribution<double> dist(min, max);
    return dist(thread_engine());
}

int randomInt(int min, int max) {
    if (min > max) {
        std::swap(min, max);
    }
    std::uniform_int_distribution<int> dist(min, max);
    return dist(thread_engine());
}

double randomNormal(double mean, double stddev) {
    if (stddev < 0.0) {
        throw std::invalid_argument("utilities::randomNormal: stddev must be non-negative");
    }
    std::normal_distribution<double> dist(mean, stddev);
    return dist(thread_engine());
}

double randomSkewNormal(double mean, double stddev, double shape_alpha) {
    if (stddev < 0.0) {
        throw std::invalid_argument("utilities::randomSkewNormal: stddev must be non-negative");
    }
    if (shape_alpha == 0.0) {
        return randomNormal(mean, stddev);
    }
    if (stddev == 0.0) {
        return mean;
    }

    std::mt19937& eng = thread_engine();
    std::normal_distribution<double> standard(0.0, 1.0);
    const double z1 = standard(eng);
    const double z2 = standard(eng);

    const double delta = shape_alpha / std::sqrt(1.0 + shape_alpha * shape_alpha);
    const double sqrt_one_minus_d2 = std::sqrt(std::max(0.0, 1.0 - delta * delta));
    const double w = delta * std::abs(z1) + sqrt_one_minus_d2 * z2;

    const double pi = std::acos(-1.0);
    const double ew = delta * std::sqrt(2.0 / pi);
    const double var_w = 1.0 - 2.0 * delta * delta / pi;
    const double std_w = std::sqrt(std::max(0.0, var_w));
    const double z = (w - ew) / std_w;

    return mean + stddev * z;
}

int randomBinomial(int n, double p) {
    if (n < 0) {
        throw std::invalid_argument("utilities::randomBinomial: n must be non-negative");
    }
    const double pc = std::clamp(p, 0.0, 1.0);
    std::binomial_distribution<int> dist(n, pc);
    return dist(thread_engine());
}

unsigned randomPoisson(double mean) {
    if (mean < 0.0) {
        throw std::invalid_argument("utilities::randomPoisson: mean must be non-negative");
    }
    std::poisson_distribution<unsigned> dist(mean);
    return dist(thread_engine());
}

} // namespace utilities
