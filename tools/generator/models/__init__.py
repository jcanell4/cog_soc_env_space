"""Pydantic models for JSON the C++ builders accept (see include/Builders.h)."""

# from .population import PopulationGen, PopulationGenerator
# from .population_process import PopulationGenProcess
# from .livingbeing import AutotrophSpecies, HeterotrophSpecies, LivingBeingSpecies, AutotrophGenerator, HeterotrophGenerator
# from .niche import NicheConf, NicheGen, NicheGenerator
# from .niche_process import NicheGenProcess
# from .species_process import (
#     AutotrophProfileProcess,
#     AutotrophSpeciesProcess,
#     HeterotrophProfileProcess,
#     HeterotrophSpeciesProcess,
# )
from .population import PopulationGenerator, Population
from .livingbeing import AutotrophGenerator, HeterotrophGenerator, Autotroph, Heterotroph
from .niche import NicheGenerator, Niche

__all__ = [
    # "AutotrophProfileProcess",
    # "AutotrophSpecies",
    # "AutotrophSpeciesProcess",
    # "PopulationGen",
    # "PopulationGenProcess",
    # "HeterotrophProfileProcess",
    # "HeterotrophSpecies",
    # "HeterotrophSpeciesProcess",
    # "LivingBeingSpecies",
    # "NicheConf",
    # "NicheGen",
    # "NicheGenProcess",
    "NicheGenerator",
    "PopulationGenerator",
    "AutotrophGenerator",
    "HeterotrophGenerator",
    "Autotroph",
    "Heterotroph",
    "Niche",
    "Population",
]
