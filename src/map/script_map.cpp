//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name script_map.cpp - The map ccl functions. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "map.h"

//Wyrmgus start
#include "editor.h"
#include "game.h"
//Wyrmgus end
#include "iolib.h"
//Wyrmgus start
#include "province.h"
#include "quest.h"
//Wyrmgus end
#include "script.h"
//Wyrmgus start
#include "settings.h"
//Wyrmgus end
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "version.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Parse a map.
**
**  @param l  Lua state.
*/
static int CclStratagusMap(lua_State *l)
{
	int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const char *value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "version")) {
			//Wyrmgus start
//			char buf[32];
			char buf[64];
			//Wyrmgus end

			const char *version = LuaToString(l, j + 1);
			strncpy(buf, VERSION, sizeof(buf));
			if (strcmp(buf, version)) {
				fprintf(stderr, "Warning not saved with this version.\n");
			}
		} else if (!strcmp(value, "uid")) {
			Map.Info.MapUID = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "description")) {
			Map.Info.Description = LuaToString(l, j + 1);
		} else if (!strcmp(value, "the-map")) {
			if (!lua_istable(l, j + 1)) {
				//Wyrmgus start
//				LuaError(l, "incorrect argument");
				LuaError(l, "incorrect argument for \"the-map\"");
				//Wyrmgus end
			}
			int subargs = lua_rawlen(l, j + 1);
			for (int k = 0; k < subargs; ++k) {
				const char *value = LuaToString(l, j + 1, k + 1);
				++k;

				if (!strcmp(value, "size")) {
					lua_rawgeti(l, j + 1, k + 1);
					CclGetPos(l, &Map.Info.MapWidth, &Map.Info.MapHeight);
					lua_pop(l, 1);

					//Wyrmgus start
//					delete[] Map.Fields;
//					Map.Fields = new CMapField[Map.Info.MapWidth * Map.Info.MapHeight];
					for (size_t z = 0; z < Map.Fields.size(); ++z) {
						delete[] Map.Fields[z];
					}
					Map.Fields.clear();
					Map.Fields.push_back(new CMapField[Map.Info.MapWidth * Map.Info.MapHeight]);
					Map.Info.MapWidths.clear();
					Map.Info.MapWidths.push_back(Map.Info.MapWidth);
					Map.Info.MapHeights.clear();
					Map.Info.MapHeights.push_back(Map.Info.MapHeight);
					//Wyrmgus end
					// FIXME: this should be CreateMap or InitMap?
				} else if (!strcmp(value, "fog-of-war")) {
					Map.NoFogOfWar = false;
					--k;
				} else if (!strcmp(value, "no-fog-of-war")) {
					Map.NoFogOfWar = true;
					--k;
				} else if (!strcmp(value, "filename")) {
					Map.Info.Filename = LuaToString(l, j + 1, k + 1);
				//Wyrmgus start
				} else if (!strcmp(value, "extra-map-layers")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument for \"extra-map-layers\"");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int z = 0; z < subsubargs; ++z) {
						lua_rawgeti(l, -1, z + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						int map_layer_width = LuaToNumber(l, -1, 1);
						int map_layer_height = LuaToNumber(l, -1, 2);
						Map.Info.MapWidths.push_back(map_layer_width);
						Map.Info.MapHeights.push_back(map_layer_height);
						Map.Fields.push_back(new CMapField[map_layer_width * map_layer_height]);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "time-of-day")) {
					Map.TimeOfDaySeconds.clear();
					Map.TimeOfDay.clear();
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument for \"time-of-day\"");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int z = 0; z < subsubargs; ++z) {
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument for \"time-of-day\"");
						}
						lua_rawgeti(l, -1, z + 1);
						int time_of_day_seconds = LuaToNumber(l, -1, 1);
						int time_of_day = LuaToNumber(l, -1, 2);
						Map.TimeOfDaySeconds.push_back(time_of_day_seconds);
						Map.TimeOfDay.push_back(time_of_day);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "layer-references")) {
					Map.Planes.clear();
					Map.Worlds.clear();
					Map.Layers.clear();
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument for \"layer-references\"");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int z = 0; z < subsubargs; ++z) {
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument for \"layer-references\"");
						}
						lua_rawgeti(l, -1, z + 1);
						Map.Planes.push_back(GetPlane(LuaToString(l, -1, 1)));
						Map.Worlds.push_back(GetWorld(LuaToString(l, -1, 2)));
						Map.Layers.push_back(LuaToNumber(l, -1, 3));
						Map.LayerConnectors.resize(z + 1);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "cultural-settlement-names")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument for \"cultural-settlement-names\"");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int z = 0; z < subsubargs; ++z) {
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument for \"cultural-settlement-names\"");
						}
						Map.CulturalSettlementNames.resize(z + 1);
						lua_rawgeti(l, -1, z + 1);
						const int subsubsubargs = lua_rawlen(l, -1);
						for (int n = 0; n < subsubsubargs; ++n) {
							std::string settlement_name = LuaToString(l, -1, n + 1);
							++n;
							int settlement_x = LuaToNumber(l, -1, n + 1);
							++n;
							int settlement_y = LuaToNumber(l, -1, n + 1);
							++n;
							int settlement_civilization = PlayerRaces.GetRaceIndexByName(LuaToString(l, -1, n + 1));
							if (settlement_civilization != -1) {
								Map.CulturalSettlementNames[z][std::tuple<int, int, int>(settlement_x, settlement_y, settlement_civilization)] = settlement_name;
							}
						}
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "faction-cultural-settlement-names")) {
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument for \"faction-cultural-settlement-names\"");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int z = 0; z < subsubargs; ++z) {
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument for \"faction-cultural-settlement-names\"");
						}
						Map.FactionCulturalSettlementNames.resize(z + 1);
						lua_rawgeti(l, -1, z + 1);
						const int subsubsubargs = lua_rawlen(l, -1);
						for (int n = 0; n < subsubsubargs; ++n) {
							std::string settlement_name = LuaToString(l, -1, n + 1);
							++n;
							int settlement_x = LuaToNumber(l, -1, n + 1);
							++n;
							int settlement_y = LuaToNumber(l, -1, n + 1);
							++n;
							CFaction *settlement_faction = PlayerRaces.GetFaction(-1, LuaToString(l, -1, n + 1));
							if (settlement_faction != NULL) {
								Map.FactionCulturalSettlementNames[z][std::tuple<int, int, CFaction *>(settlement_x, settlement_y, settlement_faction)] = settlement_name;
							}
						}
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				//Wyrmgus end
				} else if (!strcmp(value, "map-fields")) {
					//Wyrmgus start
					/*
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					const int subsubargs = lua_rawlen(l, -1);
					if (subsubargs != Map.Info.MapWidth * Map.Info.MapHeight) {
						fprintf(stderr, "Wrong tile table length: %d\n", subsubargs);
					}
					for (int i = 0; i < subsubargs; ++i) {
						lua_rawgeti(l, -1, i + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						Map.Fields[i].parse(l);
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
					*/
					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					const int subsubargs = lua_rawlen(l, -1);
					for (int z = 0; z < subsubargs; ++z) {
						lua_rawgeti(l, -1, z + 1);
						if (!lua_istable(l, -1)) {
							LuaError(l, "incorrect argument");
						}
						const int subsubsubargs = lua_rawlen(l, -1);
						if (subsubsubargs != Map.Info.MapWidths[z] * Map.Info.MapHeights[z]) {
							fprintf(stderr, "Wrong tile table length: %d\n", subsubsubargs);
						}
						for (int i = 0; i < subsubsubargs; ++i) {
							lua_rawgeti(l, -1, i + 1);
							if (!lua_istable(l, -1)) {
								LuaError(l, "incorrect argument");
							}
							//Wyrmgus start
	//						Map.Fields[i].parse(l);
							Map.Fields[z][i].parse(l);
							//Wyrmgus end
							lua_pop(l, 1);
						}
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
					//Wyrmgus end
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

/**
**  Reveal the complete map.
**
**  @param l  Lua state.
*/
static int CclRevealMap(lua_State *l)
{
	//Wyrmgus start
//	LuaCheckArgs(l, 0);
	//Wyrmgus end
	//Wyrmgus start
//	if (CclInConfigFile || !Map.Fields) {
	if (CclInConfigFile || Map.Fields.size() == 0) {
	//Wyrmgus end
		FlagRevealMap = 1;
	} else {
		//Wyrmgus start
//		Map.Reveal();
		bool only_person_players = false;
		const int nargs = lua_gettop(l);
		if (nargs == 1) {
			only_person_players = LuaToBoolean(l, 1);
		}
		Map.Reveal(only_person_players);
		//Wyrmgus end
	}
	return 0;
}

/**
**  Center the map.
**
**  @param l  Lua state.
*/
static int CclCenterMap(lua_State *l)
{
	LuaCheckArgs(l, 2);
	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));

	UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(pos));
	return 0;
}

