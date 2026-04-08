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

## CI/CD (GitHub Actions)

The workflow [`.github/workflows/build.yml`](.github/workflows/build.yml) builds the project on:

- **Linux** (ubuntu-latest)
- **Windows** (windows-latest)
- **macOS** (macos-latest)

It runs on every `push` and `pull_request` to the `main` and `master` branches. Executables are published as workflow *artifacts*.

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

### Simulation flow (niche)

1. **`Niche::update_nutrients`** — per cohort, move `return_rate × death_biomass` into `nutrients` and out of dead pool; then **`update_ecological_health`** with the nutrient delta.
2. **`Niche::update_cohorts`** — random starting index, for each cohort resolve growth demand tuples and update nutrients / biomass transfers.

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

## Licence

This project is licensed under the [MIT License](LICENSE).
