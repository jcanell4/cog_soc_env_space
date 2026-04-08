#include "Environment.h"
#include <utility>

void Environment::normalizeAdjacencySize() {
    adjacency_.resize(niches_.size());
}

const Environment::NicheContainer& Environment::getNiches() const {
    return niches_;
}

Environment::NicheContainer& Environment::getNiches() {
    return niches_;
}

const Environment::AdjacencyList& Environment::getAdjacency() const {
    return adjacency_;
}

Environment::AdjacencyList& Environment::getAdjacency() {
    return adjacency_;
}

Environment& Environment::setNiches(NicheContainer niches) {
    niches_ = std::move(niches);
    normalizeAdjacencySize();
    return *this;
}

Environment& Environment::setAdjacency(AdjacencyList adjacency) {
    adjacency_ = std::move(adjacency);
    normalizeAdjacencySize();
    return *this;
}

const std::vector<NicheEdge>& Environment::getOutgoingEdges(std::size_t niche_index) const {
    return adjacency_.at(niche_index);
}

std::vector<NicheEdge>& Environment::getOutgoingEdges(std::size_t niche_index) {
    return adjacency_.at(niche_index);
}