/**
**  Define the starting viewpoint for a given player.
**
**  @param l  Lua state.
*/
static int CclSetStartView(lua_State *l)
{
	LuaCheckArgs(l, 3);

	const int p = LuaToNumber(l, 1);
	Players[p].StartPos.x = LuaToNumber(l, 2);
	Players[p].StartPos.y = LuaToNumber(l, 3);

	return 0;
}

/**
**  Show Map Location
**
**  @param l  Lua state.
*/
static int CclShowMapLocation(lua_State *l)
{
	// Put a unit on map, use its properties, except for
	// what is listed below

	LuaCheckArgs(l, 4);
	const char *unitname = LuaToString(l, 5);
	CUnitType *unitType = UnitTypeByIdent(unitname);
	if (!unitType) {
		DebugPrint("Unable to find UnitType '%s'" _C_ unitname);
		return 0;
	}
	CUnit *target = MakeUnit(*unitType, ThisPlayer);
	if (target != NULL) {
		target->Variable[HP_INDEX].Value = 0;
		target->tilePos.x = LuaToNumber(l, 1);
		target->tilePos.y = LuaToNumber(l, 2);
		target->TTL = GameCycle + LuaToNumber(l, 4);
		target->CurrentSightRange = LuaToNumber(l, 3);
		//Wyrmgus start
		UpdateUnitSightRange(*target);
		//Wyrmgus end
		MapMarkUnitSight(*target);
	} else {
		DebugPrint("Unable to allocate Unit");
	}
	return 0;
}

/**
**  Set fog of war on/off.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWar(lua_State *l)
{
	LuaCheckArgs(l, 1);
	Map.NoFogOfWar = !LuaToBoolean(l, 1);
	//Wyrmgus start
//	if (!CclInConfigFile && Map.Fields) {
	if (!CclInConfigFile && Map.Fields.size() > 0) {
	//Wyrmgus end
		UpdateFogOfWarChange();
		// FIXME: save setting in replay log
		//CommandLog("input", NoUnitP, FlushCommands, -1, -1, NoUnitP, "fow off", -1);
	}
	return 0;
}

static int CclGetFogOfWar(lua_State *l)
{
	LuaCheckArgs(l, 0);
	lua_pushboolean(l, !Map.NoFogOfWar);
	return 1;
}

/**
**  Enable display of terrain in minimap.
**
**  @param l  Lua state.
*/
static int CclSetMinimapTerrain(lua_State *l)
{
	LuaCheckArgs(l, 1);
	UI.Minimap.WithTerrain = LuaToBoolean(l, 1);
	return 0;
}

/**
**  Fog of war opacity.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarOpacity(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int i = LuaToNumber(l, 1);
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Opacity should be 0 - 256\n");
		i = 100;
	}
	FogOfWarOpacity = i;

	if (!CclInConfigFile) {
		Map.Init();
	}
	return 0;
}

/**
**  Set forest regeneration speed.
**
**  @param l  Lua state.
**
**  @return   Old speed
*/
static int CclSetForestRegeneration(lua_State *l)
{
	LuaCheckArgs(l, 1);
	int i = LuaToNumber(l, 1);
	//Wyrmgus start
	/*
	if (i < 0 || i > 255) {
		PrintFunction();
		fprintf(stdout, "Regeneration speed should be 0 - 255\n");
		i = 100;
	}
	*/
	if (i < 0) {
		PrintFunction();
		fprintf(stdout, "Regeneration speed should be greater than 0\n");
		i = 100;
	}
	//Wyrmgus end
	const int old = ForestRegeneration;
	ForestRegeneration = i;

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Set Fog color.
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarColor(lua_State *l)
{
	LuaCheckArgs(l, 3);
	int r = LuaToNumber(l, 1);
	int g = LuaToNumber(l, 2);
	int b = LuaToNumber(l, 3);

	if ((r < 0 || r > 255) ||
		(g < 0 || g > 255) ||
		(b < 0 || b > 255)) {
		LuaError(l, "Arguments must be in the range 0-255");
	}
	FogOfWarColor.R = r;
	FogOfWarColor.G = g;
	FogOfWarColor.B = b;

	return 0;
}

/**
**  Define Fog graphics
**
**  @param l  Lua state.
*/
static int CclSetFogOfWarGraphics(lua_State *l)
{
	std::string FogGraphicFile;

	LuaCheckArgs(l, 1);
	FogGraphicFile = LuaToString(l, 1);
	if (CMap::FogGraphic) {
		CGraphic::Free(CMap::FogGraphic);
	}
	CMap::FogGraphic = CGraphic::New(FogGraphicFile, PixelTileSize.x, PixelTileSize.y);

	return 0;
}

/**
**  Set a tile
**
**  @param tileIndex   Tile number
**  @param pos    coordinate
**  @param value  Value of the tile
*/
//Wyrmgus start
//void SetTile(unsigned int tileIndex, const Vec2i &pos, int value)
void SetTile(unsigned int tileIndex, const Vec2i &pos, int value, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	if (!Map.Info.IsPointOnMap(pos)) {
	if (!Map.Info.IsPointOnMap(pos, z)) {
	//Wyrmgus end
		fprintf(stderr, "Invalid map coordonate : (%d, %d)\n", pos.x, pos.y);
		return;
	}
	if (Map.Tileset->getTileCount() <= tileIndex) {
		fprintf(stderr, "Invalid tile number: %d\n", tileIndex);
		return;
	}
	//Wyrmgus start
//	if (value < 0 || value >= 256) {
	if (value < 0) {
	//Wyrmgus end
		//Wyrmgus start
//		fprintf(stderr, "Invalid tile number: %d\n", tileIndex);
		fprintf(stderr, "Invalid tile value: %d\n", value);
		//Wyrmgus end
		return;
	}
	
	//Wyrmgus start
//	if (Map.Fields) {
	if (Map.Fields.size() > 0) {
	//Wyrmgus end
		//Wyrmgus start
//		CMapField &mf = *Map.Field(pos);
		CMapField &mf = *Map.Field(pos, z);
		//Wyrmgus end

		mf.setTileIndex(*Map.Tileset, tileIndex, value);
	}
}

//Wyrmgus start
/**
**  Set a tile
**
**  @param tileIndex   Tile number
**  @param pos    coordinate
**  @param value  Value of the tile
*/
void SetTileTerrain(std::string terrain_ident, const Vec2i &pos, int value, int z)
{
	if (!Map.Info.IsPointOnMap(pos, z)) {
		fprintf(stderr, "Invalid map coordinate : (%d, %d)\n", pos.x, pos.y);
		return;
	}
	
	CTerrainType *terrain = GetTerrainType(terrain_ident);
	
	if (!terrain) {
		fprintf(stderr, "Terrain \"%s\" doesn't exist.\n", terrain_ident.c_str());
		return;
	}
	if (value < 0) {
		fprintf(stderr, "Invalid tile value: %d\n", value);
		return;
	}
	
	if (Map.Fields.size() > 0) {
		CMapField &mf = *Map.Field(pos, z);

		mf.Value = value;
		mf.SetTerrain(terrain);
	}
}

static int CclSetMapTemplateTileTerrain(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	std::string terrain_ident = LuaToString(l, 2);
	CTerrainType *terrain = NULL;
	if (!terrain_ident.empty()) {
		terrain = GetTerrainType(terrain_ident);
		if (!terrain) {
			LuaError(l, "Terrain doesn't exist");
		}
	}

	Vec2i pos;
	CclGetPos(l, &pos.x, &pos.y, 3);
	
	if (pos.x < 0 || pos.x >= map_template->Width || pos.y < 0 || pos.y >= map_template->Height) {
		LuaError(l, "Invalid map coordinate : (%d, %d)" _C_ pos.x _C_ pos.y);
	}

	CDate date;
	date.year = 0;
	date.month = 1;
	date.day = 1;
	const int nargs = lua_gettop(l);
	if (nargs >= 4) {
		CclGetDate(l, &date, 4);
	}
	
	map_template->HistoricalTerrains.push_back(std::tuple<Vec2i, CTerrainType *, CDate>(pos, terrain, date));

	if (nargs >= 5) {
		map_template->TileLabels[std::pair<int, int>(pos.x, pos.y)] = LuaToString(l, 5);
	}
	
	return 1;
}

static int CclSetMapTemplateTileLabel(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	std::string label_string = LuaToString(l, 2);
	
	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 3);

	map_template->TileLabels[std::pair<int, int>(ipos.x, ipos.y)] = TransliterateText(label_string);
	
	return 1;
}

static int CclSetMapTemplatePathway(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	std::string terrain_ident = LuaToString(l, 2);
	CTerrainType *terrain = NULL;
	if (!terrain_ident.empty()) {
		terrain = GetTerrainType(terrain_ident);
		if (!terrain) {
			LuaError(l, "Terrain doesn't exist");
		}
	}

	Vec2i start_pos;
	if (lua_istable(l, 3)) { //coordinates
		CclGetPos(l, &start_pos.x, &start_pos.y, 3);
	} else { //settlement ident
		std::string settlement_ident = LuaToString(l, 3);
		CSettlement *settlement = GetSettlement(settlement_ident);
		if (!settlement) {
			LuaError(l, "Settlement \"%s\" doesn't exist.\n" _C_ settlement_ident.c_str());
		}
		start_pos.x = settlement->Position.x;
		start_pos.y = settlement->Position.y;
	}
	
	if (start_pos.x < 0 || start_pos.x >= map_template->Width || start_pos.y < 0 || start_pos.y >= map_template->Height) {
		LuaError(l, "Invalid map coordinate : (%d, %d)" _C_ start_pos.x _C_ start_pos.y);
	}

	Vec2i end_pos;
	if (lua_istable(l, 4)) { //coordinates
		CclGetPos(l, &end_pos.x, &end_pos.y, 4);
	} else { //settlement ident
		std::string settlement_ident = LuaToString(l, 4);
		CSettlement *settlement = GetSettlement(settlement_ident);
		if (!settlement) {
			LuaError(l, "Settlement \"%s\" doesn't exist.\n" _C_ settlement_ident.c_str());
		}
		end_pos.x = settlement->Position.x;
		end_pos.y = settlement->Position.y;
	}
	
	if (end_pos.x < 0 || end_pos.x >= map_template->Width || end_pos.y < 0 || end_pos.y >= map_template->Height) {
		LuaError(l, "Invalid map coordinate : (%d, %d)" _C_ end_pos.x _C_ end_pos.y);
	}

	CDate date;
	date.year = 0;
	date.month = 1;
	date.day = 1;
	const int nargs = lua_gettop(l);
	if (nargs >= 5) {
		CclGetDate(l, &date, 5);
	}
	
	Vec2i pos(start_pos);
	Vec2i pathway_length(end_pos - start_pos);
	Vec2i pathway_change(pathway_length.x ? pathway_length.x / abs(pathway_length.x) : 0, pathway_length.y ? pathway_length.y / abs(pathway_length.y) : 0);
	pathway_length.x = abs(pathway_length.x);
	pathway_length.y = abs(pathway_length.y);
	int offset = 0;
	while (pos != end_pos) {
		Vec2i current_length(pos - start_pos);
		current_length.x = abs(current_length.x);
		current_length.y = abs(current_length.y);
		if (pathway_length.x == pathway_length.y) {
			pos += pathway_change;
		} else if (pathway_length.x > pathway_length.y) {
			pos.x += pathway_change.x;
			if (pathway_length.y && pos.y != end_pos.y) {
				if (pathway_length.x % pathway_length.y != 0 && current_length.x % (pathway_length.x / (pathway_length.x % pathway_length.y)) == 0) {
					offset += 1;
				} else if ((current_length.x - offset) % (std::max(1, pathway_length.x / pathway_length.y)) == 0) {
					map_template->HistoricalTerrains.push_back(std::tuple<Vec2i, CTerrainType *, CDate>(Vec2i(pos), terrain, date));
					pos.y += pathway_change.y;
				}
			}
		} else if (pathway_length.y > pathway_length.x) {
			pos.y += pathway_change.y;
			if (pathway_length.x && pos.x != end_pos.x) {
				if (pathway_length.y % pathway_length.x != 0 && current_length.y % (pathway_length.y / (pathway_length.y % pathway_length.x)) == 0) {
					offset += 1;
				} else if ((current_length.y - offset) % (std::max(1, pathway_length.y / pathway_length.x)) == 0) {
					map_template->HistoricalTerrains.push_back(std::tuple<Vec2i, CTerrainType *, CDate>(Vec2i(pos), terrain, date));
					pos.x += pathway_change.x;
				}
			}
		}

		if (pos.x < 0 || pos.x >= map_template->Width || pos.y < 0 || pos.y >= map_template->Height) {
			break;
		}

		map_template->HistoricalTerrains.push_back(std::tuple<Vec2i, CTerrainType *, CDate>(Vec2i(pos), terrain, date));
	}

	return 1;
}

static int CclSetMapTemplateCulturalSettlementName(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	std::string settlement_name = LuaToString(l, 2);
	
	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 3);

	std::string civilization_ident = LuaToString(l, 4);
	int civilization = PlayerRaces.GetRaceIndexByName(civilization_ident.c_str());
	if (civilization == -1) {
		LuaError(l, "Civilization \"%s\" doesn't exist.\n" _C_ civilization_ident.c_str());
	}

	map_template->CulturalSettlementNames[std::tuple<int, int, int>(ipos.x, ipos.y, civilization)] = settlement_name;
	PlayerRaces.Civilizations[civilization]->SettlementNames.push_back(settlement_name);
	
	return 1;
}

