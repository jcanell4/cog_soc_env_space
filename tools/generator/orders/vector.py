"""Vector-valued generation orders."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict, List, Sequence, Union, Callable
from numbers import Number

# from .base import Order, Generator
from .base import Generator, NumberFromUniformRangeGenerator
from .context import GenerationContext
# from .scalar import Literal, UniformRange

#----------------------------------------new version
@dataclass(frozen=True)
class VectorGenerator(Generator[List[Union[float, int]]]):
    generator: Generator[float]
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = [self.generator.generate(ctx) for _ in range(n)]
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

@dataclass(frozen=True)
class VectorWithMaxSumValueGenerator(Generator[List[Union[float, int]]]):
    max_sum_value: Union[Generator[float], float]
    length: int | None = None
    length_from: str | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = []
        sum = 0
        a_max_sum_value = self.max_sum_value.generate(ctx) if isinstance(self.max_sum_value, Generator) else self.max_sum_value
        for i in range(n-1):
            max = a_max_sum_value * (i+1)/n
            value = NumberFromUniformRangeGenerator(min=sum, max=max).generate(ctx)
            ret.append(value - sum)
            sum += ret[i]
        ret.append(a_max_sum_value - sum)
        if self.integer:
            ret = [round(x) for x in ret]
        return ret


@dataclass(frozen=True)
class VectorIncrementalGenerator(Generator[List[Union[float, int]]]):
    initial_value: Union[Generator[float], float]
    increment: Union[Generator[float], float]
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        def get_increment(increment: float):
            if isinstance(self.increment, Generator):
                return self.increment.generate(ctx) + increment
            return float(self.increment) + increment
        a_initial_value = self.initial_value.generate(ctx) if isinstance(self.initial_value, Generator) else float(self.initial_value)
        increment = 0
        ret = [a_initial_value + (increment := get_increment(increment)) for _ in range(n)]
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret


@dataclass(frozen=True)
class VectorUniformRangeGenerator(Generator[List[Union[float, int]]]):
    min: float
    max: float
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        lo, hi = float(self.min), float(self.max)
        if lo > hi:
            lo, hi = hi, lo
        ret = [float(ctx.rng.uniform(lo, hi)) for _ in range(n)]
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret


@dataclass(frozen=True)
class VectorGaussianGenerator(Generator[List[Union[float, int]]]):
    mean: float
    std: float
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = [ctx.rng.gauss(self.mean, self.std) for _ in range(n)]
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret


@dataclass(frozen=True)
class VectorCalculateFromOther(Generator[List[Union[float, int]]]):
    lambda_function: Callable[[float], float]
    field_name: str
    other: str = None
    length: int | None = None
    length_from: str | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        if self.other is None:
            x = ctx.get_value(self.field_name)
        else:
            x = getattr(self.other, self.field_name)
        if x is None:
            raise ValueError(f"Field {self.field_name} not found in {'context' if self.other is None else 'other'}")
        if not isinstance(x, List):
            raise ValueError(f"Field {self.field_name} is not a list")
        ret = [self.lambda_function(xi) for xi in x]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

@dataclass(frozen=True)
class VectorNumberGeneratorByClass(Generator[List[Union[float, int]]]):
    class_range: tuple[float, float] = None
    class_range_name: str = None
    class_index: Union[int, Generator[int]] = None
    class_index_from: str | None = None
    subclass_index: int = -1
    stochastic_generator: str = "R"
    length: int | None = None
    length_from: str | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[float]:
        if self.class_index is None and self.class_index_from is None:
            raise ValueError("class_index or class_index_from is required")
        class_index = self.class_index
        if class_index is None:
            class_index = int(ctx.get_value(self.class_index_from, -1))
        if isinstance(class_index, Generator):
            class_index = class_index.generate(ctx)
        if self.class_range is None and self.class_range_name is None:
            raise ValueError("class_range or class_range_name is required")
        if self.class_range is None:
            class_ranges_for_classes = ctx.get_value(self.class_range_name)
            if class_ranges_for_classes is None:
                raise ValueError(f"Class range {self.class_range_name} not found")
            class_range = class_ranges_for_classes
        else:
            class_range = self.class_range
        if class_index != -1:
            class_range = class_range[class_index]
        if self.subclass_index != -1:
            class_range = class_range[self.subclass_index]  
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = []
        if self.stochastic_generator == "G":
            for i in range(n):
                mean = ((class_range[0]+class_range[1])/2)*(i+1)/n
                std = ((class_range[1]-class_range[0])/3)*(i+1)/n
                ret.append(max(class_range[0], min(class_range[1], ctx.rng.gauss(mean, std))))
        else:
            for i in range(n):
                min_value = class_range[0] *(i+1)/n
                max_value = class_range[1] *(i+1)/n
                ret.append(ctx.rng.uniform(min_value, max_value))
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

@dataclass(frozen=True)
class VectorIntGeneratorByClass(VectorNumberGeneratorByClass):
    integer: bool = True

    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[int]:
        return [
            int(round(x))
            for x in VectorNumberGeneratorByClass(
                class_range=self.class_range,
                class_range_name=self.class_range_name,
                class_index=self.class_index,
                class_index_from=self.class_index_from,
                subclass_index=self.subclass_index,
                stochastic_generator=self.stochastic_generator,
                length=self.length,
                length_from=self.length_from,
                integer=True,
            ).generate(ctx)
        ]


@dataclass(frozen=True)
class VectorArroundValueGenerator(Generator[List[Union[float, int]]]):
    value: float
    ratio: float | None = None
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[Union[float, int]]:
        ratio = 0.1 if self.ratio is None else self.ratio
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = [self.value + ctx.rng.random() * ratio * self.value] * n
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

@dataclass(frozen=True)
class VectorFromValueWithNoise(Generator[List[Union[float, int]]]):
    value: float
    noise: float
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[Union[float, int]]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = [self.value + ctx.rng.random() * self.noise] * n
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

@dataclass(frozen=True)
class VectorExpandedFromValueGenerator(Generator[List[Union[float, int]]]):
    value: float
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[Union[float, int]]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = [self.value] * n
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

@dataclass(frozen=True)
class VectorResolvedFromIndexGenerator(Generator[List[Union[float, int]]]):
    values: Union[Generator[float], float]
    values_form_index: Dict[int, Union[Generator[float], float]] = None
    length: int | None = None
    length_from: str | None = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[Union[float, int]]:
        a_length_from = self.length_from
        if a_length_from is None:
            if self.length is None:
                raise ValueError("length or length_from is required")
            a_length_from = ""
        n = int(ctx.resolved.get(a_length_from, getattr(ctx, a_length_from, self.length)))
        ret = [self.values] * n
        for index, value in self.values_form_index.items():
            ret[index] = value
        ret = [v.generate(ctx) if isinstance(v, Generator) else v for v in ret]
        if self.clamp is not None:
            ret = [max(self.clamp[0], min(self.clamp[1], x)) for x in ret]
        if self.integer:
            ret = [round(x) for x in ret]
        return ret

# #----------------------------------------old version
# @dataclass(frozen=True)
# class LiteralVector(Order[List[float]]):
#     values: Sequence[float]

#     def resolve(self, ctx: GenerationContext) -> List[float]:
#         return [float(x) for x in self.values]


# @dataclass(frozen=True)
# class VectorUniformRange(Order[List[float]]):
#     low: float
#     high: float
#     length: int | None = None
#     length_from: str = "n_factors"

#     def resolve(self, ctx: GenerationContext) -> List[float]:
#         n = self.length
#         if n is None:
#             n = int(ctx.resolved.get(self.length_from, getattr(ctx, self.length_from, 4)))
#         lo, hi = float(self.low), float(self.high)
#         if lo > hi:
#             lo, hi = hi, lo
#         if ctx.stochastic:
#             return [float(ctx.rng.uniform(lo, hi)) for _ in range(n)]
#         mid = 0.5 * (lo + hi)
#         return [mid] * n


# @dataclass(frozen=True)
# class ProportionalDecay(Order[List[float]]):
#     """return_rate[i] = r0 * (n_bins - i) / n_bins."""

#     r0: Union[Order[float], float] = 0.7
#     r0_range: tuple[float, float] | None = None

#     def resolve(self, ctx: GenerationContext) -> List[float]:
#         n_bins = ctx.n_bins
#         if n_bins < 1:
#             raise ValueError("n_bins must be >= 1")
#         if self.r0_range is not None:
#             r0_order: Order[float] = UniformRange(self.r0_range[0], self.r0_range[1])
#         elif isinstance(self.r0, Order):
#             r0_order = self.r0
#         else:
#             r0_order = Literal(float(self.r0))
#         if ctx.stochastic:
#             r0 = r0_order.resolve(ctx)
#         else:
#             r0 = 0.7 if self.r0_range is not None else (
#                 float(self.r0) if not isinstance(self.r0, Order) else 0.7
#             )
#         return [max(0.0, r0 * (n_bins - i) / n_bins) for i in range(n_bins)]


# @dataclass(frozen=True)
# class ExpandScalarToStages(Order[List[float]]):
#     """Repeat one scalar across n_stages."""

#     value: Union[Order[float], float]

#     def resolve(self, ctx: GenerationContext) -> List[float]:
#         n = ctx.n_stages
#         if n < 1:
#             raise ValueError("n_stages must be >= 1 for ExpandScalarToStages")
#         if isinstance(self.value, Order):
#             v = self.value.resolve(ctx)
#         else:
#             v = float(self.value)
#         return [v] * n
