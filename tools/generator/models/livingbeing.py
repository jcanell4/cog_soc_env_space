"""
Living being ``specie`` objects for niche JSON (``populations[].specie``).

Mirrors ``applyLivingBeingCommonFields`` and class-specific fields in
``src/Builders.cpp::buildSpecieFromSnapshotJson``. Use ``model_dump(exclude_none=True)``
for simulator-compatible JSON.
"""

from __future__ import annotations

import json
import math
import random
from typing import TYPE_CHECKING, Any, ClassVar, Dict, List, Mapping, Optional, Sequence, TypedDict, Union, Tuple
from dataclasses import dataclass
from tools.generator.orders.base import NumberArroundValueGenerator, NumberFromUniformRangeGenerator, NumberGaussianGenerator, Generator, NumberGeneratorWithNoise, StringGenerator
from tools.generator.orders.context import GenerationContext
from tools.generator.orders.vector import (
    VectorCalculateFromOther,
    VectorGenerator, 
    VectorNumberGeneratorByClass, 
    VectorFromValueWithNoise, 
    VectorGaussianGenerator, 
    VectorIncrementalGenerator, 
    VectorUniformRangeGenerator, 
    VectorWithMaxSumValueGenerator,
    VectorResolvedFromIndexGenerator,
    VectorIntGeneratorByClass
)

#---------------------------------------- New version
@dataclass(frozen=True)
class LivingBeing:
    name: str
    food_type: str
    biomass_to_energy_conversion_factor: float
    death_biomass_to_energy_conversion_factor: float
    maintenance_cost: List[float]
    max_fertility: List[float]
    resilience: List[float]
    biomass_per_individual_amount: List[float]
    individual_occupied_surface: List[float]
    characteristics_death_biomass: List[List[float]]
    death_biomass_fraction_surface: List[float]
    death_biomass_per_fraction_amount: List[float]
    death_biomass_fraction_by_size: List[List[float]]
    best_environmental_conditions: List[List[float]]
    cycles_per_stages: List[int]
    defense_strategies: List[List[float]]
    recruitment_strategies: List[List[float]]
    max_individual_growth: List[float]
    max_density: List[float]
    colony_ability_rate: float
    diet_by_population_index: List[List[Union[Tuple[int, int, int, int], Tuple[str, int, int, int]]]]

@dataclass(frozen=True)
class Autotroph(LivingBeing):
    opacity: List[float]
    stratum: List[int]
    min_light: List[float]
    seed_dispersal_rate: float

@dataclass(frozen=True)
class Heterotroph(LivingBeing):
    prospecting_ability: List[float]
    assimilation_efficiency: List[float]
    ingestion_residue_fraction_by_size: List[List[float]]
    diet_by_food_type: List[List[Tuple[str, int, int, int]]]
    prey_location: List[float]

@dataclass
class LivingBeingGenerator(Generator[LivingBeing]):
    cycles_per_stages: Union[Generator[List[int]], List[int], None] = None
    n_stages: Optional[int] = None
    name: Union[Generator[str], str, None] = None
    food_type: Union[Generator[str], str] = "0.0"
    biomass_to_energy_conversion_factor: Optional[Generator[float]] = None
    death_biomass_to_energy_conversion_factor: Optional[Generator[float]] = None
    maintenance_cost: Optional[Generator[List[float]]] = None
    fertility_from_stage: Optional[Union[Generator[int], int]] = None
    max_fertility: Optional[Generator[List[float]]] = None
    resilience: Optional[Generator[List[float]]] = None
    size_class: Optional[Union[Generator[int], int]] = None
    biomass_per_individual_amount: Optional[Generator[List[float]]] = None
    individual_occupied_surface: Optional[Generator[List[float]]] = None
    characteristics_death_biomass: Optional[Generator[List[List[float]]]] = None
    death_biomass_fraction_surface: Optional[Generator[List[float]]] = None
    death_biomass_per_fraction_amount: Optional[Generator[List[float]]] = None
    death_biomass_fraction_by_size: Optional[Generator[List[List[float]]]] = None
    best_environmental_conditions: Optional[Generator[List[List[float]]]] = None
    defense_strategies: Optional[Generator[List[List[float]]]] = None
    recruitment_strategies: Optional[Generator[List[List[float]]]] = None
    max_individual_growth: Optional[Generator[List[float]]] = None
    max_density: Optional[Generator[List[float]]] = None
    colony_ability_rate: Optional[Generator[float]] = None
    diet_by_population_index: Optional[Union[Generator[List[List[Tuple[int, int, int, int]]]], List[List[Tuple[int, int, int, int]]]]] = None

    def __post_init__(self) -> None:
        if self.cycles_per_stages is None:
            if self.n_stages is None:
                raise ValueError("Either cycles_per_stages or n_stages is required")
            self.cycles_per_stages = VectorUniformRangeGenerator(min=1, max=3, integer=True, length=self.n_stages)

    def _resolve_defaults(self, ctx: GenerationContext) -> None:
        if isinstance(self.cycles_per_stages, Generator):
            self.cycles_per_stages = self.cycles_per_stages.generate(ctx)
        n = len(self.cycles_per_stages)
        ctx.n_stages = n 
        if self.size_class is not None:
            ctx.volatile_context["size_class"] = self.size_class.generate(ctx) if isinstance(self.size_class, Generator) else self.size_class

        if self.biomass_to_energy_conversion_factor is None:
            self.biomass_to_energy_conversion_factor = NumberArroundValueGenerator(value=190.0, ratio=0.4, clamp=(0.0, 500.0))
        if self.death_biomass_to_energy_conversion_factor is None:
            self.death_biomass_to_energy_conversion_factor = NumberArroundValueGenerator(value=17.5, ratio= 0.3, clamp=(0.0, 50.0))
        if self.maintenance_cost is None:
            self.maintenance_cost = VectorIncrementalGenerator(
                initial_value=NumberFromUniformRangeGenerator(min=0.05, max=0.3), 
                increment=NumberArroundValueGenerator(value=0.01, ratio=0.1, clamp=(0.0, 0.1)),
                length_from=ctx.n_stages_name
            )
        if self.fertility_from_stage is None:
            self.fertility_from_stage = 1
        else:
            if self.max_fertility is not None:
                self.max_fertility = [0] * self.fertility_from_stage
            else:
                for i in range(self.fertility_from_stage):
                    self.max_fertility[i] = 0.0
        if self.max_fertility is None:
            self.max_fertility = [0] * self.fertility_from_stage
        if len(self.max_fertility) < n:
            lf = len(self.max_fertility)
            self.max_fertility.extend([NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 1.0)).generate(ctx) for _ in range(n - lf)])
        if self.resilience is None:
            self.resilience = VectorGaussianGenerator(mean=0.5, std=ctx.adaptation_noise, clamp=(0.0, 1.0), length=n)
        if self.biomass_per_individual_amount is None:
            if self.size_class is not None:
                self.biomass_per_individual_amount = VectorNumberGeneratorByClass(class_range_name=ctx.individual_size_by_class_range_name, class_index=self.size_class, length=n)
            else:
                self.biomass_per_individual_amount = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=1, std=10), increment=NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 0.1)), length=n)
        if self.individual_occupied_surface is None:
            if self.size_class is not None:
                self.individual_occupied_surface = VectorNumberGeneratorByClass(class_range_name=ctx.individual_area_by_class_range_name, class_index=self.size_class, length=n)
            else:
                self.individual_occupied_surface = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=1, std=10), increment=NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 0.1)), length=n)
        if self.characteristics_death_biomass is None:
            self.characteristics_death_biomass = [
                VectorGaussianGenerator(mean=0.1, std=0.1, clamp=(0.0, 1.0), length=ctx.n_bins) 
                for _ in range(n)
            ]
        
        if self.death_biomass_fraction_surface is None:
            if self.size_class is not None:
                self.death_biomass_fraction_surface = VectorCalculateFromOther(field_name="individual_occupied_surface", lambda_function=lambda x: x/2, length=ctx.n_bins)
            else:
                self.death_biomass_fraction_surface = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=1, std=10), increment=NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 0.1)), length=n)
        if self.death_biomass_per_fraction_amount is None:
            if self.size_class is not None:
                self.death_biomass_per_fraction_amount = VectorCalculateFromOther(field_name="biomass_per_individual_amount", lambda_function=lambda x:  x/2, length=n)
            else:
                self.death_biomass_fraction_surface = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=1, std=10), increment=NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 0.1)), length=n)
            
        if self.death_biomass_fraction_by_size is None:
            self.death_biomass_fraction_by_size = [VectorWithMaxSumValueGenerator(max_sum_value=1, length=ctx.n_bins)] * n

        if self.best_environmental_conditions is None:
            self.best_environmental_conditions = [VectorUniformRangeGenerator(min=0.0, max=1.0, length=ctx.n_conditions)] * n

        if self.defense_strategies is None:
            self.defense_strategies = [VectorUniformRangeGenerator(min=0.0, max=1.0, length=ctx.n_limiting_factors)] * n
        
        if self.recruitment_strategies is None:
            if ctx.adaptation_noise is not None and ctx.adaptation_noise > 0.0:
                self.recruitment_strategies = [VectorCalculateFromOther(field_name="limiting_factors", lambda_function=lambda x: NumberGeneratorWithNoise(noise=ctx.adaptation_noise, value=x).generate(ctx), length=ctx.n_limiting_factors)] * n
            else:
                self.recruitment_strategies = [VectorUniformRangeGenerator(min=0.0, max=1.0, length=ctx.n_limiting_factors)] * n
        
        if self.max_individual_growth is None:
            self.max_individual_growth = VectorCalculateFromOther(field_name="max_fertility", lambda_function=lambda x: NumberArroundValueGenerator(value=0.99-x, ratio=NumberFromUniformRangeGenerator(min=0.05, max=0.5)).generate(ctx), length=len(self.cycles_per_stages))
        
        if self.max_density is None:
            if self.size_class is not None:
                self.max_density = VectorNumberGeneratorByClass(class_range_name=ctx.individual_density_by_class_range_name, class_index=self.size_class, length=len(self.cycles_per_stages))
            else:
                self.max_density = VectorUniformRangeGenerator(min=0.0, max=1.0, length=len(self.cycles_per_stages))

        if self.colony_ability_rate is None:
            self.colony_ability_rate = NumberFromUniformRangeGenerator(min=0.0, max=1.0)

    def _generate_to_context(self, ctx: GenerationContext) -> None:
        ctx.volatile_context[ctx.individual_density_by_class_range_name] = ctx.individual_density_by_class_range[0]
        ctx.volatile_context["biomass_to_energy_conversion_factor"]=self.biomass_to_energy_conversion_factor.generate(ctx) if isinstance(self.biomass_to_energy_conversion_factor, Generator) else self.biomass_to_energy_conversion_factor
        ctx.volatile_context["death_biomass_to_energy_conversion_factor"]=self.death_biomass_to_energy_conversion_factor.generate(ctx) if isinstance(self.death_biomass_to_energy_conversion_factor, Generator) else self.death_biomass_to_energy_conversion_factor
        ctx.volatile_context["maintenance_cost"]=self.maintenance_cost.generate(ctx) if isinstance(self.maintenance_cost, Generator) else self.maintenance_cost
        ctx.volatile_context["max_fertility"] = self.max_fertility.generate(ctx) if isinstance(self.max_fertility, Generator) else self.max_fertility
        ctx.volatile_context["resilience"] = self.resilience.generate(ctx) if isinstance(self.resilience, Generator) else self.resilience
        ctx.volatile_context["biomass_per_individual_amount"] = self.biomass_per_individual_amount.generate(ctx) if isinstance(self.individual_occupied_surface, Generator) else self.individual_occupied_surface
        ctx.volatile_context["individual_occupied_surface"] = self.individual_occupied_surface.generate(ctx) if isinstance(self.individual_occupied_surface, Generator) else self.individual_occupied_surface
        ctx.volatile_context["characteristics_death_biomass"] = [chb.generate(ctx) if isinstance(chb, Generator) else chb for chb in self.characteristics_death_biomass]
        ctx.volatile_context["death_biomass_fraction_surface"] = self.death_biomass_fraction_surface.generate(ctx) if isinstance(self.death_biomass_fraction_surface, Generator) else self.death_biomass_fraction_surface
        ctx.volatile_context["death_biomass_per_fraction_amount"] = self.death_biomass_per_fraction_amount.generate(ctx) if isinstance(self.death_biomass_per_fraction_amount, Generator) else self.death_biomass_per_fraction_amount
        ctx.volatile_context["death_biomass_fraction_by_size"] = [dbfs.generate(ctx) if isinstance(dbfs, Generator) else dbfs for dbfs in self.death_biomass_fraction_by_size]
        ctx.volatile_context["best_environmental_conditions"] = [bec.generate(ctx) if isinstance(bec, Generator) else bec for bec in self.best_environmental_conditions]
        ctx.volatile_context["cycles_per_stages"] = list(self.cycles_per_stages)
        ctx.volatile_context["defense_strategies"] = [ds.generate(ctx) if isinstance(ds, Generator) else ds for ds in self.defense_strategies]
        ctx.volatile_context["recruitment_strategies"] = [rs.generate(ctx) if isinstance(rs, Generator) else rs for rs in self.recruitment_strategies]
        ctx.volatile_context["max_individual_growth"] = self.max_individual_growth.generate(ctx) if isinstance(self.max_individual_growth, Generator) else self.max_individual_growth
        ctx.volatile_context["max_density"] = self.max_density.generate(ctx) if isinstance(self.max_density, Generator) else self.max_density
        ctx.volatile_context["colony_ability_rate"] = self.colony_ability_rate.generate(ctx) if isinstance(self.colony_ability_rate, Generator) else self.colony_ability_rate
        

