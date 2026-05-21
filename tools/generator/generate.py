#!/usr/bin/env python3
"""
Entry point for config-driven niche JSON generation.

Loads ``tools/generator/config/<spec>.json`` (default: ``generator.example.json``),
validates the root object, builds populations from ``food_web`` and ``species_type``, and
writes a full niche document (``initial_data`` wrapper).

For new work, prefer the typed API (``NicheGenProcess``, ``AutotrophSpeciesProcess``,
``PopulationGenProcess`` + ``Order`` defaults) — see ``config/generator.example.001.py``.
This script still uses the legacy ``.generate(..., generation_config=...)`` path for JSON.
"""

from __future__ import annotations

import argparse
import json
import random
import sys
from pathlib import Path
from typing import Any, Dict, List, Literal, Optional, Union

from pydantic import BaseModel, ConfigDict, Field, model_validator

_ROOT = Path(__file__).resolve().parents[2]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from tools.generator.models.population import PopulationGen
from tools.generator.models.livingbeing import AutotrophSpecies, HeterotrophSpecies
from tools.generator.models.niche import NicheGen


class SpeciesTypeRecipe(BaseModel):
    """
    Parameters for :meth:`AutotrophSpecies.generate` or :meth:`HeterotrophSpecies.generate`.

    JSON must include ``\"class\": \"autotroph\"`` or ``\"heterotroph\"``.
    """

    model_config = ConfigDict(extra="forbid", populate_by_name=True)

    class_: Literal["autotroph", "heterotroph"] = Field(
        ...,
        alias="class",
        description="Which species generator to invoke.",
    )
    name: str
    cycles_per_stages: List[int]
    food_type: Optional[str] = Field(
        default=None,
        description="Omitted defaults to autotroph \"0\" / heterotroph \"1\" in generate().",
    )
    stage_special_diets: Optional[Any] = Field(
        default=None,
        description="Optional per-stage diet overrides (same semantics as species generate).",
    )
    stochastic: bool = False
    generation_config: Optional[Union[str, Dict[str, Any]]] = None


class FoodWebEntry(BaseModel):
    """One population: literal biomass vectors plus a reference into ``species_type``."""

    model_config = ConfigDict(extra="forbid")

    species_type: str = Field(
        ...,
        description="Key into the top-level species_type map.",
    )
    biomass: List[float]
    death_biomass: List[float]
    id: Optional[int] = Field(
        default=None,
        description="Optional population id; random JSON-safe id if omitted.",
    )


class GeneratorProjectConfig(BaseModel):
    """Root object for ``generator*.json`` consumed by this script."""

    model_config = ConfigDict(extra="forbid")

    version: int = 1
    description: str = ""
    random_seed: Optional[int] = Field(
        default=None,
        description="If set, seeds population ids / species stochastic draws.",
    )
    niche_conf: Dict[str, Any] = Field(
        ...,
        description="Payload for :class:`NicheGen` (``initial_data.data`` fields). "
        "Any ``populations`` entry is replaced by populations built from ``food_web``.",
    )
    species_type: Dict[str, SpeciesTypeRecipe] = Field(
        ...,
        description="Named species templates (autotroph / heterotroph generate recipes).",
    )
    food_web: List[FoodWebEntry] = Field(
        default_factory=list,
        description="Population list: each row picks a species template and literal biomass vectors.",
    )

    @model_validator(mode="after")
    def _food_web_refs(self) -> GeneratorProjectConfig:
        for i, row in enumerate(self.food_web):
            if row.species_type not in self.species_type:
                raise ValueError(
                    f"food_web[{i}].species_type {row.species_type!r} is not a key in species_type."
                )
        return self

    def build_niche_gen(self, rng: random.Random) -> NicheGen:
        """Merge ``niche_conf`` with species and populations from ``food_web``."""
        species_cache: Dict[str, Union[AutotrophSpecies, HeterotrophSpecies]] = {}
        for key, recipe in self.species_type.items():
            if recipe.class_ == "autotroph":
                species_cache[key] = AutotrophSpecies.generate(
                    recipe.name,
                    recipe.cycles_per_stages,
                    recipe.food_type,
                    recipe.stage_special_diets,
                    rng=rng,
                    stochastic=recipe.stochastic,
                    generation_config=recipe.generation_config,
                )
            else:
                species_cache[key] = HeterotrophSpecies.generate(
                    recipe.name,
                    recipe.cycles_per_stages,
                    recipe.food_type,
                    recipe.stage_special_diets,
                    rng=rng,
                    stochastic=recipe.stochastic,
                    generation_config=recipe.generation_config,
                )

        population_dicts: List[Any] = []
        for row in self.food_web:
            sp = species_cache[row.species_type]
            cg = PopulationGen.generate(
                row.biomass,
                row.death_biomass,
                specie=sp,
                rng=rng,
                id=row.id,
            )
            population_dicts.append(cg.to_population_dict())

        data = dict(self.niche_conf)
        data["populations"] = population_dicts
        return NicheGen.model_validate(data)

    def to_niche_file_dict(self, rng: random.Random) -> Dict[str, Any]:
        """Full niche JSON document (``initial_data`` wrapper) for writing to disk."""
        niche = self.build_niche_gen(rng)
        return {
            "initial_data": {
                "type": "Niche",
                "data": niche.to_data_dict(),
            }
        }


def _repo_root() -> Path:
    return _ROOT


def _load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, dict):
        raise ValueError(f"Expected JSON object in {path}")
    return data


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Generate niche JSON from generator config (NicheGen + food web)."
    )
    parser.add_argument(
        "-c",
        "--config",
        type=Path,
        default=None,
        help="Path to generator JSON (default: tools/generator/config/generator.example.json)",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Output niche JSON path (default: tools/generator/out/niche.generated.json under repo root)",
    )
    args = parser.parse_args(argv)

    root = _repo_root()
    default_cfg = root / "tools" / "generator" / "config" / "generator.example.json"
    cfg_path = (args.config if args.config is not None else default_cfg).resolve()
    if not cfg_path.is_file():
        print(f"Config not found: {cfg_path}", file=sys.stderr)
        return 1

    raw = _load_json(cfg_path)
    try:
        project = GeneratorProjectConfig.model_validate(raw)
    except Exception as exc:
        print(f"Invalid generator config: {exc}", file=sys.stderr)
        return 1

    rng = random.Random(project.random_seed)

    out_path = (
        args.output.resolve()
        if args.output is not None
        else (root / "tools" / "generator" / "out" / "niche.generated.json").resolve()
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)

    doc = project.to_niche_file_dict(rng)
    with out_path.open("w", encoding="utf-8") as f:
        json.dump(doc, f, indent=2)
        f.write("\n")

    print(f"Wrote {out_path.relative_to(root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
