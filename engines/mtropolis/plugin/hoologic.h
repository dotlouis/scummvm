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

#ifndef MTROPOLIS_PLUGIN_HOOLOGIC_H
#define MTROPOLIS_PLUGIN_HOOLOGIC_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/hoologic_data.h"

namespace MTropolis {

namespace Hoologic {

class BitmapVariableStorage : public VariableStorage {
public:
	friend class BitmapVariableModifier;

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad(Runtime *runtime) override;

	Common::SharedPtr<VariableStorage> clone() const override;

private:
	// Bitmaps aren't saved, the game recaptures them when needed
	class SaveLoad : public ModifierSaveLoad {
	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;
	};

	// Bitmaps are never modified in place, so copies of a variable can share them
	Common::SharedPtr<Graphics::ManagedSurface> _bitmap;
};

class BitmapVariableModifier : public VariableModifier, public IDisplayBitmapSource {
public:
	BitmapVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::BitmapVariableModifier &data);

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

	const Common::SharedPtr<Graphics::ManagedSurface> &getBitmap() const;
	void setBitmap(const Common::SharedPtr<Graphics::ManagedSurface> &bitmap);

	Common::SharedPtr<Graphics::ManagedSurface> getDisplayBitmap() const override;

	static BitmapVariableModifier *resolve(const VarReference &varRef);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "BitmapVariable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class CaptureBitmapModifier : public Modifier {
public:
	CaptureBitmapModifier();
	~CaptureBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::CaptureBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "CaptureBitmap Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _captureWhen;
	VarReference _bitmapVar;
};

class ImportBitmapModifier : public Modifier {
public:
	ImportBitmapModifier();
	~ImportBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::ImportBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "ImportBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class DisplayBitmapModifier : public Modifier {
public:
	DisplayBitmapModifier();
	~DisplayBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::DisplayBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "DisplayBitmap Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void setDisplayed(Runtime *runtime, bool displayed);

	Event _displayWhen;
	Event _removeWhen;
	VarReference _bitmapVar;
};

class ScaleBitmapModifier : public Modifier {
public:
	ScaleBitmapModifier();
	~ScaleBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::ScaleBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "ScaleBitmap Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusPartial; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _scaleWhen;
	VarReference _bitmapVar;
};

class SaveBitmapModifier : public Modifier {
public:
	SaveBitmapModifier();
	~SaveBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::SaveBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "SaveBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PrintBitmapModifier : public Modifier {
public:
	PrintBitmapModifier();
	~PrintBitmapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::PrintBitmapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "PrintBitmap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PainterModifier : public Modifier {
public:
	PainterModifier();
	~PainterModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::PainterModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Painter Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class KeyStateModifier : public Modifier {
public:
	KeyStateModifier();
	~KeyStateModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Hoologic::KeyStateModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "KeyState Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class HoologicPlugIn : public MTropolis::PlugIn {
public:
	HoologicPlugIn();
	~HoologicPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<BitmapVariableModifier, Data::Hoologic::BitmapVariableModifier> _bitmapVariableModifierFactory;
	PlugInModifierFactory<CaptureBitmapModifier, Data::Hoologic::CaptureBitmapModifier> _captureBitmapModifierFactory;
	PlugInModifierFactory<ImportBitmapModifier, Data::Hoologic::ImportBitmapModifier> _importBitmapModifierFactory;
	PlugInModifierFactory<DisplayBitmapModifier, Data::Hoologic::DisplayBitmapModifier> _displayBitmapModifierFactory;
	PlugInModifierFactory<ScaleBitmapModifier, Data::Hoologic::ScaleBitmapModifier> _scaleBitmapModifierFactory;
	PlugInModifierFactory<SaveBitmapModifier, Data::Hoologic::SaveBitmapModifier> _saveBitmapModifierFactory;
	PlugInModifierFactory<PrintBitmapModifier, Data::Hoologic::PrintBitmapModifier> _printBitmapModifierFactory;
	PlugInModifierFactory<PainterModifier, Data::Hoologic::PainterModifier> _painterModifierFactory;
	PlugInModifierFactory<KeyStateModifier, Data::Hoologic::KeyStateModifier> _keyStateModifierFactory;
};

} // End of namespace Hoologic

} // End of namespace MTropolis

#endif
