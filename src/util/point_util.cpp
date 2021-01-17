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
//      (c) Copyright 2020-2021 by Andrettin
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

#include "util/point_util.h"

#include "util/geocoordinate_util.h"
#include "util/util.h"

namespace wyrmgus::point {

QGeoCoordinate to_geocoordinate(const QPoint &point, const QSize &area_size, const QRectF &unsigned_georectangle)
{
	const QPointF unsigned_geocoordinate = point::to_unsigned_geocoordinate(point, area_size, unsigned_georectangle);
	return qgeocoordinate::from_unsigned_geocoordinate(unsigned_geocoordinate);
}

int distance_to(const QPoint &point, const QPoint &other_point)
{
	return isqrt(point::square_distance_to(point, other_point));
}

}
