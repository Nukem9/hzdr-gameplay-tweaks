#pragma once

#include <shared_mutex>
#include "GameModule.h"
#include "NetReplicatedObject.h"
#include "WeakPtr.h"
#include "CameraEntity.h"

namespace HRZR
{
	class AIFaction;

	class Player : public NetReplicatedObject, public WeakPtrTarget
	{
	public:
		char _pad50[0x8];
		String m_Name;							// 0x58
		String m_Unknown60;						// 0x60 Clan tag?
		char _pad68[0x8];
		Entity *m_Character;					// 0x70
		Entity *m_Entity;						// 0x78
		char _pad78[0x8];
		AIFaction *m_Faction;					// 0x88
		char _pad88[0x50];
		Array<WeakPtr<CameraEntity>> m_Cameras; // 0xE0
		mutable RecursiveMutex m_DataLock;		// 0xF0
		char _pad[0x18];

		virtual const RTTI *GetRTTI() const override;			// 0
		virtual ~Player() override;								// 1
		virtual void NetReplicatedObjectUnknown04() override;	// 4
		virtual void NetReplicatedObjectUnknown05() override;	// 5
		virtual void PlayerUnknown11();							// 11
		virtual void PlayerUnknown12();							// 12
		virtual void PlayerUnknown13();							// 13
		virtual void PlayerUnknown14();							// 14
		virtual void PlayerUnknown15();							// 15
		virtual void PlayerUnknown16();							// 16
		virtual void PlayerUnknown17();							// 17
		virtual bool PlayerUnknown18();							// 18
		virtual bool PlayerUnknown19();							// 19
		virtual void SetPlayerFaction(AIFaction *Faction);		// 20
		virtual void PlayerUnknown21();							// 21
		virtual void PlayerUnknown22();							// 22
		virtual void PlayerUnknown23();							// 23
		virtual void SerializeToStream(void *Stream);			// 24
		virtual void DeserializeFromStream(void *Stream);		// 25

		// WARNING: Do NOT call this function while other entity locks are held. It can deadlock the main thread.
		CameraEntity *GetLastActivatedCamera() const
		{
			std::lock_guard lock(m_DataLock);

			if (!m_Cameras.empty())
				return m_Cameras[m_Cameras.size() - 1];

			return nullptr;
		}

		static Player *GetLocalPlayer(uint32_t Index = 0)
		{
			if (!GameModule::GetInstance())
				return nullptr;

			const auto func = Offsets::Signature("40 53 48 83 EC 30 48 63 D9 48 8D 15 ? ? ? ? 85 C9 48 8B 0D")
								  .ToPointer<Player *(int)>();

			return func(Index);
		}
	};
	assert_offset(Player, m_Name, 0x58);
	assert_offset(Player, m_Entity, 0x78);
	assert_offset(Player, m_Faction, 0x88);
	assert_offset(Player, m_Cameras, 0xE0);
	static_assert(sizeof(Player) == 0x130);
}