@dataclass
class AutotrophGenerator(LivingBeingGenerator):
    opacity: Optional[Generator[List[float]]] = None
    stratum: Optional[Generator[List[int]]] = None
    min_light: Optional[Generator[List[float]]] = None
    seed_dispersal_rate: Optional[Generator[float]] = None

    def generate(self, ctx: GenerationContext, *, 
                    name: Union[Generator[str], str, None]=None, 
                    food_type: Union[Generator[str], str, None]=None
                ) -> Autotroph:
        old_n_stages = ctx.n_stages
        old_volatile_context = dict(ctx.volatile_context)
        self._resolve_defaults(ctx)
        ctx.n_stages = len(self.cycles_per_stages)
        if name is None:
            name = self.name if self.name is not None else StringGenerator(base="autotroph", suffix=f"_{ctx.rng.randint(1, 1000000)}")
        if food_type is None:
            if self.food_type is None:
                raise ValueError("Food type is not set")
            food_type = self.food_type.generate(ctx) if isinstance(self.food_type, Generator) else self.food_type
        diet_by_population_index = self.diet_by_population_index.generate(ctx) if isinstance(self.diet_by_population_index, Generator) else self.diet_by_population_index
        if diet_by_population_index is None:
            diet_by_population_index = [("NUTRIENTS_TYPE", 0, 0, 0)] * len(self.cycles_per_stages)
        elif isinstance(diet_by_population_index, list):
            diet_by_population_index = [diet_by_population_index]
        if len(diet_by_population_index) != len(self.cycles_per_stages):
            diet_by_population_index = diet_by_population_index.extend([("NUTRIENTS_TYPE", 0, 0, 0)] * (len(self.cycles_per_stages) - len(diet_by_population_index)))
        if self.opacity is None:
            if self.size_class is not None:
                self.opacity = VectorNumberGeneratorByClass(
                    class_range=[[0.01, 0.05],[0.01, 0.1],[0.05, 0.15],[0.1, 0.18],[0.15, 0.2],[0.15, 0.25],[0.2, 0.3],[0.2, 0.35],[0.3, 0.4],[0.3, 0.45],[0.3, 0.5]], 
                    class_index=self.size_class, 
                    length=len(self.cycles_per_stages))
            else:
                self.opacity = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=0.1, std=0.1), increment=NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 0.1)), length=len(self.cycles_per_stages))
        if self.stratum is None:
            if self.size_class is not None:
                self.stratum = VectorIntGeneratorByClass(
                    class_range=[[0, 1],[0, 1],[1, 2],[1, 2],[2, 3],[2, 3],[3, 4],[3, 4],[4, 5],[4, 5]], 
                    class_index=self.size_class, length=len(self.cycles_per_stages)
                )
            else:
                self.stratum = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=2.5, std=1, integer=True), increment=NumberGaussianGenerator(mean=1, std=1, integer=True), length=len(self.cycles_per_stages), integer=True)
        if self.min_light is None:
            if self.size_class is not None:
                self.min_light = VectorNumberGeneratorByClass(class_range=[[0.01, 0.05],[0.01, 0.1],[0.05, 0.15],[0.1, 0.18],[0.15, 0.2],[0.15, 0.25],[0.2, 0.3],[0.2, 0.35],[0.3, 0.4],[0.3, 0.45],[0.3, 0.5]], class_index=self.size_class, length=len(self.cycles_per_stages))
            else:
                self.min_light = VectorIncrementalGenerator(initial_value=NumberGaussianGenerator(mean=0.3, std=0.2), increment=NumberGaussianGenerator(mean=0.5, std=0.1, clamp=(0.0, 0.1)), length=len(self.cycles_per_stages))
        if self.seed_dispersal_rate is None:
            self.seed_dispersal_rate = NumberGaussianGenerator(mean=0.1, std=0.1, clamp=(0.0, 0.1))

        self._generate_to_context(ctx)
        
        ret = Autotroph(
            name=name.generate(ctx) if isinstance(name, Generator) else name,
            food_type=food_type.generate(ctx) if isinstance(food_type, Generator) else food_type,
            biomass_to_energy_conversion_factor=ctx.volatile_context["biomass_to_energy_conversion_factor"],
            death_biomass_to_energy_conversion_factor=ctx.volatile_context["death_biomass_to_energy_conversion_factor"],
            maintenance_cost=ctx.volatile_context["maintenance_cost"],
            max_fertility=ctx.volatile_context["max_fertility"],
            resilience=ctx.volatile_context["resilience"],
            biomass_per_individual_amount=ctx.volatile_context["biomass_per_individual_amount"],
            individual_occupied_surface=ctx.volatile_context["individual_occupied_surface"],
            characteristics_death_biomass=  ctx.volatile_context["characteristics_death_biomass"],
            death_biomass_fraction_surface=ctx.volatile_context["death_biomass_fraction_surface"],
            death_biomass_per_fraction_amount=ctx.volatile_context["death_biomass_per_fraction_amount"],
            death_biomass_fraction_by_size=ctx.volatile_context["death_biomass_fraction_by_size"],
            best_environmental_conditions=ctx.volatile_context["best_environmental_conditions"],
            cycles_per_stages=ctx.volatile_context["cycles_per_stages"],
            defense_strategies=ctx.volatile_context["defense_strategies"],
            recruitment_strategies=ctx.volatile_context["recruitment_strategies"],
            max_individual_growth=ctx.volatile_context["max_individual_growth"],
            max_density=ctx.volatile_context["max_density"],
            colony_ability_rate=ctx.volatile_context["colony_ability_rate"],
            opacity=ctx.volatile_context["opacity"],
            stratum=ctx.volatile_context["stratum"],
            min_light=ctx.volatile_context["min_light"],
            seed_dispersal_rate=ctx.volatile_context["seed_dispersal_rate"],
            diet_by_population_index=diet_by_population_index
        )
        ctx.volatile_context = old_volatile_context
        ctx.n_stages = old_n_stages
        return ret

    def _generate_to_context(self, ctx: GenerationContext) -> None:
        super()._generate_to_context(ctx)
        ctx.volatile_context["opacity"] = self.opacity.generate(ctx) if isinstance(self.opacity, Generator) else self.opacity
        ctx.volatile_context["stratum"] = self.stratum.generate(ctx) if isinstance(self.stratum, Generator) else self.stratum
        ctx.volatile_context["min_light"] = self.min_light.generate(ctx) if isinstance(self.min_light, Generator) else self.min_light
        ctx.volatile_context["seed_dispersal_rate"] = self.seed_dispersal_rate.generate(ctx) if isinstance(self.seed_dispersal_rate, Generator) else self.seed_dispersal_rate
        

@dataclass
class HeterotrophGenerator(LivingBeingGenerator):
    prospecting_ability: Optional[Generator[List[float]]] = None
    assimilation_efficiency: Optional[Generator[List[float]]] = None
    ingestion_residue_fraction_by_size: Optional[Generator[List[List[float]]]] = None
    prey_location: Optional[Generator[List[float]]] = None
    diet_by_food_type: Optional[Union[Generator[List[List[Tuple[str, int, int, int]]]], List[List[Tuple[str, int, int, int]]]]] = None
    
    def generate(self, ctx: GenerationContext, name: Union[Generator[str], str, None]=None, food_type: Union[Generator[str], str, None]=None) -> Heterotroph:
        old_n_stages = ctx.n_stages
        old_volatile_context = dict(ctx.volatile_context)
        self._resolve_defaults(ctx)
        ctx.n_stages = len(self.cycles_per_stages)
        if name is None:
            name = self.name if self.name is not None else StringGenerator(base="autotroph", suffix=f"_{ctx.rng.randint(1, 1000000)}")
        if food_type is None:
            if self.food_type is None:
                raise ValueError("Food type is not set")
            food_type = self.food_type.generate(ctx) if isinstance(self.food_type, Generator) else self.food_type
        
        if self.prospecting_ability is None:
            pa = NumberFromUniformRangeGenerator(min=0.0, max=1.0).generate(ctx)
            self.prospecting_ability = VectorGenerator(generator=NumberArroundValueGenerator(value=pa, ratio=NumberFromUniformRangeGenerator(min=0.05, max=0.2)), length=ctx.n_stages)
        
        if self.assimilation_efficiency is None:
            self.assimilation_efficiency = VectorUniformRangeGenerator(min=0.0, max=1.0, length=ctx.n_stages)

        if self.ingestion_residue_fraction_by_size is None:
            self.ingestion_residue_fraction_by_size = [VectorWithMaxSumValueGenerator(max_sum_value=1.0, length=ctx.n_bins)] * ctx.n_stages

        if self.prey_location is None:
            self.prey_location = VectorUniformRangeGenerator(min=0.0, max=1.0, length=ctx.n_stages)
            
        diet_by_population_index = self.diet_by_population_index.generate(ctx) if isinstance(self.diet_by_population_index, Generator) else self.diet_by_population_index
        if self.diet_by_food_type is not None:
            diet_by_food_type = self.diet_by_food_type.generate(ctx) if isinstance(self.diet_by_food_type, Generator) else self.diet_by_food_type
            len_by_food_type = len(diet_by_food_type)
        else:
            diet_by_food_type = []
            len_by_food_type = 0
        
        if diet_by_population_index is None or len(diet_by_population_index) < ctx.n_stages:
            if len_by_food_type < ctx.n_stages:
                if diet_by_population_index is None or len(diet_by_population_index) < ctx.n_stages:
                    l = 0 if diet_by_population_index is None else len(diet_by_population_index)
                    diet_by_population_index = [[] for _ in range(ctx.n_stages-l)]
                for i in range(ctx.n_stages):
                    if diet_by_population_index[i] == [] and (len_by_food_type < i+1 or diet_by_food_type[i] is None or diet_by_food_type[i] == []):
                        chs = ctx.get_value("populations", [])
                        lchs = len(chs)
                        if lchs > 0:
                            chi = ctx.rng.randint(0, lchs-1)
                            d = (chi,  0, len(chs[chi].specie.getCyclesPerStages())-1, ctx.rng.randint(0, 1))
                        else:
                            chi = 0
                            d = (chi,  0, ctx.n_stages-1, 1)
                        diet_by_population_index[i].append(d)
        
        self._generate_to_context(ctx)

        ret = Heterotroph(
            name=self.name.generate(ctx) if isinstance(self.name, Generator) else self.name,
            food_type=self.food_type.generate(ctx) if isinstance(self.food_type, Generator) else self.food_type,
            biomass_to_energy_conversion_factor=ctx.volatile_context["biomass_to_energy_conversion_factor"],
            death_biomass_to_energy_conversion_factor=ctx.volatile_context["death_biomass_to_energy_conversion_factor"],
            maintenance_cost=ctx.volatile_context["maintenance_cost"],
            max_fertility=ctx.volatile_context["max_fertility"],
            resilience=ctx.volatile_context["resilience"],
            biomass_per_individual_amount=ctx.volatile_context["biomass_per_individual_amount"],
            individual_occupied_surface=ctx.volatile_context["individual_occupied_surface"],
            characteristics_death_biomass=ctx.volatile_context["characteristics_death_biomass"],
            death_biomass_fraction_surface=ctx.volatile_context["death_biomass_fraction_surface"],
            death_biomass_per_fraction_amount=ctx.volatile_context["death_biomass_per_fraction_amount"],
            death_biomass_fraction_by_size=ctx.volatile_context["death_biomass_fraction_by_size"],
            best_environmental_conditions=ctx.volatile_context["best_environmental_conditions"],
            cycles_per_stages=ctx.volatile_context["cycles_per_stages"],
            defense_strategies=ctx.volatile_context["defense_strategies"],
            recruitment_strategies=ctx.volatile_context["recruitment_strategies"],
            max_individual_growth=ctx.volatile_context["max_individual_growth"],
            max_density=ctx.volatile_context["max_density"],
            colony_ability_rate=ctx.volatile_context["colony_ability_rate"],
            prospecting_ability=ctx.volatile_context["prospecting_ability"],
            assimilation_efficiency=ctx.volatile_context["assimilation_efficiency"],
            ingestion_residue_fraction_by_size=ctx.volatile_context["ingestion_residue_fraction_by_size"],
            diet_by_food_type=diet_by_food_type,
            prey_location=ctx.volatile_context["prey_location"],
            diet_by_population_index=diet_by_population_index
        )
        ctx.n_stages = old_n_stages
        ctx.volatile_context = old_volatile_context
        return ret

    def _generate_to_context(self, ctx: GenerationContext) -> None:
        super()._generate_to_context(ctx)
        ctx.volatile_context["prospecting_ability"] = self.prospecting_ability.generate(ctx) if isinstance(self.prospecting_ability, Generator) else self.prospecting_ability
        ctx.volatile_context["assimilation_efficiency"] = self.assimilation_efficiency.generate(ctx) if isinstance(self.assimilation_efficiency, Generator) else self.assimilation_efficiency
        ctx.volatile_context["ingestion_residue_fraction_by_size"] = [irfs.generate(ctx) if isinstance(irfs, Generator) else irfs for irfs in self.ingestion_residue_fraction_by_size]
        ctx.volatile_context["prey_location"] = self.prey_location.generate(ctx) if isinstance(self.prey_location, Generator) else self.prey_location
        

