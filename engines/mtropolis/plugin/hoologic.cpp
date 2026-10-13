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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/managed_surface.h"

#include "image/bmp.h"

#include "mtropolis/plugin/hoologic.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Hoologic {

BitmapVariableModifier::BitmapVariableModifier() : VariableModifier(Common::SharedPtr<VariableStorage>(new BitmapVariableStorage())) {
}

bool BitmapVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::BitmapVariableModifier &data) {
	return true;
}

bool BitmapVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kNull) {
		setBitmap(nullptr);
		return true;
	}

	// Assigning a bitmap variable to another one copies the bitmap
	if (value.getType() == DynamicValueTypes::kObject) {
		Common::SharedPtr<RuntimeObject> obj = value.getObject().object.lock();
		if (obj && obj->isModifier() && static_cast<Modifier *>(obj.get())->isVariable()) {
			BitmapVariableStorage *otherStorage = dynamic_cast<BitmapVariableStorage *>(static_cast<VariableModifier *>(obj.get())->getStorage().get());
			if (otherStorage) {
				setBitmap(otherStorage->_bitmap);
				return true;
			}
		}
	}

	return false;
}

void BitmapVariableModifier::varGetValue(DynamicValue &dest) const {
	dest.setObject(getSelfReference());
}

bool BitmapVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	// Scripts copy bitmaps with "set dest to source.object"
	if (attrib == "object") {
		result.setObject(getSelfReference());
		return true;
	}

	const Common::SharedPtr<Graphics::ManagedSurface> &bitmap = getBitmap();
	if (attrib == "width") {
		result.setInt(bitmap ? bitmap->w : 0);
		return true;
	}
	if (attrib == "height") {
		result.setInt(bitmap ? bitmap->h : 0);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

const Common::SharedPtr<Graphics::ManagedSurface> &BitmapVariableModifier::getBitmap() const {
	return static_cast<const BitmapVariableStorage *>(getStorage().get())->_bitmap;
}

void BitmapVariableModifier::setBitmap(const Common::SharedPtr<Graphics::ManagedSurface> &bitmap) {
	static_cast<BitmapVariableStorage *>(getStorage().get())->_bitmap = bitmap;
}

Common::SharedPtr<Graphics::ManagedSurface> BitmapVariableModifier::getDisplayBitmap() const {
	return getBitmap();
}

BitmapVariableModifier *BitmapVariableModifier::resolve(const VarReference &varRef) {
	Common::SharedPtr<Modifier> modifier = varRef.resolution.lock();
	if (!modifier)
		return nullptr;

	return dynamic_cast<BitmapVariableModifier *>(modifier.get());
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void BitmapVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	const Common::SharedPtr<Graphics::ManagedSurface> &bitmap = getBitmap();
	if (bitmap)
		report->declareDynamic("bitmap", Common::String::format("%ix%i", static_cast<int>(bitmap->w), static_cast<int>(bitmap->h)));
	else
		report->declareDynamic("bitmap", "None");
}
#endif

Common::SharedPtr<Modifier> BitmapVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new BitmapVariableModifier(*this));
}

const char *BitmapVariableModifier::getDefaultName() const {
	return "Bitmap Variable Modifier"; // ???
}

Common::SharedPtr<ModifierSaveLoad> BitmapVariableStorage::getSaveLoad(Runtime *runtime) {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad());
}

Common::SharedPtr<VariableStorage> BitmapVariableStorage::clone() const {
	return Common::SharedPtr<VariableStorage>(new BitmapVariableStorage(*this));
}

void BitmapVariableStorage::SaveLoad::commitLoad() const {
}

void BitmapVariableStorage::SaveLoad::saveInternal(Common::WriteStream *stream) const {
}

bool BitmapVariableStorage::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	return true;
}

CaptureBitmapModifier::CaptureBitmapModifier() {
}

CaptureBitmapModifier::~CaptureBitmapModifier() {
}

bool CaptureBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::CaptureBitmapModifier &data) {
	if (data.captureWhen.type != Data::PlugInTypeTaggedValue::kEvent || data.bitmapVar.type != Data::PlugInTypeTaggedValue::kVariableReference)
		return false;

	if (!_captureWhen.load(data.captureWhen.value.asEvent))
		return false;

	_bitmapVar = VarReference(data.bitmapVar.value.asVarRefGUID, "");

	return true;
}

bool CaptureBitmapModifier::respondsToEvent(const Event &evt) const {
	return _captureWhen.respondsTo(evt);
}