static int CclSetMapTemplateFactionCulturalSettlementName(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	std::string settlement_name = LuaToString(l, 2);
	
	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 3);

	std::string faction_ident = LuaToString(l, 4);
	CFaction *faction = PlayerRaces.GetFaction(-1, faction_ident);
	if (!faction) {
		LuaError(l, "Faction \"%s\" doesn't exist.\n" _C_ faction_ident.c_str());
	}

	map_template->FactionCulturalSettlementNames[std::tuple<int, int, CFaction *>(ipos.x, ipos.y, faction)] = settlement_name;
	faction->SettlementNames.push_back(settlement_name);
	
	return 1;
}

static int CclSetMapTemplateResource(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	lua_pushvalue(l, 2);
	CUnitType *unittype = CclGetUnitType(l);
	if (unittype == NULL) {
		LuaError(l, "Bad unittype");
	}
	lua_pop(l, 1);
	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 3);

	int resources_held = 0;
	CUniqueItem *unique = NULL;
	
	const int nargs = lua_gettop(l);
	if (nargs >= 4) {
		resources_held = LuaToNumber(l, 4);
	}
	if (nargs >= 5) {
		unique = GetUniqueItem(LuaToString(l, 5));
		if (!unique) {
			LuaError(l, "Unique item doesn't exist.\n");
		}
	}
	
	map_template->Resources[std::pair<int, int>(ipos.x, ipos.y)] = std::tuple<CUnitType *, int, CUniqueItem *>(unittype, resources_held, unique);
	
	return 1;
}

static int CclSetMapTemplateUnit(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	lua_pushvalue(l, 2);
	CUnitType *unittype = CclGetUnitType(l);
	if (unittype == NULL) {
		LuaError(l, "Bad unittype");
	}
	lua_pop(l, 1);
	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 4);

	std::string faction_name = LuaToString(l, 3);
	CFaction *faction = PlayerRaces.GetFaction(-1, faction_name);

	int start_year = 0;
	int end_year = 0;
	CUniqueItem *unique = NULL;
	const int nargs = lua_gettop(l);
	if (nargs >= 5) {
		start_year = LuaToNumber(l, 5);
	}
	if (nargs >= 6) {
		end_year = LuaToNumber(l, 6);
	}
	if (nargs >= 7) {
		unique = GetUniqueItem(LuaToString(l, 7));
		if (!unique) {
			LuaError(l, "Unique item doesn't exist.\n");
		}
	}
	
	map_template->Units.push_back(std::tuple<Vec2i, CUnitType *, CFaction *, int, int, CUniqueItem *>(ipos, unittype, faction, start_year, end_year, unique));
	
	return 1;
}

static int CclSetMapTemplateHero(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	CCharacter *hero = GetCharacter(LuaToString(l, 2));
	if (hero == NULL) {
		LuaError(l, "Hero doesn't exist");
	}

	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 4);

	std::string faction_name = LuaToString(l, 3);
	CFaction *faction = PlayerRaces.GetFaction(-1, faction_name);
	if (!faction) {
		LuaError(l, "Faction doesn't exist.\n");
	}

	CDate start_date;
	CDate end_date;
	start_date.year = 0;
	start_date.month = 1;
	start_date.day = 1;
	end_date.year = 0;
	end_date.month = 1;
	end_date.day = 1;
	const int nargs = lua_gettop(l);
	if (nargs >= 5) {
		CclGetDate(l, &start_date, 5);
	}
	if (nargs >= 6) {
		CclGetDate(l, &end_date, 6);
	}
	
	map_template->Heroes.push_back(std::tuple<Vec2i, CCharacter *, CFaction *, CDate, CDate>(ipos, hero, faction, start_date, end_date));
	
	return 1;
}

