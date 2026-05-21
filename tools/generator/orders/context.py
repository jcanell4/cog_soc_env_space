"""Runtime context passed to every :class:`Order.resolve`."""

from __future__ import annotations

import random
from copy import deepcopy
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional

_DEFAULT_INDIVIDUAL_SIZE_BY_CLASS_RANGE: List[List[float]] = [
    [1e-3, 0.1],
    [0.1, 3],
    [3, 30],
    [30, 250],
    [250, 5000],
]
_DEFAULT_INDIVIDUAL_AREA_BY_CLASS_RANGE: List[List[float]] = [
    [1e-3, 0.05],
    [0.05, 0.3],
    [0.3, 1],
    [1, 5],
    [5, 40],
]
_DEFAULT_INDIVIDUAL_DENSITY_BY_CLASS_RANGE: List[List[List[float]]] = [
    [[50, 600], [20, 200], [10, 150], [6, 110], [5, 100]],
    [[50, 100], [40, 90], [30, 80], [20, 65], [10, 50]],
]


def _default_individual_size_by_class_range() -> List[List[float]]:
    return deepcopy(_DEFAULT_INDIVIDUAL_SIZE_BY_CLASS_RANGE)


def _default_individual_area_by_class_range() -> List[List[float]]:
    return deepcopy(_DEFAULT_INDIVIDUAL_AREA_BY_CLASS_RANGE)


def _default_individual_density_by_class_range() -> List[List[List[float]]]:
    return deepcopy(_DEFAULT_INDIVIDUAL_DENSITY_BY_CLASS_RANGE)


@dataclass
class GenerationContext:
    rng: random.Random
    # @deprecated("It will be removed in the future")
    stochastic: bool = False
    surface: float = 10000.0
    n_bins: int = 3
    # @deprecated("It will be removed in the future. Use n_conditions or n_limiting instead")
    n_factors: int = 4
    n_conditions: int = 4
    n_limiting_factors: int = 4
    n_stages: int = 3
    n_stratums: int = 3
    fertility_from_stage: int = 1
    active_default_keys: Dict[str, str] = field(default_factory=dict)
    resolved: Dict[str, Any] = field(default_factory=dict)
    volatile_context: Dict[str, Any] = field(default_factory=dict)
    individual_size_by_class_range: List[List[float]] = field(
        default_factory=_default_individual_size_by_class_range
    )
    individual_area_by_class_range: List[List[float]] = field(
        default_factory=_default_individual_area_by_class_range
    )
    individual_density_by_class_range: List[List[List[float]]] = field(
        default_factory=_default_individual_density_by_class_range
    )
    adaptation_noise: Optional[float] = None
    surface_name: str = "surface"
    n_bins_name: str = "n_bins"
    n_factors_name: str = "n_factors"
    n_conditions_name: str = "n_conditions"
    n_limiting_factors_name: str = "n_limiting_factors"
    n_stages_name: str = "n_stages"
    n_stratums_name: str = "n_stratums"
    fertility_from_stage_name: str = "fertility_from_stage"
    individual_size_by_class_range_name: str = "individual_size_by_class_range"
    individual_area_by_class_range_name: str = "individual_area_by_class_range"
    individual_density_by_class_range_name: str = "individual_density_by_class_range"
    adaptation_noise_name: str = "adaptation_noise"

    def with_resolved(self, key: str, value: Any) -> GenerationContext:
        self.resolved[key] = value
        return self

    def get_value(self, key: str, default: Any = None) -> Any:
        return self.volatile_context.get(key, self.resolved.get(key, getattr(self, key, default)))
