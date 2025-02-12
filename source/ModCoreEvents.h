#pragma once

#include <unordered_set>
#include "HRZR/Core/CoreFileManager.h"
#include "HRZR/Core/RTTIObjectTweaker.h"
#include "HRZR/PCore/Common.h"

namespace HRZR
{
	class RTTI;
	class RTTIRefObject;
}

class ModCoreEvents : public HRZR::CoreFileManager::Events
{
private:
	struct RTTICallbackPatch
	{
		void (*m_Callback)(HRZR::Ref<HRZR::RTTIRefObject>&);
	};

	struct RTTIValuePatch
	{
		HRZR::String m_Path;
		HRZR::String m_Value;
	};

	using RTTIPatch = std::variant<RTTICallbackPatch, RTTIValuePatch>;

	class ValuePatchVisitor : public HRZR::RTTIObjectTweaker::SetValueVisitor
	{
	public:
		ValuePatchVisitor(const RTTIValuePatch& Patch);
		ValuePatchVisitor(const ValuePatchVisitor&) = delete;
		virtual ~ValuePatchVisitor() override = default;

		virtual void Visit(void *Object, const HRZR::RTTI *Type) override;
		virtual EMode GetMode() override;
	};

	// Patching callbacks
	std::unordered_map<const HRZR::RTTI *, std::vector<RTTIPatch>> m_RTTIPatchesByType;
	std::unordered_map<HRZR::GGUUID, std::vector<RTTIPatch>> m_RTTIPatchesByUUID;
	std::unordered_map<const HRZR::RTTI *, std::vector<RTTICallbackPatch>> m_RTTIUnloadsByType;

	// Character override
	std::optional<HRZR::GGUUID> m_CharacterOverrideRootUUID;
	std::optional<HRZR::GGUUID> m_CharacterOverrideVariantUUID;

public:
	HRZR::SharedMutex m_CachedDataMutex;
	std::unordered_set<HRZR::RTTIRefObject *> m_CachedAIFactions;
	std::unordered_set<HRZR::RTTIRefObject *> m_CachedSpawnSetups;
	std::unordered_set<HRZR::RTTIRefObject *> m_CachedWeatherSetups;
	std::unordered_set<HRZR::RTTIRefObject *> m_CachedInventoryItems;
	float m_CachedLODRangeModifierHack = std::numeric_limits<float>::max();

	ModCoreEvents();
	ModCoreEvents(const ModCoreEvents&) = delete;
	virtual ~ModCoreEvents() = default;

	void RegisterParsedPatches();
	void RegisterHardcodedPatches();
	bool ShouldHandleInventoryItem(const HRZR::RTTI *Type, const HRZR::GGUUID& UUID);

	virtual void OnStartLoading(const HRZR::String& CorePath) override;
	virtual void OnFinishLoading(const HRZR::String& CorePath) override;
	virtual void OnStartUnloading(const HRZR::String& CorePath) override;
	virtual void OnFinishUnloading() override;
	virtual void OnReadFile(const HRZR::String& CorePath, const HRZR::Array<HRZR::Ref<HRZR::RTTIRefObject>>& Objects) override;
	virtual void OnLoadAsset(const HRZR::String& CorePath, const HRZR::Array<HRZR::Ref<HRZR::RTTIRefObject>>& Objects) override;
	virtual void OnUnloadAsset(const HRZR::String& CorePath, const HRZR::Array<HRZR::Ref<HRZR::RTTIRefObject>>& Objects) override;

	static ModCoreEvents& GetInstance();
};