static int CclSetMapTemplateLayerConnector(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	lua_pushvalue(l, 2);
	CUnitType *unittype = CclGetUnitType(l);
	if (unittype == NULL) {
		LuaError(l, "Bad unittype");
	}
	lua_pop(l, 1);
	Vec2i ipos;
	CclGetPos(l, &ipos.x, &ipos.y, 3);

	CUniqueItem *unique = NULL;
	
	const int nargs = lua_gettop(l);
	if (nargs >= 5) {
		unique = GetUniqueItem(LuaToString(l, 5));
		if (!unique) {
			LuaError(l, "Unique item doesn't exist.\n");
		}
	}
	
	if (lua_isnumber(l, 4)) {
		int layer = LuaToNumber(l, 4);
		map_template->LayerConnectors.push_back(std::tuple<Vec2i, CUnitType *, int, CUniqueItem *>(ipos, unittype, layer, unique));
	} else if (lua_isstring(l, 4)) {
		std::string realm = LuaToString(l, 4);
		if (GetWorld(realm)) {
			map_template->WorldConnectors.push_back(std::tuple<Vec2i, CUnitType *, CWorld *, CUniqueItem *>(ipos, unittype, GetWorld(realm), unique));
		} else if (GetPlane(realm)) {
			map_template->PlaneConnectors.push_back(std::tuple<Vec2i, CUnitType *, CPlane *, CUniqueItem *>(ipos, unittype, GetPlane(realm), unique));
		} else {
			LuaError(l, "incorrect argument");
		}
	} else {
		LuaError(l, "incorrect argument");
	}
	
	return 1;
}

/*
static std::string map_terrains[64][64];

static int CclCreateMapTemplateTerrainFile(lua_State *l)
{
	std::string map_template_ident = LuaToString(l, 1);
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	if (!map_template) {
		LuaError(l, "Map template doesn't exist.\n");
	}

	bool overlay = LuaToBoolean(l, 2);

	FileWriter *fw = NULL;
	std::string map_filename = "scripts/map_templates/" + map_template_ident;
	if (overlay) {
		map_filename += "_overlay";
	}
	map_filename += ".map";
	
	for (int x = 0; x < map_template->Width; ++x) {
		for (int y = 0; y < map_template->Height; ++y) {
			unsigned int index = x + y * map_template->Width;
			CTerrainType *terrain = NULL;
			if (!overlay && index < map_template->TileTerrains.size() && map_template->TileTerrains[index] != -1) {
				terrain = TerrainTypes[map_template->TileTerrains[index]];
			} else if (overlay && index < map_template->TileOverlayTerrains.size() && map_template->TileOverlayTerrains[index] != -1) {
				terrain = TerrainTypes[map_template->TileOverlayTerrains[index]];
			}
			if (terrain && !terrain->Character.empty()) {
				map_terrains[x][y] = terrain->Character;
			} else {
				map_terrains[x][y] = "0";
			}
		}
	}

	try {
		fw = CreateFileWriter(map_filename);

		for (int y = 0; y < map_template->Height; ++y) {
			for (int x = 0; x < map_template->Width; ++x) {
				fw->printf("%s", map_terrains[x][y].c_str());
			}
			fw->printf("\n");
		}
			
		fw->printf("\n");
	} catch (const FileException &) {
		fprintf(stderr, "Couldn't write the map setup: \"%s\"\n", map_filename.c_str());
		delete fw;
		return 1;
	}
	
	delete fw;
	
	return 1;
}
*/

void ApplyMapTemplate(std::string map_template_ident, int template_start_x, int template_start_y, int map_start_x, int map_start_y, int z)
{
	CMapTemplate *map_template = GetMapTemplate(map_template_ident);
	
	if (!map_template) {
		fprintf(stderr, "Map template \"%s\" doesn't exist.\n", map_template_ident.c_str());
		return;
	}
	
	map_template->Apply(Vec2i(template_start_x, template_start_y), Vec2i(map_start_x, map_start_y), z);
}
//Wyrmgus end

/**
**  Define the type of each player available for the map
**
**  @param l  Lua state.
*/
static int CclDefinePlayerTypes(lua_State *l)
{
	int numplayers = lua_gettop(l); /* Number of players == number of arguments */
	if (numplayers < 2) {
		LuaError(l, "Not enough players");
	}

	for (int i = 0; i < numplayers && i < PlayerMax; ++i) {
		if (lua_isnil(l, i + 1)) {
			numplayers = i;
			break;
		}
		const char *type = LuaToString(l, i + 1);
		if (!strcmp(type, "neutral")) {
			Map.Info.PlayerType[i] = PlayerNeutral;
		} else if (!strcmp(type, "nobody")) {
			Map.Info.PlayerType[i] = PlayerNobody;
		} else if (!strcmp(type, "computer")) {
			Map.Info.PlayerType[i] = PlayerComputer;
		} else if (!strcmp(type, "person")) {
			Map.Info.PlayerType[i] = PlayerPerson;
		} else if (!strcmp(type, "rescue-passive")) {
			Map.Info.PlayerType[i] = PlayerRescuePassive;
		} else if (!strcmp(type, "rescue-active")) {
			Map.Info.PlayerType[i] = PlayerRescueActive;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ type);
		}
	}
	for (int i = numplayers; i < PlayerMax - 1; ++i) {
		Map.Info.PlayerType[i] = PlayerNobody;
	}
	if (numplayers < PlayerMax) {
		Map.Info.PlayerType[PlayerMax - 1] = PlayerNeutral;
	}
	return 0;
}

/**
** Load the lua file which will define the tile models
**
**  @param l  Lua state.
*/
static int CclLoadTileModels(lua_State *l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	Map.TileModelsFileName = LuaToString(l, 1);
	const std::string filename = LibraryFileName(Map.TileModelsFileName.c_str());
	if (LuaLoadFile(filename) == -1) {
		DebugPrint("Load failed: %s\n" _C_ filename.c_str());
	}
	return 0;
}

/**
**  Define tileset
**
**  @param l  Lua state.
*/
static int CclDefineTileset(lua_State *l)
{
	Map.Tileset->parse(l);

	//  Load and prepare the tileset
	PixelTileSize = Map.Tileset->getPixelTileSize();

	ShowLoadProgress(_("Loading Tileset \"%s\""), Map.Tileset->ImageFile.c_str());
	Map.TileGraphic = CGraphic::New(Map.Tileset->ImageFile, PixelTileSize.x, PixelTileSize.y);
	Map.TileGraphic->Load();
	//Wyrmgus start
	for (size_t i = 0; i != Map.Tileset->solidTerrainTypes.size(); ++i) {
		if (!Map.Tileset->solidTerrainTypes[i].ImageFile.empty()) {
			Map.SolidTileGraphics[i] = CGraphic::New(Map.Tileset->solidTerrainTypes[i].ImageFile, PixelTileSize.x, PixelTileSize.y);
			Map.SolidTileGraphics[i]->Load();
		}
	}
	//Wyrmgus end
	return 0;
}
/**
** Build tileset tables like humanWallTable or mixedLookupTable
**
** Called after DefineTileset and only for tilesets that have wall,
** trees and rocks. This function will be deleted when removing
** support of walls and alike in the tileset.
*/
static int CclBuildTilesetTables(lua_State *l)
{
	LuaCheckArgs(l, 0);

	Map.Tileset->buildTable(l);
	return 0;
}
/**
**  Set the flags like "water" for a tile of a tileset
**
**  @param l  Lua state.
*/
static int CclSetTileFlags(lua_State *l)
{
	if (lua_gettop(l) < 2) {
		LuaError(l, "No flags defined");
	}
	const unsigned int tilenumber = LuaToNumber(l, 1);

	if (tilenumber >= Map.Tileset->tiles.size()) {
		LuaError(l, "Accessed a tile that's not defined");
	}
	int j = 0;
	int flags = 0;

	ParseTilesetTileFlags(l, &flags, &j);
	Map.Tileset->tiles[tilenumber].flag = flags;
	return 0;
}

//Wyrmgus start
/**
**  Get the ident of the current tileset.
**
**  @param l  Lua state.
**
**  @return   The name of the terrain of the tile.
*/
static int CclGetCurrentTileset(lua_State *l)
{
	const CTileset &tileset = *Map.Tileset;
	lua_pushstring(l, tileset.Ident.c_str());
	return 1;
}
//Wyrmgus end

