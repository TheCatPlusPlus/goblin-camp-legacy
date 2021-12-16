| :warning: | **This repository is provided for archival and reference only. The project is unmaintained and unsupported.** | :warning: |
| --- | --- | --- |

# Goblin Camp

Goblin Camp aims to be a roguelike city builder with an emphasis on
macromanaging the economy and military. It has drawn inspiration from games
such as Anno 1404, Dwarf Fortress and Dungeon Keeper.

This project is frozen and unsupported since around 2012. This repository is not a revival.

## Notes

- Python support removed (was basically only used for the tutorial and is quite annoying buildwise), config file is now JSON
- Boost.Build replaced with CMake + vcpkg
- Should compile as C++17
- Doesn't build on OSX because Apple is Apple and SDL 1.x no longer compiles (and it's way too much of a mess to replace it)
- Builds and runs with VS2022 on Windows, and Clang 10 on Linux
- Tileset rendering is completely broken
- Boost was updated, so old saves will most definitely not load

Personally I don't have patience for this codebase, so not doing any bigger changes. This should be good enough
as a reference point to kick off a modern rewrite in not-C++ (no promises though). YMMV.

