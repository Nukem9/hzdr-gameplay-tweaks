#include <format>
#include <ranges>
#include <shared_mutex>
#include "HRZR/Core/IStreamingManager.h"
#include "HRZR/Core/RTTI.h"
#include "HRZR/Core/RTTIRefObject.h"
#include "ModConfiguration.h"
#include "ModCoreEvents.h"

ModCoreEvents::ValuePatchVisitor::ValuePatchVisitor(const RTTIValuePatch& Patch)
{
	m_ValueToSet = Patch.m_Value;
}

void ModCoreEvents::ValuePatchVisitor::Visit(void *Object, const HRZR::RTTI *Type)
{
	if (!Type->DeserializeObject(Object, m_ValueToSet))
		m_LastError = std::format("Failed to set value '{}'.", m_ValueToSet);
}

HRZR::RTTIObjectTweaker::SetValueVisitor::EMode ModCoreEvents::ValuePatchVisitor::GetMode()
{
	return EMode::MODE_WRITE;
}

ModCoreEvents::ModCoreEvents()
{
	RegisterParsedPatches();
	RegisterHardcodedPatches();
}

void ModCoreEvents::RegisterParsedPatches()
{
	auto splitStringByDelimiter = [](const std::string_view& Text, char Delimiter, auto&& Callback)
	{
		for (const auto& part : Text | std::views::split(Delimiter))
			Callback(std::string_view(part));
	};

	// Create all of the patch instances from mod configuration data
	for (const auto& entry : ModConfiguration.AssetOverrides)
	{
		if (!entry.Enabled)
			continue;

		if (entry.ObjectTypes.empty() && entry.ObjectUUIDs.empty())
			spdlog::error("ObjectTypes and ObjectUUIDs are both empty. Asset override will have no effect.");

		// Lookup by type
		splitStringByDelimiter(
			entry.ObjectTypes,
			',',
			[&](const std::string_view& Type)
			{
				auto rtti = HRZR::RTTI::FindTypeByName(Type);

				if (rtti)
					m_RTTIPatchesByType[rtti].emplace_back(RTTIValuePatch { entry.Path, entry.Value });
				else
					spdlog::error("Failed to resolve override type name '{}'. Skipping.", Type);
			});

		// Lookup by UUID
		splitStringByDelimiter(
			entry.ObjectUUIDs,
			',',
			[&](const std::string_view& UUID)
			{
				auto uuid = HRZR::GGUUID::TryParse(UUID);

				if (uuid)
					m_RTTIPatchesByUUID[uuid.value()].emplace_back(RTTIValuePatch { entry.Path, entry.Value });
				else
					spdlog::error("Failed to parse override UUID '{}'. Skipping.", UUID);
			});
	}

	// Then also handle character override entries
	for (const auto& entry : ModConfiguration.CharacterOverrides)
	{
		if (entry.RootUUID.empty() || entry.VariantUUID.empty())
		{
			spdlog::error("RootUUID or VariantUUID is empty. Character override will have no effect.");
			continue;
		}

		m_CharacterOverrideRootUUID = entry.RootUUID;
		m_CharacterOverrideVariantUUID = entry.VariantUUID;
	}

	if (m_CharacterOverrideVariantUUID)
		spdlog::info("Using character override variant UUID {}", m_CharacterOverrideVariantUUID.value());
}

