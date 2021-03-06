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
/**@name icons.cpp - The icons. */
//
//      (c) Copyright 1998-2012 by Lutz Sammer and Jimmy Salmon
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

#include "icons.h"

#include "menus.h"
#include "player.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "video.h"

#include <map>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

//Wyrmgus start
//typedef std::map<std::string, CIcon *> IconMap;
//static IconMap Icons;   /// Map of ident to icon.
IconMap Icons;   /// Map of ident to icon.
//Wyrmgus end


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  CIcon constructor
*/
//Wyrmgus start
//CIcon::CIcon(const std::string &ident) : G(NULL), GScale(NULL), Frame(0), Ident(ident)
CIcon::CIcon(const std::string &ident) : G(NULL), GScale(NULL), Frame(0), Ident(ident), Loaded(false)
//Wyrmgus end
{
}

/**
**  CIcon destructor
*/
CIcon::~CIcon()
{
	CPlayerColorGraphic::Free(this->G);
	CPlayerColorGraphic::Free(this->GScale);
}

/**
**  Create a new icon
**
**  @param ident  Icon identifier
**
**  @return       New icon
*/
/* static */ CIcon *CIcon::New(const std::string &ident)
{
	CIcon *&icon = Icons[ident];

	if (icon == NULL) {
		icon = new CIcon(ident);
	}
	return icon;
}

/**
**  Get an icon
**
**  @param ident  Icon identifier
**
**  @return       The icon
*/
/* static */ CIcon *CIcon::Get(const std::string &ident)
{
	IconMap::iterator it = Icons.find(ident);
	if (it == Icons.end()) {
		DebugPrint("icon not found: %s\n" _C_ ident.c_str());
	}
	return it->second;
}

void CIcon::Load()
{
	//Wyrmgus start
	if (this->Loaded) {
		return;
	}
	//Wyrmgus end
	Assert(G);
	G->Load();
	if (Preference.GrayscaleIcons) {
		GScale = G->Clone(true);
	}
	if (Frame >= G->NumFrames) {
		DebugPrint("Invalid icon frame: %s - %d\n" _C_ Ident.c_str() _C_ Frame);
		Frame = 0;
	}
	//Wyrmgus start
	this->Loaded = true;
	//Wyrmgus end
}

/**
**  Draw icon at pos.
**
**  @param player  Player pointer used for icon colors
**  @param pos     display pixel position
*/
//Wyrmgus start
//void CIcon::DrawIcon(const PixelPos &pos, const int player) const
void CIcon::DrawIcon(const PixelPos &pos, const int player, int hair_color) const
//Wyrmgus end
{
	if (player != -1 ) {
		//Wyrmgus start
//		this->G->DrawPlayerColorFrameClip(player, this->Frame, pos.x, pos.y);
		this->G->DrawPlayerColorFrameClip(player, this->Frame, pos.x, pos.y, true, hair_color);
		//Wyrmgus end
	} else {
		this->G->DrawFrameClip(this->Frame, pos.x, pos.y);
	}
}

/**
**  Draw grayscale icon at pos.
**
**  @param pos     display pixel position
*/
void CIcon::DrawGrayscaleIcon(const PixelPos &pos, const int player) const
{
	if (this->GScale) {
		if (player != -1) {
			this->GScale->DrawPlayerColorFrameClip(player, this->Frame, pos.x, pos.y);
		} else {
			this->GScale->DrawFrameClip(this->Frame, pos.x, pos.y);
		}
	}
}

/**
**  Draw cooldown spell effect on icon at pos.
**
**  @param pos       display pixel position
**  @param percent   cooldown percent
*/
void CIcon::DrawCooldownSpellIcon(const PixelPos &pos, const int percent) const
{
	// TO-DO: implement more effect types (clock-like)
	if (this->GScale) {
		this->GScale->DrawFrameClip(this->Frame, pos.x, pos.y);
		const int height = (G->Height * (100 - percent)) / 100;
		this->G->DrawSubClip(G->frame_map[Frame].x, G->frame_map[Frame].y + G->Height - height,
							 G->Width, height, pos.x, pos.y + G->Height - height);
	} else {
		DebugPrint("Enable grayscale icon drawing in your game to achieve special effects for cooldown spell icons");
		this->DrawIcon(pos);
	}
}

