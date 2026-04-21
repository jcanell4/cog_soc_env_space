# Environment configuration (JSON)

This folder holds JSON files that describe an **Environment**: a graph of **Niches**, each with environmental **conditions**, **nutrients**, rates, and **Cohorts** that reference **Species** definitions.

## Top-level object

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `version` | number | no | Schema version for future migrations. |
| `description` | string | no | Human-readable note. |
| `species` | array | **yes** | Global species catalogue (see below). |
| `niches` | array | **yes** | Ordered list of niches (index = graph node id). |
| `adjacency` | array | no | Outgoing edges per niche, parallel to `niches` (row `i` = edges from niche `i`). If omitted, empty rows are used. |

## Species entry (`species[]`)

Each object must include:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `id` | string | **yes** | Stable key referenced by cohorts (`species_id`). Must be unique. |
| `kind` | string | **yes** | Species implementation. Supported: `autotroph` (legacy alias: `autotroph_by_rates`). |
| `name` | string | **yes** | Display / model name (passed to the concrete type). |

For `kind: "autotroph"` (or legacy `autotroph_by_rates`), the following optional fields apply:

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `energy_content` | number | `17.5` | Energy per unit biomass. |
| `max_growth_rate` | number | `0` | Maximum intrinsic growth rate (`LivingBeing::setMaxGrowthRate`); autotroph effective rate after init also scales conditions/substrate/nutrients. |
| `growth_rate` | number | — | **Deprecated.** Same as `max_growth_rate`; accepted for backward compatibility. |
| `base_death_rate` | number | `0` | Death parameter. |
| `tolerances` | array of numbers | `[]` | Positional tolerance vector (float); mirrored for environment sensitivity. |
| `stress_death_ratio` | number | (class default) | Optional; passed to `LivingBeing::setStressDeathRatio` when present. |
| `resilience` | number | — | **Deprecated.** Same as `stress_death_ratio`; accepted for backward compatibility. |

## Niche entry (`niches[]`)

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `surface` | number | **yes** | Total niche surface. |
| `biological_potential` | number | **yes** | Potential per unit surface (model units). |
| `ecological_health` | number | no | Clamped to `[0, 1]` in code. |
| `nutrients` | number | no | Inorganic nutrients pool. |
| `return_rate` | array of numbers | **yes** | Recycling rate per dead-biomass size bin (dynamic length). |
| `conditions` | array of numbers | no | Normalized traits in `[0, 1]` (e.g. humidity, light, temperature). |
| `cohorts` | array | no | Cohorts living in this niche (see below). |

## Cohort entry (`niches[].cohorts[]`)

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `species_id` | string | **yes** | Must match a `species[].id`. |
| `biomass` | number | no | Living biomass (default `0`). |
| `death_biomass` | array of numbers | no | Dead biomass by size bin (default `[]`; index 0 = finest). |

## Adjacency (`adjacency[]`)

- Outer array length should match `niches.length` (the loader resizes to match).
- `adjacency[i]` is an array of edge objects from niche `i`.
- Each edge object currently supports:
  - `to` (number): destination niche index.

For an undirected graph, add symmetric entries in both `adjacency[i]` and `adjacency[j]`.

## Example

See `environment.example.json` in this folder.

## Loading from C++

Use `loadEnvironmentFromJson` / `loadEnvironmentFromJsonFile` from `EnvironmentConfig.h` (see implementation in `src/EnvironmentConfig.cpp`).