void ModCoreEvents::RegisterHardcodedPatches()
{
	using namespace HRZR;

	auto addCallback = [this]<typename T>(T Value, void (*Callback)(Ref<RTTIRefObject>&))
	{
		if constexpr (std::is_same_v<T, GGUUID>)
			m_RTTIPatchesByUUID[Value].emplace_back(RTTICallbackPatch { Callback });
		else if constexpr (std::is_same_v<T, const RTTI *>)
			m_RTTIPatchesByType[Value].emplace_back(RTTICallbackPatch { Callback });
		else
			static_assert(!sizeof(T), "Invalid type");
	};

	auto addUnloadCallback = [this](const RTTI *Value, void (*Callback)(Ref<RTTIRefObject>&))
	{
		m_RTTIUnloadsByType[Value].emplace_back(Callback);
	};

	//
	// Enable auto-pickup for various machine resources and plants.
	//
	if (ModConfiguration.EnableAutoLoot)
	{
		const static auto knownResourceUUIDs = []()
		{
			std::unordered_set<GGUUID> s;

			// UseLocationResource
			s.emplace(GGUUID::Parse("ABCEDDF1-56FB-584C-964F-083489E4F906")); // templatehealthplant
			s.emplace(GGUUID::Parse("EE815E03-60AB-FD4B-A9A2-1926CEE94204")); // templateplant

			// EntityResource (items)
			s.emplace(GGUUID::Parse("76CC6B9A-160C-813D-B979-5A5E90CA6D6E")); // Blaze
			s.emplace(GGUUID::Parse("C1F7F218-D6E3-643E-B579-5D7FEB0E8E74")); // Chillwater
			s.emplace(GGUUID::Parse("FAD119E1-D20E-2E30-83E5-1E929E5C281E")); // Echo Shell
			s.emplace(GGUUID::Parse("28A34A9B-AA20-7331-8161-F34B20325D97")); // Metalburn
			s.emplace(GGUUID::Parse("814C7B08-2E30-EE32-9343-DD88033FF7BB")); // Metal Vessel
			s.emplace(GGUUID::Parse("0EDAD8F7-2421-1A36-81BE-39251B2BE0EC")); // Metal Shards
			s.emplace(GGUUID::Parse("7BD8FDA6-04D5-2F4A-AF19-5B18CD145E7D")); // Rock
			s.emplace(GGUUID::Parse("6B371311-8F65-5332-A64D-147880F5F1A4")); // Sparker
			s.emplace(GGUUID::Parse("1FFB70B4-A34C-793C-BE3B-9CB1B5CF4F17")); // Wire

			return s;
		}();

		addCallback(
			RTTI::FindTypeByName("PickUpComponentResource"),
			[](auto& Object)
			{
				if (auto& useLocationResource = Object->GetMemberRefUnsafe<Ref<RTTIRefObject>>("UseLocationResource"))
				{
					bool patchRequired = knownResourceUUIDs.contains(useLocationResource->m_UUID);

					if (!patchRequired)
					{
						if (auto& itemEntityResource = Object->GetMemberRefUnsafe<Ref<RTTIRefObject>>("Item"))
							patchRequired = knownResourceUUIDs.contains(itemEntityResource->m_UUID);
					}

					if (patchRequired)
					{
						if (useLocationResource->GetMemberRefUnsafe<bool>("IsUsableByPlayer"))
							useLocationResource->GetMemberRefUnsafe<bool>("AutoUsePlayer") = true;
					}
				}
			});
	}

	//
	// Increase inventory item stacks to 1,000,000 or multiply defaults by a user-defined value.
	//
	if (ModConfiguration.IncreaseInventoryStacks || ModConfiguration.InventoryStackMultiplier > 0)
	{
		addCallback(
			RTTI::FindTypeByName("StackableComponentResource"),
			[](auto& Object)
			{
				auto& stackLimit = Object->GetMemberRefUnsafe<int>("StackLimit");

				if (ModConfiguration.InventoryStackMultiplier > 0)
					stackLimit *= ModConfiguration.InventoryStackMultiplier;
				else
					stackLimit = std::max(stackLimit, 1000000);
			});

		addCallback(
			RTTI::FindTypeByName("UpgradableStackableComponentResource"),
			[](auto& Object)
			{
				auto& stackSizes = Object->GetMemberRefUnsafe<Array<int>>("UpgradedLimits");

				for (auto& limit : stackSizes)
				{
					if (ModConfiguration.InventoryStackMultiplier > 0)
						limit *= ModConfiguration.InventoryStackMultiplier;
					else
						limit = std::max(limit, 1000000);
				}
			});

		addCallback(
			RTTI::FindTypeByName("InventoryCapacityComponentResource"),
			[](auto& Object)
			{
				auto patchCapacityField = [&](const char *FieldName)
				{
					auto& limit = Object->GetMemberRefUnsafe<int>(FieldName);

					if (limit > 0)
					{
						const auto hardCap = std::max(limit, 500); // Increasing this causes menu lag and stutter

						if (ModConfiguration.InventoryStackMultiplier > 0)
							limit = std::min(limit * ModConfiguration.InventoryStackMultiplier, hardCap);
						else
							limit = hardCap;
					}
				};

				patchCapacityField("WeaponsCapacity");
				patchCapacityField("ToolsCapacity");
				patchCapacityField("ModificationsCapacity");
				patchCapacityField("OutfitsCapacity");
				patchCapacityField("ResourcesCapacity");
			});
	}

	//
	// Character overrides. Direct BodyVariant pointer replacement.
	//
	if (m_CharacterOverrideVariantUUID)
	{
		// TODO
	}

	//
	// HACK: Read the fake "InternalLODRangeModifier" asset override value and forward it to the debug menu. GameViewResource
	// isn't actually present in the game files. We can remove it entirely.
	//
	if (auto itr = m_RTTIPatchesByType.find(RTTI::FindTypeByName("GameViewResource")); itr != m_RTTIPatchesByType.end())
	{
		if (!itr->second.empty())
		{
			const auto& value = std::get<RTTIValuePatch>(itr->second.front()).m_Value;

			if (!value.empty())
				std::from_chars(value.data(), value.data() + value.size(), m_CachedLODRangeModifierHack);
		}

		m_RTTIPatchesByType.erase(itr);
	}

	//
	// Global object cache lists
	//
#define RegisterCacheCallback(RTTIName, MemberName)			\
	addCallback(											\
		RTTI::FindTypeByName(RTTIName),				        \
		[](auto& Object)									\
		{													\
			auto& e = GetInstance();						\
			std::scoped_lock lock(e.m_CachedDataMutex);		\
			e.MemberName.emplace(Object);					\
		});													\
	addUnloadCallback(										\
		RTTI::FindTypeByName(RTTIName),			            \
		[](auto& Object)									\
		{													\
			auto& e = GetInstance();						\
			std::scoped_lock lock(e.m_CachedDataMutex);		\
			e.MemberName.erase(Object);						\
		});

	RegisterCacheCallback("AIFaction", m_CachedAIFactions);

	RegisterCacheCallback("SpawnSetup", m_CachedSpawnSetups);
	RegisterCacheCallback("SpawnSetupGroup", m_CachedSpawnSetups);
	RegisterCacheCallback("SpawnSetupPlaceholder", m_CachedSpawnSetups);

	RegisterCacheCallback("WeatherSetup", m_CachedWeatherSetups);
	RegisterCacheCallback("LocalWeatherSetup", m_CachedWeatherSetups);

	// See ShouldHandleInventoryItem() for handling of cached inventory items

#undef RegisterCacheCallback
}

