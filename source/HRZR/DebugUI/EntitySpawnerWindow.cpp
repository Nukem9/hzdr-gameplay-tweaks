#include <algorithm>
#include <format>
#include "../../ModConfiguration.h"
#include "../../ModCoreEvents.h"
#include "../Core/Entity.h"
#include "../Core/JobHeaderCPU.h"
#include "../Core/Player.h"
#include "../PCore/UUID.h"
#include "EntitySpawnerWindow.h"

namespace HRZR::DebugUI
{
	void EntitySpawnerLoaderCallback::OnLoaded(RTTIRefObject *Object, void *Userdata)
	{
		if constexpr (false)
			spdlog::info("Received entity spawner callback with root UUID {}", Object->m_UUID);
	}

	void EntitySpawnerLoaderCallback::OnUnloaded(RTTIRefObject *Object, void *Userdata)
	{
	}

	void EntitySpawnerWindow::Render()
	{
		ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin(GetId().c_str(), &m_WindowOpen))
		{
			ImGui::End();
			return;
		}

		// Draw entity list
		m_SpawnerNameFilter.Draw();

		if (ImGui::BeginListBox("##SpawnSetupSelector", ImVec2(-FLT_MIN, -200)))
		{
			for (size_t i = 0; i < ModConfiguration.CachedSpawnSetups.size(); i++)
			{
				const auto& spawnSetup = ModConfiguration.CachedSpawnSetups[i];

				char fullName[256] = {};
				std::format_to_n(fullName, std::size(fullName) - 1, "{}, {}", spawnSetup.UUID, spawnSetup.Name);

				if (m_SpawnerNameFilter.PassFilter(fullName))
				{
					const bool isSelected = m_LastSelectedSetupIndex == i;

					if (ImGui::Selectable(fullName, isSelected, ImGuiSelectableFlags_AllowDoubleClick))
					{
						m_LastSelectedSetupIndex = i;

						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							ForceSpawnEntityClick();
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}

		// Draw settings
		static int spawnCount = 1;
		static int spawnLocationType = 0;
		static WorldPosition customSpawnPosition;
		static RTTIRefObject *customFaction = nullptr;

		const bool allowSpawn = m_LastSelectedSetupIndex < ModConfiguration.CachedSpawnSetups.size() && m_OutstandingSpawnCount == 0;

		ImGui::Separator();
		ImGui::BeginDisabled(!allowSpawn);
		ImGui::PushItemWidth(300);
		ImGui::InputInt("##entitycount", &spawnCount);
		{
			// Draw faction list
			auto& modCore = ModCoreEvents::GetInstance();

			std::shared_lock lock(modCore.m_CachedDataMutex);
			String previewString = "<Unset Faction>";
			
			if (!modCore.m_CachedAIFactions.contains(customFaction))
				customFaction = nullptr;
			else
				previewString = customFaction->GetMemberRefUnsafe<String>("Name");

			if (ImGui::BeginCombo("##factioncombo", previewString.c_str()))
			{
				std::vector sortedFactions(modCore.m_CachedAIFactions.begin(), modCore.m_CachedAIFactions.end());

				std::ranges::sort(
					sortedFactions,
					[](auto A, auto B)
					{
						return A->GetMemberRefUnsafe<String>("Name") < B->GetMemberRefUnsafe<String>("Name");
					});

				if (ImGui::Selectable("<Unset Faction>", customFaction == nullptr))
					customFaction = nullptr;

				for (auto faction : sortedFactions)
				{
					const bool isSelected = customFaction == faction;

					if (ImGui::Selectable(faction->GetMemberRefUnsafe<String>("Name").c_str(), isSelected))
						customFaction = faction;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
		ImGui::PopItemWidth();
		ImGui::Spacing();
		ImGui::RadioButton("Spawn at player position", &spawnLocationType, 0);
		ImGui::RadioButton("Spawn at crosshair position", &spawnLocationType, 1);
		ImGui::RadioButton("Spawn at custom position", &spawnLocationType, 2);
		ImGui::Spacing();

		if (spawnLocationType == 2)
		{
			ImGui::PushItemWidth(200);
			ImGui::InputDouble("X", &customSpawnPosition.X, 1.0, 20.0, "%.3f");
			ImGui::InputDouble("Y", &customSpawnPosition.Y, 1.0, 20.0, "%.3f");
			ImGui::InputDouble("Z", &customSpawnPosition.Z, 1.0, 20.0, "%.3f");
			ImGui::PopItemWidth();
			ImGui::Spacing();
		}

		// Spawn button
		if (ImGui::Button("Spawn") || (m_DoSpawnOnNextFrame && allowSpawn))
		{
			m_NextSpawnTransformType = spawnLocationType;
			m_NextSpawnCustomTransform = customSpawnPosition;

			m_NextSpawnSelectedIndex = m_LastSelectedSetupIndex;
			m_NextFaction = customFaction;

			m_OutstandingSpawnCount = spawnCount;
		}

		ImGui::Spacing();
		ImGui::PushTextWrapPos(0.0f);
		ImGui::TextDisabled("Note: Many humanoid and scripted entities will crash the game.");
		ImGui::PopTextWrapPos();
		ImGui::EndDisabled();
		ImGui::End();

		RunSpawnCommands();
		m_DoSpawnOnNextFrame = false;
	}

	bool EntitySpawnerWindow::Close()
	{
		return !m_WindowOpen;
	}

	std::string EntitySpawnerWindow::GetId() const
	{
		return "Entity Spawner";
	}

	void EntitySpawnerWindow::RunSpawnCommands()
	{
		// Faction setup logic
		if (!m_FactionSetsPending.empty())
		{
			JobHeaderCPU::SubmitCallable(
				[this]
				{
					std::scoped_lock lock(m_FactionSetupMutex);

					std::erase_if(
						m_FactionSetsPending,
						[&](const auto& Pair)
						{
							const auto getSpawnpointEntity = Offsets::Signature(
																 "48 85 C9 74 1D 48 8B 89 88 01 00 00 48 85 C9")
																 .ToPointer<Entity *(RTTIRefObject *)>();

							if (auto entity = getSpawnpointEntity(Pair.first))
							{
								entity->SetFaction(reinterpret_cast<HRZR::AIFaction *>(Pair.second));
								return true;
							}

							return false;
						});
				});
		}

		// Streaming and spawnpoint logic
		if (m_OutstandingSpawnCount <= 0)
			return;

		const auto& corePath = ModConfiguration.CachedSpawnSetups[m_NextSpawnSelectedIndex].CorePath;
		const auto spawnSetupUUID = ModConfiguration.CachedSpawnSetups[m_NextSpawnSelectedIndex].UUID;

		const auto targetSpawnSetup = [&]() -> Ref<RTTIRefObject>
		{
			auto& modEvents = ModCoreEvents::GetInstance();
			std::shared_lock lock(modEvents.m_CachedDataMutex);

			auto itr = std::ranges::find_if(
				modEvents.m_CachedSpawnSetups,
				[&](const auto& Setup)
				{
					return Setup->m_UUID == spawnSetupUUID;
				});

			if (itr != modEvents.m_CachedSpawnSetups.end())
				return *itr;

			return nullptr;
		}();

		// If the setup isn't already loaded we'll have to stream the whole object group in
		if (!targetSpawnSetup && !m_StreamerRequestPending)
		{
			auto streamingManager = StreamingManager::GetInstance();

			// HACK: Normally we'd call StreamingRefBase->Clear() here, but we're forced to leak memory since we don't
			// know how long the spawnpoint will persist. There's no way to figure out if the parent Core should remain
			// loaded.
			auto refHandle = new StreamingRefBase();

			streamingManager->Register2(*refHandle, AssetPath { corePath }, spawnSetupUUID);
			streamingManager->RegisterCallback(*refHandle, EStreamingRefCallbackMode::OnLoad, &m_LoaderCallback, this);
			streamingManager->Resolve(*refHandle, EStreamingRefPriority::Normal);

			m_StreamerRequestPending = true;
		}
		else if (targetSpawnSetup)
		{
			m_StreamerRequestPending = false;

			spdlog::debug(
				"Spawning {} entities with UUID {}",
				m_OutstandingSpawnCount.load(),
				ModConfiguration.CachedSpawnSetups[m_NextSpawnSelectedIndex].UUID);

			JobHeaderCPU::SubmitCallable(
				[this,
				 spawnSetup = targetSpawnSetup]()
				{
					const static auto spawnpointRTTI = RTTI::FindTypeByName("Spawnpoint")->AsCompound();
					const auto spawnTransform = GetSpawnTransform(m_NextSpawnTransformType, m_NextSpawnCustomTransform);

					for (uint32_t i = 0; i < m_OutstandingSpawnCount; i++)
					{
						Ref<RTTIRefObject> spawnpoint = static_cast<RTTIRefObject *>(spawnpointRTTI->CreateInstance()); // TODO: MsgInit?

						spawnpointRTTI->SetMemberValue<GGUUID>(spawnpoint, "ObjectUUID", GGUUID::Generate());
						spawnpointRTTI->SetMemberValue<WorldTransform>(spawnpoint, "Orientation", spawnTransform);
						spawnpointRTTI->SetMemberValue<Ref<RTTIRefObject>>(spawnpoint, "SpawnSetup", spawnSetup);
						spawnpointRTTI->SetMemberValue<uint8_t>(spawnpoint, "FactsLifetime", 0);
						spawnpointRTTI->SetMemberValue<bool>(spawnpoint, "AutoSpawn", false);

						const auto spawnpointSpawn = Offsets::Signature("48 85 C9 74 25 80 B9 80 01 00 00 00 75 1C")
														 .ToPointer<void(RTTIRefObject *)>();
						spawnpointSpawn(spawnpoint);

						if (m_NextFaction)
						{
							std::scoped_lock lock(m_FactionSetupMutex);
							m_FactionSetsPending.emplace_back(std::move(spawnpoint), m_NextFaction);
						}
					}

					m_NextFaction = nullptr;
					m_OutstandingSpawnCount.store(0);
				});
		}
	}

	WorldTransform EntitySpawnerWindow::GetSpawnTransform(uint32_t Type, const WorldPosition& CustomPosition)
	{
		auto player = Player::GetLocalPlayer();
		auto currentTransform = player->m_Entity->GetWorldTransform();

		if (Type == 0)
		{
			// Player position
			currentTransform.Position = player->m_Entity->GetWorldTransform().Position;
		}
		else if (Type == 1)
		{
			// Crosshair position - project forwards
			const auto cameraMatrix = player->GetLastActivatedCamera()->GetWorldTransform();
			const auto moveDirection = cameraMatrix.Orientation.Forward() * 200.0f;

			currentTransform.Position += moveDirection;

			// Raycast
			WorldPosition rayHitPosition;
			float unknownFloat;
			Entity *unknownEntity;
			Vec3 normal;
			uint16_t materialType;

			const auto intersectLine = Offsets::Signature("48 8B C4 48 89 58 20 48 89 50 10 48 89 48 08 55 56 57 41 54 41 55")
										   .ToPointer<int(
											   const WorldPosition&, // a1
											   const WorldPosition&, // a2
											   int,					 // a3 EPhysicsCollisionLayerGame
											   const Entity *,		 // a4
											   bool,				 // a5
											   WorldPosition *,		 // a6
											   Vec3 *,				 // a7
											   float *,				 // a8
											   Entity **,			 // a9
											   uint16_t *)>();		 // a10

			intersectLine(
				cameraMatrix.Position,
				currentTransform.Position,
				47,
				nullptr,
				false,
				&rayHitPosition,
				&normal,
				&unknownFloat,
				&unknownEntity,
				&materialType);

			currentTransform.Position = rayHitPosition;
		}
		else if (Type == 2)
		{
			// Custom position
			currentTransform.Position = CustomPosition;
		}

		return currentTransform;
	}

	void EntitySpawnerWindow::ForceSpawnEntityClick()
	{
		m_DoSpawnOnNextFrame = true;
	}
}
