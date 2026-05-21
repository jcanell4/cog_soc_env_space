"""
Pydantic generator for niche ``initial_data.data`` (environment fields consumed by C++).

See ``NicheBuilder::fromJson`` / ``include/Niche.h`` for accepted snapshot fields.
"""

from __future__ import annotations

import json
import random
from typing import TYPE_CHECKING, Any, Dict, List, Optional, Union
from dataclasses import dataclass, asdict

from ..orders.base import Generator
from ..orders.context import GenerationContext
from .population import Population, PopulationGenerator

#---------------------------------------- New version
@dataclass(frozen=True)
class Niche:
    surface: float
    nutrients: float
    return_rate: List[float]
    conditions: List[float]
    limiting_factors: List[float]
    prospecting_scan_sharpness: float
    populations: List[Population]

    def to_data_dict(self) -> dict[str, Any]:
        """Serialize for merging under ``initial_data['data']``."""
        return asdict(self)



@dataclass(frozen=True)
class NicheGenerator(Generator[Niche]):
    surface: Generator[float]
    nutrients: Generator[float]
    return_rate: Generator[List[float]]
    conditions: Generator[List[float]]
    limiting_factors: Generator[List[float]]
    prospecting_scan_sharpness: Generator[float]
    populations: Union[Generator[List[Union[Population, Generator[Population]]]], List[Union[Population, Generator[Population]]]]

    def generate(self, ctx: GenerationContext) -> Niche:
        ctx.volatile_context = {}
        ctx.volatile_context["limiting_factors"] = self.limiting_factors.generate(ctx) if isinstance(self.limiting_factors, Generator) else self.limiting_factors
        ctx.volatile_context["conditions"] = self.conditions.generate(ctx) if isinstance(self.conditions, Generator) else self.conditions
        if isinstance(self.populations, Generator):
            populations = self.populations.generate(ctx)
        elif isinstance(self.populations, list):
            populations = [population.generate(ctx, i) if isinstance(population, Generator) else population for i, population in enumerate(self.populations)]
        else:
            populations = []
        ret = Niche(
            surface=self.surface.generate(ctx),
            nutrients=self.nutrients.generate(ctx),
            return_rate=self.return_rate.generate(ctx),
            conditions=ctx.volatile_context["conditions"],
            limiting_factors=ctx.volatile_context["limiting_factors"],
            prospecting_scan_sharpness=self.prospecting_scan_sharpness.generate(ctx),
            populations=populations,
        )
        ctx.resolved["niche"] = ret
        ctx.resolved["populations"] = populations
        ctx.volatile_context = {}
        return ret


# #---------------------------------------- Old version
# if TYPE_CHECKING:
#     from .niche_process import NicheGenProcess

# from pydantic import BaseModel, ConfigDict, Field, model_validator

# _DEFAULT_NUTRIENTS_TIER = 2
# _DEFAULT_N_FACTORS = 4
# _DEFAULT_RETURN_RATE_RANGE = (0.0, 1.0)
# _DEFAULT_CONDITIONS_RANGE = (0.4, 0.9)
# _DEFAULT_LIMITING_FACTORS_RANGE = (0.02, 0.35)
# _DEFAULT_PROSPECTING_MEAN = 0.85
# _DEFAULT_PROSPECTING_STD = 0.3


# def _parse_generation_config(raw: Optional[Union[str, dict[str, Any]]]) -> dict[str, Any]:
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


# def _nutrients_from_tier(
#     rng: random.Random,
#     surface: float,
#     tier: int,
#     stochastic: bool,
# ) -> float:
#     """
#     Map ``tier`` in 0..4 onto one of five equal intervals in ``[0, 1000 * surface]``.

#     Draw a Gaussian centered in that interval (stochastic) or the interval midpoint
#     (deterministic), then clamp to ``[0, cap]``.
#     """
#     cap = 1000.0 * max(0.0, float(surface))
#     if cap <= 0.0:
#         return 0.0
#     span = cap / 5.0
#     k = max(0, min(4, int(tier)))
#     lo, hi = k * span, (k + 1) * span
#     mid = 0.5 * (lo + hi)
#     width = max(hi - lo, 1e-12)
#     std = width * 0.28
#     if stochastic:
#         v = mid + std * rng.gauss(0.0, 1.0)
#     else:
#         v = mid
#     return float(max(0.0, min(cap, v)))


