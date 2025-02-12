#include <spdlog/sinks/basic_file_sink.h>
#include <toml++/toml.h>
#include "HRZR/DebugUI/LogWindow.h"
#include "ModConfiguration.h"

namespace InternalModConfig
{
	InternalModConfig::GlobalSettings& GetModifiableSettings()
	{
		static GlobalSettings configuration;
		return configuration;
	}

	std::filesystem::path& GetModifiableRootDirectory()
	{
		static std::filesystem::path rootDirectory;
		return rootDirectory;
	}
}

const InternalModConfig::GlobalSettings& ModConfiguration = InternalModConfig::GetModifiableSettings();

namespace InternalModConfig
{
	void ParseSettings(GlobalSettings& s, const toml::table& Table);
	void PostProcessSettings(GlobalSettings& s);

	void Initialize(const std::filesystem::path& RootDirectory)
	{
		GetModifiableRootDirectory() = RootDirectory;

		if (!InitializeConfigurationFile())
			throw std::runtime_error("Failed to load configuration file.");

		// Main logger (debug UI and log sink)
		if (auto logLevel = spdlog::level::debug; logLevel != spdlog::level::off)
		{
			std::vector<spdlog::sink_ptr> sinks;

			auto debugUISink = std::make_shared<HRZR::DebugUI::LogWindow::LogSink>();
			sinks.emplace_back(std::move(debugUISink));

			if constexpr (false)
			{
				auto logFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(GetModRelativePath("mod_log.txt").string(), true);
				sinks.emplace_back(std::move(logFileSink));
			}

			auto logger = std::make_shared<spdlog::logger>("main_logger", sinks.begin(), sinks.end());
			logger->set_level(logLevel);
			logger->flush_on(spdlog::level::debug);
			logger->set_pattern("[%H:%M:%S] [%l] %v");

			spdlog::set_default_logger(std::move(logger));
		}

		if (!Offsets::Initialize())
			throw std::runtime_error("Failed to initialize offsets. The game likely updated and is no longer supported.");

		if (!Hooks::Initialize())
			throw std::runtime_error("Failed to initialize hooks.");
	}

	bool InitializeConfigurationFile()
	{
		auto tryParseTomlFile = [](GlobalSettings& Settings, const std::filesystem::path& Path)
		{
			toml::table table;

			try
			{
				table = toml::parse_file(Path.string());
			}
			catch (const toml::parse_error&)
			{
				return false;
			}

			ParseSettings(Settings, table);
			return true;
		};

		// mod_config.ini takes precedence
		if (!tryParseTomlFile(GetModifiableSettings(), GetModRelativePath("mod_config.ini")))
			return false;

		// Then check for mod_*.ini files in the game directory
		for (const auto& entry : std::filesystem::directory_iterator(GetModRelativePath("")))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".ini")
				continue;

			if (!entry.path().filename().string().starts_with("mod_") || entry.path().filename().string() == "mod_config.ini")
				continue;