/**
**  Get the name of the terrain of the tile.
**
**  @param l  Lua state.
**
**  @return   The name of the terrain of the tile.
*/
static int CclGetTileTerrainName(lua_State *l)
{
	//Wyrmgus start
//	LuaCheckArgs(l, 2);
	int z = 0;
	const int nargs = lua_gettop(l);
	if (nargs >= 3) {
		z = LuaToNumber(l, 3);
	}
	//Wyrmgus end

	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));

	//Wyrmgus start
	/*
	const CMapField &mf = *Map.Field(pos);
	const CTileset &tileset = *Map.Tileset;
	const int index = tileset.findTileIndexByTile(mf.getGraphicTile());
	Assert(index != -1);
	const int baseTerrainIdx = tileset.tiles[index].tileinfo.BaseTerrain;

	lua_pushstring(l, tileset.getTerrainName(baseTerrainIdx).c_str());
	*/
	lua_pushstring(l, Map.GetTileTopTerrain(pos, false, z)->Ident.c_str());
	//Wyrmgus end
	return 1;
}

/**
**  Get the name of the mixed terrain of the tile.
**
**  @param l  Lua state.
**
**  @return   The name of the terrain of the tile.
*/
//Wyrmgus start
/*
static int CclGetTileTerrainMixedName(lua_State *l)
{
	LuaCheckArgs(l, 2);

	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));

	const CMapField &mf = *Map.Field(pos);
	const CTileset &tileset = *Map.Tileset;
	//Wyrmgus start
//	const int index = tileset.findTileIndexByTile(mf.getGraphicTile());
	const int index = mf.getTileIndex();
	//Wyrmgus end
	Assert(index != -1);
	const int mixTerrainIdx = tileset.tiles[index].tileinfo.MixTerrain;

	lua_pushstring(l, mixTerrainIdx ? tileset.getTerrainName(mixTerrainIdx).c_str() : "");
	return 1;
}
*/
//Wyrmgus end

/**
**  Check if the tile's terrain has a particular flag.
**
**  @param l  Lua state.
**
**  @return   True if has the flag, false if not.
*/
static int CclGetTileTerrainHasFlag(lua_State *l)
{
	//Wyrmgus start
//	LuaCheckArgs(l, 3);
	int z = 0;
	const int nargs = lua_gettop(l);
	if (nargs >= 4) {
		z = LuaToNumber(l, 4);
	}
	//Wyrmgus end

	const Vec2i pos(LuaToNumber(l, 1), LuaToNumber(l, 2));

	//Wyrmgus start
	if (pos.x < 0 || pos.x >= Map.Info.MapWidths[z] || pos.y < 0 || pos.y >= Map.Info.MapHeights[z]) {
		lua_pushboolean(l, 0);
		return 1;
	}
	
//	unsigned short flag = 0;
	unsigned long flag = 0;
	//Wyrmgus end
	const char *flag_name = LuaToString(l, 3);
	if (!strcmp(flag_name, "water")) {
		flag = MapFieldWaterAllowed;
	} else if (!strcmp(flag_name, "land")) {
		flag = MapFieldLandAllowed;
	} else if (!strcmp(flag_name, "coast")) {
		flag = MapFieldCoastAllowed;
	} else if (!strcmp(flag_name, "no-building")) {
		flag = MapFieldNoBuilding;
	} else if (!strcmp(flag_name, "unpassable")) {
		flag = MapFieldUnpassable;
	//Wyrmgus start
	} else if (!strcmp(flag_name, "air-unpassable")) {
		flag = MapFieldAirUnpassable;
	} else if (!strcmp(flag_name, "dirt")) {
		flag = MapFieldDirt;
	} else if (!strcmp(flag_name, "grass")) {
		flag = MapFieldGrass;
	} else if (!strcmp(flag_name, "gravel")) {
		flag = MapFieldGravel;
	} else if (!strcmp(flag_name, "mud")) {
		flag = MapFieldMud;
	} else if (!strcmp(flag_name, "railroad")) {
		flag = MapFieldRailroad;
	} else if (!strcmp(flag_name, "road")) {
		flag = MapFieldRoad;
	} else if (!strcmp(flag_name, "no-rail")) {
		flag = MapFieldNoRail;
	} else if (!strcmp(flag_name, "stone-floor")) {
		flag = MapFieldStoneFloor;
	} else if (!strcmp(flag_name, "stumps")) {
		flag = MapFieldStumps;
	//Wyrmgus end
	} else if (!strcmp(flag_name, "wall")) {
		flag = MapFieldWall;
	} else if (!strcmp(flag_name, "rock")) {
		flag = MapFieldRocks;
	} else if (!strcmp(flag_name, "forest")) {
		flag = MapFieldForest;
	}

	//Wyrmgus start
//	const CMapField &mf = *Map.Field(pos);
	const CMapField &mf = *Map.Field(pos, z);
	//Wyrmgus end

	if (mf.getFlag() & flag) {
		lua_pushboolean(l, 1);
	} else {
		lua_pushboolean(l, 0);
	}

	return 1;
}

