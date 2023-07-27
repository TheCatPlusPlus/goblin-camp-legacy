| :warning: | **This repository is provided for archival and reference purposes only. See below for active development related to the game.** | :warning: |
| --- | --- | --- |

# Goblin Camp

Goblin Camp aims to be a roguelike city builder with an emphasis on
macromanaging the economy and military. It has drawn inspiration from games
such as Anno 1404, Dwarf Fortress and Dungeon Keeper.

This repository contains the code as of the original project's end (circa 2012) plus some changes in auxiliary systems to make
building the project easier on a modern system. It's meant to stay frozen in this state, no further changes will be made here.

## Active development

While this repository is not concerned with active development, there are two relevant efforts in progress (as of July 2023):

* Nikolay Shaplov et al have an active branch going on at <https://gitlab.com/dhyannataraj/goblin-camp> and are actively implementing
  new features, improvements, and making end-user releases. If you'd like to play or contribute to the game, you should check that out.

* The original developer Ilkka Halila announced a commercial remake of the original game that will be available on Steam.
  You can find that at <https://store.steampowered.com/app/2431980/Goblin_Camp/> (note: it's not derived from this codebase at all, and will likely not be open-source).

## Changes in this repo

- Python support removed (was basically only used for the tutorial and is quite annoying buildwise), config file is now JSON
- Boost.Build replaced with CMake + vcpkg
- Should compile as C++17
- Doesn't build on OSX because Apple is Apple and SDL 1.x no longer compiles (and it's way too much of a mess to replace it)
- Builds and runs with VS2022 on Windows, and Clang 10 on Linux
- Tileset rendering is completely broken
- Boost was updated, so old saves will most definitely not load
