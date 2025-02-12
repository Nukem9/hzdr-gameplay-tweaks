#pragma once

#include <filesystem>
#include "HRZR/PCore/UUID.h"

namespace InternalModConfig
{
	struct AssetOverride
	{
		bool Enabled = false;
		std::string ObjectUUIDs;
		std::string ObjectTypes;
		std::string Path;
		std::string Value;
	};

	struct GlobalSettings
	{
		GlobalSettings() = default;
		GlobalSettings(const GlobalSettings&) = delete;

		// [General]
		bool EnableDebugMenu;
		bool EnableAssetLogging;
		bool EnableAssetOverrides;
		float DebugMenuFontScale;

		bool SkipIntroLogo;
		bool SkipPSNAccountLinking;

		// [Gameplay]
		bool UnlockMapBoundaries;
		bool UnlockWorldMapMenu;
		bool UnlockMountRestrictions;
		bool UnlockEntitlementExtras;
		bool UnlockPhotomodeRestrictions;

		bool EnableAutoLoot;
		bool EnableFreePerks;
		bool EnableFreeCrafting;
		bool EnableGodMode;
		bool EnableInfiniteClipAmmo;

		bool IncreaseInventoryStacks;
		int InventoryStackMultiplier;

		bool DisableCameraMagnetism;
		bool DisableFocusMagnetism;

		bool ForceLeftAlignedCamera;

		// [Hotkeys]
		struct
		{
			int ToggleDebugUI;
			int TogglePauseGameLogic;
			int ToggleFreeCamera;
			int ToggleNoclip;
			int IncreaseTimescale;
			int DecreaseTimescale;
			int ToggleTimescale;
			int SpawnEntity;
			int QuickSave;
			int QuickLoad;
			int ToggleAIProcessing;
		} Hotkeys;

		// [AssetOverride]
		std::vector<AssetOverride> AssetOverrides;

		// [CharacterOverride]
		struct CharacterOverride
		{
			HRZR::GGUUID RootUUID;
			HRZR::GGUUID VariantUUID;
		};

		std::vector<CharacterOverride> CharacterOverrides;

		// [CoreObjectCache]
		struct ObjectCacheEntry
		{
			HRZR::GGUUID UUID;
			std::string CorePath;
			std::string Name;

			friend auto operator<=>(const ObjectCacheEntry& Lhs, const ObjectCacheEntry& Rhs)
			{
				return Lhs.UUID <=> Rhs.UUID;
			}

			friend auto operator<=>(const ObjectCacheEntry& Lhs, const HRZR::GGUUID& Rhs)
			{
				return Lhs.UUID <=> Rhs;
			}
		};

		std::vector<ObjectCacheEntry> CachedSpawnSetups;
		std::vector<ObjectCacheEntry> CachedWeatherSetups;
		std::vector<ObjectCacheEntry> CachedInventoryItems;
	};

	void Initialize(const std::filesystem::path& RootDirectory);
	bool InitializeConfigurationFile();
	std::filesystem::path GetModRelativePath(const std::string_view& PartialPath);
}

extern const InternalModConfig::GlobalSettings& ModConfiguration;
