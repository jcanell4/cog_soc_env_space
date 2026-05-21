# """Default order catalogs for :class:`NicheGenProcess` fields."""

# from __future__ import annotations

# from typing import Dict, List

# from ..orders import (
#     FieldDefaultCatalog,
#     Gaussian,
#     Literal,
#     NutrientsFromTier,
#     ProspectingSharpness,
#     ProportionalDecay,
#     UniformRange,
#     VectorUniformRange,
# )
# from ..orders.base import Order

# _DEFAULT_NUTRIENTS_TIER = 2
# _DEFAULT_RETURN_RATE_RANGE = (0.0, 1.0)
# _DEFAULT_CONDITIONS_RANGE = (0.4, 0.9)
# _DEFAULT_LIMITING_FACTORS_RANGE = (0.02, 0.35)


# class NicheGenDefaultCatalog:
#     """Named default variants per :class:`NicheGenProcess` field."""

#     nutrients = FieldDefaultCatalog(
#         {
#             "interval_tier": NutrientsFromTier(tier=_DEFAULT_NUTRIENTS_TIER),
#             "tier_3": NutrientsFromTier(tier=3),
#             "gaussian_cap": Gaussian(mean=1500.0, std=200.0),
#             "literal": Literal(1000.0),
#             "uniform_range": UniformRange(0.0, 2000.0),
#         },
#         active="interval_tier",
#     )

#     return_rate = FieldDefaultCatalog(
#         {
#             "proportional_decay": ProportionalDecay(r0_range=_DEFAULT_RETURN_RATE_RANGE),
#             "fixed_r0": ProportionalDecay(r0=0.7),
#         },
#         active="proportional_decay",
#     )

#     conditions = FieldDefaultCatalog(
#         {
#             "uniform_mid": VectorUniformRange(
#                 _DEFAULT_CONDITIONS_RANGE[0],
#                 _DEFAULT_CONDITIONS_RANGE[1],
#             ),
#         },
#         active="uniform_mid",
#     )

#     limiting_factors = FieldDefaultCatalog(
#         {
#             "uniform_mid": VectorUniformRange(
#                 _DEFAULT_LIMITING_FACTORS_RANGE[0],
#                 _DEFAULT_LIMITING_FACTORS_RANGE[1],
#             ),
#         },
#         active="uniform_mid",
#     )

#     prospecting_scan_sharpness = FieldDefaultCatalog(
#         {
#             "gaussian": ProspectingSharpness(mean=0.85, std=0.3),
#             "literal": Literal(1.0),
#         },
#         active="gaussian",
#     )

#     n_factors = FieldDefaultCatalog(
#         {"four": Literal(4.0)},
#         active="four",
#     )

#     @classmethod
#     def field_catalogs(cls) -> Dict[str, FieldDefaultCatalog]:
#         return {
#             "nutrients": cls.nutrients,
#             "return_rate": cls.return_rate,
#             "conditions": cls.conditions,
#             "limiting_factors": cls.limiting_factors,
#             "prospecting_scan_sharpness": cls.prospecting_scan_sharpness,
#             "n_factors": cls.n_factors,
#         }
