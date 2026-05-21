"""Order protocol (command pattern) for field generation."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Generic, Mapping, Optional, Sequence, TypeVar, Union, List
from numbers import Number
from dataclasses import dataclass

from .context import GenerationContext
# from .defaults import FieldDefaultCatalog

T = TypeVar("T")
R = TypeVar("R", bound=Number)

#----------------------------------------new version
class Generator(ABC, Generic[T]):
    @abstractmethod
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> T:
        ...


@dataclass(frozen=True)
class GeneratorFromValue(Generator[T], Generic[T, R]):
    @abstractmethod
    def generate(self, ctx: GenerationContext, value: R, *args, **kwargs) -> T:
        ...
  
@dataclass(frozen=True)
class LiteralGenerator(Generator[T]):
    value: T
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> T:
        return self.value

@dataclass(frozen=True)
class StringGenerator(Generator[str]):
    base: Union[Generator[str], str]
    prefix: Union[Generator[str], str, Generator[List[str]], List[str]] = ""
    suffix: Union[Generator[str], str] = ""
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> str:
        a_base = self.base.generate(ctx) if isinstance(self.base, Generator) else str(self.base)
        a_prefix = self.prefix.generate(ctx) if isinstance(self.prefix, Generator) else str(self.prefix)
        a_suffix = self.suffix.generate(ctx) if isinstance(self.suffix, Generator) else str(self.suffix)
        if isinstance(a_base, str):
            return f"{a_prefix}{a_base}{a_suffix}"
        return f"{a_prefix}{ctx.rng.choice(a_base)}{a_suffix}"

@dataclass(frozen=True)
class NumberGeneratorByClass(Generator[List[T]]):
    class_range_name: str
    class_index: int
    subclass_index: int = -1
    stochastic_generator: str = "R"
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> List[T]:
        class_range = ctx.get_value(self.class_range_name)
        if class_range is None:
            raise ValueError(f"Class range {self.class_range_name} not found")
        if self.subclass_index != -1:
            class_range = class_range[self.subclass_index]            
        if self.stochastic_generator == "G":
            mean = (class_range[self.class_index][0]+class_range[self.class_index][1])/2
            std = (class_range[self.class_index][1]-class_range[self.class_index][0])/3
            ret = NumberGaussianGenerator(mean, std, clamp=(class_range[self.class_index][0], class_range[self.class_index][1])).generate(ctx)
        else:
            ret = NumberFromUniformRangeGenerator(class_range[self.class_index][0], class_range[self.class_index][1]).generate(ctx)
        if self.integer:
            ret = round(ret)
        return ret


@dataclass(frozen=True)
class NumberArroundValueGenerator(GeneratorFromValue[float, Union[Generator[float], float]]):
    value: Union[Generator[float], float]
    ratio: Optional[Union[Generator[float], float]] = None
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, value: Union[Generator[float], float] = None, *args, **kwargs) -> float:
        if self.ratio is None:
            self.ratio = 0.1
        if value is None:
            if self.value is None:
                raise ValueError("Value is not set")
            value = self.value
        a_ratio = self.ratio.generate(ctx) if isinstance(self.ratio, Generator) else float(self.ratio)
        a_value = value.generate(ctx) if isinstance(value, Generator) else float(self.value)
        ret = a_value + a_ratio * a_value
        if self.clamp is not None:
            ret = max(self.clamp[0], min(self.clamp[1], ret))
        if self.integer:
            ret = round(ret)
        return ret

class NegativeNumberArroundValueGenerator(NumberArroundValueGenerator):
    def generate(self, ctx: GenerationContext, value: Union[Generator[float], float], *args, **kwargs) -> float:
        ret = super().generate(ctx, value, *args, **kwargs)
        return -ret if ret > 0 else ret

@dataclass(frozen=True)
class NumberGeneratorWithNoise(GeneratorFromValue[Union[float, int], Union[Generator[Union[float, int]], Union[float, int]]]):
    value: Union[Generator[Union[float, int]], Union[float, int]]
    noise: Union[Generator[float], float]
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, value: Union[Generator[Union[float, int]], Union[float, int]]=None, *args, **kwargs) -> Union[float, int]:
        a_noise = self.noise.generate(ctx) if isinstance(self.noise, Generator) else float(self.noise)
        if value is None:
            if self.value is None:
                raise ValueError("Value is not set")
            value = self.value
        value = value.generate(ctx) if isinstance(self.value, Generator) else float(value)
        v_noise =ctx.rng.random() * a_noise
        ret = value + v_noise
        if self.clamp is not None:
            ret = max(self.clamp[0], min(self.clamp[1], ret))
        if self.integer:
            ret = round(ret)
        return ret

class NegativeNumberGeneratorWithNoise(NumberGeneratorWithNoise):
    def generate(self, ctx: GenerationContext, value: Union[Generator[Union[float, int]], Union[float, int]]=None, *args, **kwargs) -> Union[float, int]:
        ret = super().generate(ctx, value, *args, **kwargs)
        return -ret if ret > 0 else ret

@dataclass(frozen=True)
class NumberGaussianGenerator(GeneratorFromValue[Union[float, int], Union[Generator[Union[float, int]], Union[float, int]]]):
    mean: float
    std: float      
    clamp: tuple[float, float] | None = None
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> float:
        ret = ctx.rng.gauss(self.mean, self.std)
        if self.clamp is not None:
            ret = max(self.clamp[0], min(self.clamp[1], ret))
        if self.integer:
            ret = round(ret)
        return ret

class NegativeNumberGaussianGenerator(NumberGaussianGenerator):
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> float:
        ret = super().generate(ctx, *args, **kwargs)
        return -ret if ret > 0 else ret


@dataclass(frozen=True)
class NumberFromUniformRangeGenerator(Generator[float]):
    min: Union[Generator[float], float]
    max: Union[Generator[float], float]
    integer: bool = False
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> float:
        a_lo = self.min.generate(ctx) if isinstance(self.min, Generator) else float(self.min)
        a_hi = self.max.generate(ctx) if isinstance(self.max, Generator) else float(self.max)
        if a_lo > a_hi:
            a_lo, a_hi = a_hi, a_lo
        ret = float(ctx.rng.uniform(a_lo, a_hi))
        if self.integer:
            ret = round(ret)
        return ret


class NegativeNumberFromUniformRangeGenerator(NumberFromUniformRangeGenerator):
    def generate(self, ctx: GenerationContext, *args, **kwargs) -> float:
        ret = super().generate(ctx, *args, **kwargs)
        return -ret if ret > 0 else ret


# #----------------------------------------old version

# class Order(ABC, Generic[T]):
#     @abstractmethod
#     def resolve(self, ctx: GenerationContext) -> T:
#         ...

#     def set_defaults(
#         self,
#         catalog: Union[FieldDefaultCatalog[T], Mapping[str, Order[T]], Sequence[Order[T]]],
#         *,
#         key: str | int | None = None,
#     ) -> Order[T]:
#         """Inject catalog for :class:`DefaultOrder`; no-op for concrete orders."""
#         return self


# class DefaultOrder(Order[T]):
#     """Resolve via a :class:`FieldDefaultCatalog` entry (by key or active)."""

#     def __init__(self, key: str | int | None = None) -> None:
#         self._key = key
#         self._catalog: Optional[FieldDefaultCatalog[T]] = None

#     def set_defaults(
#         self,
#         catalog: Union[FieldDefaultCatalog[T], Mapping[str, Order[T]], Sequence[Order[T]]],
#         *,
#         key: str | int | None = None,
#     ) -> DefaultOrder[T]:
#         if isinstance(catalog, FieldDefaultCatalog):
#             self._catalog = catalog
#         else:
#             self._catalog = FieldDefaultCatalog(catalog, active=key if key is not None else self._key)
#         if key is not None:
#             self._key = key
#         return self

#     def resolve(self, ctx: GenerationContext) -> T:
#         if self._catalog is None:
#             raise ValueError(
#                 "DefaultOrder: catalog not set; call apply_defaults(process) or set_defaults()"
#             )
#         pick_key = self._key
#         if pick_key is None and self._catalog.active is not None:
#             pick_key = self._catalog.active
#         order = self._catalog.pick(pick_key)
#         return order.resolve(ctx)


# def apply_field_defaults(
#     fields: Mapping[str, Order],
#     catalogs: Mapping[str, FieldDefaultCatalog],
# ) -> None:
#     for name, order in fields.items():
#         if isinstance(order, DefaultOrder) and name in catalogs:
#             order.set_defaults(catalogs[name])