/**
**  Draw unit icon 'icon' with border on x,y
**
**  @param style   Button style
**  @param flags   State of icon (clicked, mouse over...)
**  @param pos     display pixel position
**  @param text    Optional text to display
*/
void CIcon::DrawUnitIcon(const ButtonStyle &style, unsigned flags,
						 //Wyrmgus start
//						 const PixelPos &pos, const std::string &text, int player) const
						 const PixelPos &pos, const std::string &text, int player, int hair_color, bool transparent, bool grayscale) const
						 //Wyrmgus end
{
	ButtonStyle s(style);

	//Wyrmgus start
//	s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = this->G;
	if (grayscale) {
		s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = this->GScale;
	} else {
		s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = this->G;
	}
	//Wyrmgus end
	s.Default.Frame = s.Hover.Frame = s.Clicked.Frame = this->Frame;
	if (!(flags & IconSelected) && (flags & IconAutoCast)) {
		s.Default.BorderColorRGB = UI.ButtonPanel.AutoCastBorderColorRGB;
		s.Default.BorderColor = 0;
		//Wyrmgus start
//		s.Default.BorderSize = 2;
		//Wyrmgus end
	}
	//Wyrmgus start
	if (!(flags & IconAutoCast)) {
		s.Default.BorderSize = 0;
	}
	//Wyrmgus end
	//Wyrmgus start
	if (Preference.IconsShift && Preference.IconFrameG && Preference.PressedIconFrameG) {
		Video.FillRectangle(ColorBlack, pos.x, pos.y, 46, 38);
		if (flags & IconActive) { // Code to make a border appear around the icon when the mouse hovers over it.
//			Video.DrawRectangle(ColorGray, pos.x - 4, pos.y - 4, 54, 46);
//			DrawUIButton(&s, flags, pos.x, pos.y, text, player, hair_color, transparent);
		}

		if (flags & IconClicked) { // Shift the icon a bit to make it look like it's been pressed.
			DrawUIButton(&s, flags, pos.x + 1, pos.y + 1, text, player, hair_color, transparent);
			if (flags & IconSelected) {
				Video.DrawRectangle(ColorGreen, pos.x, pos.y, 48, 40);
			} else if (flags & IconAutoCast) {
				Video.DrawRectangle(Video.MapRGB(TheScreen->format, UI.ButtonPanel.AutoCastBorderColorRGB), pos.x, pos.y, 48, 40);
			}
			if (!Preference.CommandButtonFrameG || !(flags & IconCommandButton)) {
				Preference.PressedIconFrameG->DrawClip(pos.x - 4, pos.y - 4);
			} else {
				Preference.CommandButtonFrameG->DrawClip(pos.x - 5, pos.y - 4);
			}
//			Video.DrawRectangle(ColorGray, pos.x - 4, pos.y - 4, 54, 46);
		} else {
			DrawUIButton(&s, flags, pos.x, pos.y, text, player, hair_color, transparent);
			if (flags & IconSelected) {
				Video.DrawRectangle(ColorGreen, pos.x, pos.y, 46, 38);
			} else if (flags & IconAutoCast) {
				Video.DrawRectangle(Video.MapRGB(TheScreen->format, UI.ButtonPanel.AutoCastBorderColorRGB), pos.x, pos.y, 46, 38);
			}
			if (Preference.CommandButtonFrameG && (flags & IconCommandButton)) {
				Preference.CommandButtonFrameG->DrawClip(pos.x - 5, pos.y - 4);
			} else {
				Preference.IconFrameG->DrawClip(pos.x - 4, pos.y - 4);
			}
		}
	//Wyrmgus end
	//Wyrmgus start
//	if (Preference.IconsShift) {
	} else if (Preference.IconsShift) {
	//Wyrmgus end
		// Left and top edge of Icon
		Video.DrawHLine(ColorWhite, pos.x - 1, pos.y - 1, 49);
		Video.DrawVLine(ColorWhite, pos.x - 1, pos.y, 40);
		Video.DrawVLine(ColorWhite, pos.x, pos.y + 38, 2);
		Video.DrawHLine(ColorWhite, pos.x + 46, pos.y, 2);

		// Bottom and Right edge of Icon
		Video.DrawHLine(ColorGray, pos.x + 1, pos.y + 38, 47);
		Video.DrawHLine(ColorGray, pos.x + 1, pos.y + 39, 47);
		Video.DrawVLine(ColorGray, pos.x + 46, pos.y + 1, 37);
		Video.DrawVLine(ColorGray, pos.x + 47, pos.y + 1, 37);

		Video.DrawRectangle(ColorBlack, pos.x - 3, pos.y - 3, 52, 44);
		Video.DrawRectangle(ColorBlack, pos.x - 4, pos.y - 4, 54, 46);

		if (flags & IconActive) { // Code to make a border appear around the icon when the mouse hovers over it.
			Video.DrawRectangle(ColorGray, pos.x - 4, pos.y - 4, 54, 46);
			//Wyrmgus start
//			DrawUIButton(&s, flags, pos.x, pos.y, text, player);
			DrawUIButton(&s, flags, pos.x, pos.y, text, player, hair_color, transparent);
			//Wyrmgus end
		}

		if (flags & IconClicked) { // Shift the icon a bit to make it look like it's been pressed.
			//Wyrmgus start
//			DrawUIButton(&s, flags, pos.x + 1, pos.y + 1, text, player);
			DrawUIButton(&s, flags, pos.x + 1, pos.y + 1, text, player, hair_color, transparent);
			//Wyrmgus end
			if (flags & IconSelected) {
				Video.DrawRectangle(ColorGreen, pos.x + 1, pos.y + 1, 46, 38);
			}			
			Video.DrawRectangle(ColorGray, pos.x, pos.y, 48, 40);
			Video.DrawVLine(ColorDarkGray, pos.x - 1, pos.y - 1, 40);
			Video.DrawHLine(ColorDarkGray, pos.x - 1, pos.y - 1, 49);
			Video.DrawHLine(ColorDarkGray, pos.x - 1, pos.y + 39, 2);

			Video.DrawRectangle(ColorGray, pos.x - 4, pos.y - 4, 54, 46);
		} else {
			//Wyrmgus start
//			DrawUIButton(&s, flags, pos.x, pos.y, text, player);
			DrawUIButton(&s, flags, pos.x, pos.y, text, player, hair_color, transparent);
			//Wyrmgus end
			if (flags & IconSelected) {
				Video.DrawRectangle(ColorGreen, pos.x, pos.y, 46, 38);
			}
		}
	} else {
		//Wyrmgus start
//		DrawUIButton(&s, flags, pos.x, pos.y, text, player);
		DrawUIButton(&s, flags, pos.x, pos.y, text, player, hair_color, transparent);
		//Wyrmgus end
	}
}

