# """Generation process for autotroph / heterotroph species (typed profile orders)."""

# from __future__ import annotations

# import random
# from dataclasses import dataclass, field
# from typing import Any, Dict, List, Mapping, Optional, Sequence, Union

# from ..orders import DefaultOrder, GenerationContext, Order, apply_field_defaults
# from ..orders.base import Order as OrderBase
# from ..orders.composite import EnergyFactor, SizeClassDict, SizeClasses
# from ..orders.death_biomass import DeathBiomassOrder
# from ..orders.scalar import Gaussian
# from ..orders.stratum import StratumConfigOrder
# from ..orders.vector import LiteralVector
# from .livingbeing import AutotrophSpecies, HeterotrophSpecies
# from .species_defaults import AutotrophProfileDefaultCatalog, HeterotrophProfileDefaultCatalog


# def _resolve_profile_field(val: Any, ctx: GenerationContext) -> Any:
#     if val is None:
#         return None
#     if isinstance(val, (DeathBiomassOrder, StratumConfigOrder)):
#         return val.resolve(ctx)
#     if isinstance(val, DefaultOrder):
#         return val.resolve(ctx)
#     if isinstance(val, OrderBase):
#         return val.resolve(ctx)
#     return val


# def _resolve_size_for_gcfg(val: Any, ctx: GenerationContext) -> Any:
#     """Map :class:`SizeClasses` to heterotroph ``size`` dict shape."""
#     resolved = _resolve_profile_field(val, ctx)
#     if isinstance(val, SizeClasses) or (
#         isinstance(resolved, list) and not isinstance(resolved, dict)
#     ):
#         tiers = [int(x) for x in resolved]
#         return {"biomass_classes": tiers, "surface_classes": list(tiers)}
#     if isinstance(val, SizeClassDict):
#         return resolved
#     return resolved


# @dataclass
# class AutotrophProfileProcess:
#     """Stochastic profile as orders; resolves to ``generation_config`` for the legacy builder."""

#     biomass_energy: Union[Order[Any], Mapping[str, Any], float] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     death_biomass_energy: Union[Order[Any], Mapping[str, Any], float] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     maintenance_cost_range: Union[Order[List[float]], Sequence[float]] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     max_fertility_range: Union[Order[List[float]], Sequence[float]] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     size: Union[Order[List[int]], Sequence[int]] = field(default_factory=lambda: DefaultOrder())
#     death_biomass: Union[DeathBiomassOrder, None] = None
#     stratum_config: Union[StratumConfigOrder, None] = None
#     max_individual_growth_bonus_range: Union[Order[List[float]], Sequence[float], None] = None
#     _defaults_applied: bool = field(default=False, repr=False)

#     def apply_defaults(
#         self, catalog: type[AutotrophProfileDefaultCatalog] = AutotrophProfileDefaultCatalog
#     ) -> None:
#         fields: Dict[str, Order] = {}
#         for name, val in (
#             ("biomass_energy", self.biomass_energy),
#             ("death_biomass_energy", self.death_biomass_energy),
#             ("maintenance_cost_range", self.maintenance_cost_range),
#             ("max_fertility_range", self.max_fertility_range),
#             ("size", self.size),
#         ):
#             if isinstance(val, DefaultOrder):
#                 fields[name] = val
#         apply_field_defaults(fields, catalog.field_catalogs())

#         if isinstance(self.death_biomass, DeathBiomassOrder):
#             self.death_biomass.apply_subdefaults()
#         elif isinstance(self.death_biomass, DefaultOrder):
#             self.death_biomass.set_defaults(catalog.death_biomass)

#         if isinstance(self.stratum_config, StratumConfigOrder):
#             self.stratum_config.apply_subdefaults()
#         elif isinstance(self.stratum_config, DefaultOrder):
#             self.stratum_config.set_defaults(catalog.stratum_config)

#         self._defaults_applied = True

#     def to_generation_config(self, ctx: GenerationContext) -> Dict[str, Any]:
#         if not self._defaults_applied:
#             self.apply_defaults()
#         out: Dict[str, Any] = {}
#         for key, val in (
#             ("biomass_energy", self.biomass_energy),
#             ("death_biomass_energy", self.death_biomass_energy),
#             ("maintenance_cost_range", self.maintenance_cost_range),
#             ("max_fertility_range", self.max_fertility_range),
#             ("size", self.size),
#             ("death_biomass", self.death_biomass),
#             ("stratum_config", self.stratum_config),
#             ("max_individual_growth_bonus_range", self.max_individual_growth_bonus_range),
#         ):
#             if val is None:
#                 continue
#             resolved = _resolve_profile_field(val, ctx)
#             if resolved is not None and resolved != {}:
#                 out[key] = resolved
#         return out


# @dataclass
# class HeterotrophProfileProcess:
#     """Heterotroph stochastic profile (orders → ``generation_config``)."""

