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

#ifndef MTROPOLIS_AUTOPLAY_H
#define MTROPOLIS_AUTOPLAY_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/str.h"

class OSystem;

namespace MTropolis {

class Runtime;
class RuntimeObject;
class Structural;

// Debug-only input driver for automated testing. Enabled by setting the
// MTROPOLIS_AUTOPLAY environment variable to the path of a command file.
// The file is followed like "tail -f": lines appended while the game runs
// are executed too. One command per line:
//   wait <ms>                 pause the script
//   move <x> <y>              move the mouse
//   click <x> <y>             move, then left button down and up
//   down <x> <y> / up <x> <y> move, then press or release the left button (drag = down, moves, up)
//   clickon <name>            click the first visible element with that name that is on screen, wherever
//                             it is now (for moving targets), at the point nearest its center where it
//                             gets the click (not covered, not transparent)
//   clickwhen <name> [x1 y1 x2 y2] [ms]  wait until that element's center is inside the area (default:
//                             the whole screen), follow it with the mouse for 250 ms (some games move
//                             their cursor element with a delay), then click it; gives up after ms
//                             (default 30000)
//   dragto <name> <x> <y> [<nx> <ny> [<gx> <gy>]]  drag the first visible element with that name (or the one
//                             nearest to (nx, ny)) from a point where it gets the click (or from its center moved
//                             by (gx, gy)), so that its center ends at (x, y), correcting for size and offsets
//   key <char|name>           press and release a key (name: esc, enter,
//                             backspace, up, down, left, right, space, tab)
//   type <text>               type each character of text
//   shot <file.png>           save the current screen as PNG
//   scene                     log the active main scene name
//   elements                  log the visible elements in render order
//   hotspots                  log the visible elements that react to the mouse now
//   info <guid>               log every object with that GUID: parents, visibility, rect, layer
//   attr <guid>[#n] <a.b.c>   log the value of an attribute path read from the object (structural or
//                             modifier) with that GUID (the nth one for clones), like a script would
//   dumpimage <guid> <file.png>  save the picture of an image element (for maps)
//   hide <guid> / show <guid> change the visibility of a visual element (hex GUID)
//   skipif <msg> <n>          skip the next n command lines if author message <msg> (hex) was sent so far
//   send <guid> <msg>         send author message <msg> (hex) to a structural object, cascading
//   port [ms]                 play the harbour crate game of Uncle Albert's Fabulous Voyage (01-B-port):
//                             switch safe crates up and bombs right, stamp the crates on the lower rails;
//                             stops when the game is won (message 3ba) or after ms (default 300000)
//   nile [ms]                 steer the boat of the Nile game of Uncle Albert's Fabulous Voyage (03-B-nil) away
//                             from crocodiles, islands and floating bombs (box01/02); stops when the game is won (message 3ba) or after ms
//   borneo [ms]               drag the dragonfly of the Borneo game (05-B-borneo) from seeds to the lily pad,
//                             coming from the left and waiting when an open carnivorous plant is in the way;
//                             stops when the game is won (message 3df) or after ms
//   manille                   pilot the Manila motorboat (06-A-manille): plans a water path on the map picture
//                             through the two refuel points to Crocodile Island and clicks along it
//   sydney [ms]               dive the Sydney submarine (06-C-sydney) to the bottom, plug the cracks with boards,
//                             photograph the gulper when it attacks, then rise; stops at the surface or after ms
//   sacred [ms]               drop each parchment piece of the sacred spot (04-C-parchemin) on its good_pos
//   mexico                    repeat the jewel sequence of the Mexico memory test (List_light of zone_lampes)
//   hittest <x> <y>          log the elements under the point and which mouse target tests they pass
//   tahiti [ms]               cut both ropes of Tahiti (07-A-tahiti) with the crab: drop it next to each frayed part
//                             (bug_rope) so that it ends there facing it after its walk, then click it to pinch
//   waitscene <name> [ms]     wait until the main scene is <name>
//   goto <name>               change to the scene <name> (skips the game logic
//                             that normally leads there)
//   quit                      quit the engine
class AutoPlayer {
public:
	AutoPlayer();