			if (!tryParseTomlFile(GetModifiableSettings(), entry.path()))
				return false;
		}

		PostProcessSettings(GetModifiableSettings());
		return true;
	}

	std::filesystem::path GetModRelativePath(const std::string_view& PartialPath)
	{
		auto path = GetModifiableRootDirectory();
		path.append(PartialPath);

		return path;
	}

	void ParseSettings(GlobalSettings& o, const toml::table& Table)
	{
#define PARSE_TOML_MEMBER(obj, x) o.x = (*obj)[#x].value_or(decltype(o.x) {})
#define PARSE_TOML_HOTKEY(obj, x) o.Hotkeys.x = (*obj)[#x].value_or(-1)

		// [General]
		if (auto general = Table["General"].as_table())
		{
			PARSE_TOML_MEMBER(general, EnableDebugMenu);
			PARSE_TOML_MEMBER(general, EnableAssetLogging);
			PARSE_TOML_MEMBER(general, EnableAssetOverrides);
			PARSE_TOML_MEMBER(general, DebugMenuFontScale);

			PARSE_TOML_MEMBER(general, SkipIntroLogo);
			PARSE_TOML_MEMBER(general, SkipPSNAccountLinking);
		}

		if (o.DebugMenuFontScale <= 0.0f)
			o.DebugMenuFontScale = 1.0f;

		// [Gameplay]
		if (auto gameplay = Table["Gameplay"].as_table())
		{
			PARSE_TOML_MEMBER(gameplay, UnlockMapBoundaries);
			PARSE_TOML_MEMBER(gameplay, UnlockWorldMapMenu);
			PARSE_TOML_MEMBER(gameplay, UnlockMountRestrictions);
			PARSE_TOML_MEMBER(gameplay, UnlockEntitlementExtras);
			PARSE_TOML_MEMBER(gameplay, UnlockPhotomodeRestrictions);

			PARSE_TOML_MEMBER(gameplay, EnableFreePerks);
			PARSE_TOML_MEMBER(gameplay, EnableFreeCrafting);
			PARSE_TOML_MEMBER(gameplay, EnableGodMode);
			PARSE_TOML_MEMBER(gameplay, EnableInfiniteClipAmmo);
			PARSE_TOML_MEMBER(gameplay, EnableAutoLoot);

			PARSE_TOML_MEMBER(gameplay, IncreaseInventoryStacks);
			PARSE_TOML_MEMBER(gameplay, InventoryStackMultiplier);

			PARSE_TOML_MEMBER(gameplay, DisableCameraMagnetism);
			PARSE_TOML_MEMBER(gameplay, DisableFocusMagnetism);

			PARSE_TOML_MEMBER(gameplay, ForceLeftAlignedCamera);
		}

		// [Hotkeys]
		if (auto hotkeys = Table["Hotkeys"].as_table())
		{
			PARSE_TOML_HOTKEY(hotkeys, ToggleDebugUI);
			PARSE_TOML_HOTKEY(hotkeys, TogglePauseGameLogic);
			PARSE_TOML_HOTKEY(hotkeys, ToggleFreeCamera);
			PARSE_TOML_HOTKEY(hotkeys, ToggleNoclip);
			PARSE_TOML_HOTKEY(hotkeys, IncreaseTimescale);
			PARSE_TOML_HOTKEY(hotkeys, DecreaseTimescale);
			PARSE_TOML_HOTKEY(hotkeys, ToggleTimescale);
			PARSE_TOML_HOTKEY(hotkeys, SpawnEntity);
			PARSE_TOML_HOTKEY(hotkeys, QuickSave);
			PARSE_TOML_HOTKEY(hotkeys, QuickLoad);
			PARSE_TOML_HOTKEY(hotkeys, ToggleAIProcessing);
		}

		// [AssetOverride]
		if (auto overrideArray = Table["AssetOverride"].as_array())
		{
			auto parseAssetOverride = [](const toml::table *Table, bool Enable)
			{
				AssetOverride o;

				o.Enabled = Enable;
				PARSE_TOML_MEMBER(Table, ObjectUUIDs);
				PARSE_TOML_MEMBER(Table, ObjectTypes);
				PARSE_TOML_MEMBER(Table, Path);
				PARSE_TOML_MEMBER(Table, Value);

				return o;
			};

			overrideArray->for_each(
				[&](const toml::table& t)
				{
					const bool enabled = t["Enable"].value_or(false);

					if (auto overridePatchArray = t["Patch"].as_array())
					{
						for (const auto& patchEntry : *overridePatchArray)
							o.AssetOverrides.emplace_back(parseAssetOverride(patchEntry.as_table(), enabled));
					}
				});
		}

		// [CharacterOverride]
		if (auto overrideArray = Table["CharacterOverride"].as_array())
		{
			overrideArray->for_each(
				[&](const toml::table& t)
				{
					o.CharacterOverrides.emplace_back(
						HRZR::GGUUID::Parse(t["RootUUID"].value_or("")),
						HRZR::GGUUID::Parse(t["VariantUUID"].value_or("")));
				});
		}

		// [CoreObjectCache]
		auto parseCoreObjectCacheTable = [&Table](const std::string_view& Name, auto& TargetVector)
		{
			if (auto cachedObjectArray = Table["CoreObjectCache"][Name].as_array())
			{
				TargetVector.clear();
				TargetVector.reserve(cachedObjectArray->size());

				cachedObjectArray->for_each(
					[&](const toml::array& a)
					{
						TargetVector.emplace_back(
							a.size() > 0 ? HRZR::GGUUID::Parse(a[0].value_or("")) : HRZR::GGUUID {},
							a.size() > 1 ? a[1].value_or("") : "",
							a.size() > 2 ? a[2].value_or("") : "");
					});

				std::stable_sort(TargetVector.begin(), TargetVector.end());
			}
		};

		parseCoreObjectCacheTable("CachedSpawnSetups", o.CachedSpawnSetups);
		parseCoreObjectCacheTable("CachedWeatherSetups", o.CachedWeatherSetups);
		parseCoreObjectCacheTable("CachedInventoryItems", o.CachedInventoryItems);

#undef PARSE_TOML_HOTKEY
#undef PARSE_TOML_MEMBER
	}

	void PostProcessSettings(GlobalSettings& s)
	{
		// clang-format off
		if (!s.EnableAssetOverrides)
			s.AssetOverrides.clear();

		// SkipIntroLogo
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.SkipIntroLogo,
			.ObjectTypes = "InGameMenuResource",
			.Path = "@StartupIntro",
			.Value = "@StartPage",
		});

		// UnlockMapBoundaries
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockMapBoundaries,
			.ObjectUUIDs = "40DA43CC-1953-6E4D-8D5C-017DB5353968", // GraphProgramResource in levels/worlds/world/features/worldborder/scenes/worldborder_colliders_script
			.Path = "EntryPointsData[2].EntryPoint",
			.Value = "",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockMapBoundaries,
			.ObjectTypes = "OutOfBoundsQueryComponentResource",
			.Path = "OutOfBoundsAreaTags",
			.Value = "",
		});

		// UnlockWorldMapMenu
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockWorldMapMenu,
			.ObjectTypes = "DiscoverableArea",
			.Path = "InitialState",
			.Value = "Completed",
		});

		// UnlockMountRestrictions
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockMountRestrictions,
			.ObjectTypes = "HorseCallComponentResource,HorseControllerComponentResource",
			.Path = "HorseNotAllowedNavMeshAreaTags",
			.Value = "",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockMountRestrictions,
			.ObjectTypes = "HorseCallComponentResource",
			.Path = "DisallowedFacts",
			.Value = "",
		});

		// UnlockEntitlementExtras
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockEntitlementExtras,
			.ObjectTypes = "UnlockableFacePaint,UnlockableFocusModel",
			.Path = "NewGamePlusCompletedDifficulty",
			.Value = "None",
		});

		// UnlockPhotomodeRestrictions
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockPhotomodeRestrictions,
			.ObjectUUIDs = "9D995414-CEF7-6830-AAFA-87CCF64E0448", // GraphProgramResource in default_assets/photomode/photomode
			.Path = "EntryPointsData[0].EntryPoint",
			.Value = "EntryPoint_GraphResource_217800417_0_EvaluateLogic",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockPhotomodeRestrictions,
			.ObjectUUIDs = "1C710F84-C54D-9643-8353-379EC8927545", // PhotoModeResource in default_assets/photomode/photomode
			.Path = "PanXRange",
			.Value = "[-10000, 10000]",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.UnlockPhotomodeRestrictions,
			.ObjectUUIDs = "1C710F84-C54D-9643-8353-379EC8927545", // PhotoModeResource in default_assets/photomode/photomode
			.Path = "PanZRange",
			.Value = "[-10000, 10000]",
		});

		// EnableFreePerks
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.EnableFreePerks,
			.ObjectTypes = "PerkLevel",
			.Path = "Cost",
			.Value = "0",
		});

		// EnableFreeCrafting
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.EnableFreeCrafting,
			.ObjectTypes = "CraftingRecipe,CharacterUpgradeRecipe,ItemRecipe,UpgradeRecipe,AmmoRecipe",
			.Path = "Ingredients",
			.Value = "",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.EnableFreeCrafting,
			.ObjectTypes = "InventoryCollectionMerchantTradingItem",
			.Path = "TradingItems",
			.Value = "",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.EnableFreeCrafting,
			.ObjectTypes = "InventoryCollectionMerchantTradingItem",
			.Path = "TradeCostMultipliers",
			.Value = "",
		});

		// DisableCameraMagnetism
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.DisableCameraMagnetism,
			.ObjectTypes = "CameraOrbitFollowResource",
			.Path = "IsAiming.Override",
			.Value = "true",
		});

		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.DisableCameraMagnetism,
			.ObjectTypes = "CameraOrbitFollowResource",
			.Path = "IsAiming.Val",
			.Value = "true",
		});

		// DisableFocusMagnetism (FOCUS_* CameraMagnetParmResource in entities/cameras/cameramagnetmanager)
		s.AssetOverrides.emplace_back(AssetOverride
		{
			.Enabled = s.DisableFocusMagnetism,
			.ObjectUUIDs = "{21C36C16-CE92-F049-9388-58102D6BA86B},{49A8B7AA-7687-3D48-82FA-596294B25685},{A44C00B3-2534-5B40-91FB-CFD89E72DFE3}",
			.Path = "Strength.Val",
			.Value = "0",
		});
		// clang-format on
	}

#undef XORS
}
