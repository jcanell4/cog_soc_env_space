#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace diet_food_type_match {

/** True if @a prey_taxonomy equals @a diet_prefix or continues it with a dot (e.g. "0.1.1" ⊆ "0.1.1.2.5.4"). */
inline bool foodTypeMatchesHierarchyPrefix(const std::string& prey_taxonomy, const std::string& diet_prefix) {
    if (diet_prefix.empty()) {
        return false;
    }
    if (prey_taxonomy == diet_prefix) {
        return true;
    }
    if (prey_taxonomy.size() <= diet_prefix.size()) {
        return false;
    }
    if (prey_taxonomy.compare(0, diet_prefix.size(), diet_prefix) != 0) {
        return false;
    }
    return prey_taxonomy[diet_prefix.size()] == '.';
}

/** @param rules (hierarchy_prefix, min_prey_stage, max_prey_stage inclusive). */
inline bool isFoodTypeMyDiet(const std::vector<std::tuple<std::string, int, int>>& rules,
                             const std::string& prey_food_type,
                             int prey_stage) {
    for (const auto& rule : rules) {
        const std::string& prefix = std::get<0>(rule);
        const int min_stage = std::get<1>(rule);
        const int max_stage = std::get<2>(rule);
        if (min_stage > max_stage) {
            continue;
        }
        if (!foodTypeMatchesHierarchyPrefix(prey_food_type, prefix)) {
            continue;
        }
        if (prey_stage >= min_stage && prey_stage <= max_stage) {
            return true;
        }
    }
    return false;
}

/**
 * @return (min_prey_stage, max_prey_stage) for the first @a rules entry whose hierarchy matches
 *         @a prey_food_type; @c (-1, -1) if none match or all matching rows have invalid stage range.
 */
inline std::tuple<int, int> rangeForMatchingFoodType(const std::vector<std::tuple<std::string, int, int>>& rules,
                                                     const std::string& prey_food_type) {
    for (const auto& rule : rules) {
        const std::string& prefix = std::get<0>(rule);
        const int min_stage = std::get<1>(rule);
        const int max_stage = std::get<2>(rule);
        if (min_stage > max_stage) {
            continue;
        }
        if (foodTypeMatchesHierarchyPrefix(prey_food_type, prefix)) {
            return {min_stage, max_stage};
        }
    }
    return {-1, -1};
}

}  // namespace diet_food_type_match