# #---------------------------------------- Old version
# if TYPE_CHECKING:
#     from .species_process import AutotrophSpeciesProcess, HeterotrophSpeciesProcess

# from pydantic import BaseModel, ConfigDict, Field, model_validator

# # Diet rows accept mixed JSON shapes: objects with population_index / min_stage / max_stage
# # or 4-element arrays (population_index, min, max, matter_type).
# DietStageRow = List[Any]
# DietByPopulationIndex = List[DietStageRow]

# # Sentinel population_index strings accepted by JsonEnumNames::parseDietPopulationIndexValue
# _SPECIAL_DIET_COHORT_KEYS = frozenset(
#     {"NUTRIENTS_TYPE", "CATABOLIC_TYPE", "PARENTAL_SUPPLY_TYPE", "HETEROTROPH_TYPE"}
# )

# # Defaults for stochastic energy factors (Gaussian around prototype means).
# _DEFAULT_BIOMASS_ENERGY_STD = 40.0  # between ~30–50 as suggested
# _DEFAULT_DEATH_BIOMASS_ENERGY_STD = 10.0
# _DEFAULT_MAINTENANCE_RANGE = (0.05, 0.5)
# _DEFAULT_MAX_FERTILITY_RANGE = (0.3, 0.9)

# # Optional keys inside the unified ``generation_config`` JSON for energy / maintenance /
# # fertility randomization (subset replaces module defaults).
# _ENERGY_RANDOMIZATION_KEYS = frozenset(
#     {
#         "biomass_energy",
#         "death_biomass_energy",
#         "maintenance_cost_range",
#         "max_fertility_range",
#     }
# )


# def _energy_mean_std_from_config(
#     cfg: Mapping[str, Any],
#     key: str,
#     default_std: float,
# ) -> tuple[Optional[float], float]:
#     """
#     Read optional Gaussian mean and std for an energy conversion factor.

#     ``cfg[key]`` may be absent (use default std, mean from prototype at call site), a
#     non-negative number (treat as ``std`` only), or a dict with optional ``mean`` / ``MEAN``
#     and ``std`` / ``STD``.
#     """
#     std = float(default_std)
#     mean_override: Optional[float] = None

#     if key not in cfg:
#         return mean_override, std

#     raw = cfg[key]
#     if isinstance(raw, (int, float)):
#         std = float(raw)
#         if std < 0:
#             raise ValueError(f"{key}: when a number, value must be non-negative (interpreted as std).")
#         return mean_override, std

#     if isinstance(raw, dict):
#         if "mean" in raw or "MEAN" in raw:
#             mv = raw["mean"] if "mean" in raw else raw["MEAN"]
#             mean_override = float(mv)
#         if "std" in raw or "STD" in raw:
#             sv = raw["std"] if "std" in raw else raw["STD"]
#             std = float(sv)
#         if std < 0:
#             raise ValueError(f"{key}.std must be non-negative.")
#         return mean_override, std

#     raise ValueError(
#         f"{key} must be a number (std only) or an object with optional mean/std; got {type(raw).__name__}."
#     )


# def _energy_params_from_config(
#     cfg: Mapping[str, Any],
# ) -> tuple[Optional[float], float, Optional[float], float, tuple[float, float], tuple[float, float]]:
#     """Read energy-related knobs from a unified config dict (other keys ignored)."""
#     bio_mean, biomass_std = _energy_mean_std_from_config(cfg, "biomass_energy", _DEFAULT_BIOMASS_ENERGY_STD)
#     death_mean, death_std = _energy_mean_std_from_config(
#         cfg, "death_biomass_energy", _DEFAULT_DEATH_BIOMASS_ENERGY_STD
#     )
#     maint_range = tuple(float(x) for x in _DEFAULT_MAINTENANCE_RANGE)
#     fert_range = tuple(float(x) for x in _DEFAULT_MAX_FERTILITY_RANGE)
#     if "maintenance_cost_range" in cfg:
#         value = cfg["maintenance_cost_range"]
#         if not isinstance(value, (list, tuple)) or len(value) != 2:
#             raise ValueError("maintenance_cost_range must be a pair [low, high].")
#         lo, hi = float(value[0]), float(value[1])
#         if lo > hi:
#             raise ValueError("maintenance_cost_range: low must be <= high.")
#         maint_range = (lo, hi)
#     if "max_fertility_range" in cfg:
#         value = cfg["max_fertility_range"]
#         if not isinstance(value, (list, tuple)) or len(value) != 2:
#             raise ValueError("max_fertility_range must be a pair [low, high].")
#         lo, hi = float(value[0]), float(value[1])
#         if lo > hi:
#             raise ValueError("max_fertility_range: low must be <= high.")
#         fert_range = (lo, hi)

#     return bio_mean, biomass_std, death_mean, death_std, maint_range, fert_range


# def _config_has_energy_randomization(cfg: Mapping[str, Any]) -> bool:
#     return any(k in cfg for k in _ENERGY_RANDOMIZATION_KEYS)


# def _parse_generation_config(
#     raw: Optional[Union[str, dict[str, Any]]],
# ) -> dict[str, Any]:
#     """
#     Parse the unified ``generation_config`` object (dict or JSON string).

#     May include energy keys (``biomass_energy``, ``death_biomass_energy`` as a number or as
#     ``{"mean": ..., "std": ...}``, ``maintenance_cost_range``, ``max_fertility_range``) together with keys documented
#     on :meth:`AutotrophSpecies.generate`.
#     """
#     if raw is None:
#         return {}
#     if isinstance(raw, dict):
#         return dict(raw)
#     if isinstance(raw, str):
#         try:
#             parsed = json.loads(raw)
#         except json.JSONDecodeError as exc:
#             raise ValueError(f"generation_config: invalid JSON ({exc})") from exc
#         if not isinstance(parsed, dict):
#             raise ValueError("generation_config JSON must decode to an object.")
#         return parsed
#     raise ValueError(
#         "generation_config must be None, a dict, or a JSON string; "
#         f"got {type(raw).__name__}."
#     )


# # Five plant-size tiers (0 = very small … 4 = very large): Gaussian sampling uses these intervals.
# _SIZE_BIOMASS_INTERVALS: List[tuple[float, float]] = [
#     (0.002, 0.018),
#     (0.015, 0.035),
#     (0.03, 0.06),
#     (0.055, 0.095),
#     (0.085, 0.16),
# ]
# _SIZE_SURFACE_INTERVALS: List[tuple[float, float]] = [
#     (2e-5, 6e-4),
#     (5e-4, 0.012),
#     (0.008, 0.028),
#     (0.022, 0.055),
#     (0.045, 0.14),
# ]

# _DEFAULT_RESILIENCE_MEAN = 0.5
# _DEFAULT_RESILIENCE_STD = 0.25
# _DEFAULT_CYCLES_GAUSS_STD = 2.0
# _DEFAULT_COLONY_MEAN = 0.25
# _DEFAULT_COLONY_STD = 0.5
# _DEFAULT_MAX_GROWTH_FERT0_MEAN = 0.8
# _DEFAULT_MAX_GROWTH_FERT0_STD = 0.2
# _DEFAULT_MAX_GROWTH_FERTPOS_STD = 0.2
# # Uniform range for ``r_bonus`` in ``_max_individual_growth_vector`` (fertile stages).
# _DEFAULT_MAX_INDIVIDUAL_GROWTH_BONUS_RANGE = (0.0, 0.55)


# def _positive_gaussian(rng: random.Random, mean: float, std: float, floor: float = 1e-3) -> float:
#     """Sample N(mean, std), clamped below at ``floor`` (avoids non-positive energy factors)."""
#     return max(floor, mean + std * rng.gauss(0.0, 1.0))


# def _clamp_int(x: float, lo: int, hi: int) -> int:
#     return int(max(lo, min(hi, round(x))))


# def _clamp01(x: float) -> float:
#     return max(0.0, min(1.0, float(x)))


# def _gaussian_in_interval(
#     rng: random.Random, lo: float, hi: float, std_frac: float = 0.28
# ) -> float:
#     """Sample from a Gaussian centered in ``[lo, hi]`` with std proportional to width."""
#     lo, hi = float(lo), float(hi)
#     if lo > hi:
#         lo, hi = hi, lo
#     mid = 0.5 * (lo + hi)
#     width = max(hi - lo, 1e-12)
#     std = width * std_frac
#     v = mid + std * rng.gauss(0.0, 1.0)
#     return max(lo, min(hi, v))


# def _sample_size_tier(rng: random.Random, tier: int, intervals: Sequence[tuple[float, float]]) -> float:
#     t = max(0, min(4, int(tier)))
#     lo, hi = intervals[t]
#     return _gaussian_in_interval(rng, lo, hi)


# def _default_plant_size_profile(n_stages: int) -> List[int]:
#     base = [0, 1, 2]
#     if n_stages <= 0:
#         return []
#     if n_stages <= len(base):
#         return base[:n_stages]
#     return base + [2] * (n_stages - len(base))


# def _coerce_int_keys(m: Any) -> Dict[int, Any]:
#     if not isinstance(m, dict):
#         return {}
#     out: Dict[int, Any] = {}
#     for k, v in m.items():
#         try:
#             out[int(k)] = v
#         except (TypeError, ValueError) as exc:
#             raise ValueError(f"Expected integer-like dict keys, got {k!r}") from exc
#     return out


# def _normalize_distribution_row(row: List[float], eps: float = 1e-12) -> List[float]:
#     s = sum(row)
#     if s <= eps:
#         n = len(row)
#         return [1.0 / n] * n if n else row
#     return [max(0.0, float(x)) / s for x in row]


# def _parse_resilience_config(
#     raw: Any, n_stages: int
# ) -> tuple[List[float], List[float]]:
#     """Returns parallel lists (means, stds) length ``n_stages``."""
#     if raw is None:
#         return (
#             [_DEFAULT_RESILIENCE_MEAN] * n_stages,
#             [_DEFAULT_RESILIENCE_STD] * n_stages,
#         )
#     if isinstance(raw, dict):
#         m = float(raw.get("mean", _DEFAULT_RESILIENCE_MEAN))
#         s = float(raw.get("std", _DEFAULT_RESILIENCE_STD))
#         if s < 0:
#             raise ValueError("resilience std must be non-negative.")
#         return ([m] * n_stages, [s] * n_stages)
#     if isinstance(raw, list):
#         means: List[float] = []
#         stds: List[float] = []
#         for i in range(n_stages):
#             entry = raw[i] if i < len(raw) else raw[-1]
#             if isinstance(entry, dict):
#                 means.append(float(entry.get("mean", _DEFAULT_RESILIENCE_MEAN)))
#                 sd = float(entry.get("std", _DEFAULT_RESILIENCE_STD))
#             elif isinstance(entry, (list, tuple)) and len(entry) >= 2:
#                 means.append(float(entry[0]))
#                 sd = float(entry[1])
#             else:
#                 raise ValueError(f"resilience[{i}]: expected object or [mean, std], got {entry!r}")
#             if sd < 0:
#                 raise ValueError(f"resilience[{i}]: std must be non-negative.")
#             stds.append(sd)
#         return means, stds
#     raise ValueError("resilience must be null, an object with mean/std, or a list per stage.")


