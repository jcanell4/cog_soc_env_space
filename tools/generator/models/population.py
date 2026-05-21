"""
Pydantic generator for niche ``populations[]`` entries (see ``include/Population.h`` / niche JSON).
"""

from __future__ import annotations

import random
from typing import TYPE_CHECKING, Any, List, Optional, Sequence, Union
from dataclasses import dataclass, replace
from tools.generator.orders.base import Generator
from tools.generator.orders.context import GenerationContext

from .livingbeing import Autotroph, Heterotroph, LivingBeingGenerator, AutotrophGenerator, HeterotrophGenerator


_MAX_JSON_SAFE_INT = 9_007_199_254_740_990

@dataclass(frozen=True)
class Population:
    id: int
    biomass: List[float]
    death_biomass: List[float]
    specie: Union[Autotroph, Heterotroph]

@dataclass(frozen=True)
class PopulationGenerator(Generator[Population]):
    name: Union[Generator[str], str]
    biomass: Union[Generator[List[float]], List[float]]
    death_biomass: Union[Generator[List[float]], List[float]]
    specie: Union[Generator[Union[Autotroph, Heterotroph]], Autotroph, Heterotroph]
    def generate(self, ctx: GenerationContext, id: Union[int, Generator[int], None]=None) -> Population:
        if id is None:
            id = ctx.rng.randint(1, _MAX_JSON_SAFE_INT)
        elif isinstance(id, Generator):
            id = id.generate(ctx)
        specie = self.specie.generate(ctx) if isinstance(self.specie, Generator) else self.specie
        specie = replace(
            specie,
            name=self.name.generate(ctx) if isinstance(self.name, Generator) else self.name,
            food_type=f"{specie.food_type}.{id}",
        )
        return Population(
            id=id, 
            biomass=self.biomass.generate(ctx) if isinstance(self.biomass, Generator) else self.biomass, 
            death_biomass=self.death_biomass.generate(ctx) if isinstance(self.death_biomass, Generator) else self.death_biomass, 
            specie=specie
        )


# if TYPE_CHECKING:
#     from .population_process import PopulationGenProcess

# from pydantic import BaseModel, ConfigDict, Field, model_validator

# from .livingbeing import AutotrophSpecies, HeterotrophSpecies

# # JSON Number safe integer upper bound (IEEE-754 double integer precision).
# _MAX_JSON_SAFE_INT = 9_007_199_254_740_990


# class PopulationGen(BaseModel):
#     """One population snapshot: ``id``, ``biomass``, ``death_biomass``, nested ``specie``."""

#     model_config = ConfigDict(extra="forbid", populate_by_name=True)

#     class GenerateParam:
#         """String keys matching :meth:`generate` parameters when building call kwargs or JSON helpers."""

#         BIOMASS = "biomass"
#         DEATH_BIOMASS = "death_biomass"
#         SPECIE = "specie"
#         RNG = "rng"
#         ID = "id"

#     class FieldKey:
#         """JSON keys for a population object."""

#         ID = "id"
#         BIOMASS = "biomass"
#         DEATH_BIOMASS = "death_biomass"
#         SPECIE = "specie"

#     id: int = Field(..., ge=1, description="Population identifier (JSON number; unique in practice).")
#     biomass: List[float] = Field(..., description="Living biomass per life-history stage.")
#     death_biomass: List[float] = Field(..., description="Death biomass per stage.")
#     specie: Union[AutotrophSpecies, HeterotrophSpecies] = Field(
#         ...,
#         description="Embedded species spec from :class:`AutotrophSpecies` or :class:`HeterotrophSpecies`.",
#     )

#     @model_validator(mode="after")
#     def _vectors_match_specie_stages(self) -> PopulationGen:
#         n = len(self.specie.cycles_per_stages)
#         if len(self.biomass) != n:
#             raise ValueError(f"biomass length {len(self.biomass)} != number of stages {n} from specie.")
#         if len(self.death_biomass) != n:
#             raise ValueError(
#                 f"death_biomass length {len(self.death_biomass)} != number of stages {n} from specie."
#             )
#         return self

#     def to_population_dict(self) -> dict[str, Any]:
#         """Serialize for ``initial_data.data.populations`` (JSON-compatible primitives)."""
#         return {
#             self.FieldKey.ID: self.id,
#             self.FieldKey.BIOMASS: [float(x) for x in self.biomass],
#             self.FieldKey.DEATH_BIOMASS: [float(x) for x in self.death_biomass],
#             self.FieldKey.SPECIE: self.specie.to_specie_dict(),
#         }

#     @staticmethod
#     def _random_id(rng: random.Random) -> int:
#         return rng.randint(1, _MAX_JSON_SAFE_INT)

#     @classmethod
#     def generate_from(
#         cls,
#         process: "PopulationGenProcess",
#         *,
#         rng: Optional[random.Random] = None,
#     ) -> PopulationGen:
#         from .population_process import generate_population_from_process

#         return generate_population_from_process(process, rng=rng)

#     @classmethod
#     def generate(
#         cls,
#         biomass: Sequence[float],
#         death_biomass: Sequence[float],
#         *,
#         specie: Union[AutotrophSpecies, HeterotrophSpecies],
#         rng: Optional[random.Random] = None,
#         id: Optional[int] = None,
#     ) -> PopulationGen:
#         """
#         Build a population from **literal** biomass vectors and a species object produced by
#         :meth:`AutotrophSpecies.generate` or :meth:`HeterotrophSpecies.generate` (or any
#         constructed instance of those types).

#         ``biomass`` and ``death_biomass`` must have the same length as
#         ``len(specie.cycles_per_stages)``.

#         ``id``: if omitted, a random integer in ``[1, 2**53-2]`` is drawn (JSON-safe integer
#         range). Pass ``id`` explicitly when you need a stable or coordinated identifier.

#         Parameter names are also available on :class:`PopulationGen.GenerateParam` for config builders.
#         """
#         r = rng if rng is not None else random.Random()
#         cid = int(id) if id is not None else cls._random_id(r)
#         if cid < 1:
#             raise ValueError("id must be >= 1.")
#         if cid > _MAX_JSON_SAFE_INT:
#             raise ValueError(f"id must be <= {_MAX_JSON_SAFE_INT} for JSON numeric safety.")
#         return cls(
#             id=cid,
#             biomass=[float(x) for x in biomass],
#             death_biomass=[float(x) for x in death_biomass],
#             specie=specie,
#         )
