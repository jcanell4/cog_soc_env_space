# env_soc_cog_space

**Environmental Social Cognitive Space** – Cross-platform C++ project (Linux, Windows, macOS).

## Requirements

- [CMake](https://cmake.org/) ≥ 3.14
- C++17 compiler (GCC, Clang, MSVC)

## Local build

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run (Linux/macOS; optional path to simulation JSON, default: config/simulation.example.json)
./build/env_soc_cog_space
./build/env_soc_cog_space path/to/simulation.json

# Windows: build\Release\env_soc_cog_space.exe
```

## Raylib snapshot viewer (optional)

The project can build an additional visualization executable that replays `output/simulation.json`.

```bash
# Configure with viewer enabled
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_RAYLIB_VIEWER=ON
cmake --build build --target cog_soc_env_space_viewer

# Run (optional argument: snapshot json path)
./build/cog_soc_env_space_viewer
./build/cog_soc_env_space_viewer output/simulation.json
```

Viewer controls:
- `SPACE` play/pause timeline
- `LEFT` / `RIGHT` step one frame while paused
- `1` / `2` / `3` playback speed (`0.5x`, `1x`, `2x`)
- `R` restart from first frame
- `ESC` close window

## CI/CD (GitHub Actions)

The workflow [`.github/workflows/build.yml`](.github/workflows/build.yml) builds the project on:

- **Linux** (ubuntu-latest)
- **Windows** (windows-latest)
- **macOS** (macos-latest)

It runs on every `push` and `pull_request` to the `main` and `master` branches. Executables are published as workflow *artifacts*.

Each CI run now uploads, for every platform:
- a platform package containing both executables (`...-package`)
- the simulation executable only (`...-sim`)
- the viewer executable only (`...-viewer`)

Linux packages also include helper launchers:
- `bin/run_sim.sh`
- `bin/run_viewer.sh`

These scripts check for missing runtime libraries and print a suggested `apt install` command instead of failing silently.

### Quick start (Linux package)

After extracting the Linux package, use the helper launchers first:

```bash
cd path/to/extracted/package
./bin/run_sim.sh
./bin/run_viewer.sh output/simulation.json
```

If runtime libraries are missing, the scripts print a suggested `apt install` command.

## Creating the repository on GitHub

1. Create a new repository at [github.com/new](https://github.com/new):
   - Name: `env_soc_cog_space`
   - Owner: **jcanell4**
   - Public, do not initialize with a README (you already have one in the project)

2. Link the local project and push the code:

```bash
cd /home/josep/Dropbox/projecteRicard/colletive_intelligence/implementation/language_c/env_soc_cog_space
git init
git add .
git commit -m "Initial commit: env_soc_cog_space C++ project with cross-platform build"
git branch -M main
git remote add origin https://github.com/jcanell4/env_soc_cog_space.git
git push -u origin main
```

3. Under **Actions** in the repository you will see the "Build (Linux, Windows, macOS)" workflow. After each push, executables for all three platforms will be generated.

## Structure

```
env_soc_cog_space/
├── CMakeLists.txt
├── LICENSE            # MIT License
├── README.md
├── .github/workflows/
│   └── build.yml      # CI: Linux, Windows, macOS
├── include/           # Public API headers (documented below)
├── config/            # Environment JSON, simulation.example.json (see below)
├── src/
│   └── main.cpp
└── .gitignore
```

## API overview

Public types live under `include/`. Headers use **Doxygen-style** comments (`@file`, `@class`, `@brief`, `@param`, `@return`) so you can generate HTML/LaTeX reference with [Doxygen](https://www.doxygen.nl/):

```bash
doxygen -g Doxyfile   # once: edit INPUT = include src, RECURSIVE = YES, etc.
doxygen Doxyfile
```

### Class summary

| Type | Header | Role |
|------|--------|------|
| **LivingBeing** | `LivingBeing.h` | Abstract species: name, energy per biomass, death & growth demand contracts. |
| **ConsumerLivingBeing** | `ConsumerLivingBeing.h` | Intermediate base for heterotrophs and decomposers: assimilation efficiency, prospecting, per-stage residue fractions; helpers for waste routing to death bins and optional parental supply. |
| **Heterotroph** | `Heterotroph.h` | Predator: search/capture efficiency, taxonomic diet; `process_individual_growth` uses the shared two-pass ingestion pipeline (see below). |
| **Decomposer** | `Decomposer.h` | Detritivore: uptake from other cohorts’ dead biomass pools; same two-pass pipeline as `Heterotroph` but theory built from donor death bins and trait compatibility (fixed decomposition intensity in implementation). |
| **Autotroph** | `Autotroph.h` | Producer: tolerances, stress death ratio, environment coupling, nutrient-limited growth/death; JSON via `AutotrophBuilder`. |
| **Cohort** | `Cohort.h` | Population of one species: living/dead biomass, `update_biomass`, `calculate_growth_demand`. |
| **Niche** | `Niche.h` | Contains cohorts, nutrients, conditions; `step()` runs nutrient recycling and cohort updates. |
| **Constants** | `Constants.h` | `NUTRIENTS_POS` — channel code for nutrient-limited growth tuples. |
| **SimulationConfig** | `SimulationConfig.h` | Global simulation parameters loaded once from JSON; `global()` read-only access. |

### Growth-demand tuple convention

`LivingBeing::calculate_growth_biomass` / `Cohort::calculate_growth_demand` return `std::vector<std::tuple<int,double>>`:

- **`NUTRIENTS_POS`** (`Constants.h`) — first int: demand competes for niche **nutrients** (autotroph path in `Niche::update_cohorts`).
- **Negative `code`** — decomposer: donor cohort index = `-(code + 1)`.
- **Non-negative `code`** (and not `NUTRIENTS_POS`) — heterotroph: prey cohort index = `code`; a predator may emit several tuples (split intake across preys).

`LivingBeing::diet_by_cohort_index` is stage-indexed and stores per-consumer-stage diet rules:

- C++ type: `std::vector<std::vector<std::tuple<int,int,int>>>`
- Outer index: consumer stage
- Inner tuple: `(source_cohort_index, min_stage, max_stage)` with **inclusive** bounds

For **heterotrophs**, `min_stage`…`max_stage` are prey life-history stages; for **decomposers**, they are donor **dead-biomass bin** indices.

`cohort_index` accepts both formats in input JSON:
- integer code (legacy/current)
- strict constant name (exact match): `NUTRIENTS_TYPE`, `CATABOLIC_TYPE`, `PARENTAL_SUPPLY_TYPE`, `HETEROTROPH_TYPE`

JSON shape under each species:

```json
"diet_by_cohort_index": [
  [
    { "cohort_index": 3, "min_stage": 0, "max_stage": 1 }
  ],
  [
    [5, 1, 2]
  ]
]
```

Rule formats accepted inside each stage list:
- object: `{ "cohort_index": X, "min_stage": Y, "max_stage": Z }`
- numeric triplet: `[X, Y, Z]`

`ConsumerLivingBeing::diet_by_food_type` also uses stage-indexed structure:

- C++ type: `std::vector<std::vector<std::tuple<std::string,int,int>>>`
- Outer index: consumer stage
- Inner tuple: `(food_type_prefix, min_stage, max_stage)` with inclusive bounds

JSON shape:

```json
"diet_by_food_type": [
  [
    { "food_type_prefix": "0.1", "min_stage": 0, "max_stage": 1 }
  ],
  [
    { "food_type_prefix": "0.1.2", "min_stage": 1, "max_stage": 2 }
  ]
]
```

### Simulation flow (niche)

1. **`Niche::update_nutrients`** — per cohort, move `return_rate × death_biomass` into `nutrients` and out of dead pool; then **`update_ecological_health`** with the nutrient delta.
2. **`Niche::update_cohorts`** — random starting index, for each cohort resolve growth demand tuples and update nutrients / biomass transfers.

### Heterotroph encounter and ingestion model

`Heterotroph` and `Decomposer` both inherit **`ConsumerLivingBeing`**. Ingestion—global gross cap, assimilation vs. residue return to donor death bins, and optional parental supply—follows the **same staged sequence**. Predators differ only in how **theoretical** intake per source is computed (live prey encounter/capture vs. dead-pool scanning on donor bins).

`Heterotroph::process_individual_growth` applies a staged predation model aligned with the ecological restart design:

1. **Encounter probability**
   - Per prey stage, occupied area is estimated from:
     - prey biomass,
     - prey biomass-per-individual,
     - prey occupied surface-per-individual.
   - Colony behavior modifies detectability with:
     - `COLONY_SURFACE_GAIN_ETA`,
     - `COLONY_MIX_GAMMA`.
   - Predator movement/prospecting rate is converted into scanned niche fraction with:
     - `PROSPECTING_SCAN_SHARPNESS`.
   - Final per-stage find probability mixes individual-stage and whole-colony channels.

2. **Capture and growth cap**
   - Capture compatibility is computed via
     `LivingBeing::calculate_effective_recruitment_efficiency(recruitment, defense)`.
   - The algorithm performs two passes:
     - build theoretical captures for every prey cohort/stage,
     - scale all captures uniformly so total gross ingestion does not exceed the growth-limited cap derived from:
       - predator stage biomass,
       - `max_individual_growth`,
       - `assimilation_efficiency`.
   - This avoids iteration-order bias across prey species/stages.

3. **Assimilation and biomass transfers**
   - Prey biomass is reduced by realized gross intake (after the global cap `α`).
   - Non-assimilated prey intake is routed to prey dead-biomass size bins
     (`death_biomass[s]`, configurable per predator stage).
   - Predator gain is assimilated intake minus maintenance cost.

4. **Parental supply extension**
   - If diet contains `DietType::PARENTAL_SUPPLY_TYPE`, remaining unmet ingestion can be sourced from fertile stages of the same cohort.
  - Donor contribution is proportional to fertility weights, with stochastic correction (`SimulationConfig::noise_stdv`) to avoid deterministic full-cap attainment each step.
   - Non-assimilated parental intake is routed to the cohort dead-biomass size bins.

### Decomposer detritus ingestion model

`Decomposer::process_individual_growth` mirrors the heterotroph pipeline:

1. **Theory pass** — For each diet rule and donor cohort (not self), for each death-bin index `s` with positive mass, a theoretical gross take is built from prospecting (scan), a fixed decomposition-intensity factor, donor availability in that bin, and `calculate_effective_recruitment_efficiency` using the donor’s death-trait row for `s`. Entries are keyed by `(donor_cohort_index, bin_index)` like heterotroph’s `(prey_cohort, prey_stage)`.

2. **Global cap** — `α = min(1, max_gross_ingestion / theory_total)` uses the same growth-limited cap as predators (stage biomass, `max_individual_growth`, assimilation efficiency).

3. **Assimilation, residues** — Realized take is debited from the donor’s dead biomass, then assimilated fraction updates decomposer biomass; non-assimilated mass is returned to the **donor’s** death bins via `ingestion_residue_fraction_by_size`.

4. **Parental supply** — Same optional `DietType::PARENTAL_SUPPLY_TYPE` branch as `Heterotroph`, implemented in `ConsumerLivingBeing::applyParentalSupplyGross`.

Restart scaffolding builders: `HeterotrophBuilder` and **`DecomposerBuilder`** expose the shared consumer parameters (prospecting, assimilation, residue grids); `HeterotrophBuilder` also supports search/capture efficiency.

### Dead biomass size bins (dynamic length)

- `death_biomass` is a dynamic vector (`std::vector<double>`) per cohort; there is no fixed bin count.
- Bin convention: index `0` is the finest / most degraded detritus class.
- `Niche::return_rate` is also dynamic; nutrient recycling applies per bin index and treats missing
  indices as `0`.

### Global simulation configuration (`SimulationConfig`)

Runtime parameters are loaded **once** at startup from JSON and exposed read-only via `SimulationConfig::global()` (`include/SimulationConfig.h`). Call `SimulationConfig::loadFromFile(path)` before any code that reads the config (the executable does this automatically; default file: `config/simulation.example.json` relative to the current working directory). A second load throws.

**Dedicated file** — the JSON root is a single object with the keys below.

**Embedded in a larger file** — put the same keys under a `"simulation"` object; the loader uses that sub-object when present (e.g. shared scenario file with environment data).

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `version` | number | `1` | Schema version (must be ≥ 1). |
| `random_seed` | integer | `0` | Non-negative seed; `0` means “unset” for callers that supply a default. |
| `time_step` | number | `1.0` | Positive step length in simulation units. |
| `max_steps` | integer | `0` | Non-negative cap; `0` means no fixed cap (main loop policy). |
| `min_growth_rate_supported` | number | `0.0` | Global lower bound for supported growth rate (model units). If only this key is set, `max` is set equal to `min`. |
| `max_growth_rate_supported` | number | `0.0` | Global upper bound; must be ≥ `min`. If only this key is set, `min` is set equal to `max`. |
| `min_half_saturation_constant_supported` | number | `0.0` | Global lower bound for half-saturation constant (model units). If only this key is set, `max` is set equal to `min`. |
| `max_half_saturation_constant_supported` | number | `0.0` | Global upper bound; must be ≥ `min`. If only this key is set, `min` is set equal to `max`. |
| `verbose` | boolean | `false` | If true, the program may print extra diagnostics. |

Example: [`config/simulation.example.json`](config/simulation.example.json).

### Ecosystem JSON snapshots (`JsonEcosystem`)

`JsonEcosystem` provides helper methods to serialize runtime ecosystem state into an append-only JSON structure:

- `createJson(const Niche&)` creates:
  - `initial_data` with a full `Niche` snapshot (including cohorts and species details)
  - `step_data` as an empty array
- `updateJson(const Niche&, int elapsed_cycles, json&)` appends one full niche snapshot to `step_data`
- `saveJsonToFile(const json&, const std::string&, int indent = 2)` writes JSON to disk

Enum-like fields support mixed input and named output:
- `specie.class_type` input accepts integer or strict constant name: `AUTOTROPH`, `HETEROTROPH`, `DECOMPOSER`
- `specie.diet_by_cohort_index[].cohort_index` input accepts integer or strict constant name: `NUTRIENTS_TYPE`, `CATABOLIC_TYPE`, `PARENTAL_SUPPLY_TYPE`, `HETEROTROPH_TYPE`
- Output prefers constant names when a mapping exists; otherwise writes the numeric literal in the same field

Output shape:

```json
{
  "initial_data": {
    "type": "Niche",
    "data": { "...": "full initial niche snapshot" }
  },
  "step_data": [
    {
      "type": "Niche",
      "elapsed_cycles": 10,
      "data": { "...": "full niche snapshot at cycle 10" }
    }
  ]
}
```

## Licence

This project is licensed under the [MIT License](LICENSE).
