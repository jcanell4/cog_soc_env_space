"""Example: typed GenerationProcess API (orders + defaults)."""

from __future__ import annotations

import json
import sys
from pathlib import Path
import random

_ROOT = Path(__file__).resolve().parents[3]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from tools.generator.orders.context import GenerationContext
from tools.generator.models import (
    AutotrophGenerator,
    PopulationGenerator,
    HeterotrophGenerator,
    NicheGenerator,
    # AutotrophProfileProcess,
    # AutotrophSpecies,
    # AutotrophSpeciesProcess,
    # PopulationGen,
    # PopulationGenProcess,
    # HeterotrophProfileProcess,
    # HeterotrophSpecies,
    # HeterotrophSpeciesProcess,
    # NicheGen,
    # NicheGenProcess,
)
# from tools.generator.orders import (
#     BinSizeMap,
#     DeathBiomassOrder,
#     FromContext,
#     NutrientsFromTier,
#     Partial,
#     ProspectingSharpness,
#     Proportion,
#     SlipLastValue,
#     StageHeightMap,
#     StratumConfigOrder,
# )
# from tools.generator.orders.composite import EnergyFactor, LiteralValue, SizeClasses
# from tools.generator.orders.scalar import Gaussian, Literal
# from tools.generator.orders.vector import LiteralVector

from tools.generator.orders.base import(
    NumberGaussianGenerator,
    NumberArroundValueGenerator,
    NumberGeneratorWithNoise,
    NumberFromUniformRangeGenerator,
    NegativeNumberGaussianGenerator,
    NegativeNumberArroundValueGenerator,
    NegativeNumberGeneratorWithNoise,
    NegativeNumberFromUniformRangeGenerator
)
from tools.generator.orders.vector import( 
    VectorGaussianGenerator,
    VectorExpandedFromValueGenerator,
    VectorIncrementalGenerator,
    VectorUniformRangeGenerator
)

ctx = GenerationContext(
    n_bins=4,
    n_conditions=5,
    n_limiting_factors=6,
    n_stratums=3,
    adaptation_noise= 0.5,
    rng=random.Random(42),
)

autotroph_a = AutotrophGenerator(
    name="little plant",
    food_type="0.0.0",
    maintenance_cost=VectorIncrementalGenerator(initial_value=NumberFromUniformRangeGenerator(min=0.05, max=0.1), 
                                        increment=NumberArroundValueGenerator(value=0.01, ratio=0.1, clamp=(0.0, 0.1)),
                                        length_from=GenerationContext.n_stages_name),
    size_class=0,
    n_stages=2,
)

autotroph_b = AutotrophGenerator(
    name="little tree",
    food_type="0.0.1",
    maintenance_cost=VectorIncrementalGenerator(initial_value=NumberFromUniformRangeGenerator(min=0.05, max=0.1), 
                                        increment=NumberArroundValueGenerator(value=0.01, ratio=0.1, clamp=(0.0, 0.1)),
                                        length_from=GenerationContext.n_stages_name),
    size_class=1,
    n_stages=3,
)

herbivore_a = HeterotrophGenerator(
    name="little herbivore",
    food_type="0.1.0",
    maintenance_cost=VectorIncrementalGenerator(initial_value=NumberFromUniformRangeGenerator(min=0.05, max=0.1), 
                                        increment=NumberArroundValueGenerator(value=0.01, ratio=0.1, clamp=(0.0, 0.1)),
                                        length_from=GenerationContext.n_stages_name),
    size_class=0,
    n_stages=5,
    diet_by_food_type=[
        [("0.0", 0, 1, 0)],     
        [("0.0", 0, 4, 0)], 
        [("0.0", 0, 4, 0)], 
        [("0.0", 0, 4, 0)], 
        [("0.0", 0, 4, 0)], 
    ]
)

