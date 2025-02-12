#include <algorithm>
#include <imgui.h>
#include <Windows.h>
#include "../../ModConfiguration.h"
#include "../../ModCoreEvents.h"
#include "../Core/DebugSettings.h"
#include "../Core/Destructibility.h"
#include "../Core/GameModule.h"
#include "../Core/JobHeaderCPU.h"
#include "../Core/Mover.h"
#include "../Core/PlayerGame.h"
#include "../Core/RTTIRefObject.h"
#include "../Core/RTTIYamlExporter.h"
#include "../Core/WorldState.h"
#include "../RTTIScanner.h"
#include "DebugUI.h"
#include "DemoWindow.h"
#include "EntitySpawnerWindow.h"
#include "LogWindow.h"
#include "MainMenuBar.h"
#include "PlayerInventoryWindow.h"
#include "WeatherSetupWindow.h"

namespace HRZR::DebugUI
{
	MainMenuBar::MainMenuBar()
	{
		m_EnableGodMode = ModConfiguration.EnableGodMode;
		m_EnableInfiniteClipAmmo = ModConfiguration.EnableInfiniteClipAmmo;
		m_LODRangeModifier = ModCoreEvents::GetInstance().m_CachedLODRangeModifierHack;
	}

	void MainMenuBar::Render()
	{
		if (!m_IsVisible || !ImGui::BeginMainMenuBar())
			return;

		// Empty space for MSI afterburner display
		ImGui::BeginMenu("                        ", false);

		// "Gameplay" menu
		if (ImGui::BeginMenu("Gameplay"))
		{
			DrawGameplayMenu();
			ImGui::EndMenu();
		}

		// "Cheats" menu
		if (ImGui::BeginMenu("Cheats", Player::GetLocalPlayer() != nullptr))
		{
			DrawCheatsMenu();
			ImGui::EndMenu();
		}

		// "Miscellaneous" menu
		if (ImGui::BeginMenu("Miscellaneous"))
		{
			DrawMiscellaneousMenu();
			ImGui::EndMenu();
		}

		// Credits
		XorStr encryptedCreditsBuf("Game keyboard input blocked | HZDR Gameplay Tweaks & Cheat Menu by Nukem\0");
		const auto creditsBuf = encryptedCreditsBuf.Decrypt();

		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(creditsBuf.data()).x);
		ImGui::BeginMenu(creditsBuf.data(), false);