VThreadState CaptureBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (!_captureWhen.respondsTo(msg->getEvent()))
		return kVThreadReturn;

	BitmapVariableModifier *bitmapVar = BitmapVariableModifier::resolve(_bitmapVar);
	if (!bitmapVar) {
		warning("Capture Bitmap modifier couldn't resolve its bitmap variable");
		return kVThreadReturn;
	}

	Structural *owner = findStructuralOwner();
	Common::SharedPtr<Window> window = runtime->getMainWindow().lock();
	if (!owner || !owner->isElement() || !static_cast<Element *>(owner)->isVisual() || !window)
		return kVThreadReturn;

	// Capture what was last drawn on screen in the area of the parent element
	VisualElement *element = static_cast<VisualElement *>(owner);
	const Graphics::ManagedSurface &screen = *window->getSurface();

	Common::Point origin = element->getGlobalPosition();
	const Common::Rect &relRect = element->getRelativeRect();
	Common::Rect captureRect(origin.x, origin.y, origin.x + relRect.width(), origin.y + relRect.height());
	captureRect.clip(Common::Rect(screen.w, screen.h));

	if (captureRect.isEmpty()) {
		bitmapVar->setBitmap(nullptr);
		return kVThreadReturn;
	}

	Common::SharedPtr<Graphics::ManagedSurface> bitmap(new Graphics::ManagedSurface(captureRect.width(), captureRect.height(), screen.format));
	bitmap->blitFrom(screen, captureRect, Common::Point(0, 0));
	bitmapVar->setBitmap(bitmap);

	return kVThreadReturn;
}

void CaptureBitmapModifier::disable(Runtime *runtime) {
}

void CaptureBitmapModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_bitmapVar.linkInternalReferences(scope);
}

void CaptureBitmapModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_bitmapVar.visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void CaptureBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> CaptureBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CaptureBitmapModifier(*this));
}

const char *CaptureBitmapModifier::getDefaultName() const {
	return "Capture Bitmap Modifier"; // ???
}

namespace {

// Bitmaps saved by the Save Bitmap modifier go to the save folder, named after the game target and the path
// the game gives (for example "<player>/photo1/photo3" for Uncle Albert's photos), and Import Bitmap reads them back.
Common::String bitmapFileName(const Common::String &path) {
	Common::String name;
	bool inToken = false;
	for (char c : path) {
		if (c == '<')
			inToken = true;
		else if (c == '>')
			inToken = false;
		else if (inToken)
			continue;
		else if (Common::isAlnum(c) || c == '_' || c == '-')
			name += c;
		else if (!name.empty() && name.lastChar() != '-')
			name += '-';
	}
	while (!name.empty() && name.lastChar() == '-')
		name.deleteLastChar();
	return ConfMan.getActiveDomainName() + ".bitmap-" + name + ".bmp";
}

// Of two variable references, returns the bitmap variable and the other variable
void sortBitmapVars(const VarReference &a, const VarReference &b, BitmapVariableModifier *&outBitmapVar, VariableModifier *&outOtherVar) {
	outBitmapVar = BitmapVariableModifier::resolve(a);
	Common::SharedPtr<Modifier> other = b.resolution.lock();
	if (!outBitmapVar) {
		outBitmapVar = BitmapVariableModifier::resolve(b);
		other = a.resolution.lock();
	}
	outOtherVar = (other && other->isVariable()) ? static_cast<VariableModifier *>(other.get()) : nullptr;
}

bool getPathFromVar(VariableModifier *var, Common::String &outPath) {
	if (!var)
		return false;
	DynamicValue value;
	var->varGetValue(value);
	if (value.getType() != DynamicValueTypes::kString)
		return false;
	outPath = value.getString();
	return !outPath.empty();
}

} // End of anonymous namespace

ImportBitmapModifier::ImportBitmapModifier() {
}

ImportBitmapModifier::~ImportBitmapModifier() {
}

bool ImportBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::ImportBitmapModifier &data) {
	// The two booleans are unknown. One variable is the bitmap, the other one holds the file path.
	if (data.unknown1Event.type != Data::PlugInTypeTaggedValue::kEvent || !_when.load(data.unknown1Event.value.asEvent))
		return false;
	if (data.unknown4VarRef.type == Data::PlugInTypeTaggedValue::kVariableReference)
		_var1 = VarReference(data.unknown4VarRef.value.asVarRefGUID, "");
	if (data.unknown5VarRef.type == Data::PlugInTypeTaggedValue::kVariableReference)
		_var2 = VarReference(data.unknown5VarRef.value.asVarRefGUID, "");
	return true;
}

