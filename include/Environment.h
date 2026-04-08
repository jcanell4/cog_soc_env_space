#pragma once

/**
 * @file Environment.h
 * @brief Minimal environment graph container for the restart.
 */

#include "Niche.h"

#include <cstddef>
#include <vector>

struct NicheEdge {
    std::size_t to_niche_index{};
};

class Environment {
public:
    using NicheContainer = std::vector<Niche>;
    using AdjacencyList = std::vector<std::vector<NicheEdge>>;

    Environment() = default;

    const NicheContainer& getNiches() const;
    NicheContainer& getNiches();

    const AdjacencyList& getAdjacency() const;
    AdjacencyList& getAdjacency();

    Environment& setNiches(NicheContainer niches);
    Environment& setAdjacency(AdjacencyList adjacency);

    const std::vector<NicheEdge>& getOutgoingEdges(std::size_t niche_index) const;
    std::vector<NicheEdge>& getOutgoingEdges(std::size_t niche_index);

private:
    void normalizeAdjacencySize();

    NicheContainer niches_;
    AdjacencyList adjacency_;
};