niche_new = NicheGenerator(
    surface=NumberGaussianGenerator(mean=10000.0, std=1000.0),
    nutrients=NumberGaussianGenerator(mean=10000.0, std=1000.0),
    return_rate=VectorIncrementalGenerator(initial_value=NumberGeneratorWithNoise(value=0.8, noise=0.2), 
                                        increment=NegativeNumberGeneratorWithNoise(value=0.5, noise=0.4),
                                        length_from=GenerationContext.n_bins_name,
                                        clamp=(0.0, 1.0)),
    conditions=VectorUniformRangeGenerator(min=0.0, max=1.0, length_from=GenerationContext.n_conditions_name),
    limiting_factors=VectorUniformRangeGenerator(min=0.0, max=1.0, length_from=GenerationContext.n_limiting_factors_name),
    prospecting_scan_sharpness=NumberGaussianGenerator(mean=0.85, std=0.3, clamp=(0.0, 1.0)),
    populations=[
        PopulationGenerator(name="little plant a", biomass=[1000.0, 2000.0, 3000.0], death_biomass=[1000, 1000, 1000], specie=autotroph_a),
        PopulationGenerator(name="little plant b", biomass=[3000.0, 1000.0, 1000.0], death_biomass=[1000, 1000, 1000], specie=autotroph_a),
        PopulationGenerator(name="little tree a", biomass=[100.0, 200.0, 200.0, 100.0], death_biomass=[1000, 1000, 1000, 1000], specie=autotroph_b),
        PopulationGenerator(name="herbivore a", biomass=[100.0, 200.0, 200.0, 100.0], death_biomass=[1000, 1000, 1000, 1000], specie=herbivore_a),
    ],
).generate(ctx)



# niche = NicheGen.generate_from(
#     NicheGenProcess(
#         surface=10000.0,
#         n_bins=4,
#         nutrients=NutrientsFromTier(tier=3),
#         prospecting_scan_sharpness=ProspectingSharpness(mean=0.85, std=0.3),
#         stochastic=True,
#     )
# )

# plant_profile = AutotrophProfileProcess(
#     biomass_energy=EnergyFactor(std=0.1),
#     death_biomass_energy=EnergyFactor(std=0.2),
#     maintenance_cost_range=LiteralVector([0.05, 0.3]),
#     max_fertility_range=LiteralVector([0.5, 0.9]),
#     size=SizeClasses([0, 0, 1]),
#     death_biomass=DeathBiomassOrder(
#         bins=FromContext("n_bins"),
#         size=BinSizeMap(Partial({1: 0, 2: 1})),
#         fraction=Proportion(Partial({0: {0: 0.6}})),
#     ),
#     stratum_config=StratumConfigOrder(
#         n_strata=Literal(2),
#         stage_height_class=StageHeightMap(SlipLastValue(Partial({0: 0, 1: 0, 2: 1}))),
#     ),
#     max_individual_growth_bonus_range=LiteralVector([0.0, 0.2]),
# )

# tree_profile = AutotrophProfileProcess(
#     biomass_energy=EnergyFactor(mean=200.0, std=0.3),
#     death_biomass_energy=EnergyFactor(std=0.2),
#     maintenance_cost_range=LiteralVector([0.05, 0.1]),
#     max_fertility_range=LiteralVector([0.5, 0.7]),
#     size=SizeClasses([0, 1, 2, 3]),
#     death_biomass=DeathBiomassOrder(
#         bins=FromContext("n_bins"),
#         size=BinSizeMap(Partial({1: 0, 2: 1})),
#         fraction=Proportion(Partial({0: {0: 0.6}})),
#     ),
#     stratum_config=StratumConfigOrder(
#         n_strata=Literal(2),
#         stage_height_class=StageHeightMap(SlipLastValue(Partial({0: 0, 1: 0, 2: 1}))),
#     ),
# )

