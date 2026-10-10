/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MTROPOLIS_HACKS_H
#define MTROPOLIS_HACKS_H

#include "common/rect.h"
#include "common/ptr.h"
#include "common/hashmap.h"

namespace MTropolis {

class AssetHooks;
class ModifierHooks;
class SaveLoadHooks;
class SceneTransitionHooks;
class StructuralHooks;
class SaveLoadMechanismHooks;
struct IAutoSaveProvider;
struct MTropolisGameDescription;

struct Hacks {
	Hacks();
	~Hacks();

	void addStructuralHooks(uint32 guid, const Common::SharedPtr<StructuralHooks> &hooks);
	void addModifierHooks(uint32 guid, const Common::SharedPtr<ModifierHooks> &hooks);
	void addAssetHooks(const Common::SharedPtr<AssetHooks> &hooks);
	void addSceneTransitionHooks(const Common::SharedPtr<SceneTransitionHooks> &hooks);
	void addSaveLoadHooks(const Common::SharedPtr<SaveLoadHooks> &hooks);
	void addSaveLoadMechanismHooks(const Common::SharedPtr<SaveLoadMechanismHooks> &hooks);

	bool ignoreMismatchedProjectNameInObjectLookups;
	bool removeQuickTimeEdits;
	bool ignoreMToonMaintainRateFlag;
	bool allowAssetsFromOtherScenes;
	bool mtiVariableReferencesHack;
	bool mtiSceneReturnHack;
	bool mtiHispaniolaDamagedStringHack;

	bool ignoreSceneUnloads;

	// Parent changes requested during a frame are only made at the end of the next frame, once nothing else is
	// pending. Uncle Albert's Fabulous Voyage counts the replies that cloned objects send when their parent
	// changes, and expects them after the loop that clones them has ended.
	bool deferParentChangesToEndOfNextFrame;

	// Point lists restored from ScummVM saves get their X and Y swapped, like the variable files of the original
	// player, because the game swaps them back after loading.
	bool swapRestoredPointLists;

	// A Set Attribute "visible" message that changes the visibility also sends the Show or Hide event.
	// Uncle Albert's Fabulous Voyage restores some notebook objects this way and waits for their Show event.
	bool attribSetVisibleShowsAndHides;

	uint midiVolumeScale;	// 256 = 1.0

	uint32 minTransitionDuration;

	Common::Point reportDisplaySize;	// If X or Y is non-zero, report this as the display size
	Common::Point mainWindowOffset;		// Coordinate offset of the main window

	Common::HashMap<uint32, Common::SharedPtr<StructuralHooks> > structuralHooks;
	Common::HashMap<uint32, Common::SharedPtr<ModifierHooks> > modifierHooks;
	Common::SharedPtr<StructuralHooks> defaultStructuralHooks;
	Common::Array<Common::SharedPtr<SceneTransitionHooks> > sceneTransitionHooks;
	Common::Array<Common::SharedPtr<AssetHooks> > assetHooks;
	Common::Array<Common::SharedPtr<SaveLoadHooks> > saveLoadHooks;
	Common::Array<Common::SharedPtr<SaveLoadMechanismHooks> > saveLoadMechanismHooks;
};

namespace HackSuites {

void addObsidianQuirks(const MTropolisGameDescription &desc, Hacks &hacks);
void addObsidianBugFixes(const MTropolisGameDescription &desc, Hacks &hacks);
void addObsidianAutoSaves(const MTropolisGameDescription &desc, Hacks &hacks, IAutoSaveProvider *autoSaveProvider);
void addObsidianSaveMechanism(const MTropolisGameDescription &desc, Hacks &hacks);
void addObsidianImprovedWidescreen(const MTropolisGameDescription &desc, Hacks &hacks);

void addMTIQuirks(const MTropolisGameDescription &desc, Hacks &hacks);

} // End of namespace HackSuites

} // End of namespace MTropolis

#endif