# def _apply_cycles_stochastic(
#     rng: random.Random,
#     cps_in: Sequence[int],
#     cycles_cfg: Any,
# ) -> List[int]:
#     n = len(cps_in)
#     if n == 0:
#         return []
#     if cycles_cfg is None:
#         return [int(x) for x in cps_in]
#     if not isinstance(cycles_cfg, dict):
#         raise ValueError("cycles must be an object or null.")
#     mode = str(cycles_cfg.get("mode", "literal")).lower()
#     if mode in ("literal", "none", "fixed"):
#         return [int(x) for x in cps_in]
#     if mode == "uniform":
#         lo = int(cycles_cfg.get("low", cycles_cfg.get("min", 1)))
#         hi = int(cycles_cfg.get("high", cycles_cfg.get("max", 10)))
#         if lo > hi:
#             lo, hi = hi, lo
#         if lo < 1:
#             raise ValueError("cycles.uniform: low must be >= 1.")
#         return [rng.randint(lo, hi) for _ in range(n)]
#     if mode == "gaussian":
#         std = float(cycles_cfg.get("std", _DEFAULT_CYCLES_GAUSS_STD))
#         means = cycles_cfg.get("means")
#         if isinstance(means, (list, tuple)) and len(means) >= n:
#             mlist = [float(means[i]) for i in range(n)]
#         else:
#             mlist = [float(cps_in[i]) for i in range(n)]
#         hi = int(cycles_cfg.get("high", 10))
#         lo = int(cycles_cfg.get("low", 1))
#         if lo < 1:
#             raise ValueError("cycles.gaussian: low must be >= 1.")
#         out: List[int] = []
#         for i in range(n):
#             v = mlist[i] + std * rng.gauss(0.0, 1.0)
#             out.append(_clamp_int(v, lo, hi))
#         return out
#     raise ValueError(f"Unknown cycles.mode {mode!r}; use literal, uniform, or gaussian.")


# def _fill_death_fraction_row(
#     rng: random.Random,
#     n_bins: int,
#     fixed: Mapping[int, float],
# ) -> List[float]:
#     row = [0.0] * n_bins
#     used = set()
#     fixed_sum = 0.0
#     for b, v in fixed.items():
#         bi = int(b)
#         if bi < 0 or bi >= n_bins:
#             raise ValueError(f"fraction bin index {bi} out of range for bins={n_bins}")
#         fv = float(v)
#         if fv < 0 or fv > 1.0:
#             raise ValueError(f"fraction value for bin {bi} must be in [0,1], got {fv}")
#         row[bi] = fv
#         used.add(bi)
#         fixed_sum += fv
#     if fixed_sum > 1.0 + 1e-9:
#         raise ValueError(f"Fixed death fractions sum to {fixed_sum} > 1.")
#     rem = 1.0 - fixed_sum
#     free = [i for i in range(n_bins) if i not in used]
#     if rem < 1e-15:
#         return _normalize_distribution_row(row)
#     if not free:
#         return _normalize_distribution_row(row)
#     weights = [rng.random() for _ in free]
#     sw = sum(weights) or 1.0
#     for idx, wi in zip(free, weights):
#         row[idx] += rem * (wi / sw)
#     return _normalize_distribution_row(row)


# def _death_bin_amount_surface(
#     rng: random.Random,
#     bin_index: int,
#     eff_size_class: int,
#     live_biomass: float,
#     live_surface: float,
# ) -> tuple[float, float]:
#     """Returns (amount_scale, surface_scale) for one dead-matter bin."""
#     if bin_index == 0:
#         return 0.001, max(1e-6, 0.001 * (0.5 + 0.5 * rng.random()) * live_surface / max(live_biomass, 1e-6))
#     # Shift mean left vs live tissue (smaller / lighter detritus on average).
#     cls = max(0, min(4, eff_size_class))
#     b_lo, b_hi = _SIZE_BIOMASS_INTERVALS[cls]
#     s_lo, s_hi = _SIZE_SURFACE_INTERVALS[cls]
#     shrink = 0.45 + 0.25 * rng.random()
#     amt = _gaussian_in_interval(rng, b_lo * shrink, b_hi * shrink * 0.85)
#     surf = _gaussian_in_interval(rng, s_lo * shrink, s_hi * shrink * 0.85)
#     amt = min(amt, live_biomass * (0.35 + 0.2 * rng.random()))
#     surf = min(surf, live_surface * (0.4 + 0.2 * rng.random()))
#     return max(1e-6, amt), max(1e-8, surf)


# def _build_death_biomass_bundle(
#     rng: random.Random,
#     n_stages: int,
#     biomass_per_stage: Sequence[float],
#     surface_per_stage: Sequence[float],
#     plant_size_classes: Sequence[int],
#     death_cfg: Any,
# ) -> tuple[
#     List[List[float]],
#     List[float],
#     List[float],
#     List[List[float]],
# ]:
#     """
#     Returns (characteristics_death_biomass, fraction_surface, per_fraction_amount,
#     death_biomass_fraction_by_size).
#     """
#     if death_cfg is None:
#         n_bins = 3
#         char_lo, char_hi = 0.0, 1.0
#         size_map: Dict[int, int] = {}
#         stage_fixed = {}
#     elif isinstance(death_cfg, dict):
#         if "bins" not in death_cfg:
#             raise ValueError("death_biomass: key 'bins' is required when death_biomass is set.")
#         n_bins = int(death_cfg["bins"])
#         if n_bins < 2:
#             raise ValueError("death_biomass.bins must be >= 2.")
#         ch = death_cfg.get("characteristics")
#         if ch is None:
#             char_lo, char_hi = 0.0, 1.0
#         elif isinstance(ch, (list, tuple)) and len(ch) == 2:
#             char_lo, char_hi = float(ch[0]), float(ch[1])
#             if char_lo > char_hi:
#                 char_lo, char_hi = char_hi, char_lo
#         else:
#             raise ValueError("death_biomass.characteristics must be [low, high] or omitted.")
#         size_map = {k: int(v) for k, v in _coerce_int_keys(death_cfg.get("size", {})).items()}
#         frac_raw = death_cfg.get("fraction", {})
#         stage_fixed = {}
#         if isinstance(frac_raw, dict):
#             for sk, inner in frac_raw.items():
#                 try:
#                     si = int(sk)
#                 except (TypeError, ValueError) as exc:
#                     raise ValueError(f"fraction stage key {sk!r} must be int-like") from exc
#                 if not isinstance(inner, dict):
#                     raise ValueError(f"fraction[{si}] must be an object bin->value.")
#                 stage_fixed[si] = {int(bk): float(bv) for bk, bv in _coerce_int_keys(inner).items()}
#     else:
#         raise ValueError("death_biomass must be an object or null.")

#     characteristics: List[List[float]] = []
#     for _ in range(n_stages):
#         row3 = [
#             rng.uniform(char_lo, char_hi),
#             rng.uniform(char_lo, char_hi),
#             rng.uniform(char_lo, char_hi),
#         ]
#         characteristics.append(row3)

#     d_frac_surface: List[float] = []
#     d_per_amt: List[float] = []
#     d_by_size: List[List[float]] = []

#     for s in range(n_stages):
#         plant_cls = max(0, min(4, int(plant_size_classes[s])))
#         fixed_bins = dict(stage_fixed.get(s, {}))
#         row_frac = _fill_death_fraction_row(rng, n_bins, fixed_bins)

#         amounts: List[float] = []
#         surfaces: List[float] = []
#         for b in range(n_bins):
#             eff = size_map.get(b, plant_cls)
#             if b == 0:
#                 eff = 0  # bin 0 uses fixed anchor path inside helper
#             a, surf = _death_bin_amount_surface(
#                 rng, b, int(eff), float(biomass_per_stage[s]), float(surface_per_stage[s])
#             )
#             amounts.append(a)
#             surfaces.append(surf)

#         w_amt = sum(row_frac[b] * amounts[b] for b in range(n_bins))
#         w_surf = sum(row_frac[b] * surfaces[b] for b in range(n_bins))
#         d_per_amt.append(max(1e-6, w_amt))
#         live_s = float(surface_per_stage[s])
#         surf_scale = w_surf / max(live_s * max(n_bins, 1), 1e-9)
#         d_frac_surface.append(float(max(1e-4, min(0.95, surf_scale * 0.22))))

#         d_by_size.append(row_frac)

#     return characteristics, d_frac_surface, d_per_amt, d_by_size


# def _build_ingestion_residue_fraction_by_size(
#     rng: random.Random,
#     n_stages: int,
#     plant_size_classes: Sequence[int],
#     ingestion_cfg: Any,
# ) -> List[List[float]]:
#     """
#     Same row contract as ``death_biomass_fraction_by_size`` (each row sums to 1).

#     ``ingestion_cfg`` mirrors ``death_biomass``: ``bins`` (required if set), optional
#     ``fraction`` (stage→bin→fixed share), optional ``size`` (bin→tier) — only ``bins``
#     and ``fraction`` affect the matrix here.
#     """
#     if ingestion_cfg is None:
#         n_bins = 3
#         stage_fixed: Dict[int, Dict[int, float]] = {}
#     elif isinstance(ingestion_cfg, dict):
#         if "bins" not in ingestion_cfg:
#             raise ValueError("ingestion_residue: key 'bins' is required when ingestion_residue is set.")
#         n_bins = int(ingestion_cfg["bins"])
#         if n_bins < 2:
#             raise ValueError("ingestion_residue.bins must be >= 2.")
#         frac_raw = ingestion_cfg.get("fraction", {})
#         stage_fixed = {}
#         if isinstance(frac_raw, dict):
#             for sk, inner in frac_raw.items():
#                 try:
#                     si = int(sk)
#                 except (TypeError, ValueError) as exc:
#                     raise ValueError(f"ingestion_residue.fraction stage key {sk!r} must be int-like") from exc
#                 if not isinstance(inner, dict):
#                     raise ValueError(f"ingestion_residue.fraction[{si}] must be an object bin->value.")
#                 stage_fixed[si] = {int(bk): float(bv) for bk, bv in _coerce_int_keys(inner).items()}
#     else:
#         raise ValueError("ingestion_residue must be an object or null.")

#     out: List[List[float]] = []
#     for s in range(n_stages):
#         _ = plant_size_classes[s]  # reserved for future bin-tier coupling (symmetry with death biomass)
#         fixed_bins = dict(stage_fixed.get(s, {}))
#         out.append(_fill_death_fraction_row(rng, n_bins, fixed_bins))
#     return out


# def _prospecting_ability_vector(
#     rng: random.Random,
#     max_fertility: Sequence[float],
#     prospecting_cfg: Any,
# ) -> List[float]:
#     """
#     Random prospecting per stage in ``[lo, hi]`` (default ``[1, 1000]``).

#     Stages with ``max_fertility`` (approximately) zero are capped below the minimum
#     prospecting drawn among fertile (reproductive) stages.
#     """
#     p_lo, p_hi = 1.0, 1000.0
#     if isinstance(prospecting_cfg, dict):
#         pr = prospecting_cfg.get("range", prospecting_cfg.get("prospecting_range"))
#         if isinstance(pr, (list, tuple)) and len(pr) == 2:
#             p_lo, p_hi = float(pr[0]), float(pr[1])
#     elif isinstance(prospecting_cfg, (list, tuple)) and len(prospecting_cfg) == 2:
#         p_lo, p_hi = float(prospecting_cfg[0]), float(prospecting_cfg[1])
#     if p_lo > p_hi:
#         p_lo, p_hi = p_hi, p_lo
#     if p_lo < 1.0:
#         p_lo = 1.0

#     n = len(max_fertility)
#     fertile = [i for i in range(n) if float(max_fertility[i]) > 1e-9]
#     infertile = [i for i in range(n) if float(max_fertility[i]) <= 1e-9]

#     prospecting = [0.0] * n
#     if not fertile:
#         for i in range(n):
#             prospecting[i] = rng.uniform(p_lo, p_hi)
#         return prospecting

#     min_fertile = float("inf")
#     for i in fertile:
#         v = rng.uniform(p_lo, p_hi)
#         prospecting[i] = v
#         min_fertile = min(min_fertile, v)

#     cap = max(p_lo, min_fertile * 0.999)
#     for i in infertile:
#         prospecting[i] = rng.uniform(p_lo, min(p_hi, cap))
#     return prospecting


# def _assimilation_efficiency_vector(
#     rng: random.Random,
#     n_stages: int,
#     assimilation_cfg: Any,
# ) -> List[float]:
#     """
#     Draw the **last** (highest-index) stage in ``[lo, hi]``, then walk backwards subtracting
#     ``Uniform(0, 0.5)`` per step, clamped to ``[0.05, 1.0]``.
#     """
#     lo, hi = 0.55, 0.98
#     if isinstance(assimilation_cfg, dict):
#         ar = assimilation_cfg.get("range", assimilation_cfg.get("assimilation_efficiency_range"))
#         if isinstance(ar, (list, tuple)) and len(ar) == 2:
#             lo, hi = float(ar[0]), float(ar[1])
#     elif isinstance(assimilation_cfg, (list, tuple)) and len(assimilation_cfg) == 2:
#         lo, hi = float(assimilation_cfg[0]), float(assimilation_cfg[1])
#     if lo > hi:
#         lo, hi = hi, lo

#     out = [0.0] * n_stages
#     if n_stages < 1:
#         return out
#     out[n_stages - 1] = rng.uniform(lo, hi)
#     for s in range(n_stages - 2, -1, -1):
#         out[s] = max(0.05, out[s + 1] - rng.uniform(0.0, 0.5))
#     return [min(1.0, float(x)) for x in out]