# def _return_rate_vector(
#     rng: random.Random,
#     n_bins: int,
#     lo: float,
#     hi: float,
#     stochastic: bool,
# ) -> List[float]:
#     """
#     ``return_rate[i]`` decreases proportionally with bin index; bin 0 lies in ``(0, 0.7]``.

#     Uses ``r[i] = r0 * (n_bins - i) / n_bins`` with ``r0`` sampled when stochastic.
#     """
#     if n_bins < 1:
#         raise ValueError("n_bins must be >= 1.")
#     if stochastic:
#         r0 = rng.uniform(lo, hi)
#     else:
#         r0 = 0.7
#     return [max(0.0, r0 * (n_bins - i) / n_bins) for i in range(n_bins)]


# def _vector_in_range(
#     rng: random.Random,
#     n: int,
#     lo: float,
#     hi: float,
#     stochastic: bool,
# ) -> List[float]:
#     if lo > hi:
#         lo, hi = hi, lo
#     if stochastic:
#         return [rng.uniform(lo, hi) for _ in range(n)]
#     mid = 0.5 * (lo + hi)
#     return [mid] * n


# def _prospecting_sharpness(
#     rng: random.Random,
#     cfg: Any,
#     stochastic: bool,
# ) -> float:
#     mean = _DEFAULT_PROSPECTING_MEAN
#     std = _DEFAULT_PROSPECTING_STD
#     if isinstance(cfg, dict):
#         mean = float(cfg.get("mean", mean))
#         std = float(cfg.get("std", std))
#     if std < 0:
#         raise ValueError("prospecting_scan_sharpness std must be non-negative.")
#     if stochastic:
#         v = mean + std * rng.gauss(0.0, 1.0)
#     else:
#         v = mean
#     return float(max(0.0, v))


# class NicheGen(BaseModel):
#     """Generated niche environment block (``initial_data.data`` without ``type``)."""

#     model_config = ConfigDict(extra="forbid", populate_by_name=True)

#     class GenerateParam:
#         """String keys for JSON configs that drive :meth:`generate`."""

#         SURFACE = "surface"
#         N_BINS = "n_bins"
#         STOCHASTIC = "stochastic"
#         GENERATION_CONFIG = "generation_config"

#     class GenerationConfigKey:
#         """Keys inside unified ``generation_config`` (dict or JSON string)."""

#         NUTRIENTS_CONF = "nutrients_conf"
#         N_FACTORS = "n_factors"
#         RETURN_RATE_RANGE = "return_rate_range"
#         CONDITIONS_RANGE = "conditions_range"
#         LIMITING_FACTORS_RANGE = "limiting_factors_range"
#         PROSPECTING_SCAN_SHARPNESS = "prospecting_scan_sharpness"
#         MEAN = "mean"
#         STD = "std"

#     surface: float = Field(..., ge=0.0, description="Niche surface (arbitrary area units).")
#     ecological_health: float = Field(default=1.0, ge=0.0, le=1.0)
#     nutrients: float = Field(..., ge=0.0)
#     return_rate: List[float] = Field(..., description="Per-bin nutrient return weights; length = n_bins.")
#     conditions: List[float] = Field(..., description="Environmental conditions; C++ clamps to [0,1].")
#     limiting_factors: List[float] = Field(..., description="Limiting factor strengths.")
#     prospecting_scan_sharpness: float = Field(..., ge=0.0)
#     populations: List[Any] = Field(default_factory=list, description="Population snapshots; usually empty at generate time.")

#     @model_validator(mode="after")
#     def _conditions_unit_interval(self) -> NicheGen:
#         for i, v in enumerate(self.conditions):
#             if not 0.0 <= float(v) <= 1.0:
#                 raise ValueError(f"conditions[{i}] must be in [0, 1] for simulator clamp semantics.")
#         return self

#     def to_data_dict(self) -> dict[str, Any]:
#         """Serialize for merging under ``initial_data['data']``."""
#         return self.model_dump(mode="json", exclude_none=True)

#     @property
#     def bins(self) -> int:
#         return len(self.return_rate)

#     @classmethod
#     def generate_from(
#         cls,
#         process: "NicheGenProcess",
#         *,
#         rng: Optional[random.Random] = None,
#     ) -> NicheGen:
#         """Build from a typed :class:`~tools.generator.models.niche_process.NicheGenProcess`."""
#         from .niche_process import generate_niche_from_process

