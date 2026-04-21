#pragma once

/**
 * @file Utilities.h
 * @brief Reusable helpers: thread-local RNG and common random distributions.
 */

#include <cstdint>

#include <random>

namespace utilities {

/**
 * @brief Seed the thread-local RNG.
 *
 * If @p seed is 0, entropy is taken from @c std::random_device (non-deterministic).
 * Otherwise the 64-bit value is folded into a @c std::seed_seq for @c std::mt19937.
 */
void seedRng(std::uint64_t seed);

/** @brief Thread-local Mersenne Twister; use with custom @c std::*_distribution. */
std::mt19937& rng();

/** @brief Continuous uniform on [0,1). */
double randomUniform01();

/**
 * @brief Continuous uniform on [min,max). If min > max, arguments are swapped.
 */
double randomUniform(double min, double max);

/**
 * @brief Discrete uniform on [min,max] (both inclusive). If min > max, arguments are swapped.
 */
int randomInt(int min, int max);

/** @brief Normal (Gaussian) with given mean and standard deviation. @p stddev must be >= 0. */
double randomNormal(double mean, double stddev);

/**
 * @brief Skew-normal (Azzalini): asymmetric extension of the Gaussian.
 *
 * Uses @p shape_alpha as skewness shape (0 gives @ref randomNormal). Positive values typically
 * produce right-skewed draws. The returned values have population mean @p mean and standard
 * deviation @p stddev (after centering/scaling of the standard construction).
 */
double randomSkewNormal(double mean, double stddev, double shape_alpha);

/**
 * @brief Sample from Binomial(n,p). @p n >= 0; @p p is clamped to [0,1].
 */
int randomBinomial(int n, double p);

/**
 * @brief Sample from Poisson(mean). @p mean must be >= 0.
 */
unsigned randomPoisson(double mean);

} // namespace utilities