# def _prey_location_vector(rng: random.Random, n_stages: int, prey_cfg: Any) -> List[float]:
#     mean = 0.5
#     std = 0.3
#     if isinstance(prey_cfg, dict):
#         mean = float(prey_cfg.get("mean", mean))
#         std = float(prey_cfg.get("std", std))
#     if std < 0:
#         raise ValueError("prey_location std must be non-negative.")
#     return [_clamp01(mean + std * rng.gauss(0.0, 1.0)) for _ in range(n_stages)]


# def _resize_diet_by_population_index(rows: DietByPopulationIndex, n: int) -> DietByPopulationIndex:
#     if n <= 0:
#         raise ValueError("n must be positive.")
#     if not rows:
#         return [[_nutrients_rule_object()] for _ in range(n)]
#     if len(rows) >= n:
#         return [list(r) for r in rows[:n]]
#     out = [list(r) for r in rows]
#     last = list(rows[-1])
#     while len(out) < n:
#         out.append(list(last))
#     return out


# def _sample_matrix_uniform(
#     rng: random.Random, n_rows: int, n_cols: int, lo: float, hi: float
# ) -> List[List[float]]:
#     if lo > hi:
#         lo, hi = hi, lo
#     return [[rng.uniform(lo, hi) for _ in range(n_cols)] for _ in range(n_rows)]


# def _max_individual_growth_vector(
#     rng: random.Random,
#     max_fertility: Sequence[float],
#     bonus_range: tuple[float, float] = _DEFAULT_MAX_INDIVIDUAL_GROWTH_BONUS_RANGE,
# ) -> List[float]:
#     n = len(max_fertility)
#     out: List[float] = []
#     lo, hi = float(bonus_range[0]), float(bonus_range[1])
#     if lo > hi:
#         lo, hi = hi, lo
#     r_bonus = rng.uniform(lo, hi)
#     for s, fert in enumerate(max_fertility):
#         f = float(fert)
#         if f <= 1e-12:
#             g = _positive_gaussian(
#                 rng, _DEFAULT_MAX_GROWTH_FERT0_MEAN, _DEFAULT_MAX_GROWTH_FERT0_STD, floor=0.0
#             )
#             out.append(_clamp01(g))
#         else:
#             if n > 1:
#                 w = (n - 1 - s) / (n - 1)
#             else:
#                 w = 1.0
#             mean = f + r_bonus * w
#             g = mean + _DEFAULT_MAX_GROWTH_FERTPOS_STD * rng.gauss(0.0, 1.0)
#             out.append(_clamp01(g))
#     return out


# def _max_individual_growth_bonus_range_from_config(
#     gcfg: Mapping[str, Any],
#     key: str,
# ) -> tuple[float, float]:
#     """Read ``[low, high]`` for the uniform bonus used in ``_max_individual_growth_vector``."""
#     raw = gcfg.get(key)
#     if raw is None:
#         return _DEFAULT_MAX_INDIVIDUAL_GROWTH_BONUS_RANGE
#     if not isinstance(raw, (list, tuple)) or len(raw) != 2:
#         raise ValueError(f"{key} must be [low, high].")
#     lo, hi = float(raw[0]), float(raw[1])
#     if lo > hi:
#         lo, hi = hi, lo
#     return (lo, hi)


# def _colony_rate(
#     rng: random.Random,
#     colony_cfg: Any,
# ) -> float:
#     mean = _DEFAULT_COLONY_MEAN
#     std = _DEFAULT_COLONY_STD
#     if isinstance(colony_cfg, dict):
#         mean = float(colony_cfg.get("mean", mean))
#         std = float(colony_cfg.get("std", std))
#     if std < 0:
#         raise ValueError("colony std must be non-negative.")
#     v = mean + std * rng.gauss(0.0, 1.0)
#     return _clamp01(v)


# def _seed_dispersal(rng: random.Random, seed_cfg: Any) -> float:
#     if seed_cfg is None:
#         lo, hi = 0.0, 1.0
#     elif isinstance(seed_cfg, dict):
#         rng_pair = seed_cfg.get("range", seed_cfg.get("interval", [0.0, 1.0]))
#         if not isinstance(rng_pair, (list, tuple)) or len(rng_pair) != 2:
#             raise ValueError("seed_dispersal.range must be [low, high].")
#         lo, hi = float(rng_pair[0]), float(rng_pair[1])
#     else:
#         raise ValueError("seed_dispersal must be an object or null.")
#     if lo > hi:
#         lo, hi = hi, lo
#     return _clamp01(rng.uniform(lo, hi))


# def _stratum_opacity_min_light(
#     rng: random.Random,
#     n_stages: int,
#     biomass: Sequence[float],
#     surface: Sequence[float],
#     stratum_cfg: Any,
# ) -> tuple[List[int], List[float], List[float]]:
#     """Derive stratum, opacity, min_light from ``stratum_config`` and plant geometry."""
#     if stratum_cfg is None or not isinstance(stratum_cfg, dict):
#         n_strata = max(3, min(8, n_stages + 2))
#         height_classes = _default_plant_size_profile(n_stages)
#     else:
#         n_strata = int(stratum_cfg.get("n_strata", stratum_cfg.get("strata", max(3, n_stages + 2))))
#         if n_strata < 2:
#             raise ValueError("stratum_config.n_strata must be >= 2.")
#         hc = stratum_cfg.get("stage_height_class", stratum_cfg.get("heights"))
#         if isinstance(hc, (list, tuple)) and len(hc) >= n_stages:
#             height_classes = [max(0, min(4, int(hc[i]))) for i in range(n_stages)]
#         else:
#             height_classes = _default_plant_size_profile(n_stages)

#     max_stratum = n_strata - 1
#     stratum: List[int] = []
#     opacity: List[float] = []
#     min_light: List[float] = []

#     for s in range(n_stages):
#         tier = height_classes[s]
#         target = (tier / 4.0) * max_stratum if max_stratum > 0 else 0.0
#         st = _clamp_int(target + rng.gauss(0.0, 0.45), 0, max_stratum)
#         stratum.append(st)

#         bio, surf = float(biomass[s]), float(surface[s])
#         height_norm = (st + 1) / max(1, n_strata)
#         mass_proxy = bio * (1.0 + 2.0 * height_norm)
#         op = mass_proxy * math.sqrt(max(surf, 1e-12)) * (0.12 + 0.25 * rng.random())
#         opacity.append(float(max(1e-5, min(0.95, op))))

#         stage_norm = (s + 1) / max(1, n_stages)
#         base = 0.04 + 0.42 * (st / max(1, max_stratum)) * stage_norm
#         ml = base * (0.75 + 0.5 * rng.random())
#         min_light.append(float(max(0.01, min(0.95, ml))))

#     return stratum, opacity, min_light


# class StageSpecialDiet(TypedDict):
#     """Per-stage diet override for :meth:`AutotrophSpecies.generate`."""

#     stage: int
#     diet: List[str]


# def _resize_vector_float(values: Sequence[float], n: int) -> List[float]:
#     if n <= 0:
#         raise ValueError("Number of stages must be positive (non-empty cycles_per_stages).")
#     if len(values) == n:
#         return [float(x) for x in values]
#     if len(values) < n:
#         pad = float(values[-1]) if values else 0.0
#         return [float(x) for x in values] + [pad] * (n - len(values))
#     return [float(x) for x in values[:n]]


# def _resize_vector_int(values: Sequence[int], n: int) -> List[int]:
#     if n <= 0:
#         raise ValueError("Number of stages must be positive (non-empty cycles_per_stages).")
#     if len(values) == n:
#         return [int(x) for x in values]
#     if len(values) < n:
#         pad = int(values[-1]) if values else 0
#         return [int(x) for x in values] + [pad] * (n - len(values))
#     return [int(x) for x in values[:n]]


# def _resize_matrix(rows: Sequence[Sequence[float]], n_rows: int) -> List[List[float]]:
#     if n_rows <= 0:
#         raise ValueError("Number of stages must be positive (non-empty cycles_per_stages).")
#     if not rows:
#         return [[0.0] * 3 for _ in range(n_rows)]
#     ncol = len(rows[0])
#     if len(rows) >= n_rows:
#         return [[float(x) for x in row[:ncol]] for row in rows[:n_rows]]
#     out: List[List[float]] = [[float(x) for x in row] for row in rows]
#     last = [float(x) for x in rows[-1]]
#     while len(out) < n_rows:
#         out.append(list(last))
#     return out


# def _nutrients_rule_object() -> dict[str, Union[str, int]]:
#     return {"population_index": "NUTRIENTS_TYPE", "min_stage": 0, "max_stage": 0}


# def _rule_from_special_token(token: str) -> dict[str, Union[str, int]]:
#     if token in _SPECIAL_DIET_COHORT_KEYS:
#         return {"population_index": token, "min_stage": 0, "max_stage": 0}
#     raise ValueError(
#         f"Unknown diet token {token!r}; expected one of {sorted(_SPECIAL_DIET_COHORT_KEYS)} "
#         "or use a numeric population index via explicit dict in JSON."
#     )


# def _build_diet_rows(
#     n_stages: int,
#     stage_special_diets: Optional[Sequence[StageSpecialDiet]],
# ) -> DietByPopulationIndex:
#     overrides: Dict[int, List[str]] = {}
#     if stage_special_diets:
#         for entry in stage_special_diets:
#             st = int(entry["stage"])
#             if st < 0 or st >= n_stages:
#                 raise ValueError(f"stage {st} out of range for n_stages={n_stages}")
#             d = entry.get("diet")
#             if not isinstance(d, list) or not d:
#                 raise ValueError(f"Each override needs non-empty 'diet' list, got {entry!r}")
#             overrides[st] = [str(x) for x in d]

#     rows: DietByPopulationIndex = []
#     for s in range(n_stages):
#         if s in overrides:
#             rows.append([_rule_from_special_token(t) for t in overrides[s]])
#         else:
#             rows.append([_nutrients_rule_object()])
#     return rows


# class LivingBeingSpecies(BaseModel):
#     """Shared ``populations[].specie`` fields (``applyLivingBeingCommonFields`` in Builders.cpp)."""

#     model_config = ConfigDict(extra="forbid", populate_by_name=True)

#     class SpecieField:
#         """JSON keys common to autotroph and heterotroph ``specie`` objects."""

#         CLASS_TYPE = "class_type"
#         CLASS_NAME = "class_name"
#         NAME = "name"
#         FOOD_TYPE = "food_type"
#         BIOMASS_TO_ENERGY_CONVERSION_FACTOR = "biomass_to_energy_conversion_factor"
#         DEATH_BIOMASS_TO_ENERGY_CONVERSION_FACTOR = "death_biomass_to_energy_conversion_factor"
#         MAINTENANCE_COST = "maintenance_cost"
#         MAX_FERTILITY = "max_fertility"
#         RESILIENCE = "resilience"
#         BIOMASS_PER_INDIVIDUAL_AMOUNT = "biomass_per_individual_amount"
#         INDIVIDUAL_OCCUPIED_SURFACE = "individual_occupied_surface"
#         CHARACTERISTICS_DEATH_BIOMASS = "characteristics_death_biomass"
#         DEATH_BIOMASS_FRACTION_SURFACE = "death_biomass_fraction_surface"
#         DEATH_BIOMASS_PER_FRACTION_AMOUNT = "death_biomass_per_fraction_amount"
#         DEATH_BIOMASS_FRACTION_BY_SIZE = "death_biomass_fraction_by_size"
#         BEST_ENVIRONMENTAL_CONDITIONS = "best_environmental_conditions"
#         CYCLES_PER_STAGES = "cycles_per_stages"
#         DEFENSE_STRATEGIES = "defense_strategies"
#         RECRUITMENT_STRATEGIES = "recruitment_strategies"
#         MAX_INDIVIDUAL_GROWTH = "max_individual_growth"
#         MAX_DENSITY = "max_density"
#         COLONY_ABILITY_RATE = "colony_ability_rate"
#         DIET_BY_COHORT_INDEX = "diet_by_population_index"

#     class_type: Union[int, str] = Field(
#         default=0,
#         description="LivingBeingClassType; subclasses narrow default (AUTOTROPH / HETEROTROPH).",
#     )
#     class_name: str = Field(default="LivingBeing", description="Display / snapshot class name.")
#     name: str = Field(..., description="Species name.")
#     food_type: str = Field(default="0.0.0", description="Taxonomic diet prefix for rebuild_diet.")

#     biomass_to_energy_conversion_factor: float = Field(
#         default=190.5, description="Maps to C++ biomass_to_energy_conversion_factor."
#     )
#     death_biomass_to_energy_conversion_factor: float = Field(
#         default=19.5, description="Maps to death_biomass_to_energy_conversion_factor."
#     )