#         return generate_niche_from_process(process, rng=rng)

#     @classmethod
#     def generate(
#         cls,
#         surface: float = 1000.0,
#         n_bins: int = 3,
#         *,
#         rng: Optional[random.Random] = None,
#         stochastic: bool = False,
#         generation_config: Optional[Union[str, dict[str, Any]]] = None,
#     ) -> NicheGen:
#         """
#         Build a niche environment payload.

#         ``nutrients`` is derived from ``nutrients_conf`` in ``generation_config`` (tier
#         0–4: very scarce → very abundant). The range ``[0, 1000 * surface]`` is split into
#         five equal intervals; a value is drawn (Gaussian per interval if ``stochastic``,
#         else midpoint) inside the tier's interval.

#         ``return_rate`` has length ``n_bins``: bin 0 in ``(0, 1.0]`` (stochastic uniform or
#         fixed 0.7 deterministic); higher bins scale down proportionally.

#         ``conditions`` and ``limiting_factors`` have length ``n_factors`` (default 4),
#         each entry uniform in configurable ranges (defaults documented on
#         ``GenerationConfigKey``).

#         ``prospecting_scan_sharpness``: Gaussian with mean ``0.85``, std ``0.3``, clamped
#         below at ``0``; override via ``generation_config['prospecting_scan_sharpness']``
#         as ``{"mean": ..., "std": ...}``.
#         """
#         stochastic = stochastic or generation_config is not None

#         if surface < 0:
#             raise ValueError("surface must be non-negative.")
#         if n_bins < 1:
#             raise ValueError("n_bins must be >= 1.")

#         r = rng if rng is not None else random.Random()
#         gcfg = _parse_generation_config(generation_config)

#         k = cls.GenerationConfigKey
#         tier = int(gcfg.get(k.NUTRIENTS_CONF, _DEFAULT_NUTRIENTS_TIER))
#         tier = max(0, min(4, tier))

#         n_factors = int(gcfg.get(k.N_FACTORS, _DEFAULT_N_FACTORS))
#         if n_factors < 1:
#             raise ValueError("n_factors must be >= 1.")

#         rr = gcfg.get(k.RETURN_RATE_RANGE, list(_DEFAULT_RETURN_RATE_RANGE))
#         cr = gcfg.get(k.CONDITIONS_RANGE, list(_DEFAULT_CONDITIONS_RANGE))
#         lr = gcfg.get(k.LIMITING_FACTORS_RANGE, list(_DEFAULT_LIMITING_FACTORS_RANGE))
#         if not isinstance(cr, (list, tuple)) or len(cr) != 2:
#             raise ValueError("conditions_range must be [low, high].")
#         if not isinstance(lr, (list, tuple)) or len(lr) != 2:
#             raise ValueError("limiting_factors_range must be [low, high].")
#         if not isinstance(rr, (list, tuple)) or len(rr) != 2:
#             raise ValueError("return_rate_range must be [low, high].")
#         r_lo, r_hi = float(rr[0]), float(rr[1])
#         c_lo, c_hi = float(cr[0]), float(cr[1])
#         l_lo, l_hi = float(lr[0]), float(lr[1])

#         nutrients = _nutrients_from_tier(r, surface, tier, stochastic)
#         return_rate = _return_rate_vector(r, n_bins, r_lo, r_hi, stochastic)
#         conditions = _vector_in_range(r, n_factors, c_lo, c_hi, stochastic)
#         limiting_factors = _vector_in_range(r, n_factors, l_lo, l_hi, stochastic)
#         prospecting = _prospecting_sharpness(r, gcfg.get(k.PROSPECTING_SCAN_SHARPNESS), stochastic)

#         conditions = [max(0.0, min(1.0, float(x))) for x in conditions]

#         return cls(
#             surface=float(surface),
#             ecological_health=1.0,
#             nutrients=nutrients,
#             return_rate=return_rate,
#             conditions=conditions,
#             limiting_factors=limiting_factors,
#             prospecting_scan_sharpness=prospecting,
#             populations=[],
#         )


# # Alias for generator JSON field ``niche_conf`` (same schema as :class:`NicheGen`).
# NicheConf = NicheGen
