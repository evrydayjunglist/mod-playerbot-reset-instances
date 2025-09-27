# Chronomancer Noriol — AzerothCore Module

Adds **Chronomancer Noriol**, a mysterious NPC who manipulates timelines on your behalf.

## Features

- **Instance Lockout Reset** — Resets all raid and dungeon lockouts for you **and your Playerbots**.
- **Playerbot Boost** — Instantly boost your Playerbots to level 80, one by one, with optional gold cost.
- **Outland Skip** — If you're **level 58**, Noriol can boost your main character directly to level 68 to skip Outland entirely.
- **Lore-Friendly Dialogues** — Immersive NPC text and branching gossip options.
- **Configurable Costs** for each service:
  - Instance reset: default **10 gold**
  - Bot boost: default **1000 gold** per bot
  - Outland skip: default **5000 gold**
- **Spell FX and Emotes** — Time-altering animations to enhance immersion.

## Installation
- Checkout the module folder to your AzerothCore modules folder.
- Run CMake and build AzerothCore.
- Import the included SQL in the acore_world db to spawn Chronomancer Noriol in your world.
- Adjust settings in `mod-playerbots-instance-reset.conf`.

## Usage
- Place Chronomancer Noriol anywhere in your world with **.npc add 190012**

## Notes
- Instance Reset and Playerbot Boost is available at level 80.
- Outland skip is available during level 58.

Enjoy your adventures in time!