bool ModCoreEvents::ShouldHandleInventoryItem(const HRZR::RTTI *Type, const HRZR::GGUUID& UUID)
{
	const static auto entityResourceRTTI = HRZR::RTTI::FindTypeByName("EntityResource");

	if (Type->IsKindOf(entityResourceRTTI))
	{
		const static std::unordered_set<const HRZR::RTTI *> knownItemTypes = {
			HRZR::RTTI::FindTypeByName("InventoryEntityResource"),
			HRZR::RTTI::FindTypeByName("InventoryPlaceEntityAbilityResource"),
			HRZR::RTTI::FindTypeByName("InventoryReviveDroneAbilityResource"),
			HRZR::RTTI::FindTypeByName("InventoryWeaponResource"),
			HRZR::RTTI::FindTypeByName("InventoryActionAbilityResource"),
			HRZR::RTTI::FindTypeByName("InventoryContextualOrderAbilityResource"),
			HRZR::RTTI::FindTypeByName("InventoryAmmoEjectorResource"),
			HRZR::RTTI::FindTypeByName("InventoryNothingResource"),
			HRZR::RTTI::FindTypeByName("InventoryReviveAbilityResource"),
			HRZR::RTTI::FindTypeByName("InventoryThrowableResource"),
			HRZR::RTTI::FindTypeByName("InventoryGrenadeResource"),
			HRZR::RTTI::FindTypeByName("AmmoResource"),
			HRZR::RTTI::FindTypeByName("EntityProjectileAmmoResource"),
			HRZR::RTTI::FindTypeByName("RopeAmmoResource"),
		};

		if (knownItemTypes.contains(Type))
			return true;

		if (std::binary_search(ModConfiguration.CachedInventoryItems.begin(), ModConfiguration.CachedInventoryItems.end(), UUID))
			return true;
	}

	return false;
}

void ModCoreEvents::OnStartLoading(const HRZR::String& CorePath)
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnStartLoading({0:})", CorePath);
}

void ModCoreEvents::OnFinishLoading(const HRZR::String& CorePath)
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnFinishLoading({0:})", CorePath);
}