//Wyrmgus start
/**
**  Define a terrain type.
**
**  @param l  Lua state.
*/
static int CclDefineTerrainType(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument (expected table)");
	}

	std::string terrain_ident = LuaToString(l, 1);
	CTerrainType *terrain = GetTerrainType(terrain_ident);
	if (terrain == NULL) {
		terrain = new CTerrainType;
		terrain->Ident = terrain_ident;
		terrain->ID = TerrainTypes.size();
		TerrainTypes.push_back(terrain);
		TerrainTypeStringToIndex[terrain_ident] = terrain->ID;
	}
	
	//  Parse the list:
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		const char *value = LuaToString(l, -2);
		
		if (!strcmp(value, "Name")) {
			terrain->Name = LuaToString(l, -1);
		} else if (!strcmp(value, "Character")) {
			terrain->Character = LuaToString(l, -1);
			if (TerrainTypeCharacterToIndex.find(terrain->Character) != TerrainTypeCharacterToIndex.end()) {
				LuaError(l, "Character \"%s\" is already used by another terrain type." _C_ terrain->Character.c_str());
			}
			TerrainTypeCharacterToIndex[terrain->Character] = terrain->ID;
		} else if (!strcmp(value, "Color")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			terrain->Color.R = LuaToNumber(l, -1, 1);
			terrain->Color.G = LuaToNumber(l, -1, 2);
			terrain->Color.B = LuaToNumber(l, -1, 3);
			if (TerrainTypeColorToIndex.find(std::tuple<int, int, int>(terrain->Color.R, terrain->Color.G, terrain->Color.B)) != TerrainTypeColorToIndex.end()) {
				LuaError(l, "Color is already used by another terrain type.");
			}
			if (TerrainFeatureColorToIndex.find(std::tuple<int, int, int>(terrain->Color.R, terrain->Color.G, terrain->Color.B)) != TerrainFeatureColorToIndex.end()) {
				LuaError(l, "Color is already used by a terrain feature.");
			}
			TerrainTypeColorToIndex[std::tuple<int, int, int>(terrain->Color.R, terrain->Color.G, terrain->Color.B)] = terrain->ID;
		} else if (!strcmp(value, "Overlay")) {
			terrain->Overlay = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Buildable")) {
			terrain->Buildable = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AllowSingle")) {
			terrain->AllowSingle = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SolidAnimationFrames")) {
			terrain->SolidAnimationFrames = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BaseTerrains")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CTerrainType *base_terrain = GetTerrainType(LuaToString(l, -1, j + 1));
				if (base_terrain == NULL) {
					LuaError(l, "Terrain doesn't exist.");
				}
				terrain->BaseTerrains.push_back(base_terrain);
			}
		} else if (!strcmp(value, "InnerBorderTerrains")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CTerrainType *border_terrain = GetTerrainType(LuaToString(l, -1, j + 1));
				if (border_terrain == NULL) {
					LuaError(l, "Terrain doesn't exist.");
				}
				terrain->InnerBorderTerrains.push_back(border_terrain);
				terrain->BorderTerrains.push_back(border_terrain);
				border_terrain->OuterBorderTerrains.push_back(terrain);
				border_terrain->BorderTerrains.push_back(terrain);
			}
		} else if (!strcmp(value, "OuterBorderTerrains")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CTerrainType *border_terrain = GetTerrainType(LuaToString(l, -1, j + 1));
				if (border_terrain == NULL) {
					LuaError(l, "Terrain doesn't exist.");
				}
				terrain->OuterBorderTerrains.push_back(border_terrain);
				terrain->BorderTerrains.push_back(border_terrain);
				border_terrain->InnerBorderTerrains.push_back(terrain);
				border_terrain->BorderTerrains.push_back(terrain);
			}
		} else if (!strcmp(value, "OverlayTerrains")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CTerrainType *overlay_terrain = GetTerrainType(LuaToString(l, -1, j + 1));
				if (overlay_terrain == NULL) {
					LuaError(l, "Terrain doesn't exist.");
				}
				overlay_terrain->BaseTerrains.push_back(terrain);
			}
		} else if (!strcmp(value, "Flags")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			terrain->Flags = 0;
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				std::string tile_flag = LuaToString(l, -1, j + 1);
				if (tile_flag == "land") {
					terrain->Flags |= MapFieldLandAllowed;
				} else if (tile_flag == "coast") {
					terrain->Flags |= MapFieldCoastAllowed;
				} else if (tile_flag == "water") {
					terrain->Flags |= MapFieldWaterAllowed;
				} else if (tile_flag == "no-building") {
					terrain->Flags |= MapFieldNoBuilding;
				} else if (tile_flag == "unpassable") {
					terrain->Flags |= MapFieldUnpassable;
				} else if (tile_flag == "wall") {
					terrain->Flags |= MapFieldWall;
				} else if (tile_flag == "rock") {
					terrain->Flags |= MapFieldRocks;
				} else if (tile_flag == "forest") {
					terrain->Flags |= MapFieldForest;
				} else if (tile_flag == "air-unpassable") {
					terrain->Flags |= MapFieldAirUnpassable;
				} else if (tile_flag == "dirt") {
					terrain->Flags |= MapFieldDirt;
				} else if (tile_flag == "grass") {
					terrain->Flags |= MapFieldGrass;
				} else if (tile_flag == "gravel") {
					terrain->Flags |= MapFieldGravel;
				} else if (tile_flag == "mud") {
					terrain->Flags |= MapFieldMud;
				} else if (tile_flag == "railroad") {
					terrain->Flags |= MapFieldRailroad;
				} else if (tile_flag == "road") {
					terrain->Flags |= MapFieldRoad;
				} else if (tile_flag == "no-rail") {
					terrain->Flags |= MapFieldNoRail;
				} else if (tile_flag == "stone-floor") {
					terrain->Flags |= MapFieldStoneFloor;
				} else if (tile_flag == "stumps") {
					terrain->Flags |= MapFieldStumps;
				} else {
					LuaError(l, "Flag \"%s\" doesn't exist." _C_ tile_flag.c_str());
				}
			}
		} else if (!strcmp(value, "Graphics")) {
			std::string graphics_file = LuaToString(l, -1);
			if (!CanAccessFile(graphics_file.c_str())) {
				LuaError(l, "File \"%s\" doesn't exist." _C_ graphics_file.c_str());
			}
			if (CGraphic::Get(graphics_file) == NULL) {
				CGraphic *graphics = CGraphic::New(graphics_file, 32, 32);
			}
			terrain->Graphics = CGraphic::Get(graphics_file);
		} else if (!strcmp(value, "SolidTiles")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				terrain->SolidTiles.push_back(LuaToNumber(l, -1, j + 1));
			}
		} else if (!strcmp(value, "DamagedTiles")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				terrain->DamagedTiles.push_back(LuaToNumber(l, -1, j + 1));
			}
		} else if (!strcmp(value, "DestroyedTiles")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				terrain->DestroyedTiles.push_back(LuaToNumber(l, -1, j + 1));
			}
		} else if (!strcmp(value, "TransitionTiles")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				std::string transition_terrain_name = LuaToString(l, -1, j + 1);
				CTerrainType *transition_terrain = GetTerrainType(transition_terrain_name);
				if (transition_terrain == NULL && transition_terrain_name != "any") {
					LuaError(l, "Terrain doesn't exist.");
				}
				int transition_terrain_id = transition_terrain_name == "any" ? -1 : transition_terrain->ID;
				++j;
				
				int transition_type = GetTransitionTypeIdByName(LuaToString(l, -1, j + 1));
				if (transition_type == -1) {
					LuaError(l, "Transition type doesn't exist.");
				}
				++j;
				
				terrain->TransitionTiles[std::tuple<int, int>(transition_terrain_id, transition_type)].push_back(LuaToNumber(l, -1, j + 1));
			}
		} else if (!strcmp(value, "AdjacentTransitionTiles")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				std::string transition_terrain_name = LuaToString(l, -1, j + 1);
				CTerrainType *transition_terrain = GetTerrainType(transition_terrain_name);
				if (transition_terrain == NULL && transition_terrain_name != "any") {
					LuaError(l, "Terrain doesn't exist.");
				}
				int transition_terrain_id = transition_terrain_name == "any" ? -1 : transition_terrain->ID;
				++j;
				
				int transition_type = GetTransitionTypeIdByName(LuaToString(l, -1, j + 1));
				if (transition_type == -1) {
					LuaError(l, "Transition type doesn't exist.");
				}
				++j;
				
				terrain->AdjacentTransitionTiles[std::tuple<int, int>(transition_terrain_id, transition_type)].push_back(LuaToNumber(l, -1, j + 1));
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	
	return 0;
}

/**
**  Define a map template.
**
**  @param l  Lua state.
*/
static int CclDefineMapTemplate(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument (expected table)");
	}

	std::string map_ident = LuaToString(l, 1);
	CMapTemplate *map = GetMapTemplate(map_ident);
	if (map == NULL) {
		map = new CMapTemplate;
		map->Ident = map_ident;
		MapTemplates.push_back(map);
		MapTemplateIdentToPointer[map_ident] = map;
	}
	
	//  Parse the list:
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		const char *value = LuaToString(l, -2);
		
		if (!strcmp(value, "Name")) {
			map->Name = LuaToString(l, -1);
		} else if (!strcmp(value, "Plane")) {
			CPlane *plane = GetPlane(LuaToString(l, -1));
			if (!plane) {
				LuaError(l, "Plane doesn't exist.");
			}
			map->Plane = plane;
		} else if (!strcmp(value, "World")) {
			CWorld *world = GetWorld(LuaToString(l, -1));
			if (!world) {
				LuaError(l, "World doesn't exist.");
			}
			map->World = world;
		} else if (!strcmp(value, "Layer")) {
			map->Layer = LuaToNumber(l, -1);
			if (map->Layer > 0) {
				map->TimeOfDaySeconds = 0; // no time of day for underground maps
			}
		} else if (!strcmp(value, "TerrainFile")) {
			map->TerrainFile = LuaToString(l, -1);
		} else if (!strcmp(value, "OverlayTerrainFile")) {
			map->OverlayTerrainFile = LuaToString(l, -1);
		} else if (!strcmp(value, "TerrainImage")) {
			map->TerrainImage = LuaToString(l, -1);
		} else if (!strcmp(value, "OverlayTerrainImage")) {
			map->OverlayTerrainImage = LuaToString(l, -1);
		} else if (!strcmp(value, "Width")) {
			map->Width = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Height")) {
			map->Height = LuaToNumber(l, -1);
		} else if (!strcmp(value, "TimeOfDaySeconds")) {
			map->TimeOfDaySeconds = LuaToNumber(l, -1);
		} else if (!strcmp(value, "SubtemplatePosition")) {
			CclGetPos(l, &map->SubtemplatePosition.x, &map->SubtemplatePosition.y);
		} else if (!strcmp(value, "MainTemplate")) {
			CMapTemplate *main_template = GetMapTemplate(LuaToString(l, -1));
			if (!main_template) {
				LuaError(l, "Map template doesn't exist.");
			}
			map->MainTemplate = main_template;
			main_template->Subtemplates.push_back(map);
		} else if (!strcmp(value, "BaseTerrain")) {
			CTerrainType *terrain = GetTerrainType(LuaToString(l, -1));
			if (!terrain) {
				LuaError(l, "Terrain doesn't exist.");
			}
			map->BaseTerrain = terrain;
		} else if (!strcmp(value, "BorderTerrain")) {
			CTerrainType *terrain = GetTerrainType(LuaToString(l, -1));
			if (!terrain) {
				LuaError(l, "Terrain doesn't exist.");
			}
			map->BorderTerrain = terrain;
		} else if (!strcmp(value, "SurroundingTerrain")) {
			CTerrainType *terrain = GetTerrainType(LuaToString(l, -1));
			if (!terrain) {
				LuaError(l, "Terrain doesn't exist.");
			}
			map->SurroundingTerrain = terrain;
		} else if (!strcmp(value, "GeneratedTerrains")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CTerrainType *terrain = GetTerrainType(LuaToString(l, -1, j + 1));
				if (!terrain) {
					LuaError(l, "Terrain doesn't exist.");
				}
				++j;
				
				int degree_level = GetDegreeLevelIdByName(LuaToString(l, -1, j + 1));
				if (degree_level == -1) {
					LuaError(l, "Degree level doesn't exist.");
				}
				
				map->GeneratedTerrains.push_back(std::pair<CTerrainType *, int>(terrain, degree_level));
			}
		} else if (!strcmp(value, "ExternalGeneratedTerrains")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CTerrainType *terrain = GetTerrainType(LuaToString(l, -1, j + 1));
				if (!terrain) {
					LuaError(l, "Terrain doesn't exist.");
				}
				++j;
				
				int degree_level = GetDegreeLevelIdByName(LuaToString(l, -1, j + 1));
				if (degree_level == -1) {
					LuaError(l, "Degree level doesn't exist.");
				}
				
				map->ExternalGeneratedTerrains.push_back(std::pair<CTerrainType *, int>(terrain, degree_level));
			}
		} else if (!strcmp(value, "GeneratedNeutralUnits")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CUnitType *unit_type = UnitTypeByIdent(LuaToString(l, -1, j + 1));
				if (!unit_type) {
					LuaError(l, "Unit type doesn't exist.");
				}
				++j;
				
				int quantity = LuaToNumber(l, -1, j + 1);
				
				map->GeneratedNeutralUnits.push_back(std::pair<CUnitType *, int>(unit_type, quantity));
			}
		} else if (!strcmp(value, "PlayerLocationGeneratedNeutralUnits")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CUnitType *unit_type = UnitTypeByIdent(LuaToString(l, -1, j + 1));
				if (!unit_type) {
					LuaError(l, "Unit type doesn't exist.");
				}
				++j;
				
				int quantity = LuaToNumber(l, -1, j + 1);
				
				map->PlayerLocationGeneratedNeutralUnits.push_back(std::pair<CUnitType *, int>(unit_type, quantity));
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	
	return 0;
}