#     biomass_energy: Union[Order[Any], Mapping[str, Any], float] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     death_biomass_energy: Union[Order[Any], Mapping[str, Any], float] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     maintenance_cost_range: Union[Order[List[float]], Sequence[float]] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     max_fertility_range: Union[Order[List[float]], Sequence[float]] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     size: Union[SizeClasses, SizeClassDict, Order, None] = field(default_factory=lambda: DefaultOrder())
#     death_biomass: Union[DeathBiomassOrder, None] = None
#     max_individual_growth_bonus_range: Union[Order[List[float]], Sequence[float], None] = None
#     prospecting_range: Union[Order[List[float]], Sequence[float], None] = None
#     assimilation_efficiency_range: Union[Order[List[float]], Sequence[float], None] = None
#     ingestion_residue: Union[DeathBiomassOrder, Dict[str, Any], None] = None
#     prey_location: Union[Gaussian, Order, Dict[str, float], None] = None
#     diet_by_food_type: Any = None
#     _defaults_applied: bool = field(default=False, repr=False)

#     def apply_defaults(
#         self, catalog: type[HeterotrophProfileDefaultCatalog] = HeterotrophProfileDefaultCatalog
#     ) -> None:
#         fields: Dict[str, Order] = {}
#         for name, val in (
#             ("biomass_energy", self.biomass_energy),
#             ("death_biomass_energy", self.death_biomass_energy),
#             ("maintenance_cost_range", self.maintenance_cost_range),
#             ("max_fertility_range", self.max_fertility_range),
#             ("size", self.size if isinstance(self.size, DefaultOrder) else None),
#         ):
#             if val is not None and isinstance(val, DefaultOrder):
#                 fields[name] = val
#         apply_field_defaults(fields, catalog.field_catalogs())

#         if isinstance(self.death_biomass, DeathBiomassOrder):
#             self.death_biomass.apply_subdefaults()
#         if isinstance(self.ingestion_residue, DeathBiomassOrder):
#             self.ingestion_residue.apply_subdefaults()

#         self._defaults_applied = True

#     def to_generation_config(self, ctx: GenerationContext) -> Dict[str, Any]:
#         if not self._defaults_applied:
#             self.apply_defaults()
#         out: Dict[str, Any] = {}
#         for key, val in (
#             ("biomass_energy", self.biomass_energy),
#             ("death_biomass_energy", self.death_biomass_energy),
#             ("maintenance_cost_range", self.maintenance_cost_range),
#             ("max_fertility_range", self.max_fertility_range),
#             ("death_biomass", self.death_biomass),
#             ("max_individual_growth_bonus_range", self.max_individual_growth_bonus_range),
#             ("prospecting_range", self.prospecting_range),
#             ("assimilation_efficiency_range", self.assimilation_efficiency_range),
#             ("ingestion_residue", self.ingestion_residue),
#             ("prey_location", self.prey_location),
#             ("diet_by_food_type", self.diet_by_food_type),
#         ):
#             if val is None:
#                 continue
#             if key == "prey_location" and isinstance(val, Gaussian):
#                 resolved = {"mean": val.mean, "std": val.std}
#             elif key == "ingestion_residue" and isinstance(val, DeathBiomassOrder):
#                 resolved = val.resolve(ctx)
#             else:
#                 resolved = _resolve_profile_field(val, ctx)
#             if resolved is not None and resolved != {}:
#                 out[key] = resolved

#         if self.size is not None:
#             size_out = _resolve_size_for_gcfg(self.size, ctx)
#             if size_out:
#                 out["size"] = size_out

#         return out


# @dataclass
# class AutotrophSpeciesProcess:
#     name: str
#     cycles_per_stages: Sequence[int]
#     food_type: str = "0"
#     profile: AutotrophProfileProcess = field(default_factory=AutotrophProfileProcess)
#     stochastic: bool = True


# def generate_autotroph_from_process(
#     process: AutotrophSpeciesProcess,
#     *,
#     rng: Optional[random.Random] = None,
#     n_bins: int = 3,
# ) -> AutotrophSpecies:
#     r = rng if rng is not None else random.Random()
#     ctx = GenerationContext(
#         rng=r,
#         stochastic=process.stochastic,
#         n_stages=len(process.cycles_per_stages),
#         n_bins=n_bins,
#     )
#     gcfg = process.profile.to_generation_config(ctx)
#     return AutotrophSpecies.generate(
#         process.name,
#         list(process.cycles_per_stages),
#         process.food_type,
#         stochastic=process.stochastic or bool(gcfg),
#         generation_config=gcfg if gcfg else None,
#         rng=r,
#     )


# @dataclass
# class HeterotrophSpeciesProcess:
#     name: str
#     cycles_per_stages: Sequence[int]
#     food_type: str = "1"
#     profile: HeterotrophProfileProcess = field(default_factory=HeterotrophProfileProcess)
#     stochastic: bool = True


# def generate_heterotroph_from_process(
#     process: HeterotrophSpeciesProcess,
#     *,
#     rng: Optional[random.Random] = None,
#     n_bins: int = 3,
# ) -> HeterotrophSpecies:
#     r = rng if rng is not None else random.Random()
#     ctx = GenerationContext(
#         rng=r,
#         stochastic=process.stochastic,
#         n_stages=len(process.cycles_per_stages),
#         n_bins=n_bins,
#     )
#     gcfg = process.profile.to_generation_config(ctx)
#     return HeterotrophSpecies.generate(
#         process.name,
#         list(process.cycles_per_stages),
#         process.food_type,
#         stochastic=process.stochastic or bool(gcfg),
#         generation_config=gcfg if gcfg else None,
#         rng=r,
#     )
