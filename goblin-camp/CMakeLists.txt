cmake_minimum_required(VERSION 3.20)

project(
	goblin-camp-legacy
	LANGUAGES CXX C
	VERSION 0.21.1
)

#
# common settings
#

add_library(BuildSettings INTERFACE)

target_compile_features(BuildSettings INTERFACE
	cxx_std_17
)

target_compile_definitions(BuildSettings INTERFACE
	$<$<CONFIG:Debug>:DEBUG>
	$<$<CONFIG:Debug>:_DEBUG>
	$<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(WIN32)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.17763.0")

	# NB support for ancient systems have long been dropped from MSVC and Windows SDK
	# should work with 8 but tested with 10 only

	target_compile_definitions(BuildSettings INTERFACE
		WIN32_LEAN_AND_MEAN
		NOMINMAX
		_STRICT
		STRICT
		_WINDOWS
		WINDOWS
		_UNICODE
		UNICODE

		_WIN32_WINNT=0x0602
		WINVER=0x0602
	)

	if(MSVC)
		target_compile_definitions(BuildSettings INTERFACE
			_SCL_SECURE_NO_WARNINGS
			_CRT_SECURE_NO_WARNINGS
			_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
		)

		target_compile_options(BuildSettings INTERFACE $<$<COMPILE_LANGUAGE:CXX>:/bigobj>)
	endif()
endif()

#
# dependencies
#

find_package(Boost REQUIRED
	COMPONENTS
		filesystem
		serialization
		iostreams
		json
)

find_package(ZLIB REQUIRED)
find_package(libpng CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

set(SDL_BUILDING_LIBRARY ON)
find_package(SDL REQUIRED)

#
# SDL_image 1.x (not packaged)
#

add_library(SDL_Image STATIC
	vendor/sdl/src/IMG.c
	vendor/sdl/src/IMG_png.c
)

target_include_directories(SDL_Image PUBLIC vendor/sdl/include ${SDL_INCLUDE_DIR})
target_link_libraries(SDL_Image PUBLIC png_static ${SDL_LIBRARY})
target_compile_definitions(SDL_Image PRIVATE SDL_IMAGE_USE_COMMON_BACKEND LOAD_PNG)

#
# libtcod (compatibility)
#

add_library(Libtcod STATIC
	vendor/libtcod/src/bresenham.cpp
	vendor/libtcod/src/bresenham_c.c
	vendor/libtcod/src/bsp.cpp
	vendor/libtcod/src/bsp_c.c
	vendor/libtcod/src/color.cpp
	vendor/libtcod/src/color_c.c
	vendor/libtcod/src/console.cpp
	vendor/libtcod/src/console_c.c
	vendor/libtcod/src/fov.cpp
	vendor/libtcod/src/fov_c.c
	vendor/libtcod/src/fov_circular_raycasting.c
	vendor/libtcod/src/fov_diamond_raycasting.c
	vendor/libtcod/src/fov_digital.c
	vendor/libtcod/src/fov_permissive.c
	vendor/libtcod/src/fov_permissive2.c
	vendor/libtcod/src/fov_recursive_shadowcasting.c
	vendor/libtcod/src/fov_restrictive.c
	vendor/libtcod/src/heightmap.cpp
	vendor/libtcod/src/heightmap_c.c
	vendor/libtcod/src/image.cpp
	vendor/libtcod/src/image_c.c
	vendor/libtcod/src/lex.cpp
	vendor/libtcod/src/lex_c.c
	vendor/libtcod/src/list_c.c
	vendor/libtcod/src/mersenne.cpp
	vendor/libtcod/src/mersenne_c.c
	vendor/libtcod/src/mouse.cpp
	vendor/libtcod/src/namegen.cpp
	vendor/libtcod/src/namegen_c.c
	vendor/libtcod/src/noise.cpp
	vendor/libtcod/src/noise_c.c
	vendor/libtcod/src/parser.cpp
	vendor/libtcod/src/parser_c.c
	vendor/libtcod/src/path.cpp
	vendor/libtcod/src/path_c.c
	vendor/libtcod/src/sys.cpp
	vendor/libtcod/src/sys_c.c
	vendor/libtcod/src/sys_opengl_c.c
	vendor/libtcod/src/sys_sdl_c.c
	vendor/libtcod/src/sys_sdl_img_bmp.c
	vendor/libtcod/src/sys_sdl_img_png.c
	vendor/libtcod/src/tree_c.c
	vendor/libtcod/src/txtfield.cpp
	vendor/libtcod/src/txtfield_c.c
	vendor/libtcod/src/wrappers.c
	vendor/libtcod/src/zip.cpp
	vendor/libtcod/src/zip_c.c
)

target_include_directories(Libtcod PUBLIC vendor/libtcod/include)
target_link_libraries(Libtcod PRIVATE BuildSettings)
target_link_libraries(Libtcod PUBLIC SDL_Image png_static ZLIB::ZLIB)

#
# the game
#

add_executable(Game
	$<$<PLATFORM_ID:Windows>:goblin-camp.rc>
	$<$<PLATFORM_ID:Windows>:src/platform/windows/CrashHandler.cpp>
	$<$<PLATFORM_ID:Windows>:src/platform/windows/CommandLine.cpp>
	$<$<PLATFORM_ID:Windows>:src/platform/windows/DataImpl.cpp>
	$<$<PLATFORM_ID:Windows>:src/platform/windows/DebugImpl.cpp>
	$<$<PLATFORM_ID:Windows>:src/platform/windows/main.cpp>
	$<$<PLATFORM_ID:Windows>:src/platform/windows/TSSCleanup.cpp>

	$<$<PLATFORM_ID:Linux,Darwin>:src/platform/unix/main.cpp>

	$<$<PLATFORM_ID:Linux>:src/platform/linux/DataImpl.cpp>

	$<$<PLATFORM_ID:Darwin>:src/platform/osx/DataImpl.cpp>
	$<$<PLATFORM_ID:Darwin>:src/platform/osx/GoblinCamp.icns>
	$<$<PLATFORM_ID:Darwin>:src/platform/osx/Info.plist>

	src/data/Config.cpp
	src/data/Config.hpp
	src/data/Data.cpp
	src/data/Data.hpp
	src/data/Mods.cpp
	src/data/Mods.hpp
	src/data/Paths.cpp
	src/data/Paths.hpp
	src/data/Serialization.cpp
	src/data/Serialization.hpp
	src/data/Tilesets.cpp
	src/data/Tilesets.hpp

	src/tileRenderer/ogl/OGLFunctionExt.cpp
	src/tileRenderer/ogl/OGLFunctionExt.hpp
	src/tileRenderer/ogl/OGLResources.cpp
	src/tileRenderer/ogl/OGLResources.hpp
	src/tileRenderer/ogl/OGLSprite.cpp
	src/tileRenderer/ogl/OGLSprite.hpp
	src/tileRenderer/ogl/OGLTilesetRenderer.cpp
	src/tileRenderer/ogl/OGLTilesetRenderer.hpp
	src/tileRenderer/ogl/OGLViewportLayer.cpp
	src/tileRenderer/ogl/OGLViewportLayer.hpp

	src/tileRenderer/sdl/SDLSprite.cpp
	src/tileRenderer/sdl/SDLSprite.hpp
	src/tileRenderer/sdl/SDLTilesetRenderer.cpp
	src/tileRenderer/sdl/SDLTilesetRenderer.hpp

	src/tileRenderer/ConstructionSprite.cpp
	src/tileRenderer/ConstructionSprite.hpp
	src/tileRenderer/Corner.hpp
	src/tileRenderer/DrawConstructionVisitor.cpp
	src/tileRenderer/DrawConstructionVisitor.hpp
	src/tileRenderer/ItemSprite.cpp
	src/tileRenderer/ItemSprite.hpp
	src/tileRenderer/NatureObjectSpriteSet.cpp
	src/tileRenderer/NatureObjectSpriteSet.hpp
	src/tileRenderer/NPCSprite.cpp
	src/tileRenderer/NPCSprite.hpp
	src/tileRenderer/PermutationTable.cpp
	src/tileRenderer/PermutationTable.hpp
	src/tileRenderer/SpellSpriteSet.cpp
	src/tileRenderer/SpellSpriteSet.hpp
	src/tileRenderer/Sprite.cpp
	src/tileRenderer/Sprite.hpp
	src/tileRenderer/SpriteSetFactories.cpp
	src/tileRenderer/SpriteSetFactories.hpp
	src/tileRenderer/StatusEffectSprite.cpp
	src/tileRenderer/StatusEffectSprite.hpp
	src/tileRenderer/TerrainSprite.cpp
	src/tileRenderer/TerrainSprite.hpp
	src/tileRenderer/TileSet.cpp
	src/tileRenderer/TileSet.hpp
	src/tileRenderer/TileSetLoader.cpp
	src/tileRenderer/TileSetLoader.hpp
	src/tileRenderer/TilesetParserV1.cpp
	src/tileRenderer/TilesetParserV1.hpp
	src/tileRenderer/TilesetParserV2.cpp
	src/tileRenderer/TilesetParserV2.hpp
	src/tileRenderer/TileSetRenderer.cpp
	src/tileRenderer/TileSetRenderer.hpp
	src/tileRenderer/TileSetTexture.cpp
	src/tileRenderer/TileSetTexture.hpp

	src/UI/AnnounceDialog.cpp
	src/UI/AnnounceDialog.hpp
	src/UI/ConstructionDialog.cpp
	src/UI/ConstructionDialog.hpp
	src/UI/JobDialog.cpp
	src/UI/JobDialog.hpp
	src/UI/Menu.cpp
	src/UI/Menu.hpp
	src/UI/MessageBox.cpp
	src/UI/MessageBox.hpp
	src/UI/NPCDialog.cpp
	src/UI/NPCDialog.hpp
	src/UI/SideBar.cpp
	src/UI/SideBar.hpp
	src/UI/SquadsDialog.cpp
	src/UI/SquadsDialog.hpp
	src/UI/StockManagerDialog.cpp
	src/UI/StockManagerDialog.hpp
	src/UI/Tooltip.cpp
	src/UI/Tooltip.hpp
	src/UI/UIComponents.cpp
	src/UI/UIComponents.hpp

	src/Announce.cpp
	src/Announce.hpp
	src/Attack.cpp
	src/Attack.hpp
	src/Blood.cpp
	src/Blood.hpp
	src/Camp.cpp
	src/Camp.hpp
	src/Construction.cpp
	src/Construction.hpp
	src/Container.cpp
	src/Container.hpp
	src/Coordinate.cpp
	src/Coordinate.hpp
	src/Door.cpp
	src/Door.hpp
	src/Entity.cpp
	src/Entity.hpp
	src/Events.cpp
	src/Events.hpp
	src/Faction.cpp
	src/Faction.hpp
	src/Farmplot.cpp
	src/Farmplot.hpp
	src/Filth.cpp
	src/Filth.hpp
	src/Fire.cpp
	src/Fire.hpp
	src/Game.cpp
	src/Game.hpp
	src/GCamp.cpp
	src/GCamp.hpp
	src/Item.cpp
	src/Item.hpp
	src/Job.cpp
	src/Job.hpp
	src/JobManager.cpp
	src/JobManager.hpp
	src/KuhnMunkres.cpp
	src/KuhnMunkres.hpp
	src/Logger.cpp
	src/Logger.hpp
	src/Map.cpp
	src/Map.hpp
	src/MapMarker.cpp
	src/MapMarker.hpp
	src/MapRenderer.cpp
	src/MapRenderer.hpp
	src/NatureObject.cpp
	src/NatureObject.hpp
	src/NPC.cpp
	src/NPC.hpp
	src/Random.cpp
	src/Random.hpp
	src/SpawningPool.cpp
	src/SpawningPool.hpp
	src/Spell.cpp
	src/Spell.hpp
	src/Squad.cpp
	src/Squad.hpp
	src/Stats.cpp
	src/Stats.hpp
	src/StatusEffect.cpp
	src/StatusEffect.hpp
	src/StockManager.cpp
	src/StockManager.hpp
	src/Stockpile.cpp
	src/Stockpile.hpp
	src/TCODMapRenderer.cpp
	src/TCODMapRenderer.hpp
	src/Tile.cpp
	src/Tile.hpp
	src/Trap.cpp
	src/Trap.hpp
	src/UI.cpp
	src/UI.hpp
	src/Version.cpp
	src/Version.hpp
	src/Water.cpp
	src/Water.hpp
	src/Weather.cpp
	src/Weather.hpp
)

target_include_directories(Game PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	.
	src
)

target_link_libraries(Game PRIVATE
	BuildSettings Libtcod
	Boost::boost Boost::filesystem Boost::serialization Boost::iostreams Boost::json
	spdlog::spdlog spdlog::spdlog_header_only
)

if(WIN32)
	target_link_libraries(Game PRIVATE
		dbghelp kernel32 user32 gdi32 opengl32 dxguid winmm
	)
elseif(APPLE)
else()
	target_link_libraries(Game PRIVATE
		X11 GL dl
	)
endif()

target_precompile_headers(Game PRIVATE [["stdafx.hpp"]])

target_compile_definitions(Game PRIVATE
	GC_PRECOMPILED_HEADER
	BOOST_BIND_GLOBAL_PLACEHOLDERS
)

set_target_properties(Game PROPERTIES
	OUTPUT_NAME goblin-camp
)

set(GC_VERSION "${PROJECT_VERSION}")
configure_file(goblin-camp.version.h.in goblin-camp.version.h @ONLY)

if(WIN32)
	set(GC_BIN_DIRECTORY .)
	set(GC_DATA_DIRECTORY .)
elseif(APPLE)
	set(GC_BUNDLE_DIRECTORY GoblinCamp.app/Contents)
	set(GC_BIN_DIRECTORY ${GC_BUNDLE_DIRECTORY}/MacOS)
	set(GC_DATA_DIRECTORY ${GC_BUNDLE_DIRECTORY}/Resources)
else()
	set(GC_BIN_DIRECTORY bin)
	set(GC_DATA_DIRECTORY share/goblin-camp)
endif()

install(
	TARGETS Game
	RUNTIME DESTINATION ${GC_BIN_DIRECTORY}
)

install(
	DIRECTORY lib
	DESTINATION ${GC_DATA_DIRECTORY}
)

install(
	FILES terminal.png
	DESTINATION ${GC_DATA_DIRECTORY}
)

install(
	FILES Readme.txt
	DESTINATION ${GC_DATA_DIRECTORY}
)

install(
	FILES COPYING.txt
	DESTINATION ${GC_DATA_DIRECTORY}
)

install(
	FILES LIBTCOD-LICENSE.txt
	DESTINATION ${GC_DATA_DIRECTORY}
)

if(MSVC)
	install(
		FILES $<TARGET_PDB_FILE:Game>
		DESTINATION ${GC_BIN_DIRECTORY}
		OPTIONAL
	)
endif()

if(APPLE)
	install(
		FILES src/platform/osx/GoblinCamp.icns
		DESTINATION ${GC_DATA_DIRECTORY}
	)

	install(
		FILES src/platform/osx/Info.plist
		DESTINATION ${GC_BUNDLE_DIRECTORY}
	)
endif()