bool ImportBitmapModifier::respondsToEvent(const Event &evt) const {
	return _when.respondsTo(evt);
}

VThreadState ImportBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (!_when.respondsTo(msg->getEvent()))
		return kVThreadReturn;

	BitmapVariableModifier *bitmapVar = nullptr;
	VariableModifier *pathVar = nullptr;
	sortBitmapVars(_var1, _var2, bitmapVar, pathVar);
	Common::String path;
	if (!bitmapVar || !getPathFromVar(pathVar, path)) {
		warning("Import Bitmap modifier couldn't resolve its variables");
		return kVThreadReturn;
	}

	Common::SharedPtr<Window> window = runtime->getMainWindow().lock();
	Common::ScopedPtr<Common::InSaveFile> file(g_system->getSavefileManager()->openForLoading(bitmapFileName(path)));
	Image::BitmapDecoder decoder;
	if (!window || !file || !decoder.loadStream(*file)) {
		warning("Import Bitmap modifier couldn't load '%s'", path.c_str());
		return kVThreadReturn;
	}

	const Graphics::PixelFormat &format = window->getSurface()->format;
	Common::SharedPtr<Graphics::ManagedSurface> bitmap(new Graphics::ManagedSurface());
	Graphics::Surface *converted = decoder.getSurface()->convertTo(format, decoder.getPalette().data(), decoder.getPalette().size());
	bitmap->copyFrom(*converted);
	converted->free();
	delete converted;
	bitmapVar->setBitmap(bitmap);
	runtime->setSceneGraphDirty();

	return kVThreadReturn;
}

void ImportBitmapModifier::disable(Runtime *runtime) {
}

void ImportBitmapModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_var1.linkInternalReferences(scope);
	_var2.linkInternalReferences(scope);
}

void ImportBitmapModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_var1.visitInternalReferences(visitor);
	_var2.visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ImportBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> ImportBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ImportBitmapModifier(*this));
}

const char *ImportBitmapModifier::getDefaultName() const {
	return "Import Bitmap Modifier"; // ???
}

DisplayBitmapModifier::DisplayBitmapModifier() {
}

DisplayBitmapModifier::~DisplayBitmapModifier() {
}

bool DisplayBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::DisplayBitmapModifier &data) {
	if (data.displayWhen.type != Data::PlugInTypeTaggedValue::kEvent || data.removeWhen.type != Data::PlugInTypeTaggedValue::kEvent || data.bitmapVar.type != Data::PlugInTypeTaggedValue::kVariableReference)
		return false;

	if (!_displayWhen.load(data.displayWhen.value.asEvent) || !_removeWhen.load(data.removeWhen.value.asEvent))
		return false;

	_bitmapVar = VarReference(data.bitmapVar.value.asVarRefGUID, "");

	return true;
}

bool DisplayBitmapModifier::respondsToEvent(const Event &evt) const {
	return _displayWhen.respondsTo(evt) || _removeWhen.respondsTo(evt);
}

VThreadState DisplayBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_displayWhen.respondsTo(msg->getEvent()))
		setDisplayed(runtime, true);
	else if (_removeWhen.respondsTo(msg->getEvent()))
		setDisplayed(runtime, false);

	return kVThreadReturn;
}

void DisplayBitmapModifier::disable(Runtime *runtime) {
	setDisplayed(runtime, false);
}

void DisplayBitmapModifier::setDisplayed(Runtime *runtime, bool displayed) {
	Structural *owner = findStructuralOwner();
	if (!owner || !owner->isElement() || !static_cast<Element *>(owner)->isVisual())
		return;

	// The element draws the current contents of the variable, so later changes to the bitmap are displayed too
	VisualElement *element = static_cast<VisualElement *>(owner);
	BitmapVariableModifier *bitmapVar = displayed ? BitmapVariableModifier::resolve(_bitmapVar) : nullptr;
	if (bitmapVar) {
		element->setDisplayBitmapSource(bitmapVar->getSelfReference());

		// The element takes the size of the bitmap it starts displaying. Uncle Albert's Fabulous Voyage shrinks the page
		// it turns by setting the element width, and relies on this to have the page full size on the next turn.
		const Common::SharedPtr<Graphics::ManagedSurface> &bitmap = bitmapVar->getBitmap();
		if (bitmap) {
			const Common::Rect &rect = element->getRelativeRect();
			if (rect.width() != bitmap->w || rect.height() != bitmap->h)
				element->setRelativeRect(Common::Rect(rect.left, rect.top, rect.left + bitmap->w, rect.top + bitmap->h));
		}
	} else
		element->setDisplayBitmapSource(Common::WeakPtr<RuntimeObject>());
}

void DisplayBitmapModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_bitmapVar.linkInternalReferences(scope);
}

void DisplayBitmapModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_bitmapVar.visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void DisplayBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> DisplayBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new DisplayBitmapModifier(*this));
}

const char *DisplayBitmapModifier::getDefaultName() const {
	return "Display Bitmap Modifier"; // ???
}

ScaleBitmapModifier::ScaleBitmapModifier() {
}

ScaleBitmapModifier::~ScaleBitmapModifier() {
}

bool ScaleBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::ScaleBitmapModifier &data) {
	if (data.scaleWhen.type != Data::PlugInTypeTaggedValue::kEvent || data.bitmapVar.type != Data::PlugInTypeTaggedValue::kVariableReference)
		return false;

	// The size is usually the incoming data, but can be a constant or a variable
	if (!_size.load(data.size))
		return false;

	if (!_scaleWhen.load(data.scaleWhen.value.asEvent))
		return false;

	_bitmapVar = VarReference(data.bitmapVar.value.asVarRefGUID, "");

	return true;
}

bool ScaleBitmapModifier::respondsToEvent(const Event &evt) const {
	return _scaleWhen.respondsTo(evt);
}

VThreadState ScaleBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (!_scaleWhen.respondsTo(msg->getEvent()))
		return kVThreadReturn;

	BitmapVariableModifier *bitmapVar = BitmapVariableModifier::resolve(_bitmapVar);
	if (!bitmapVar) {
		warning("Scale Bitmap modifier couldn't resolve its bitmap variable");
		return kVThreadReturn;
	}

	DynamicValue size = _size.produceValue(msg->getValue()).dereference();
	if (size.getType() != DynamicValueTypes::kPoint) {
		warning("Scale Bitmap modifier expected a point size");
		return kVThreadReturn;
	}

	const Common::SharedPtr<Graphics::ManagedSurface> &bitmap = bitmapVar->getBitmap();
	if (!bitmap)
		return kVThreadReturn;

	int16 newWidth = size.getPoint().x;
	int16 newHeight = size.getPoint().y;
	if (newWidth <= 0 || newHeight <= 0) {
		bitmapVar->setBitmap(nullptr);
	} else if (newWidth != bitmap->w || newHeight != bitmap->h) {
		bitmapVar->setBitmap(Common::SharedPtr<Graphics::ManagedSurface>(bitmap->scale(newWidth, newHeight)));
	}

	// Elements displaying the bitmap need to be redrawn
	runtime->setSceneGraphDirty();

	return kVThreadReturn;
}

void ScaleBitmapModifier::disable(Runtime *runtime) {
}

void ScaleBitmapModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_bitmapVar.linkInternalReferences(scope);
	_size.linkInternalReferences(scope);
}

void ScaleBitmapModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_bitmapVar.visitInternalReferences(visitor);
	_size.visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ScaleBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> ScaleBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ScaleBitmapModifier(*this));
}

const char *ScaleBitmapModifier::getDefaultName() const {
	return "Scale Bitmap Modifier"; // ???
}

SaveBitmapModifier::SaveBitmapModifier() {
}

SaveBitmapModifier::~SaveBitmapModifier() {
}

bool SaveBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::SaveBitmapModifier &data) {
	// The two booleans are unknown. One variable is the bitmap, the other one holds the file path.
	if (data.unknown1Event.type != Data::PlugInTypeTaggedValue::kEvent || !_when.load(data.unknown1Event.value.asEvent))
		return false;
	if (data.unknown2VarRef.type == Data::PlugInTypeTaggedValue::kVariableReference)
		_var1 = VarReference(data.unknown2VarRef.value.asVarRefGUID, "");
	if (data.unknown5VarRef.type == Data::PlugInTypeTaggedValue::kVariableReference)
		_var2 = VarReference(data.unknown5VarRef.value.asVarRefGUID, "");
	return true;
}

bool SaveBitmapModifier::respondsToEvent(const Event &evt) const {
	return _when.respondsTo(evt);
}