#     maintenance_cost: List[float] = Field(default_factory=lambda: [0.04, 0.0566, 0.0568])
#     max_fertility: List[float] = Field(default_factory=lambda: [0.0, 0.75, 0.7])
#     resilience: List[float] = Field(default_factory=lambda: [0.2, 0.6, 0.4])
#     biomass_per_individual_amount: List[float] = Field(
#         default_factory=lambda: [0.01, 0.04, 0.05]
#     )
#     individual_occupied_surface: List[float] = Field(
#         default_factory=lambda: [0.001, 0.02, 0.03]
#     )

#     characteristics_death_biomass: List[List[float]] = Field(
#         default_factory=lambda: [[0.1, 0.1, 0.1], [0.1, 0.3, 0.3], [0.1, 0.2, 0.4]]
#     )
#     death_biomass_fraction_surface: List[float] = Field(
#         default_factory=lambda: [0.001, 0.01, 0.1]
#     )
#     death_biomass_per_fraction_amount: List[float] = Field(
#         default_factory=lambda: [0.01, 0.04, 0.05]
#     )
#     death_biomass_fraction_by_size: List[List[float]] = Field(
#         default_factory=lambda: [[0.9, 0.1, 0.0], [0.6, 0.4, 0.0], [0.5, 0.5, 0.0]]
#     )
#     best_environmental_conditions: List[List[float]] = Field(
#         default_factory=lambda: [[0.9, 0.4, 0.4], [0.6, 0.5, 0.2], [0.6, 0.4, 0.1]]
#     )
#     cycles_per_stages: List[int] = Field(default_factory=lambda: [2, 3, 2])
#     defense_strategies: List[List[float]] = Field(
#         default_factory=lambda: [[0.2, 0.2, 0.2], [0.2, 0.3, 0.3], [0.2, 0.4, 0.2]]
#     )
#     recruitment_strategies: List[List[float]] = Field(
#         default_factory=lambda: [[0.3, 0.3, 0.2], [0.4, 0.3, 0.3], [0.3, 0.1, 0.5]]
#     )
#     max_individual_growth: List[float] = Field(default_factory=lambda: [0.9, 0.8, 0.7])
#     max_density: List[float] = Field(default_factory=lambda: [2000.0, 600.0, 405.0])
#     colony_ability_rate: float = Field(default=0.0, ge=0.0, le=1.0)

#     diet_by_population_index: Optional[DietByPopulationIndex] = Field(
#         default=None,
#         description="Per consumer stage diet tuples; omit to let C++ initialize() inject nutrients rules.",
#     )

#     @model_validator(mode="after")
#     def _lengths_match_stages(self) -> LivingBeingSpecies:
#         n = len(self.cycles_per_stages)
#         if n == 0:
#             raise ValueError("cycles_per_stages must be non-empty.")
#         pairs = [
#             ("maintenance_cost", self.maintenance_cost),
#             ("max_fertility", self.max_fertility),
#             ("resilience", self.resilience),
#             ("biomass_per_individual_amount", self.biomass_per_individual_amount),
#             ("individual_occupied_surface", self.individual_occupied_surface),
#             ("death_biomass_fraction_surface", self.death_biomass_fraction_surface),
#             ("death_biomass_per_fraction_amount", self.death_biomass_per_fraction_amount),
#             ("max_individual_growth", self.max_individual_growth),
#             ("max_density", self.max_density),
#         ]
#         for label, seq in pairs:
#             if len(seq) != n:
#                 raise ValueError(f"{label} length {len(seq)} != number of stages {n}")
#         matrices = [
#             ("characteristics_death_biomass", self.characteristics_death_biomass),
#             ("death_biomass_fraction_by_size", self.death_biomass_fraction_by_size),
#             ("best_environmental_conditions", self.best_environmental_conditions),
#             ("defense_strategies", self.defense_strategies),
#             ("recruitment_strategies", self.recruitment_strategies),
#         ]
#         for label, mat in matrices:
#             if len(mat) != n:
#                 raise ValueError(f"{label} row count {len(mat)} != number of stages {n}")
#         for si, row in enumerate(self.death_biomass_fraction_by_size):
#             if not row:
#                 raise ValueError("death_biomass_fraction_by_size rows must be non-empty.")
#             srow = sum(float(x) for x in row)
#             if abs(srow - 1.0) > 1e-4:
#                 raise ValueError(
#                     f"death_biomass_fraction_by_size row {si} must sum to 1.0 (tolerance 1e-4), got {srow}"
#                 )
#         if self.diet_by_population_index is not None and len(self.diet_by_population_index) != n:
#             raise ValueError(
#                 f"diet_by_population_index length {len(self.diet_by_population_index)} != number of stages {n}"
#             )
#         return self

#     def to_specie_dict(self) -> dict[str, Any]:
#         """Serialize for embedding under population ``specie`` (JSON-compatible primitives)."""
#         return self.model_dump(mode="json", exclude_none=True)


# class AutotrophSpecies(LivingBeingSpecies):
#     """Autotroph ``populations[].specie`` JSON (``class_type`` autotroph)."""

#     class GenerateParam:
#         """String keys matching :meth:`generate` kwargs when building config JSON."""

#         NAME = "name"
#         CYCLES_PER_STAGES = "cycles_per_stages"
#         STAGE_SPECIAL_DIETS = "stage_special_diets"
#         FOOD_TYPE = "food_type"
#         STOCHASTIC = "stochastic"
#         GENERATION_CONFIG = "generation_config"

#     class GenerationConfigKey:
#         """Keys for ``generation_config`` (unified JSON: energy knobs + stochastic profile)."""

#         # Energy / maintenance / fertility (optional; same names as former ``stochastic_overrides``).
#         BIOMASS_ENERGY = "biomass_energy"
#         DEATH_BIOMASS_ENERGY = "death_biomass_energy"
#         MAINTENANCE_COST_RANGE = "maintenance_cost_range"
#         MAX_FERTILITY_RANGE = "max_fertility_range"

#         RESILIENCE = "resilience"
#         DEATH_BIOMASS = "death_biomass"
#         CYCLES = "cycles"
#         STRATEGIES = "strategies"
#         COLONY = "colony"
#         STRATUM_CONFIG = "stratum_config"
#         SEED_DISPERSAL = "seed_dispersal"
#         MAX_INDIVIDUAL_GROWTH_BONUS_RANGE = "max_individual_growth_bonus_range"

#         BIOMASS_CLASSES = "biomass_classes"
#         SURFACE_CLASSES = "surface_classes"
#         DEFENSE_RANGE = "defense_range"
#         RECRUITMENT_RANGE = "recruitment_range"
#         BINS = "bins"
#         # Top-level ``size`` profile (biomass/surface tiers); same key ``size`` nested under ``death_biomass``.
#         SIZE = "size"
#         CHARACTERISTICS = "characteristics"
#         FRACTION = "fraction"
#         MODE = "mode"
#         LOW = "low"
#         HIGH = "high"
#         STD = "std"
#         MEANS = "means"
#         N_STRATA = "n_strata"
#         STAGE_HEIGHT_CLASS = "stage_height_class"
#         MEAN = "mean"
#         RANGE = "range"

#     class StageSpecialDietParam:
#         """Keys for each object inside ``stage_special_diets``."""

#         STAGE = "stage"
#         DIET = "diet"

#     class SpecieField(LivingBeingSpecies.SpecieField):
#         """All ``specie`` JSON keys for an autotroph (common + autotroph-only)."""

#         OPACITY = "opacity"
#         STRATUM = "stratum"
#         MIN_LIGHT = "min_light"
#         SEED_DISPERSAL_RATE = "seed_dispersal_rate"

#     class_type: Union[int, str] = Field(
#         default=0,
#         description="LivingBeingClassType; 0 or string AUTOTROPH (see JsonEnumNames).",
#     )
#     class_name: str = Field(default="Autotroph", description="Display / snapshot class name.")

#     opacity: List[float] = Field(default_factory=lambda: [0.001, 0.02, 0.05])
#     stratum: List[int] = Field(default_factory=lambda: [0, 1, 1])
#     min_light: List[float] = Field(default_factory=lambda: [0.2, 0.2, 0.4])
#     seed_dispersal_rate: float = Field(default=0.15, ge=0.0, le=1.0)

#     @model_validator(mode="after")
#     def _autotroph_vector_lengths(self) -> AutotrophSpecies:
#         n = len(self.cycles_per_stages)
#         for label, seq in (
#             ("opacity", self.opacity),
#             ("stratum", self.stratum),
#             ("min_light", self.min_light),
#         ):
#             if len(seq) != n:
#                 raise ValueError(f"{label} length {len(seq)} != number of stages {n}")
#         return self

#     @staticmethod
#     def _prototype_3_stage() -> AutotrophSpecies:
#         """Internal 3-stage template aligned with config/niche.example.000.json (first autotroph)."""
#         return AutotrophSpecies(
#             name="_template_",
#             diet_by_population_index=[
#                 [_nutrients_rule_object()],
#                 [["NUTRIENTS_TYPE", 0, 0]],
#                 [_nutrients_rule_object()],
#             ],
#         )

#     @classmethod
#     def generate_from(
#         cls,
#         process: "AutotrophSpeciesProcess",
#         *,
#         rng: Optional[random.Random] = None,
#         n_bins: int = 3,
#     ) -> AutotrophSpecies:
#         from .species_process import generate_autotroph_from_process

#         return generate_autotroph_from_process(process, rng=rng, n_bins=n_bins)

#     @classmethod
#     def generate(
#         cls,
#         name: str,
#         cycles_per_stages: Sequence[int],
#         food_type: str = "0",
#         stage_special_diets: Optional[Sequence[StageSpecialDiet]] = None,
#         *,
#         rng: Optional[random.Random] = None,
#         stochastic: bool = False,
#         generation_config: Optional[Union[str, dict[str, Any]]] = None,
#     ) -> AutotrophSpecies:
#         """
#         Build an autotroph spec for a given number of life-history stages.

#         ``cycles_per_stages`` defines the number of stages (``n``). When ``stochastic`` is
#         True, entries may be replaced according to ``generation_config["cycles"]`` (see
#         below); otherwise each value must be a positive integer.

#         ``diet_by_population_index``: each stage defaults to a single NUTRIENTS_TYPE rule.
#         ``stage_special_diets`` overrides per stage (see class docstring).

#         **Unified ``generation_config``** (``None``, a ``dict``, or a JSON object string):
#         may combine optional energy / maintenance / fertility knobs with the stochastic
#         profile keys below. Energy-related keys (see ``GenerationConfigKey``) are read for
#         random draws whenever ``stochastic`` is True **or** any of those keys is present
#         (so you can randomize only energy while keeping ``stochastic=False``).

#         **Energy / maintenance / fertility** (optional top-level keys in the same object):

#         - ``biomass_energy``, ``death_biomass_energy``: each may be a non-negative number
#           (Gaussian ``std`` around the prototype mean) or an object with optional ``mean`` /
#           ``MEAN`` and ``std`` / ``STD`` for the Gaussian draw of that conversion factor.
#         - ``maintenance_cost_range``, ``max_fertility_range`` (each ``[low, high]``).

#         **Full stochastic profile** (only when ``stochastic`` is True); other keys in the
#         same JSON are ignored for this block but do not cause errors:

#         - ``resilience``, ``size`` (biomass / surface tier lists; shared plants and animals),
#           ``death_biomass``, ``cycles``, ``strategies``,
#           ``colony``, ``stratum_config``, ``seed_dispersal`` (see previous documentation).
#         - ``max_individual_growth_bonus_range``: ``[low, high]`` for the uniform bonus
#           scaled by stage weight in ``max_individual_growth`` for fertile stages (default
#           ``[0, 0.55]``).

#         Deterministic prototype resize when ``stochastic`` is False (see
#         :meth:`example_default`); keys other than the energy group are then ignored.
#         """
#         stochastic = stochastic or generation_config is not None
        
#         cps = [int(x) for x in cycles_per_stages]
#         if not cps:
#             raise ValueError("cycles_per_stages must be non-empty.")

#         if food_type is None:
#             food_type = "0"

#         r = rng if rng is not None else random.Random()
#         proto = cls._prototype_3_stage()
#         gcfg = _parse_generation_config(generation_config)

#         if stochastic:
#             cps = _apply_cycles_stochastic(r, cps, gcfg.get("cycles"))
#             if any(x <= 0 for x in cps):
#                 raise ValueError(
#                     "Each cycles_per_stages entry must be a positive integer "
#                     "(after stochastic cycles transform)."
#                 )
#         else:
#             if any(x <= 0 for x in cps):
#                 raise ValueError("Each cycles_per_stages entry must be a positive integer.")

#         n = len(cps)
#         diet_rows = _build_diet_rows(n, stage_special_diets)

