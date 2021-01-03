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
//      (c) Copyright 2002-2020 by Lutz Sammer, Nehal Mistry, Jimmy Salmon
//                                 and Andrettin
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

#include "stratagus.h"

#include "music.h"

#include "iolib.h"
#include "script.h"
#include "script/condition/and_condition.h"
#include "sound/music_player.h"
#include "sound/music_sample.h"
#include "sound/music_type.h"
#include "sound/sound_server.h"

static bool MusicFinished;                /// Music ended and we need a new file

bool CallbackMusic;                       /// flag true callback ccl if stops

#ifdef USE_OAML
#include <oaml.h>

std::unique_ptr<oamlApi> oaml;
bool enableOAML = false;
#endif

namespace wyrmgus {

music::music(const std::string &identifier) : data_entry(identifier)
{
}

music::~music()
{
}

void music::process_sml_property(const sml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "file") {
		this->file = value;
	} else {
		data_entry::process_sml_property(property);
	}
}

void music::process_sml_scope(const sml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "intro_music") {
		for (const std::string &value : values) {
			this->intro_music.push_back(music::get(value));
		}
	} else if (tag == "submusic") {
		for (const std::string &value : values) {
			this->submusic.push_back(music::get(value));
		}
	} else if (tag == "conditions") {
		this->conditions = std::make_unique<and_condition>();
		database::process_sml_data(this->conditions, scope);
	} else {
		data_entry::process_sml_scope(scope);
	}
}

void music::initialize()
{
	if (this->type != music_type::none) {
		music::music_by_type[this->type].push_back(this);
	}

	if (!this->file.empty()) {
		const std::string filename = LibraryFileName(this->file.string().c_str());
		this->sample = std::make_unique<wyrmgus::music_sample>(filename);
	}

	data_entry::initialize();
}

void music::check() const
{
	if (this->file.empty() && this->get_intro_music().empty() && this->get_submusic().empty()) {
		throw std::runtime_error("Music \"" + this->get_identifier() + "\" has neither a file nor submusic.");
	}

	if (this->get_conditions() != nullptr) {
		this->get_conditions()->check_validity();
	}
}

void music::load() const
{
	if (this->sample != nullptr && !this->sample->is_loaded()) {
		this->sample->load();
	}

	for (const music *intro_music : this->intro_music) {
		intro_music->load();
	}

	for (const music *submusic : this->submusic) {
		submusic->load();
	}
}

void music::clear_sample()
{
	this->sample.reset();
}

}

/**
**  Callback for when music has finished
**  Note: we are in the sdl audio thread
*/
static void MusicFinishedCallback()
{
	MusicFinished = true;
}

/**
**  Check if music is finished and play the next song
*/
void CheckMusicFinished()
{
	const bool proceed = MusicFinished;
	MusicFinished = false;

	if (proceed && SoundEnabled() && IsMusicEnabled() && CallbackMusic) {
		wyrmgus::music_player::get()->play();
	}
}

/**
**  Init music
*/
void InitMusic()
{
	MusicFinished = false;
	SetMusicFinishedCallback(MusicFinishedCallback);
}

#ifdef USE_OAML
static void* oamlOpen(const char *filename) {
	auto f = std::make_unique<CFile>();
	const std::string name = LibraryFileName(filename);
	if (f->open(name.c_str(), CL_OPEN_READ) == -1) {
		return nullptr;
	}
	return f.release();
}

static size_t oamlRead(void *ptr, size_t size, size_t nitems, void *fd) {
	CFile *f = (CFile*)fd;
	return f->read(ptr, size*nitems);
}

static int oamlSeek(void *fd, long offset, int whence) {
	CFile *f = (CFile*)fd;
	return f->seek(offset, whence);
}

static long oamlTell(void *fd) {
	CFile *f = (CFile*)fd;
	return f->tell();
}

static int oamlClose(void *fd) {
	CFile *f = reinterpret_cast<CFile *>(fd);
	int ret = f->close();
	delete f;
	return ret;
}

static oamlFileCallbacks fileCbs = {
	&oamlOpen,
	&oamlRead,
	&oamlSeek,
	&oamlTell,
	&oamlClose
};

void InitMusicOAML()
{
	const std::string filename = LibraryFileName("oaml.defs");
	oaml = std::make_unique<oamlApi>();
	oaml->SetFileCallbacks(&fileCbs);
	oaml->Init(filename.c_str());

	enableOAML = true;
	SetMusicVolume(GetMusicVolume());
}

#endif
void LoadOAMLDefinitionsFile(const std::string &file_path)
{
#ifdef USE_OAML
	const std::string filename = LibraryFileName(file_path.c_str());
	oaml->ReadDefsFile(filename.c_str());
#endif
}

#ifdef USE_OAML
void ShutdownMusicOAML()
{
	if (oaml) {
		oaml->Shutdown();
		oaml.reset();
	}
	enableOAML = false;
}
#else

void InitMusicOAML()
{
}

void ShutdownMusicOAML()
{
}

#endif
