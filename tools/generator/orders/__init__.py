"""Generation orders (command pattern) for niche/species builders."""

# from .base import DefaultOrder, Order, apply_field_defaults
# from .composite import EnergyFactor, LiteralDict, LiteralValue, SizeClassDict, SizeClasses
# from .context import GenerationContext
# from .death_biomass import (
#     BinSizeMap,
#     DeathBiomassDefaultCatalog,
#     DeathBiomassOrder,
#     Proportion,
# )
# from .defaults import FieldDefaultCatalog
# from .nested import FromContext, Partial, StructOrder, apply_subdefaults
# from .scalar import Gaussian, Literal, NutrientsFromTier, ProspectingSharpness, UniformRange
# from .stratum import (
#     DefaultStrataCount,
#     SlipLastValue,
#     StageHeightMap,
#     StratumConfigDefaultCatalog,
#     StratumConfigOrder,
# )
# from .vector import ExpandScalarToStages, LiteralVector, ProportionalDecay, VectorUniformRange
from .base import (
    Generator, 
    GeneratorFromValue, 
    LiteralGenerator, 
    StringGenerator, 
    NumberGeneratorByClass, 
    NumberArroundValueGenerator, 
    NumberFromUniformRangeGenerator, 
    NumberGaussianGenerator, 
    NumberGeneratorWithNoise, 
    NegativeNumberGeneratorWithNoise, 
    NegativeNumberGaussianGenerator, 
    NumberFromUniformRangeGenerator, 
    NegativeNumberFromUniformRangeGenerator, 
    NumberGaussianGenerator, 
    NegativeNumberGaussianGenerator
)
from .context import GenerationContext
from .vector import (
    VectorGenerator, 
    VectorWithMaxSumValueGenerator, 
    VectorIncrementalGenerator, 
    VectorUniformRangeGenerator, 
    VectorGaussianGenerator, 
    VectorCalculateFromOther, 
    VectorFromValueWithNoise, 
    VectorResolvedFromIndexGenerator, 
    VectorIntGeneratorByClass, 
    VectorArroundValueGenerator, 
    VectorFromValueWithNoise, 
    VectorExpandedFromValueGenerator 
)

__all__ = [
    # "BinSizeMap",
    # "DeathBiomassDefaultCatalog",
    # "DeathBiomassOrder",
    # "DefaultOrder",
    # "DefaultStrataCount",
    # "EnergyFactor",
    # "ExpandScalarToStages",
    # "FieldDefaultCatalog",
    # "FromContext",
    # "GenerationContext",
    # "Gaussian",
    # "Literal",
    # "LiteralDict",
    # "LiteralValue",
    # "LiteralVector",
    # "NutrientsFromTier",
    # "Order",
    # "Partial",
    # "Proportion",
    # "ProspectingSharpness",
    # "ProportionalDecay",
    # "SizeClassDict",
    # "SizeClasses",
    # "SlipLastValue",
    # "StageHeightMap",
    # "StratumConfigDefaultCatalog",
    # "StratumConfigOrder",
    # "StructOrder",
    # "UniformRange",
    # "VectorUniformRange",
    # "apply_field_defaults",
    # "apply_subdefaults",
    "Generator",
    "GeneratorFromValue",
    "LiteralGenerator",
    "StringGenerator",
    "NumberGeneratorByClass",
    "NumberArroundValueGenerator",
    "NumberFromUniformRangeGenerator",
    "NumberGaussianGenerator",
    "NumberGeneratorWithNoise",
    "NegativeNumberGeneratorWithNoise",
    "NegativeNumberGaussianGenerator",
    "NegativeNumberFromUniformRangeGenerator",
    "NumberGaussianGenerator",
    "NegativeNumberGaussianGenerator",
    "GenerationContext",
    "VectorGenerator",
    "VectorWithMaxSumValueGenerator",
    "VectorIncrementalGenerator",
    "VectorUniformRangeGenerator",
    "VectorGaussianGenerator",
    "VectorCalculateFromOther",
    "VectorFromValueWithNoise",
    "VectorResolvedFromIndexGenerator",
    "VectorIntGeneratorByClass",
    "VectorArroundValueGenerator",
    "VectorFromValueWithNoise",
    "VectorExpandedFromValueGenerator",
]