#         use_energy_random = stochastic or _config_has_energy_randomization(gcfg)
#         if use_energy_random:
#             (
#                 biomass_mean,
#                 biomass_std,
#                 death_mean,
#                 death_std,
#                 maintenance_cost_range,
#                 max_fertility_range,
#             ) = _energy_params_from_config(gcfg)
#             m_lo, m_hi = maintenance_cost_range
#             f_lo, f_hi = max_fertility_range
#             bio_base = (
#                 float(proto.biomass_to_energy_conversion_factor)
#                 if biomass_mean is None
#                 else float(biomass_mean)
#             )
#             death_base = (
#                 float(proto.death_biomass_to_energy_conversion_factor)
#                 if death_mean is None
#                 else float(death_mean)
#             )
#             biomass_e = _positive_gaussian(r, bio_base, float(biomass_std))
#             death_e = _positive_gaussian(r, death_base, float(death_std))
#             maintenance = [r.uniform(m_lo, m_hi) for _ in range(n)]
#             max_fert = [r.uniform(f_lo, f_hi) for _ in range(n)]
#         else:
#             biomass_e = float(proto.biomass_to_energy_conversion_factor)
#             death_e = float(proto.death_biomass_to_energy_conversion_factor)
#             maintenance = _resize_vector_float(proto.maintenance_cost, n)
#             max_fert = _resize_vector_float(proto.max_fertility, n)

#         if stochastic:
#             rm, rs = _parse_resilience_config(gcfg.get("resilience"), n)
#             resilience = [_clamp01(rm[i] + rs[i] * r.gauss(0.0, 1.0)) for i in range(n)]

#             plant_cfg = gcfg.get(cls.GenerationConfigKey.SIZE)
#             if not isinstance(plant_cfg, dict):
#                 plant_cfg = {}
#             default_prof = _default_plant_size_profile(n)
#             bio_raw = plant_cfg.get("biomass_classes")
#             surf_raw = plant_cfg.get("surface_classes")
#             if isinstance(bio_raw, (list, tuple)) and len(bio_raw) >= n:
#                 biomass_tiers = [max(0, min(4, int(bio_raw[i]))) for i in range(n)]
#             else:
#                 biomass_tiers = list(default_prof)
#             if isinstance(surf_raw, (list, tuple)) and len(surf_raw) >= n:
#                 surface_tiers = [max(0, min(4, int(surf_raw[i]))) for i in range(n)]
#             else:
#                 surface_tiers = list(default_prof)

#             biomass_per = [_sample_size_tier(r, biomass_tiers[s], _SIZE_BIOMASS_INTERVALS) for s in range(n)]
#             surface_per = [_sample_size_tier(r, surface_tiers[s], _SIZE_SURFACE_INTERVALS) for s in range(n)]

#             death_cfg = gcfg.get("death_biomass")
#             char_mat, d_fs, d_pfa, d_by_sz = _build_death_biomass_bundle(
#                 r, n, biomass_per, surface_per, biomass_tiers, death_cfg
#             )

#             st_cfg = gcfg.get("strategies")
#             if not isinstance(st_cfg, dict):
#                 st_cfg = {}
#             dr = st_cfg.get("defense_range", [0.0, 1.0])
#             rr = st_cfg.get("recruitment_range", [0.0, 1.0])
#             if not isinstance(dr, (list, tuple)) or len(dr) != 2:
#                 raise ValueError("strategies.defense_range must be [low, high].")
#             if not isinstance(rr, (list, tuple)) or len(rr) != 2:
#                 raise ValueError("strategies.recruitment_range must be [low, high].")
#             d_lo, d_hi = float(dr[0]), float(dr[1])
#             r_lo, r_hi = float(rr[0]), float(rr[1])
#             defense = _sample_matrix_uniform(r, n, 3, d_lo, d_hi)
#             recruitment = _sample_matrix_uniform(r, n, 3, r_lo, r_hi)

#             mg_bonus_key = cls.GenerationConfigKey.MAX_INDIVIDUAL_GROWTH_BONUS_RANGE
#             bonus_range = _max_individual_growth_bonus_range_from_config(gcfg, mg_bonus_key)
#             max_growth = _max_individual_growth_vector(r, max_fert, bonus_range)
#             max_density = [min(1e7, 1.0 / max(surface_per[i], 1e-12)) for i in range(n)]

#             stratum, opacity, min_light = _stratum_opacity_min_light(
#                 r, n, biomass_per, surface_per, gcfg.get("stratum_config")
#             )

#             colony = _colony_rate(r, gcfg.get("colony"))
#             seed_rate = _seed_dispersal(r, gcfg.get("seed_dispersal"))

#             bec = _resize_matrix(proto.best_environmental_conditions, n)
#         else:
#             resilience = _resize_vector_float(proto.resilience, n)
#             biomass_per = _resize_vector_float(proto.biomass_per_individual_amount, n)
#             surface_per = _resize_vector_float(proto.individual_occupied_surface, n)
#             char_mat = _resize_matrix(proto.characteristics_death_biomass, n)
#             d_fs = _resize_vector_float(proto.death_biomass_fraction_surface, n)
#             d_pfa = _resize_vector_float(proto.death_biomass_per_fraction_amount, n)
#             d_by_sz = _resize_matrix(proto.death_biomass_fraction_by_size, n)
#             defense = _resize_matrix(proto.defense_strategies, n)
#             recruitment = _resize_matrix(proto.recruitment_strategies, n)
#             max_growth = _resize_vector_float(proto.max_individual_growth, n)
#             max_density = _resize_vector_float(proto.max_density, n)
#             stratum = _resize_vector_int(proto.stratum, n)
#             opacity = _resize_vector_float(proto.opacity, n)
#             min_light = _resize_vector_float(proto.min_light, n)
#             colony = float(proto.colony_ability_rate)
#             seed_rate = float(proto.seed_dispersal_rate)
#             bec = _resize_matrix(proto.best_environmental_conditions, n)

#         return cls(
#             name=name,
#             food_type=food_type,
#             cycles_per_stages=cps,
#             diet_by_population_index=diet_rows,
#             biomass_to_energy_conversion_factor=biomass_e,
#             death_biomass_to_energy_conversion_factor=death_e,
#             maintenance_cost=maintenance,
#             max_fertility=max_fert,
#             resilience=resilience,
#             biomass_per_individual_amount=biomass_per,
#             individual_occupied_surface=surface_per,
#             death_biomass_fraction_surface=d_fs,
#             death_biomass_per_fraction_amount=d_pfa,
#             max_individual_growth=max_growth,
#             max_density=max_density,
#             opacity=opacity,
#             stratum=stratum,
#             min_light=min_light,
#             characteristics_death_biomass=char_mat,
#             death_biomass_fraction_by_size=d_by_sz,
#             best_environmental_conditions=bec,
#             defense_strategies=defense,
#             recruitment_strategies=recruitment,
#             colony_ability_rate=colony,
#             seed_dispersal_rate=seed_rate,
#         )

#     @classmethod
#     def example_default(cls) -> AutotrophSpecies:
#         """Deterministic spec: ``generate(..., stochastic=False)`` (niche.example.000 shape)."""
#         return cls.generate("AutotrophExampleA", [2, 3, 2], None, stochastic=False)


# DietByFoodTypeStage = List[Dict[str, Any]]
# DietByFoodType = List[DietByFoodTypeStage]


# class HeterotrophSpecies(LivingBeingSpecies):
#     """Heterotroph ``populations[].specie`` JSON (``class_type`` heterotroph)."""

#     GenerateParam: ClassVar[type] = AutotrophSpecies.GenerateParam
#     GenerationConfigKey: ClassVar[type] = AutotrophSpecies.GenerationConfigKey
#     StageSpecialDietParam: ClassVar[type] = AutotrophSpecies.StageSpecialDietParam

#     class HeterotrophConfigKey:
#         """Extra ``generation_config`` keys for :meth:`HeterotrophSpecies.generate`."""

#         PROSPECTING_RANGE = "prospecting_range"
#         PROSPECTING_ABILITY = "prospecting_ability"
#         INGESTION_RESIDUE = "ingestion_residue"
#         ASSIMILATION_EFFICIENCY_RANGE = "assimilation_efficiency_range"
#         PREY_LOCATION = "prey_location"
#         DIET_BY_FOOD_TYPE = "diet_by_food_type"

#     class SpecieField(LivingBeingSpecies.SpecieField):
#         """All ``specie`` JSON keys for a heterotroph (common + consumer-specific)."""

#         PROSPECTING_ABILITY = "prospecting_ability"
#         ASSIMILATION_EFFICIENCY = "assimilation_efficiency"
#         INGESTION_RESIDUE_FRACTION_BY_SIZE = "ingestion_residue_fraction_by_size"
#         DIET_BY_FOOD_TYPE = "diet_by_food_type"
#         PREY_LOCATION = "prey_location"

#     class_type: Union[int, str] = Field(
#         default="HETEROTROPH",
#         description="LivingBeingClassType; 1 or string HETEROTROPH (see JsonEnumNames).",
#     )
#     class_name: str = Field(default="Heterotroph", description="Display / snapshot class name.")

#     prospecting_ability: List[float] = Field(
#         default_factory=lambda: [50.0, 500.0, 400.0],
#         description="Also accepted as prospecting_ability_rate in C++ JSON.",
#     )
#     assimilation_efficiency: List[float] = Field(
#         default_factory=lambda: [0.6, 0.8, 0.8],
#     )
#     ingestion_residue_fraction_by_size: List[List[float]] = Field(
#         default_factory=lambda: [
#             [0.3, 0.3, 0.4],
#             [0.4, 0.3, 0.3],
#             [0.3, 0.3, 0.4],
#         ],
#     )
#     diet_by_food_type: Optional[DietByFoodType] = Field(
#         default=None,
#         description="Optional diet by food-type prefix (see Builders readDietByFoodType).",
#     )
#     prey_location: Optional[List[float]] = Field(
#         default=None,
#         description="Optional prey-location vector (Builders applyHeterotrophConsumerFields).",
#     )

#     @model_validator(mode="after")
#     def _heterotroph_lengths(self) -> HeterotrophSpecies:
#         n = len(self.cycles_per_stages)
#         if len(self.prospecting_ability) != n:
#             raise ValueError(
#                 f"prospecting_ability length {len(self.prospecting_ability)} != number of stages {n}"
#             )
#         if len(self.assimilation_efficiency) != n:
#             raise ValueError(
#                 f"assimilation_efficiency length {len(self.assimilation_efficiency)} != number of stages {n}"
#             )
#         if len(self.ingestion_residue_fraction_by_size) != n:
#             raise ValueError(
#                 "ingestion_residue_fraction_by_size row count "
#                 f"{len(self.ingestion_residue_fraction_by_size)} != number of stages {n}"
#             )
#         if self.diet_by_food_type is not None and len(self.diet_by_food_type) != n:
#             raise ValueError(
#                 f"diet_by_food_type length {len(self.diet_by_food_type)} != number of stages {n}"
#             )
#         for si, row in enumerate(self.ingestion_residue_fraction_by_size):
#             if not row:
#                 raise ValueError("ingestion_residue_fraction_by_size rows must be non-empty.")
#             srow = sum(float(x) for x in row)
#             if abs(srow - 1.0) > 1e-4:
#                 raise ValueError(
#                     "ingestion_residue_fraction_by_size row "
#                     f"{si} must sum to 1.0 (tolerance 1e-4), got {srow}"
#                 )
#         return self

#     @staticmethod
#     def _prototype_3_stage() -> HeterotrophSpecies:
#         """Internal 3-stage template aligned with ``example_default`` shape."""
#         return HeterotrophSpecies.example_default().model_copy(update={"name": "_template_"})

#     @classmethod
#     def generate_from(
#         cls,
#         process: "HeterotrophSpeciesProcess",
#         *,
#         rng: Optional[random.Random] = None,
#         n_bins: int = 3,
#     ) -> HeterotrophSpecies:
#         from .species_process import generate_heterotroph_from_process

#         return generate_heterotroph_from_process(process, rng=rng, n_bins=n_bins)

#     @classmethod
#     def generate(
#         cls,
#         name: str,
#         cycles_per_stages: Sequence[int],
#         food_type: str = "1",
#         stage_special_diets: Optional[Sequence[StageSpecialDiet]] = None,
#         *,
#         rng: Optional[random.Random] = None,
#         stochastic: bool = False,
#         generation_config: Optional[Union[str, dict[str, Any]]] = None,
#     ) -> HeterotrophSpecies:
#         """
#         Build a heterotroph spec for a given number of life-history stages.

#         Accepts the same ``generation_config`` keys as :meth:`AutotrophSpecies.generate`
#         for shared living-being stochastic blocks (energy, ``cycles``, ``resilience``,
#         ``size``, ``death_biomass``, ``strategies``, ``colony``,
#         ``max_individual_growth_bonus_range`` — autotroph-only
#         keys such as ``stratum_config`` / ``seed_dispersal`` are ignored.

#         Heterotroph-specific keys (optional):