/**
**  Define a settlement.
**
**  @param l  Lua state.
*/
static int CclDefineSettlement(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument (expected table)");
	}

	std::string settlement_ident = LuaToString(l, 1);
	CSettlement *settlement = GetSettlement(settlement_ident);
	if (settlement == NULL) {
		settlement = new CSettlement;
		settlement->Ident = settlement_ident;
		Settlements.push_back(settlement);
		SettlementIdentToPointer[settlement_ident] = settlement;
	}
	
	//  Parse the list:
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		const char *value = LuaToString(l, -2);
		
		if (!strcmp(value, "Name")) {
			settlement->Name = LuaToString(l, -1);
		} else if (!strcmp(value, "Position")) {
			CclGetPos(l, &settlement->Position.x, &settlement->Position.y);
		} else if (!strcmp(value, "MapTemplate")) {
			CMapTemplate *map_template = GetMapTemplate(LuaToString(l, -1));
			if (!map_template) {
				LuaError(l, "Map template doesn't exist.");
			}
			settlement->MapTemplate = map_template;
		} else if (!strcmp(value, "CulturalNames")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument (expected table)");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				int civilization = PlayerRaces.GetRaceIndexByName(LuaToString(l, -1, j + 1));
				if (civilization == -1) {
					LuaError(l, "Civilization doesn't exist.");
				}
				++j;
				
				std::string cultural_name = LuaToString(l, -1, j + 1);
				
				settlement->CulturalNames[civilization] = cultural_name;
				PlayerRaces.Civilizations[civilization]->SettlementNames.push_back(cultural_name);
			}
		} else if (!strcmp(value, "HistoricalOwners")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CDate date;
				date.year = 0;
				date.month = 1;
				date.day = 1;
				lua_rawgeti(l, -1, j + 1);
				CclGetDate(l, &date);
				lua_pop(l, 1);
				++j;
				std::string owner_ident = LuaToString(l, -1, j + 1);
				if (!owner_ident.empty()) {
					CFaction *owner_faction = PlayerRaces.GetFaction(-1, owner_ident);
					if (!owner_faction) {
						LuaError(l, "Faction \"%s\" doesn't exist." _C_ owner_ident.c_str());
					}
					settlement->HistoricalOwners[date] = owner_faction;
				} else {
					settlement->HistoricalOwners[date] = NULL;
				}
			}
		} else if (!strcmp(value, "HistoricalPopulation")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CDate date;
				date.year = 0;
				date.month = 1;
				date.day = 1;
				lua_rawgeti(l, -1, j + 1);
				CclGetDate(l, &date);
				lua_pop(l, 1);
				++j;
				settlement->HistoricalPopulation[date] = LuaToNumber(l, -1, j + 1);
			}
		} else if (!strcmp(value, "HistoricalUnits")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CDate date;
				date.year = 0;
				date.month = 1;
				date.day = 1;
				lua_rawgeti(l, -1, j + 1);
				CclGetDate(l, &date);
				lua_pop(l, 1);
				++j;
				
				CUnitType *unit_type = UnitTypeByIdent(LuaToString(l, -1, j + 1));
				if (!unit_type) {
					LuaError(l, "Unit type doesn't exist.");
				}
				++j;
				
				int unit_quantity = LuaToNumber(l, -1, j + 1);
				++j;
				
				CFaction *unit_owner = NULL;
				lua_rawgeti(l, -1, j + 1);
				if (lua_isstring(l, -1) && !lua_isnumber(l, -1)) {
					unit_owner = PlayerRaces.GetFaction(-1, LuaToString(l, -1));
					if (!unit_owner) {
						LuaError(l, "Unit owner faction doesn't exist.\n");
					}
				} else {
					--j;
				}
				lua_pop(l, 1);

				settlement->HistoricalUnits[unit_type][date] = std::pair<int, CFaction *>(unit_quantity, unit_owner);
			}
		} else if (!strcmp(value, "HistoricalBuildings")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CDate start_date;
				start_date.year = 0;
				start_date.month = 1;
				start_date.day = 1;
				lua_rawgeti(l, -1, j + 1);
				CclGetDate(l, &start_date);
				lua_pop(l, 1);
				++j;
				CDate end_date;
				end_date.year = 0;
				end_date.month = 1;
				end_date.day = 1;
				lua_rawgeti(l, -1, j + 1);
				CclGetDate(l, &end_date);
				lua_pop(l, 1);
				++j;
				int building_class_id = GetUnitTypeClassIndexByName(LuaToString(l, -1, j + 1));
				if (building_class_id == -1) {
					LuaError(l, "Building class doesn't exist.");
				}
				++j;
				
				CUniqueItem *unique = NULL;
				lua_rawgeti(l, -1, j + 1);
				if (lua_isstring(l, -1) && !lua_isnumber(l, -1) && GetUniqueItem(LuaToString(l, -1)) != NULL) {
					unique = GetUniqueItem(LuaToString(l, -1));
				} else {
					--j;
				}
				lua_pop(l, 1);
				++j;
				
				CFaction *building_owner = NULL;
				lua_rawgeti(l, -1, j + 1);
				if (lua_isstring(l, -1) && !lua_isnumber(l, -1)) {
					building_owner = PlayerRaces.GetFaction(-1, LuaToString(l, -1));
					if (!building_owner) {
						LuaError(l, "Building owner faction doesn't exist.\n");
					}
				} else {
					--j;
				}
				lua_pop(l, 1);

				settlement->HistoricalBuildings.push_back(std::tuple<CDate, CDate, int, CUniqueItem *, CFaction *>(start_date, end_date, building_class_id, unique, building_owner));
			}
		} else if (!strcmp(value, "Regions")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				CRegion *region = GetRegion(LuaToString(l, -1, j + 1));
				if (region == NULL) {
					LuaError(l, "Region doesn't exist.");
				}
				settlement->Regions.push_back(region);
				region->Settlements.push_back(settlement);
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	
	if (settlement->MapTemplate && settlement->Position.x != -1 && settlement->Position.y != -1) {
		if (settlement->MapTemplate->Settlements.find(std::pair<int, int>(settlement->Position.x, settlement->Position.y)) != settlement->MapTemplate->Settlements.end()) {
			LuaError(l, "Position (%d, %d) of map template \"%s\" already has a settlement." _C_ settlement->Position.x _C_ settlement->Position.y _C_ settlement->MapTemplate->Ident.c_str());
		}
		settlement->MapTemplate->Settlements[std::pair<int, int>(settlement->Position.x, settlement->Position.y)] = settlement;
	}
	
	return 0;
}