# herbivore_profile = HeterotrophProfileProcess(
#     biomass_energy=EnergyFactor(mean=250.0, std=0.3),
#     death_biomass_energy=EnergyFactor(std=0.2),
#     maintenance_cost_range=LiteralVector([0.1, 0.2]),
#     max_fertility_range=LiteralVector([0.5, 0.8]),
#     size=SizeClasses([0, 0, 1]),
#     death_biomass=DeathBiomassOrder(
#         bins=FromContext("n_bins"),
#         size=BinSizeMap(Partial({1: 0, 2: 1})),
#         fraction=Proportion(Partial({0: {0: 0.6}})),
#     ),
#     prospecting_range=LiteralVector([10.0, 50.0]),
#     assimilation_efficiency_range=LiteralVector([0.4, 0.6]),
#     ingestion_residue=DeathBiomassOrder(
#         bins=FromContext("n_bins"),
#         fraction=Proportion(Partial({1: {1: 0.3}, 2: {2: 0.3}})),
#     ),
#     prey_location=Gaussian(mean=0.5, std=0.3),
#     diet_by_food_type=LiteralValue(
#         [
#             [{"food_type": "0.0", "min_stage": 0, "max_stage": 1, "matter_type": 0}],
#             [{"food_type": "0.0", "min_stage": 0, "max_stage": 1, "matter_type": 0}],
#             [{"food_type": "0.0.1", "min_bin": 2, "max_bin": 2, "matter_type": 1}],
#         ]
#     ),
# )

# populations = []
# autotroph_a = AutotrophSpecies.generate_from(
#     AutotrophSpeciesProcess(
#         name="little plant a",
#         cycles_per_stages=[3, 1, 2],
#         food_type="0.0.0.0",
#         profile=plant_profile,
#         stochastic=True,
#     ),
#     n_bins=niche.bins,
# )
# populations.append(
#     PopulationGen.generate_from(
#         PopulationGenProcess(specie=autotroph_a, biomass=10000.0, death_biomass=[0.0, 0.0, 0.0])
#     )
# )
# autotroph_b = AutotrophSpecies.generate_from(
#     AutotrophSpeciesProcess(
#         name="little plant b",
#         cycles_per_stages=[3, 1, 2],
#         food_type="0.0.0.0",
#         profile=plant_profile,
#         stochastic=True,
#     ),
#     n_bins=niche.bins,
# )
# populations.append(
#     PopulationGen.generate_from(
#         PopulationGenProcess(specie=autotroph_b, biomass=10000.0, death_biomass=[0.0, 0.0, 0.0])
#     )
# )
# autotroph_c = AutotrophSpecies.generate_from(
#     AutotrophSpeciesProcess(
#         name="little tree a",
#         cycles_per_stages=[2, 3, 5, 10],
#         food_type="0.0.1.0",
#         profile=tree_profile,
#         stochastic=True,
#     ),
#     n_bins=niche.bins,
# )
# populations.append(
#     PopulationGen.generate_from(
#         PopulationGenProcess(
#             specie=autotroph_c,
#             biomass=10000.0,
#             death_biomass=[0.0, 0.0, 0.0, 0.0],
#         )
#     )
# )

# herbivore_a = HeterotrophSpecies.generate_from(
#     HeterotrophSpeciesProcess(
#         name="little herbivore a",
#         cycles_per_stages=[2, 3, 5],
#         food_type="0.1.0.0",
#         profile=herbivore_profile,
#         stochastic=True,
#     ),
#     n_bins=niche.bins,
# )
# populations.append(
#     PopulationGen.generate_from(
#         PopulationGenProcess(
#             specie=herbivore_a,
#             biomass=[500.0, 800.0, 1200.0],
#             death_biomass=[0.0, 0.0, 0.0],
#         )
#     )
# )

# niche.populations = populations

niche_data = json.dumps(niche_new.to_data_dict(), indent=2)
print(niche_data)

_out = Path(__file__).resolve().parent.parent / "niche.json"
with open(_out, "w", encoding="utf-8") as f:
    f.write(niche_data)
