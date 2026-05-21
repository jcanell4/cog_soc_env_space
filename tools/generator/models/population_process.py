# """Generation process for :class:`PopulationGen`."""

# from __future__ import annotations

# import random
# from dataclasses import dataclass, field
# from typing import List, Optional, Sequence, Union

# from ..orders import (
#     DefaultOrder,
#     ExpandScalarToStages,
#     GenerationContext,
#     LiteralVector,
#     Order,
# )
# from ..orders.base import Order as OrderBase
# from .population import PopulationGen, _MAX_JSON_SAFE_INT
# from .population_defaults import PopulationGenDefaultCatalog
# from .livingbeing import AutotrophSpecies, HeterotrophSpecies


# def _coerce_vector_order(
#     value: Union[Order[List[float]], float, Sequence[float], None],
#     *,
#     default: Order[List[float]],
# ) -> Order[List[float]]:
#     if value is None:
#         return default
#     if isinstance(value, OrderBase):
#         return value
#     if isinstance(value, (int, float)):
#         return ExpandScalarToStages(float(value))
#     return LiteralVector(value)


# @dataclass
# class PopulationGenProcess:
#     specie: Union[AutotrophSpecies, HeterotrophSpecies]
#     biomass: Union[Order[List[float]], float, Sequence[float]] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     death_biomass: Union[Order[List[float]], float, Sequence[float]] = field(
#         default_factory=lambda: DefaultOrder()
#     )
#     id: Union[Order[int], int, None] = None
#     _defaults_applied: bool = field(default=False, repr=False)

#     def apply_defaults(self, catalog: type[PopulationGenDefaultCatalog] = PopulationGenDefaultCatalog) -> None:
#         b = _coerce_vector_order(self.biomass, default=DefaultOrder())
#         d = _coerce_vector_order(self.death_biomass, default=DefaultOrder())
#         if isinstance(b, DefaultOrder):
#             b.set_defaults(catalog.biomass)
#         if isinstance(d, DefaultOrder):
#             d.set_defaults(catalog.death_biomass)
#         self.biomass = b
#         self.death_biomass = d
#         self._defaults_applied = True


# def generate_population_from_process(
#     process: PopulationGenProcess,
#     *,
#     rng: Optional[random.Random] = None,
# ) -> PopulationGen:
#     if not process._defaults_applied:
#         process.apply_defaults()

#     r = rng if rng is not None else random.Random()
#     n_stages = len(process.specie.cycles_per_stages)
#     ctx = GenerationContext(rng=r, n_stages=n_stages)

#     if isinstance(process.biomass, OrderBase):
#         biomass = process.biomass.resolve(ctx)
#     else:
#         biomass = (
#             ExpandScalarToStages(float(process.biomass)).resolve(ctx)
#             if isinstance(process.biomass, (int, float))
#             else [float(x) for x in process.biomass]
#         )
#     if isinstance(process.death_biomass, OrderBase):
#         death_biomass = process.death_biomass.resolve(ctx)
#     else:
#         death_biomass = (
#             ExpandScalarToStages(float(process.death_biomass)).resolve(ctx)
#             if isinstance(process.death_biomass, (int, float))
#             else [float(x) for x in process.death_biomass]
#         )

#     if isinstance(process.id, OrderBase):
#         cid = process.id.resolve(ctx)
#     elif process.id is not None:
#         cid = int(process.id)
#     else:
#         cid = r.randint(1, _MAX_JSON_SAFE_INT)

#     return PopulationGen(
#         id=cid,
#         biomass=[float(x) for x in biomass],
#         death_biomass=[float(x) for x in death_biomass],
#         specie=process.specie,
#     )