/**
**  Define a terrain feature.
**
**  @param l  Lua state.
*/
static int CclDefineTerrainFeature(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument (expected table)");
	}

	std::string terrain_feature_ident = LuaToString(l, 1);
	CTerrainFeature *terrain_feature = GetTerrainFeature(terrain_feature_ident);
	if (!terrain_feature) {
		terrain_feature = new CTerrainFeature;
		terrain_feature->Ident = terrain_feature_ident;
		terrain_feature->ID = TerrainFeatures.size();
		TerrainFeatures.push_back(terrain_feature);
	}
	
	//  Parse the list:
	for (lua_pushnil(l); lua_next(l, 2); lua_pop(l, 1)) {
		const char *value = LuaToString(l, -2);
		
		if (!strcmp(value, "Name")) {
			terrain_feature->Name = LuaToString(l, -1);
		} else if (!strcmp(value, "Color")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}
			terrain_feature->Color.R = LuaToNumber(l, -1, 1);
			terrain_feature->Color.G = LuaToNumber(l, -1, 2);
			terrain_feature->Color.B = LuaToNumber(l, -1, 3);
			if (TerrainTypeColorToIndex.find(std::tuple<int, int, int>(terrain_feature->Color.R, terrain_feature->Color.G, terrain_feature->Color.B)) != TerrainTypeColorToIndex.end()) {
				LuaError(l, "Color is already used by a terrain type.");
			}
			if (TerrainFeatureColorToIndex.find(std::tuple<int, int, int>(terrain_feature->Color.R, terrain_feature->Color.G, terrain_feature->Color.B)) != TerrainFeatureColorToIndex.end()) {
				LuaError(l, "Color is already used by another terrain feature.");
			}
			TerrainFeatureColorToIndex[std::tuple<int, int, int>(terrain_feature->Color.R, terrain_feature->Color.G, terrain_feature->Color.B)] = terrain_feature->ID;
		} else if (!strcmp(value, "TerrainType")) {
			CTerrainType *terrain = GetTerrainType(LuaToString(l, -1));
			if (!terrain) {
				LuaError(l, "Terrain doesn't exist.");
			}
			terrain_feature->TerrainType = terrain;
		} else if (!strcmp(value, "World")) {
			CWorld *world = GetWorld(LuaToString(l, -1));
			if (world != NULL) {
				terrain_feature->World = world;
				world->TerrainFeatures.push_back(terrain_feature);
			} else {
				LuaError(l, "World doesn't exist.");
			}
		} else if (!strcmp(value, "CulturalNames")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument (expected table)");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				int civilization = PlayerRaces.GetRaceIndexByName(LuaToString(l, -1, j + 1));
				if (civilization == -1) {
					LuaError(l, "Civilization doesn't exist.");
				}
				++j;
				
				std::string cultural_name = LuaToString(l, -1, j + 1);
				
				terrain_feature->CulturalNames[civilization] = cultural_name;
			}
		} else if (!strcmp(value, "FactionCulturalNames")) {
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument (expected table)");
			}
			const int subargs = lua_rawlen(l, -1);
			for (int j = 0; j < subargs; ++j) {
				int civilization = PlayerRaces.GetRaceIndexByName(LuaToString(l, -1, j + 1));
				if (civilization == -1) {
					LuaError(l, "Civilization doesn't exist.");
				}
				++j;
				
				int faction = PlayerRaces.GetFactionIndexByName(civilization, LuaToString(l, -1, j + 1));
				if (faction == -1) {
					LuaError(l, "Faction doesn't exist.");
				}
				++j;
				
				std::string cultural_name = LuaToString(l, -1, j + 1);
				
				terrain_feature->FactionCulturalNames[PlayerRaces.Factions[civilization][faction]] = cultural_name;
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	
	if (terrain_feature->World == NULL) {
		LuaError(l, "Terrain feature \"%s\" is not assigned to any world." _C_ terrain_feature->Ident.c_str());
	}
	
	return 0;
}

/**
**  Get terrain feature data.
**
**  @param l  Lua state.
*/
static int CclGetTerrainFeatureData(lua_State *l)
{
	if (lua_gettop(l) < 2) {
		LuaError(l, "incorrect argument");
	}
	std::string terrain_feature_ident = LuaToString(l, 1);
	CTerrainFeature *terrain_feature = GetTerrainFeature(terrain_feature_ident);
	if (!terrain_feature) {
		LuaError(l, "TerrainFeature \"%s\" doesn't exist." _C_ terrain_feature_ident.c_str());
	}
	const char *data = LuaToString(l, 2);

	if (!strcmp(data, "Name")) {
		lua_pushstring(l, terrain_feature->Name.c_str());
		return 1;
	} else if (!strcmp(data, "World")) {
		if (terrain_feature->World != NULL) {
			lua_pushstring(l, terrain_feature->World->Name.c_str());
		} else {
			lua_pushstring(l, "");
		}
		return 1;
	} else {
		LuaError(l, "Invalid field: %s" _C_ data);
	}

	return 0;
}

static int CclGetTerrainFeatures(lua_State *l)
{
	lua_createtable(l, TerrainFeatures.size(), 0);
	for (size_t i = 1; i <= TerrainFeatures.size(); ++i)
	{
		lua_pushstring(l, TerrainFeatures[i-1]->Ident.c_str());
		lua_rawseti(l, -2, i);
	}
	return 1;
}
//Wyrmgus end

/**
**  Register CCL features for map.
*/
void MapCclRegister()
{
	lua_register(Lua, "StratagusMap", CclStratagusMap);
	lua_register(Lua, "RevealMap", CclRevealMap);
	lua_register(Lua, "CenterMap", CclCenterMap);
	lua_register(Lua, "SetStartView", CclSetStartView);
	lua_register(Lua, "ShowMapLocation", CclShowMapLocation);

	lua_register(Lua, "SetFogOfWar", CclSetFogOfWar);
	lua_register(Lua, "GetFogOfWar", CclGetFogOfWar);
	lua_register(Lua, "SetMinimapTerrain", CclSetMinimapTerrain);

	lua_register(Lua, "SetFogOfWarGraphics", CclSetFogOfWarGraphics);
	lua_register(Lua, "SetFogOfWarOpacity", CclSetFogOfWarOpacity);
	lua_register(Lua, "SetFogOfWarColor", CclSetFogOfWarColor);

	lua_register(Lua, "SetForestRegeneration", CclSetForestRegeneration);

	lua_register(Lua, "LoadTileModels", CclLoadTileModels);
	lua_register(Lua, "DefinePlayerTypes", CclDefinePlayerTypes);

	lua_register(Lua, "DefineTileset", CclDefineTileset);
	lua_register(Lua, "SetTileFlags", CclSetTileFlags);
	lua_register(Lua, "BuildTilesetTables", CclBuildTilesetTables);

	//Wyrmgus start
	lua_register(Lua, "GetCurrentTileset", CclGetCurrentTileset);
	//Wyrmgus end
	lua_register(Lua, "GetTileTerrainName", CclGetTileTerrainName);
	//Wyrmgus start
//	lua_register(Lua, "GetTileTerrainMixedName", CclGetTileTerrainMixedName);
	//Wyrmgus end
	lua_register(Lua, "GetTileTerrainHasFlag", CclGetTileTerrainHasFlag);
	
	//Wyrmgus start
	lua_register(Lua, "DefineTerrainType", CclDefineTerrainType);
	lua_register(Lua, "DefineMapTemplate", CclDefineMapTemplate);
	lua_register(Lua, "DefineSettlement", CclDefineSettlement);
	lua_register(Lua, "DefineTerrainFeature", CclDefineTerrainFeature);
	lua_register(Lua, "GetTerrainFeatureData", CclGetTerrainFeatureData);
	lua_register(Lua, "GetTerrainFeatures", CclGetTerrainFeatures);
	lua_register(Lua, "SetMapTemplateTileTerrain", CclSetMapTemplateTileTerrain);
	lua_register(Lua, "SetMapTemplateTileLabel", CclSetMapTemplateTileLabel);
	lua_register(Lua, "SetMapTemplatePathway", CclSetMapTemplatePathway);
	lua_register(Lua, "SetMapTemplateCulturalSettlementName", CclSetMapTemplateCulturalSettlementName);
	lua_register(Lua, "SetMapTemplateFactionCulturalSettlementName", CclSetMapTemplateFactionCulturalSettlementName);
	lua_register(Lua, "SetMapTemplateResource", CclSetMapTemplateResource);
	lua_register(Lua, "SetMapTemplateUnit", CclSetMapTemplateUnit);
	lua_register(Lua, "SetMapTemplateHero", CclSetMapTemplateHero);
	lua_register(Lua, "SetMapTemplateLayerConnector", CclSetMapTemplateLayerConnector);
//	lua_register(Lua, "CreateMapTemplateTerrainFile", CclCreateMapTemplateTerrainFile);
	//Wyrmgus end
}

//@}
