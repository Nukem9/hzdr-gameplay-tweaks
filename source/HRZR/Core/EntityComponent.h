#pragma once

#include "../PCore/Common.h"
#include "RTTIRefObject.h"
#include "WeakPtr.h"

namespace HRZR
{
	class Entity;
	class EntityComponentRep;
	class EntityComponentResource;
	class NetEntityComponentState;

	template<typename T>
	class EntityMessageProcessing
	{
	public:
	};

	class EntityComponentResource : public RTTIRefObject
	{
	};

	class EntityComponent : public RTTIRefObject, public WeakPtrTarget, public EntityMessageProcessing<EntityComponent>
	{
	public:
		Ref<EntityComponentResource> m_Resource;					 // 0x30
		bool m_IsInitialized;										 // 0x38
		EntityComponentRep *m_Representation;						 // 0x40
		Entity *m_Entity;											 // 0x48
		char _pad50[0x8];											 // 0x50

		virtual const RTTI *GetRTTI() const override;				 // 0
		virtual ~EntityComponent();									 // 1
		virtual const RTTI *GetRepresentationType() const;			 // 4
		virtual void SetEntity(Entity *Entity);						 // 5
		virtual void SetResource(EntityComponentResource *Resource); // 6
		virtual void UnknownEntityComponent07();					 // 7
		virtual void UnknownEntityComponent08();					 // 8
		virtual NetEntityComponentState *CreateNetState();			 // 9
	};
	assert_offset(EntityComponent, m_Resource, 0x30);
	static_assert(sizeof(EntityComponent) == 0x58);

	class EntityComponentContainer
	{
	public:
		Array<EntityComponent *> m_Components; // 0x0
		Array<uint16_t> m_ComponentTypes;	   // 0x10

		EntityComponent *FindComponentByRTTI(const RTTI *RTTI) const
		{
			for (auto& component : m_Components)
			{
				if (component->GetRTTI()->IsExactKindOf(RTTI))
					return component;
			}

			return nullptr;
		}
	};
	static_assert(sizeof(EntityComponentContainer) == 0x20);
}
