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
/**@name unitsound.cpp - The unit sounds. */
//
//      (c) Copyright 1999-2015 by Fabrice Rossi, Jimmy Salmon and Andrettin
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
--  Include
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "unitsound.h"

#include "animation/animation_randomsound.h"
#include "animation/animation_sound.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "sound_server.h"
#include "unit.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

bool SoundConfig::MapSound()
{
	if (!this->Name.empty()) {
		this->Sound = SoundForName(this->Name);
	}
	return this->Sound != NULL;
}

void SoundConfig::SetSoundRange(unsigned char range)
{
	if (this->Sound) {
		::SetSoundRange(this->Sound, range);
	}
}

/**
**  Load all sounds for units.
*/
void LoadUnitSounds()
{
}

static void MapAnimSound(CAnimation &anim)
{
	if (anim.Type == AnimationSound) {
		CAnimation_Sound &anim_sound = *static_cast<CAnimation_Sound *>(&anim);

		anim_sound.MapSound();
	} else if (anim.Type == AnimationRandomSound) {
		CAnimation_RandomSound &anim_rsound = *static_cast<CAnimation_RandomSound *>(&anim);

		anim_rsound.MapSound();
	}
}

/**
**  Map animation sounds
*/
static void MapAnimSounds2(CAnimation *anim)
{
	if (anim == NULL) {
		return ;
	}
	MapAnimSound(*anim);
	for (CAnimation *it = anim->Next; it != anim; it = it->Next) {
		MapAnimSound(*it);
	}
}

/**
**  Map animation sounds for a unit type
*/
static void MapAnimSounds(CUnitType &type)
{
	if (!type.Animations) {
		return;
	}
	MapAnimSounds2(type.Animations->Start);
	MapAnimSounds2(type.Animations->Still);
	MapAnimSounds2(type.Animations->Move);
	MapAnimSounds2(type.Animations->Attack);
	MapAnimSounds2(type.Animations->RangedAttack);
	MapAnimSounds2(type.Animations->SpellCast);
	for (int i = 0; i <= ANIMATIONS_DEATHTYPES; ++i) {
		MapAnimSounds2(type.Animations->Death[i]);
	}
	MapAnimSounds2(type.Animations->Repair);
	MapAnimSounds2(type.Animations->Train);
	MapAnimSounds2(type.Animations->Research);
	MapAnimSounds2(type.Animations->Upgrade);
	MapAnimSounds2(type.Animations->Build);
	for (int i = 0; i < MaxCosts; ++i) {
		MapAnimSounds2(type.Animations->Harvest[i]);
	}
	//Wyrmgus start
	for (int var_i = 0; var_i < VariationMax; ++var_i) {
		VariationInfo *varinfo = type.VarInfo[var_i];
		if (!varinfo) {
			continue;
		}
		if (!varinfo->Animations) {
			continue;
		}
		MapAnimSounds2(varinfo->Animations->Start);
		MapAnimSounds2(varinfo->Animations->Still);
		MapAnimSounds2(varinfo->Animations->Move);
		MapAnimSounds2(varinfo->Animations->Attack);
		MapAnimSounds2(varinfo->Animations->RangedAttack);
		MapAnimSounds2(varinfo->Animations->SpellCast);
		for (int i = 0; i <= ANIMATIONS_DEATHTYPES; ++i) {
			MapAnimSounds2(varinfo->Animations->Death[i]);
		}
		MapAnimSounds2(varinfo->Animations->Repair);
		MapAnimSounds2(varinfo->Animations->Train);
		MapAnimSounds2(varinfo->Animations->Research);
		MapAnimSounds2(varinfo->Animations->Upgrade);
		MapAnimSounds2(varinfo->Animations->Build);
		for (int i = 0; i < MaxCosts; ++i) {
			MapAnimSounds2(varinfo->Animations->Harvest[i]);
		}
	}
	//Wyrmgus end
}

/**
**  Map the sounds of all unit-types to the correct sound id.
**  And overwrite the sound ranges.
**  @todo the sound ranges should be configurable by user with CCL.
*/
void MapUnitSounds()
{
	if (SoundEnabled() == false) {
		return;
	}
	//Wyrmgus start
	for (size_t i = 0; i < PlayerRaces.Civilizations.size(); ++i) {
		CCivilization *civilization = PlayerRaces.Civilizations[i];
		civilization->UnitSounds.Selected.MapSound();
		civilization->UnitSounds.Acknowledgement.MapSound();
		civilization->UnitSounds.Attack.MapSound();
		civilization->UnitSounds.Idle.MapSound();
		civilization->UnitSounds.Build.MapSound();
		civilization->UnitSounds.Ready.MapSound();
		civilization->UnitSounds.Ready.SetSoundRange(INFINITE_SOUND_RANGE);
		civilization->UnitSounds.Repair.MapSound();
		for (int j = 0; j < MaxCosts; ++j) {
			civilization->UnitSounds.Harvest[j].MapSound();
		}
		civilization->UnitSounds.Help.MapSound();
		civilization->UnitSounds.Help.SetSoundRange(INFINITE_SOUND_RANGE);
		civilization->UnitSounds.HelpTown.MapSound();
		civilization->UnitSounds.HelpTown.SetSoundRange(INFINITE_SOUND_RANGE);
	}
	//Wyrmgus end

	// Parse all units sounds.
	for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
		CUnitType &type = *UnitTypes[i];

		MapAnimSounds(type);

		type.MapSound.Selected.MapSound();
		type.MapSound.Acknowledgement.MapSound();
		// type.Sound.Acknowledgement.SetSoundRange(INFINITE_SOUND_RANGE);
		type.MapSound.Attack.MapSound();
		//Wyrmgus start
		type.MapSound.Idle.MapSound();
		type.MapSound.Hit.MapSound();
		type.MapSound.Miss.MapSound();
		type.MapSound.Step.MapSound();
		type.MapSound.StepDirt.MapSound();
		type.MapSound.StepGrass.MapSound();
		type.MapSound.StepGravel.MapSound();
		type.MapSound.StepMud.MapSound();
		type.MapSound.StepStone.MapSound();
		type.MapSound.Used.MapSound();
		//Wyrmgus end
		type.MapSound.Build.MapSound();
		type.MapSound.Ready.MapSound();
		type.MapSound.Ready.SetSoundRange(INFINITE_SOUND_RANGE);
		type.MapSound.Repair.MapSound();
		for (int i = 0; i < MaxCosts; ++i) {
			type.MapSound.Harvest[i].MapSound();
		}
		type.MapSound.Help.MapSound();
		type.MapSound.Help.SetSoundRange(INFINITE_SOUND_RANGE);

		for (int i = 0; i <= ANIMATIONS_DEATHTYPES; ++i) {
			type.MapSound.Dead[i].MapSound();
		}
	}
}

//@}