#         - ``prospecting_range`` or ``prospecting_ability`` as ``[low, high]`` or
#           ``{"range": [1, 1000]}`` — random prospecting per stage; infertile stages are
#           capped below the minimum among fertile stages.
#         - ``assimilation_efficiency_range`` or ``{"range": [lo, hi]}`` — top stage uniform
#           in range, then each earlier stage subtracts ``Uniform(0, 0.5)`` from the next.
#         - ``ingestion_residue`` — same object shape as ``death_biomass`` (``bins`` required
#           when set; ``fraction`` stage→bin partial weights; rows filled to sum 1).
#         - ``prey_location`` — object ``{"mean": 0.5, "std": 0.3}`` overrides Gaussian draws
#           for one value per stage (clamped to ``[0,1]``).
#         - ``diet_by_food_type`` — if present literally in ``generation_config``, copied as
#           the diet matrix; otherwise ``None``.
#         """
#         cps = [int(x) for x in cycles_per_stages]
#         if not cps:
#             raise ValueError("cycles_per_stages must be non-empty.")

#         if food_type is None:
#             food_type = "1"

#         r = rng if rng is not None else random.Random()
#         proto = cls._prototype_3_stage()
#         gcfg = _parse_generation_config(generation_config)

#         if stochastic:
#             cps = _apply_cycles_stochastic(r, cps, gcfg.get("cycles"))
#             if any(x <= 0 for x in cps):
#                 raise ValueError(
#                     "Each cycles_per_stages entry must be a positive integer "
#                     "(after stochastic cycles transform)."
#                 )
#         else:
#             if any(x <= 0 for x in cps):
#                 raise ValueError("Each cycles_per_stages entry must be a positive integer.")

#         n = len(cps)
#         diet_rows = _build_diet_rows(n, stage_special_diets)

#         use_energy_random = stochastic or _config_has_energy_randomization(gcfg)
#         if use_energy_random:
#             (
#                 biomass_mean,
#                 biomass_std,
#                 death_mean,
#                 death_std,
#                 maintenance_cost_range,
#                 max_fertility_range,
#             ) = _energy_params_from_config(gcfg)
#             m_lo, m_hi = maintenance_cost_range
#             f_lo, f_hi = max_fertility_range
#             bio_base = (
#                 float(proto.biomass_to_energy_conversion_factor)
#                 if biomass_mean is None
#                 else float(biomass_mean)
#             )
#             death_base = (
#                 float(proto.death_biomass_to_energy_conversion_factor)
#                 if death_mean is None
#                 else float(death_mean)
#             )
#             biomass_e = _positive_gaussian(r, bio_base, float(biomass_std))
#             death_e = _positive_gaussian(r, death_base, float(death_std))
#             maintenance = [r.uniform(m_lo, m_hi) for _ in range(n)]
#             max_fert = [r.uniform(f_lo, f_hi) for _ in range(n)]
#         else:
#             biomass_e = float(proto.biomass_to_energy_conversion_factor)
#             death_e = float(proto.death_biomass_to_energy_conversion_factor)
#             maintenance = _resize_vector_float(proto.maintenance_cost, n)
#             max_fert = _resize_vector_float(proto.max_fertility, n)

#         if stochastic:
#             rm, rs = _parse_resilience_config(gcfg.get("resilience"), n)
#             resilience = [_clamp01(rm[i] + rs[i] * r.gauss(0.0, 1.0)) for i in range(n)]

#             plant_cfg = gcfg.get(cls.GenerationConfigKey.SIZE)
#             if not isinstance(plant_cfg, dict):
#                 plant_cfg = {}
#             default_prof = _default_plant_size_profile(n)
#             bio_raw = plant_cfg.get("biomass_classes")
#             surf_raw = plant_cfg.get("surface_classes")
#             if isinstance(bio_raw, (list, tuple)) and len(bio_raw) >= n:
#                 biomass_tiers = [max(0, min(4, int(bio_raw[i]))) for i in range(n)]
#             else:
#                 biomass_tiers = list(default_prof)
#             if isinstance(surf_raw, (list, tuple)) and len(surf_raw) >= n:
#                 surface_tiers = [max(0, min(4, int(surf_raw[i]))) for i in range(n)]
#             else:
#                 surface_tiers = list(default_prof)

#             biomass_per = [_sample_size_tier(r, biomass_tiers[s], _SIZE_BIOMASS_INTERVALS) for s in range(n)]
#             surface_per = [_sample_size_tier(r, surface_tiers[s], _SIZE_SURFACE_INTERVALS) for s in range(n)]

#             death_cfg = gcfg.get("death_biomass")
#             char_mat, d_fs, d_pfa, d_by_sz = _build_death_biomass_bundle(
#                 r, n, biomass_per, surface_per, biomass_tiers, death_cfg
#             )

#             st_cfg = gcfg.get("strategies")
#             if not isinstance(st_cfg, dict):
#                 st_cfg = {}
#             dr = st_cfg.get("defense_range", [0.0, 1.0])
#             rr = st_cfg.get("recruitment_range", [0.0, 1.0])
#             if not isinstance(dr, (list, tuple)) or len(dr) != 2:
#                 raise ValueError("strategies.defense_range must be [low, high].")
#             if not isinstance(rr, (list, tuple)) or len(rr) != 2:
#                 raise ValueError("strategies.recruitment_range must be [low, high].")
#             d_lo, d_hi = float(dr[0]), float(dr[1])
#             r_lo, r_hi = float(rr[0]), float(rr[1])
#             defense = _sample_matrix_uniform(r, n, 3, d_lo, d_hi)
#             recruitment = _sample_matrix_uniform(r, n, 3, r_lo, r_hi)

#             mg_bonus_key = cls.GenerationConfigKey.MAX_INDIVIDUAL_GROWTH_BONUS_RANGE
#             bonus_range = _max_individual_growth_bonus_range_from_config(gcfg, mg_bonus_key)
#             max_growth = _max_individual_growth_vector(r, max_fert, bonus_range)
#             max_density = [min(1e7, 1.0 / max(surface_per[i], 1e-12)) for i in range(n)]
#             colony = _colony_rate(r, gcfg.get("colony"))
#             bec = _resize_matrix(proto.best_environmental_conditions, n)

#             pcfg = gcfg.get(HeterotrophSpecies.HeterotrophConfigKey.PROSPECTING_RANGE)
#             if pcfg is None:
#                 pcfg = gcfg.get(HeterotrophSpecies.HeterotrophConfigKey.PROSPECTING_ABILITY)
#             prospecting = _prospecting_ability_vector(r, max_fert, pcfg)

#             assimilation = _assimilation_efficiency_vector(
#                 r, n, gcfg.get(HeterotrophSpecies.HeterotrophConfigKey.ASSIMILATION_EFFICIENCY_RANGE)
#             )

#             ing_cfg = gcfg.get(HeterotrophSpecies.HeterotrophConfigKey.INGESTION_RESIDUE)
#             ingestion = _build_ingestion_residue_fraction_by_size(
#                 r, n, biomass_tiers, ing_cfg
#             )

#             prey = _prey_location_vector(
#                 r, n, gcfg.get(HeterotrophSpecies.HeterotrophConfigKey.PREY_LOCATION)
#             )
#         else:
#             resilience = _resize_vector_float(proto.resilience, n)
#             biomass_per = _resize_vector_float(proto.biomass_per_individual_amount, n)
#             surface_per = _resize_vector_float(proto.individual_occupied_surface, n)
#             char_mat = _resize_matrix(proto.characteristics_death_biomass, n)
#             d_fs = _resize_vector_float(proto.death_biomass_fraction_surface, n)
#             d_pfa = _resize_vector_float(proto.death_biomass_per_fraction_amount, n)
#             d_by_sz = _resize_matrix(proto.death_biomass_fraction_by_size, n)
#             defense = _resize_matrix(proto.defense_strategies, n)
#             recruitment = _resize_matrix(proto.recruitment_strategies, n)
#             max_growth = _resize_vector_float(proto.max_individual_growth, n)
#             max_density = _resize_vector_float(proto.max_density, n)
#             colony = float(proto.colony_ability_rate)
#             bec = _resize_matrix(proto.best_environmental_conditions, n)
#             prospecting = _resize_vector_float(proto.prospecting_ability, n)
#             assimilation = _resize_vector_float(proto.assimilation_efficiency, n)
#             ingestion = _resize_matrix(proto.ingestion_residue_fraction_by_size, n)
#             prey = (
#                 _resize_vector_float(proto.prey_location, n)
#                 if proto.prey_location is not None
#                 else None
#             )

#         dbt: Optional[DietByFoodType] = None
#         if HeterotrophSpecies.HeterotrophConfigKey.DIET_BY_FOOD_TYPE in gcfg:
#             raw_dbt = gcfg[HeterotrophSpecies.HeterotrophConfigKey.DIET_BY_FOOD_TYPE]
#             if raw_dbt is not None:
#                 if not isinstance(raw_dbt, list):
#                     raise ValueError("generation_config.diet_by_food_type must be a list or null.")
#                 dbt = [list(stage) if isinstance(stage, list) else stage for stage in raw_dbt]  # type: ignore[list-item]
#                 if len(dbt) != n:
#                     raise ValueError(
#                         f"diet_by_food_type length {len(dbt)} != number of stages {n}"
#                     )

#         return cls(
#             name=name,
#             food_type=food_type,
#             cycles_per_stages=cps,
#             diet_by_population_index=diet_rows,
#             biomass_to_energy_conversion_factor=biomass_e,
#             death_biomass_to_energy_conversion_factor=death_e,
#             maintenance_cost=maintenance,
#             max_fertility=max_fert,
#             resilience=resilience,
#             biomass_per_individual_amount=biomass_per,
#             individual_occupied_surface=surface_per,
#             death_biomass_fraction_surface=d_fs,
#             death_biomass_per_fraction_amount=d_pfa,
#             max_individual_growth=max_growth,
#             max_density=max_density,
#             characteristics_death_biomass=char_mat,
#             death_biomass_fraction_by_size=d_by_sz,
#             best_environmental_conditions=bec,
#             defense_strategies=defense,
#             recruitment_strategies=recruitment,
#             colony_ability_rate=colony,
#             prospecting_ability=prospecting,
#             assimilation_efficiency=assimilation,
#             ingestion_residue_fraction_by_size=ingestion,
#             diet_by_food_type=dbt,
#             prey_location=prey,
#         )

#     @classmethod
#     def example_default(cls) -> HeterotrophSpecies:
#         """Shape aligned with ``config/niche.example.001.json`` heterotroph population."""
#         return cls(
#             name="HeterotrophExampleA",
#             food_type="0.1.0",
#             biomass_to_energy_conversion_factor=300.5,
#             death_biomass_to_energy_conversion_factor=19.5,
#             maintenance_cost=[0.1, 0.2, 0.3],
#             max_fertility=[0.0, 0.5, 0.4],
#             resilience=[0.6, 0.8, 0.5],
#             biomass_per_individual_amount=[0.01, 0.04, 0.05],
#             individual_occupied_surface=[0.01, 0.07, 0.07],
#             characteristics_death_biomass=[
#                 [0.2, 0.2, 0.1],
#                 [0.2, 0.4, 0.3],
#                 [0.3, 0.3, 0.3],
#             ],
#             death_biomass_fraction_surface=[0.01, 0.02, 0.05],
#             death_biomass_per_fraction_amount=[0.01, 0.04, 0.05],
#             death_biomass_fraction_by_size=[
#                 [0.4, 0.4, 0.2],
#                 [0.3, 0.4, 0.3],
#                 [0.2, 0.3, 0.5],
#             ],
#             best_environmental_conditions=[
#                 [0.2, 0.3, 0.3],
#                 [0.3, 0.4, 0.4],
#                 [0.1, 0.1, 0.1],
#             ],
#             cycles_per_stages=[2, 3, 5],
#             defense_strategies=[
#                 [0.2, 0.2, 0.2],
#                 [0.2, 0.3, 0.3],
#                 [0.2, 0.4, 0.2],
#             ],
#             recruitment_strategies=[
#                 [0.2, 0.2, 0.2],
#                 [0.3, 0.3, 0.3],
#                 [0.2, 0.4, 0.2],
#             ],
#             diet_by_population_index=[
#                 [
#                     {"population_index": 0, "min_stage": 0, "max_stage": 1},
#                     {"population_index": "PARENTAL_SUPPLY_TYPE", "min_stage": 0, "max_stage": 0},
#                 ],
#                 [[0, 0, 2]],
#                 [{"population_index": 0, "min_stage": 0, "max_stage": 2}],
#             ],
#             max_individual_growth=[0.718, 0.65, 0.4],
#             max_density=[20.0, 5.0, 3.0],
#             colony_ability_rate=0.0,
#             prospecting_ability=[50.0, 500.0, 400.0],
#             assimilation_efficiency=[0.6, 0.8, 0.8],
#             ingestion_residue_fraction_by_size=[
#                 [0.3, 0.3, 0.4],
#                 [0.4, 0.3, 0.3],
#                 [0.3, 0.3, 0.4],
#             ],
#         )