	bool isEnabled() const;
	// Called once per frame. Returns false when the script asked to quit.
	bool update(Runtime *runtime, OSystem *system);

private:
	void readNewLines();
	bool executeLine(Runtime *runtime, OSystem *system, const Common::String &line);
	void pressKey(Runtime *runtime, const Common::String &keyName);
	void saveScreenshot(OSystem *system, const Common::String &fileName);
	Common::String getSceneName(Runtime *runtime) const;
	void logElements(Runtime *runtime) const;
	void logHotspots(Runtime *runtime) const;
	Structural *findStructural(Runtime *runtime, uint32 guid) const;
	bool findOnScreenElement(Runtime *runtime, const Common::String &name, Common::Point &outCenter, const Common::Rect &area) const;
	void clickAt(Runtime *runtime, OSystem *system, const Common::Point &point);
	void gotoScene(Runtime *runtime, const Common::String &sceneName);
	void playPort(Runtime *runtime, OSystem *system, uint32 now);
	void playNile(Runtime *runtime, OSystem *system, uint32 now);
	void playBorneo(Runtime *runtime, OSystem *system, uint32 now);
	void playManille(Runtime *runtime, OSystem *system, uint32 now);
	bool planManille(Runtime *runtime);
	void playSydney(Runtime *runtime, OSystem *system, uint32 now);
	void playTahiti(Runtime *runtime, OSystem *system, uint32 now);
	void playSacred(Runtime *runtime, OSystem *system, uint32 now);
	void queueMouse(int type, int32 x, int32 y, uint32 delay);
	void queueSwipe(int32 x0, int32 y0, int32 x1, int32 y1, uint steps, uint32 delay);
	bool runMouseQueue(Runtime *runtime, OSystem *system, uint32 now);

	Common::Path _scriptPath;
	bool _enabled;
	uint32 _readOffset;
	uint _skipLines;
	Common::String _partialLine;
	Common::Array<Common::String> _pendingLines;

	Common::String _clickWhenName;
	Common::Rect _clickWhenArea;
	uint32 _clickWhenUntil;
	bool _clickWhenHovering;
	uint32 _clickWhenHoverStart;

	bool findClickPoint(Runtime *runtime, const Common::String &name, Common::Point &outPoint) const;
	bool findNearestClickPoint(Runtime *runtime, const Common::String &name, const Common::Point &near, Common::Point &outPoint) const;
	bool findElementCenter(Runtime *runtime, const Common::String &name, Common::Point &outCenter) const;
	void stepDrag(Runtime *runtime, OSystem *system, uint32 now);

	Common::String _dragName;
	Common::WeakPtr<RuntimeObject> _dragElement;
	Common::Point _dragTarget;
	Common::Point _dragStart;
	bool _dragHasFrom;
	bool _dragHasGrabOffset;
	Common::Point _dragGrabOffset;
	Common::Point _dragFrom;
	uint _dragStep;
	uint32 _dragNext;

	bool _manilleActive;
	uint32 _manillePendingUntil;
	bool _manilleNeedReplan;
	uint32 _manilleLastClick;
	uint32 _manilleLastFuel;
	uint _manilleEscape;
	Common::Array<Common::Point> _manillePath;	// Map coordinates
	Common::Array<bool> _manilleFree;
	uint _manilleLegEnds[3];
	bool _manilleRefueled[2];
	Common::Point _manilleRefuelPoint[2];
	uint32 _manilleRefuelStart;
	bool _manilleMoving;
	Common::Point _manilleMoveStart;
	Common::Point _manilleMoveTarget;
	int32 _manilleGridW;
	int32 _manilleGridH;
	uint _manilleIndex;
	Common::Point _manilleLastMapPos;
	uint32 _manilleStillSince;
	uint32 _manilleNextClick;
	uint32 _borneoUntil;
	bool _borneoDragging;
	Common::Point _borneoMouse;
	uint32 _borneoGrabTime;
	struct MouseOp {
		int type;	// 0 move, 1 press, 2 release, 3 wait
		Common::Point pt;
		uint32 delay;
	};
	Common::Array<MouseOp> _mouseOps;
	uint _mouseOpIndex;
	uint32 _mouseOpNext;
	uint32 _sydneyUntil;
	bool _sydneyPhoto;
	uint32 _sydneyLeverNext;
	uint32 _sydneyLog;
	uint32 _tahitiUntil;
	uint32 _sacredUntil;
	uint32 _sacredNext;
	uint32 _tahitiNext;
	int _tahitiTries;
	uint32 _nileUntil;
	uint32 _nileNext;
	int _nileLastDir;
	uint32 _portUntil;
	uint32 _portNextAction;
	Common::HashMap<Common::String, uint32> _portLastStamp;

	uint32 _waitUntil;
	Common::String _waitScene;
	uint32 _waitSceneUntil;
	uint32 _nextPollTime;
	bool _releaseLeftButton;
	int32 _mouseX;
	int32 _mouseY;
};

} // End of namespace MTropolis

#endif
