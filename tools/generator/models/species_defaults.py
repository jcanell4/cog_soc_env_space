# """Default catalogs for autotroph / heterotroph profile fields."""

# from __future__ import annotations

# from typing import Dict

# from ..orders import FieldDefaultCatalog
# from ..orders.composite import EnergyFactor, SizeClassDict, SizeClasses
# from ..orders.death_biomass import DeathBiomassOrder
# from ..orders.stratum import StratumConfigOrder
# from ..orders.vector import LiteralVector


# class AutotrophProfileDefaultCatalog:
#     biomass_energy = FieldDefaultCatalog(
#         {"prototype_std": EnergyFactor(std=40.0), "tight": EnergyFactor(std=0.1)},
#         active="prototype_std",
#     )
#     death_biomass_energy = FieldDefaultCatalog(
#         {"default": EnergyFactor(std=10.0), "tight": EnergyFactor(std=0.2)},
#         active="default",
#     )
#     maintenance_cost_range = FieldDefaultCatalog(
#         {"default": LiteralVector([0.05, 0.5])},
#         active="default",
#     )
#     max_fertility_range = FieldDefaultCatalog(
#         {"default": LiteralVector([0.3, 0.9])},
#         active="default",
#     )
#     size = FieldDefaultCatalog(
#         {"three_stage": SizeClasses([0, 0, 1])},
#         active="three_stage",
#     )
#     death_biomass = FieldDefaultCatalog(
#         {"standard": DeathBiomassOrder()},
#         active="standard",
#     )
#     stratum_config = FieldDefaultCatalog(
#         {"standard": StratumConfigOrder()},
#         active="standard",
#     )

#     @classmethod
#     def field_catalogs(cls) -> Dict[str, FieldDefaultCatalog]:
#         return {
#             "biomass_energy": cls.biomass_energy,
#             "death_biomass_energy": cls.death_biomass_energy,
#             "maintenance_cost_range": cls.maintenance_cost_range,
#             "max_fertility_range": cls.max_fertility_range,
#             "size": cls.size,
#             "death_biomass": cls.death_biomass,
#             "stratum_config": cls.stratum_config,
#         }


# class HeterotrophProfileDefaultCatalog:
#     biomass_energy = FieldDefaultCatalog(
#         {"default": EnergyFactor(mean=250.0, std=40.0)},
#         active="default",
#     )
#     death_biomass_energy = FieldDefaultCatalog(
#         {"default": EnergyFactor(std=10.0)},
#         active="default",
#     )
#     maintenance_cost_range = FieldDefaultCatalog(
#         {"default": LiteralVector([0.05, 0.5])},
#         active="default",
#     )
#     max_fertility_range = FieldDefaultCatalog(
#         {"default": LiteralVector([0.3, 0.9])},
#         active="default",
#     )
#     size = FieldDefaultCatalog(
#         {"three_stage": SizeClassDict([0, 0, 1])},
#         active="three_stage",
#     )

#     @classmethod
#     def field_catalogs(cls) -> Dict[str, FieldDefaultCatalog]:
#         return {
#             "biomass_energy": cls.biomass_energy,
#             "death_biomass_energy": cls.death_biomass_energy,
#             "maintenance_cost_range": cls.maintenance_cost_range,
#             "max_fertility_range": cls.max_fertility_range,
#             "size": cls.size,
#         }