/**
**  Load the Icon
*/
bool IconConfig::LoadNoLog()
{
	Assert(!Name.empty());

	Icon = CIcon::Get(Name);
	return Icon != NULL;
}

/**
**  Load the Icon
*/
bool IconConfig::Load()
{
	bool res = LoadNoLog();
	if (res == true) {
		ShowLoadProgress(_("Loading Icon \"%s\""), this->Name.c_str());
	} else {
		fprintf(stderr, _("Can't find icon %s\n"), this->Name.c_str());
	}
	return res;
}

/**
**  Get the numbers of icons.
*/
int GetIconsCount()
{
	return Icons.size();
}

/**
**  Load the graphics for the icons.
*/
void LoadIcons()
{
	for (IconMap::iterator it = Icons.begin(); it != Icons.end(); ++it) {
		CIcon &icon = *(*it).second;

		ShowLoadProgress(_("Loading Icon \"%s\""), icon.G->File.c_str());
		icon.Load();

		IncItemsLoaded();
	}
}

/**
**  Clean up memory used by the icons.
*/
void CleanIcons()
{
	for (IconMap::iterator it = Icons.begin(); it != Icons.end(); ++it) {
		CIcon *icon = (*it).second;
		delete icon;
	}
	Icons.clear();
}

//@}
