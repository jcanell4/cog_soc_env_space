# """Default order catalogs for :class:`PopulationGenProcess`."""

# from __future__ import annotations

# from typing import Dict, List

# from ..orders import ExpandScalarToStages, FieldDefaultCatalog, Literal, LiteralVector
# from ..orders.base import Order


# class PopulationGenDefaultCatalog:
#     biomass = FieldDefaultCatalog(
#         {
#             "zeros": LiteralVector([0.0]),
#             "literal_10k": ExpandScalarToStages(10000.0),
#         },
#         active="literal_10k",
#     )

#     death_biomass = FieldDefaultCatalog(
#         {
#             "zeros": LiteralVector([0.0]),
#         },
#         active="zeros",
#     )

#     @classmethod
#     def field_catalogs(cls) -> Dict[str, FieldDefaultCatalog]:
#         return {
#             "biomass": cls.biomass,
#             "death_biomass": cls.death_biomass,
#         }
