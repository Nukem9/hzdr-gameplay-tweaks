#pragma once

#include "../PCore/Common.h"
#include "PhysicsCollisionListener.h"
#include "RTTIRefObject.h"
#include "CoreObject.h"
#include "WeakPtr.h"
#include "WorldTransform.h"
#include "EntityComponent.h"
#include "IStreamingManager.h"

namespace HRZR
{
	class AIFaction;
	class Destructibility;
	class Model;
	class Mover;

	class EntityResource : public RTTIRefObject
	{
	public:
	};

	class Entity : public CoreObject, public WeakPtrTarget, public PhysicsCollisionListener
	{
	public:
		enum EFlags : int
		{
			FLAG_NONE = 0,
			FLAG_IS_CHANGED = 0x1,
			FLAG_IS_VISIBLE = 0x2,
		};

		char _pad38[0x48];																			   // 0x38
		mutable RecursiveMutex m_EntityAccessMutex;													   // 0x80
		char _padA8[0xA8];																			   // 0xA8
		WorldTransform m_WorldTransform;															   // 0x150 Lock
		char _pad190[0x20];																			   // 0x190
		StreamingRef<EntityResource> m_EntityResource;												   // 0x1B0
		char _pad1B8[0x40];																			   // 0x1B8
		Mover *m_Mover;																				   // 0x1F8 Lock
		Model *m_Model;																				   // 0x200 Lock
		Destructibility *m_Destructibility;															   // 0x208 Lock
		char _pad210[0x30];																			   // 0x210
		AIFaction *m_Faction;																		   // 0x240
		uint32_t m_Flags;																			   // 0x248
		char _pad24C[0x54];																			   // 0x24C
		EntityComponentContainer m_Components;														   // 0x2A0
		char _pad2C0[0x10];																			   // 0x2C0

		virtual const RTTI *GetRTTI() const override;												   // 0
		virtual ~Entity() override;																	   // 1
		virtual String& GetName() const override;													   // 5
		virtual void CoreObjectUnknown09() override;												   // 9
		virtual void CoreObjectUnknown10() override;												   // 10
		virtual void CoreObjectUnknown11() override;												   // 11
		virtual void SetName(String Name);															   // 16
		virtual void PlaceOnWorldTransform(const WorldTransform& Transform, bool RelativeCoordinates); // 17
		virtual WorldTransform EntityUnknown18(WorldTransform& Transform);							   // 18 No idea. Assigns a2 to a3.
		virtual void EntityUnknown19();																   // 19
		virtual void EntityUnknown20();																   // 20
		virtual void EntityUnknown21();																   // 21
		virtual void EntityUnknown22();																   // 22
		virtual void EntityUnknown23();																   // 23
		virtual void EntityUnknown24();																   // 24
		virtual void EntityUnknown25();																   // 25
		virtual void EntityUnknown26();																   // 26
		virtual void EntityUnknown27();																   // 27
		virtual void EntityUnknown28();																   // 28
		virtual void EntityUnknown29();																   // 29
		virtual void EntityUnknown30();																   // 30
		virtual void EntityUnknown31();																   // 31
		virtual void EntityUnknown32();																   // 32
		virtual void EntityUnknown33();																   // 33
		virtual void EntityUnknown34();																   // 34
		virtual void EntityUnknown35();																   // 35
		virtual void EntityUnknown36();																   // 36
		virtual void EntityUnknown37();																   // 37
		virtual void EntityUnknown38();																   // 38

		// PhysicsCollisionListener
		virtual bool OnPhysicsContactValidate() override;	// 1
		virtual void OnPhysicsContactAdded() override;		// 2
		virtual void OnPhysicsContactProcess() override;	// 3
		virtual void OnPhysicsContactRemoved() override;	// 4
		virtual void OnPhysicsOutsideBroadPhase() override; // 5

		WorldTransform GetWorldTransform() const
		{
			std::scoped_lock lock(m_EntityAccessMutex);
			return m_WorldTransform;
		}

		void SetWorldTransform(const WorldTransform& Transform)
		{
			std::scoped_lock lock(m_EntityAccessMutex);

			m_WorldTransform = Transform;
			m_Flags |= FLAG_IS_CHANGED;
		}

		void SetFaction(AIFaction *Faction)
		{
			const auto func = Offsets::Signature("48 89 5C 24 18 48 89 6C 24 20 57 48 83 EC 40 48 8B E9 48 8B FA")
								  .ToPointer<void(Entity *, AIFaction *)>();

			func(this, Faction);
		}
	};
	assert_offset(Entity, m_EntityAccessMutex, 0x80);
	assert_offset(Entity, m_WorldTransform, 0x150);
	assert_offset(Entity, m_EntityResource, 0x1B0);
	assert_offset(Entity, m_Mover, 0x1F8);
	assert_offset(Entity, m_Destructibility, 0x208);
	assert_offset(Entity, m_Flags, 0x248);
	assert_offset(Entity, m_Components, 0x2A0);
	static_assert(sizeof(Entity) == 0x2D0);
}