		ImGui::EndMainMenuBar();
	}

	bool MainMenuBar::Close()
	{
		return false;
	}

	std::string MainMenuBar::GetId() const
	{
		return "Main Menu Bar";
	}

	void MainMenuBar::DrawGameplayMenu()
	{
		auto gameModule = GameModule::GetInstance();

		if (ImGui::MenuItem("Pause Game Logic", nullptr, gameModule && gameModule->IsPaused()))
			TogglePauseGameLogic();

		if (ImGui::MenuItem("Pause AI Processing", nullptr, m_PauseAIProcessing))
			TogglePauseAIProcessing();

		ImGui::MenuItem("##sep1", nullptr, nullptr, false);

		if (ImGui::MenuItem("Force Quick Save"))
			ToggleQuickSave();

		if (ImGui::MenuItem("Force Load Previous Save"))
			ToggleQuickLoad();

		// Day/night cycle
		if (gameModule)
		{
			if (auto& worldTimeState = gameModule->m_WorldState)
			{
				float timeOfDay = worldTimeState->m_CurrentWorldTime.m_TimeOfDay;

				ImGui::MenuItem("##sep2", nullptr, nullptr, false);
				ImGui::MenuItem("Pause Time of Day", nullptr, &worldTimeState->m_PausedDayNightCycle);

				if (ImGui::MenuItem("Pause Day/Night Cycle", nullptr, !worldTimeState->m_EnableDayNightCycle))
					worldTimeState->m_EnableDayNightCycle = !worldTimeState->m_EnableDayNightCycle;

				ImGui::MenuItem("Time of Day", nullptr, nullptr, false);

				if (ImGui::SliderFloat("##timeofdaybar", &timeOfDay, 0.0f, 23.9999f))
					worldTimeState->SetTimeOfDay(timeOfDay, 0.0f);
			}
		}

		// Timescale
		ImGui::MenuItem("##sep3", nullptr, nullptr, false);
		ImGui::MenuItem("Enable Timescale Override in Menus", nullptr, &m_TimescaleOverrideInMenus);
		if (ImGui::MenuItem("Enable Timescale Override", nullptr, m_TimescaleOverride))
			ToggleTimescaleOverride();
		ImGui::MenuItem("Timescale", nullptr, nullptr, false);

		auto modifyTimescale = [](float Scale, bool SameLine = true)
		{
			char temp[64];
			sprintf_s(temp, "%g##setTs%g", Scale, Scale);

			if (ImGui::Button(temp))
				AdjustTimescale(Scale - m_Timescale);

			if (SameLine)
				ImGui::SameLine();
		};

		if (float f = m_Timescale; ImGui::SliderFloat("##TimescaleDragFloat", &f, 0.001f, 10.0f))
			AdjustTimescale(f - m_Timescale);

		modifyTimescale(0.01f);
		modifyTimescale(0.25f);
		modifyTimescale(0.5f);
		modifyTimescale(1.0f);
		modifyTimescale(2.0f);
		modifyTimescale(5.0f);
		modifyTimescale(10.0f, false);
	}

	void MainMenuBar::DrawCheatsMenu()
	{
		auto debugSettings = DebugSettings::GetInstance();
		auto player = Player::GetLocalPlayer();

		if (!player || !player->m_Entity)
			return;

		if (ImGui::MenuItem("Enable Noclip", nullptr, m_FreeCamMode == FreeCamMode::Noclip))
			ToggleNoclip();

		if (ImGui::MenuItem("Enable Free Camera", nullptr, m_FreeCamMode == FreeCamMode::Free))
			ToggleFreeflyCamera();

		if (auto destructibility = player->m_Entity->m_Destructibility)
		{
			if (ImGui::MenuItem("Enable God Mode", nullptr, &m_EnableGodMode))
			{
				m_EnableDemigodMode = false;
				destructibility->m_Invulnerable = m_EnableGodMode;
				destructibility->m_DieAtZeroHealth = true;
			}

			if (ImGui::MenuItem("Enable Demigod Mode", nullptr, &m_EnableDemigodMode))
			{
				m_EnableGodMode = false;
				destructibility->m_Invulnerable = false;
				destructibility->m_DieAtZeroHealth = !m_EnableDemigodMode;
			}
		}

		if (ImGui::MenuItem("Enable Infinite Ammo", nullptr, &debugSettings->m_InfiniteAmmo))
		{
			debugSettings->m_InfiniteSizeClip = false;
			m_EnableInfiniteClipAmmo = false;
		}

		if (ImGui::MenuItem("Enable Infinite Ammo (Clip)", nullptr, &m_EnableInfiniteClipAmmo))
		{
			debugSettings->m_InfiniteAmmo = false;
			debugSettings->m_InfiniteSizeClip = m_EnableInfiniteClipAmmo;
		}

		ImGui::MenuItem("Enable Infinite Stamina", nullptr, &debugSettings->m_Inexhaustible);
		ImGui::MenuItem("##sep1", nullptr, nullptr, false);

		if (ImGui::BeginMenu("Teleport To..."))
		{
			auto doTeleport = [&](const char *Name, WorldPosition Position)
			{
				if (!ImGui::MenuItem(std::format("{} ({:.1f}, {:.1f}, {:.1f})", Name, Position.X, Position.Y, Position.Z).c_str()))
					return;

				// Fixup so Aloy doesn't fall through the ground
				Position.Z += 0.5;

				if (m_FreeCamMode == FreeCamMode::Noclip)
				{
					m_FreeCamPosition.Position = Position;
				}
				else
				{
					JobHeaderCPU::SubmitCallable(
						[Position]()
						{
							if (auto player = Player::GetLocalPlayer())
							{
								auto worldTransform = player->m_Entity->m_WorldTransform;
								worldTransform.Position = Position;

								player->m_Entity->m_Mover->OverrideMovement(worldTransform, 0.0001f, false);
							}
						});
				}
			};

			doTeleport("Free Camera Position", m_FreeCamPosition.Position);
			ImGui::MenuItem("##septeleport1", nullptr, nullptr, false);

			const static std::vector<std::pair<const char *, WorldPosition>> areaLocations {
				{ "Naming Cliff", { 2258.91, -1097.40, 359.18 } },
				{ "Elizabet Sobeck's Ranch", { 5349.0, -2322.0, 120.0 } },
				{ "Climbing Testing Area 1", { -2278.0, -2222.0, 219.0 } },
				{ "Climbing Testing Area 2", { -2265.0, -2307.0, 224.0 } },
				{ "Terrain Slope Testing Area", { -2277.0, -2541.0, 324.0 } },
				{ "Script Testing Area", { -2523.0, -2220.0, 221.0 } },
				{ "DLC Testing Area", { 4765.22, 4832.68, 282.68 } },
			};

			for (const auto& [name, position] : areaLocations)
				doTeleport(name, position);

			const static std::vector<std::pair<const char *, WorldPosition>> unlockableLocations {
				{ "GrazerDummy_01_RostsHovel", { 2166.24, -1521.54, 286.74 } },
				{ "GrazerDummy_02_RostsHovel", { 2163.07, -1525.95, 287.56 } },
				{ "GrazerDummy_03_RostsHovel", { 2158.27, -1526.80, 287.61 } },
				{ "GrazerDummy_04_RostsHovel", { 2148.92, -1509.10, 293.15 } },
				{ "GrazerDummy_05_RostsHovel", { 2141.70, -1496.57, 290.79 } },
				{ "GrazerDummy_06_RostsHovel", { 2166.80, -1496.77, 284.57 } },
				{ "GrazerDummy_07_RostsHovel", { 2163.33, -1496.83, 284.35 } },
				{ "GrazerDummy_08_KarstsShop", { 2829.59, -1956.60, 192.76 } },
				{ "GrazerDummy_09_MothersCradle", { 2731.09, -1914.96, 178.85 } },
				{ "GrazerDummy_10_MothersHeart", { 2518.01, -1359.98, 217.37 } },
				{ "GrazerDummy_11_MothersHeart", { 2489.16, -1361.32, 217.56 } },
				{ "GrazerDummy_12_MothersRise", { 2665.99, -1315.13, 209.17 } },
				{ "GrazerDummy_13_MothersRise", { 2671.79, -1372.27, 209.78 } },
				{ "GrazerDummy_14_MothersGate", { 2401.44, -1868.80, 188.97 } },
				{ "GrazerDummy_15_MothersGate", { 2364.84, -1868.52, 190.54 } },
				{ "GrazerDummy_16_MothersCrown", { 2684.90, -726.55, 180.38 } },
				{ "GrazerDummy_17_MothersCrown", { 2685.16, -719.75, 180.12 } },
				{ "GrazerDummy_18_BanditCamp", { 3076.84, -956.83, 170.23 } },
				{ "GrazerDummy_19_BanditCamp", { 1846.26, -540.75, 305.31 } },
				{ "GrazerDummy_20_HuntersGathering", { 2300.65, -457.92, 226.96 } },
				{ "GrazerDummy_21_HuntersGathering", { 2286.95, -462.21, 227.73 } },
				{ "GrazerDummy_22_HuntingGrounds1", { 2986.81, -1562.17, 194.28 } },
				{ "GrazerDummy_23_HuntingGrounds1", { 2965.68, -1574.96, 195.57 } },
				{ "Vantage_01_Airforce", { 2963.06, -1821.89, 209.79 } },
				{ "Vantage_02_ColoradoBuilding", { 3043.23, -1162.05, 216.93 } },
				{ "Vantage_03_PioneerMuseum", { 2998.87, -854.69, 170.04 } },
				{ "Vantage_04_DenverSkyline", { 3352.45, -573.77, 198.24 } },
				{ "Vantage_05_BridalVeilFalls", { 1775.53, -694.61, 254.63 } },
				{ "Vantage_06_MesaCity", { -176.51, -677.21, 230.51 } },
				{ "Vantage_07_Drone", { -572.33, -1969.96, 123.52 } },
				{ "Vantage_08_Citadel", { -929.59, 545.55, 301.81 } },
				{ "Vantage_09_FaroBuilding", { -424.89, 985.27, 290.85 } },
				{ "Vantage_10_LakePowell", { -610.22, -305.05, 255.51 } },
				{ "Vantage_11_ExplodedMountain", { 1328.36, 1333.82, 460.66 } },
				{ "Vantage_12_RedrockTheater", { 3045.02, -153.56, 185.03 } },
				{ "Vantage_13_DenverStadium", { 3877.23, -26.01, 164.08 } },
			};

			for (const auto& [name, position] : unlockableLocations)
				doTeleport(name, position);

			ImGui::EndMenu();
		}

		// Faction
		if (ImGui::BeginMenu("Player Faction..."))
		{
			auto& modEvents = ModCoreEvents::GetInstance();

			std::shared_lock lock(modEvents.m_CachedDataMutex);
			std::vector sortedFactions(modEvents.m_CachedAIFactions.begin(), modEvents.m_CachedAIFactions.end());

			std::ranges::sort(
				sortedFactions,
				[](auto A, auto B)
				{
					return A->GetMemberRefUnsafe<String>("Name") < B->GetMemberRefUnsafe<String>("Name");
				});

			for (auto faction : sortedFactions)
			{
				if (ImGui::MenuItem(
						faction->GetMemberRefUnsafe<String>("Name").c_str(),
						nullptr,
						reinterpret_cast<RTTIRefObject *>(player->m_Entity->m_Faction) == faction))
					player->m_Entity->SetFaction(reinterpret_cast<AIFaction *>(faction));
			}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Player Inventory..."))
			AddWindow(std::make_shared<PlayerInventoryWindow>());

		if (ImGui::MenuItem("Entity Spawner..."))
			AddWindow(std::make_shared<EntitySpawnerWindow>());

		if (ImGui::MenuItem("Weather Setup..."))
			AddWindow(std::make_shared<WeatherSetupWindow>());

		ImGui::MenuItem("##sep2", nullptr, nullptr, false);
		ImGui::MenuItem("Simulate Game Completed", nullptr, &debugSettings->m_SPAllUnlocked);
		ImGui::MenuItem("Apply Photomode Settings Ingame", nullptr, &debugSettings->m_ApplyPhotoModeSettingsInGame);
	}

	void MainMenuBar::DrawMiscellaneousMenu()
	{
		if (ImGui::MenuItem("Show Log Window"))
			AddWindow(std::make_shared<LogWindow>());

		if (ImGui::MenuItem("Show ImGui Demo Window"))
			AddWindow(std::make_shared<DemoWindow>());

		if (ImGui::MenuItem("Dump RTTI Structures", nullptr, false, !RTTIScanner::GetAllTypes().empty()))
		{
			RTTIYamlExporter exporter(RTTIScanner::GetAllTypes());
			exporter.ExportRTTITypes(".");
		}

#if 0
		if (ImGui::MenuItem("Dump Player Components"))
		{
		}
#endif

		ImGui::MenuItem("##blankseparator0", nullptr, nullptr, false);

		// LOD bias
		if (ImGui::MenuItem("Enable LOD Bias Override", nullptr, m_LODRangeModifier != std::numeric_limits<float>::max()))
			m_LODRangeModifier = m_LODRangeModifier == std::numeric_limits<float>::max() ? 1.0f : std::numeric_limits<float>::max();
		ImGui::MenuItem("Bias", nullptr, nullptr, false);
		if (float t = m_LODRangeModifier == std::numeric_limits<float>::max() ? 1.0f : m_LODRangeModifier;
			ImGui::SliderFloat("##LODDragFloat", &t, 0.0f, 1.0f))
			m_LODRangeModifier = t;

		ImGui::MenuItem("##blankseparator1", nullptr, nullptr, false);
		ImGui::MenuItem("##blankseparator2", nullptr, nullptr, false);

		if (ImGui::MenuItem("Terminate Process"))
			TerminateProcess(GetCurrentProcess(), 0);
	}

	void MainMenuBar::ToggleVisibility()
	{
		m_IsVisible = !m_IsVisible;
	}

	void MainMenuBar::TogglePauseGameLogic()
	{
		if (auto gameModule = GameModule::GetInstance())
		{
			if (gameModule->IsPaused())
				gameModule->Continue();
			else
				gameModule->Pause(true);
		}
	}

	void MainMenuBar::TogglePauseAIProcessing()
	{
		m_PauseAIProcessing = !m_PauseAIProcessing;
	}

	void MainMenuBar::ToggleQuickSave()
	{
		JobHeaderCPU::SubmitCallback(
			[]()
			{
				// RestartOnSpawned determines whether the player is moved to their last position or moved to a campfire
				auto playerGame = static_cast<PlayerGame *>(Player::GetLocalPlayer());

				if (!playerGame)
					return;

				const auto func = Offsets::Signature("48 83 EC 48 44 0F B6 D1 48 8B 0D ? ? ? ? 48 85 C9 74 3B")
									  .ToPointer<void(uint8_t, bool, class AIMarker *)>();

				playerGame->m_RestartOnSpawned = true;
				func(2, false, nullptr);
			});
	}

	void MainMenuBar::ToggleQuickLoad()
	{
		JobHeaderCPU::SubmitCallback(
			[]()
			{
				if (!Player::GetLocalPlayer())
					return;

				const auto func = Offsets::Signature("40 53 48 83 EC 40 65 48 8B 04 25 58 00 00 00 BA 00 1A 00 00 48 89 7C 24 50")
									  .ToPointer<void(float)>();

				func(0.0f);
			});
	}

	void MainMenuBar::ToggleTimescaleOverride()
	{
		m_TimescaleOverride = !m_TimescaleOverride;
	}

	void MainMenuBar::AdjustTimescale(float Adjustment)
	{
		m_Timescale = std::max(m_Timescale + Adjustment, 0.001f);
		m_TimescaleOverride = true;
	}

	void MainMenuBar::ToggleFreeflyCamera()
	{
		auto player = Player::GetLocalPlayer();
		auto camera = player ? player->GetLastActivatedCamera() : nullptr;

		if (!camera)
			return;

		m_FreeCamMode = (m_FreeCamMode == FreeCamMode::Free) ? FreeCamMode::Off : FreeCamMode::Free;

		if (m_FreeCamMode == FreeCamMode::Free)
			m_FreeCamPosition = camera->GetWorldTransform();
	}

	void MainMenuBar::ToggleNoclip()
	{
		auto player = Player::GetLocalPlayer();
		auto entity = player ? player->m_Entity : nullptr;

		if (!entity)
			return;

		m_FreeCamMode = (m_FreeCamMode == FreeCamMode::Noclip) ? FreeCamMode::Off : FreeCamMode::Noclip;
		m_FreeCamPosition = entity->GetWorldTransform();
	}
}