VThreadState SaveBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (!_when.respondsTo(msg->getEvent()))
		return kVThreadReturn;

	BitmapVariableModifier *bitmapVar = nullptr;
	VariableModifier *pathVar = nullptr;
	sortBitmapVars(_var1, _var2, bitmapVar, pathVar);
	Common::String path;
	if (!bitmapVar || !getPathFromVar(pathVar, path)) {
		warning("Save Bitmap modifier couldn't resolve its variables");
		return kVThreadReturn;
	}

	const Common::SharedPtr<Graphics::ManagedSurface> &bitmap = bitmapVar->getBitmap();
	if (!bitmap)
		return kVThreadReturn;

	Common::ScopedPtr<Common::OutSaveFile> file(g_system->getSavefileManager()->openForSaving(bitmapFileName(path), false));
	if (!file || !Image::writeBMP(*file, bitmap->rawSurface())) {
		warning("Save Bitmap modifier couldn't save '%s'", path.c_str());
		return kVThreadReturn;
	}
	file->finalize();

	return kVThreadReturn;
}

void SaveBitmapModifier::disable(Runtime *runtime) {
}

void SaveBitmapModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_var1.linkInternalReferences(scope);
	_var2.linkInternalReferences(scope);
}

void SaveBitmapModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_var1.visitInternalReferences(visitor);
	_var2.visitInternalReferences(visitor);
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void SaveBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> SaveBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SaveBitmapModifier(*this));
}

const char *SaveBitmapModifier::getDefaultName() const {
	return "Save Bitmap Modifier"; // ???
}

PrintBitmapModifier::PrintBitmapModifier() {
}

PrintBitmapModifier::~PrintBitmapModifier() {
}

bool PrintBitmapModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::PrintBitmapModifier &data) {
	return true;
}

bool PrintBitmapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState PrintBitmapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void PrintBitmapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PrintBitmapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> PrintBitmapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PrintBitmapModifier(*this));
}

const char *PrintBitmapModifier::getDefaultName() const {
	return "Print Bitmap Modifier"; // ???
}

PainterModifier::PainterModifier() {
}

PainterModifier::~PainterModifier() {
}

bool PainterModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::PainterModifier &data) {
	return true;
}

bool PainterModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState PainterModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void PainterModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PainterModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> PainterModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PainterModifier(*this));
}

const char *PainterModifier::getDefaultName() const {
	return "Painter Modifier"; // ???
}

KeyStateModifier::KeyStateModifier() {
}

KeyStateModifier::~KeyStateModifier() {
}

bool KeyStateModifier::load(const PlugInModifierLoaderContext &context, const Data::Hoologic::KeyStateModifier &data) {
	return true;
}

bool KeyStateModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState KeyStateModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void KeyStateModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void KeyStateModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> KeyStateModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new KeyStateModifier(*this));
}

const char *KeyStateModifier::getDefaultName() const {
	return "KeyState Modifier"; // ???
}

HoologicPlugIn::HoologicPlugIn()
	: _bitmapVariableModifierFactory(this)
	, _captureBitmapModifierFactory(this)
	, _importBitmapModifierFactory(this)
	, _displayBitmapModifierFactory(this)
	, _scaleBitmapModifierFactory(this)
	, _saveBitmapModifierFactory(this)
	, _printBitmapModifierFactory(this)
	, _painterModifierFactory(this)
	, _keyStateModifierFactory(this)
	{	
}

HoologicPlugIn::~HoologicPlugIn() {
}

void HoologicPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	//Bitmap plugin
	registrar->registerPlugInModifier("hlBitmapVariable", &_bitmapVariableModifierFactory);
	registrar->registerPlugInModifier("hlCaptureBitmap", &_captureBitmapModifierFactory);
	registrar->registerPlugInModifier("hlImportBitmap", &_importBitmapModifierFactory);
	registrar->registerPlugInModifier("hlDisplayBitmap", &_displayBitmapModifierFactory);
	registrar->registerPlugInModifier("hlScaleBitmap", &_scaleBitmapModifierFactory);
	registrar->registerPlugInModifier("hlSaveBitmap", &_saveBitmapModifierFactory);
	registrar->registerPlugInModifier("hlPrintBitmap", &_printBitmapModifierFactory);

	//Painter plugin
	registrar->registerPlugInModifier("hlPainter", &_painterModifierFactory);

	//KeyState plugin
	registrar->registerPlugInModifier("hlKeyState", &_keyStateModifierFactory);
}

} // End of namespace Hoologic

namespace PlugIns {

Common::SharedPtr<PlugIn> createHoologic() {
	return Common::SharedPtr<PlugIn>(new Hoologic::HoologicPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