void ModCoreEvents::OnStartUnloading(const HRZR::String& CorePath)
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnStartUnloading({0:})", CorePath);
}

void ModCoreEvents::OnFinishUnloading()
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnFinishUnloading()");
}

void ModCoreEvents::OnReadFile(const HRZR::String& CorePath, const HRZR::Array<HRZR::Ref<HRZR::RTTIRefObject>>& Objects)
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnReadFile({0:}, {1:} objects)", CorePath, Objects.size());

	for (auto& object : Objects)
	{
		const auto rtti = object->GetRTTI();

		if (ModConfiguration.EnableAssetLogging)
			spdlog::debug("Asset {}: {}", object->m_UUID, rtti->GetSymbolName());

		// Apply all patches loaded from user config
		auto visitAll = [&](const auto& Entries)
		{
			auto v = [&object, &rtti](auto&& Patch)
			{
				if constexpr (std::is_same_v<std::decay_t<decltype(Patch)>, RTTIValuePatch>)
				{
					if constexpr (false)
						spdlog::debug("Patching '{}.{}' with value '{}'", rtti->GetSymbolName(), Patch.m_Path, Patch.m_Value);

					if (Patch.m_Path.starts_with('@'))
					{
						auto& src = object->GetMemberRefUnsafe<HRZR::Ref<HRZR::RTTIRefObject>>(Patch.m_Path.c_str() + 1);

						if (Patch.m_Value.empty())
							src = nullptr;
						else if (Patch.m_Value.starts_with('@'))
							src = object->GetMemberRefUnsafe<HRZR::Ref<HRZR::RTTIRefObject>>(Patch.m_Value.c_str() + 1);
						else
							spdlog::error("Error applying asset override: Invalid reference value '{}'", Patch.m_Value);
					}
					else
					{
						ValuePatchVisitor v(Patch);
						HRZR::RTTIObjectTweaker::VisitObjectPath(object, rtti, Patch.m_Path, &v);

						if (!v.m_LastError.empty())
							spdlog::error("Error applying asset override: {}", v.m_LastError);
					}
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(Patch)>, RTTICallbackPatch>)
					Patch.m_Callback(object);
				else
					static_assert(!sizeof(decltype(Patch)), "Invalid type");
			};

			for (auto& entry : Entries)
				std::visit(v, entry);
		};

		if (auto itr = m_RTTIPatchesByType.find(rtti); itr != m_RTTIPatchesByType.end())
			visitAll(itr->second);

		if (auto itr = m_RTTIPatchesByUUID.find(object->m_UUID); itr != m_RTTIPatchesByUUID.end())
			visitAll(itr->second);

		// Manually collect inventory items here. Certain RTTI types (e.g. EntityResource) are so numerous that callbacks incur
		// a load-time performance hit.
		if (ShouldHandleInventoryItem(rtti, object->m_UUID))
		{
			std::scoped_lock lock(m_CachedDataMutex);
			m_CachedInventoryItems.emplace(object);
		}
	}
}

void ModCoreEvents::OnLoadAsset(const HRZR::String& CorePath, const HRZR::Array<HRZR::Ref<HRZR::RTTIRefObject>>& Objects)
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnLoadAsset({0:}, {1:} objects)", CorePath, Objects.size());
}

void ModCoreEvents::OnUnloadAsset(const HRZR::String& CorePath, const HRZR::Array<HRZR::Ref<HRZR::RTTIRefObject>>& Objects)
{
	if (ModConfiguration.EnableAssetLogging)
		spdlog::info("OnUnloadAsset({0:}, {1:} objects)", CorePath, Objects.size());

	for (auto& object : Objects)
	{
		const auto rtti = object->GetRTTI();

		if (ShouldHandleInventoryItem(rtti, object->m_UUID))
		{
			std::scoped_lock lock(m_CachedDataMutex);
			m_CachedInventoryItems.erase(object);
		}

		if (auto itr = m_RTTIUnloadsByType.find(rtti); itr != m_RTTIUnloadsByType.end())
		{
			for (const auto& entry : itr->second)
				entry.m_Callback(object);
		}
	}
}

ModCoreEvents& ModCoreEvents::GetInstance()
{
	// Yes, I'm intentionally leaking memory. There's no virtual destructor present and this
	// never gets unregistered properly.
	static auto handler = new ModCoreEvents();
	return *handler;
}
