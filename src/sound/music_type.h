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
//      (c) Copyright 2020 by Andrettin
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

#pragma once

namespace wyrmgus {

enum class music_type {
	none,
	menu,
	loading,
	map
};

inline music_type string_to_music_type(const std::string &str)
{
	if (str == "menu") {
		return music_type::menu;
	} else if (str == "loading") {
		return music_type::loading;
	} else if (str == "map") {
		return music_type::map;
	}

	throw std::runtime_error("Invalid music type: \"" + str + "\".");
}

inline std::string music_type_to_string(const music_type type)
{
	switch (type) {
		case music_type::menu:
			return "menu";
		case music_type::loading:
			return "loading";
		case music_type::map:
			return "map";
		default:
			break;
	}

	throw std::runtime_error("Invalid music type: \"" + std::to_string(static_cast<int>(type)) + "\".");
}

}

Q_DECLARE_METATYPE(wyrmgus::music_type)