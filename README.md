# ScummVM patches for Le Fabuleux Voyage de l'Oncle Ernest

The `albert2-fr` branch of this repository is ScummVM (upstream `scummvm/scummvm` at commit `5aecb0fb`, tag
`albert2-base`) with these patches applied, one commit each. This branch only holds the same patches as files.

Apply them to upstream ScummVM:

```sh
git clone https://github.com/scummvm/scummvm.git && cd scummvm
git checkout -b albert2-fr 5aecb0fb
git am /path/to/patches/*.patch
```

The last patch (`DEBUG:`) is local tooling (autoplay driver, tracing) and is not meant for upstream; leave it out with
`git am $(ls /path/to/patches/*.patch | grep -v DEBUG)`.

| Patch | What it does | Upstream? |
|---|---|---|
| `0001` | Enables the French `albert2` game entry | Yes |
| `0002` | Loads the data files the game ships as mTropolis variable files | Yes |
| `0003` | Text label calculated fields and text entry (the name screen) | Yes |
| `0004` | Hoologic bitmap modifiers (the notebook pages and page turns) | Yes |
| `0005` | Background-transparent ink for 8-bit images | Yes |
| `0006` | Loads scenes on demand when scripts access their children | Yes |
| `0007` | `globalposition` writes, asset preload hints, `cursorhotspot` | Yes |
| `0008` | `size` attribute of the Thereware Rotator | Yes |
| `0009` | Compare object reference variables by the object they reference | Yes |
| `0010` | Show cloned elements (pending show checks, clones from library scenes) | Yes |
| `0011` | Rotator images drawn with a transparent background (no rotation yet) | Yes |
| `0012` | Fix the loop/visible attribute IDs of Set Attribute messages (Marseille page-turn freeze) | Yes |
| `0013` | Prefer name matches over unrelated objects with the same (stale) GUID | Yes |
| `0014` | Resolve child modifiers of behaviors by name | Yes |
| `0015` | Writable Rotator `size` | Yes |
| `0016` | Re-parenting to no object is ignored | Yes |
| `0017` | Remap the "with" variable of cloned messengers | Yes |
| `0018` | Default value for list reads outside the list | Yes |
| `0019` | Store the Rotator `angle` | Yes |
| `0020` | Clones inserted after their original, references remapped in the whole clone | Yes |
| `0021` | Game hack: defer parent changes to the end of the next frame (page restore) | Yes |
| `0022` | Game hack: restored point lists are swapped like the original's files (resume positions) | Yes |
| `0023` | Object paths stored in string variables (`.object`, assigning an object) | Yes |
| `0024` | Clone and Kill command messages (the developed photo) | Yes |
| `0025` | Read-only object attributes in the middle of l-value paths (placing a photo) | Yes |
| `0026` | "Set to source's parent" of object reference variables (the harbour X-ray) | Yes |
| `0027` | Hoologic Scale Bitmap with a constant or variable size (Bombay failed to load) | Yes |
| `0028` | Name the modifier that failed to load | Yes |
| `0029` | Game hack: Set Attribute "visible" sends Show/Hide (Paris page restore, page turn stuck half way) | Yes |
| `0030` | Thereware Rotator draws rotated (Istanbul discs and cover), `axisoffset` | Yes |
| `0031` | Collision check removes expired elements from the right list (crash turning to Cairo) | Yes |
| `0032` | Hoologic Save Bitmap and Import Bitmap (photos were blank) | Yes |
| `0033` | Miniscript `vector(angle, magnitude)` instruction (the Zanzibar ladybug) | Yes |
| `0034` | Hit-test paletted mToon frames by palette index (hint papers covered the tabs) | Yes |
| `0035` | `objectid` attribute (the sorcerer's toads) | Yes |
| `0036` | Messages reach the variables of compound variables (clone replies, the toads) | Yes |
| `0037` | Looping path motion wraps after its last point (crash when the ants turn the capstan) | Yes |
| `0038` | Into scene and out of scene Simple Motion (the lid of the Tahiti sextant box) | Yes |
| `0039` | Elements drawn at the size scripts give them: Rotator size, resized mToons (objects shrunk into their margin frames) | Yes |
| `0040` | Elements hidden by Scene Started scripts stay hidden (the "ATTENTION" texts at the top of the screen) | Yes |
| `0041` | Text label `<variable>` fields filled in from the start and on every update (the `<texte_trad>` photo frame captions) | Yes |
| `0042` | `cursorelement` / `cursorhotspot`: tool cursors follow the mouse everywhere (the loupe stuck on the whistle in the bottom margin) | Yes |
| last, `DEBUG:` | Autoplay driver and tracing | **No**, local tooling only |

The tools, saves, autoplay scripts and notes are in the `oncle-ernest` repository. No game data is included.
