#pragma once

#include "EntityComponent.h"

namespace HRZR
{
	enum class EInventoryItemAddType : uint8_t
	{
		Regular = 0,
		IgnoreCapacity = 1,
		Transfer = 2,
		LoadSave = 3,
		Craft = 4,
		Merchant = 5,
		Remember = 6,
		BuyBack = 7
	};

	enum class EInventoryItemRemoveType : uint8_t
	{
		Destroy = 0,
		Transfer = 1,
		Drop = 2,
		Craft = 3,
		Consume = 4,
		Remember = 5,
	};

	enum class ECheckQuestItems : uint8_t
	{
		ResourceDefault = 0,
		NoQuestItems = 1,
		OnlyQuestItems = 2,
		AllItems = 3,
	};

	class InventoryEntity;
	class EntityResource;

	class Inventory : public EntityComponent
	{
	public:
		Array<Entity *> m_Items;												  // 0x58
		char _pad68[0x68];														  // 0x68

		virtual const RTTI *GetRTTI() const override;							  // 0
		virtual ~Inventory() override;											  // 1
		virtual const RTTI *GetRepresentationType() const override;				  // 4
		virtual void UnknownEntityComponent07() override;						  // 7
		virtual void UnknownEntityComponent08() override;						  // 8
		virtual NetEntityComponentState *CreateNetState() override;				  // 9
		virtual Entity *Add1(EntityResource *, int, EInventoryItemAddType, bool); // 10
		virtual Entity *Add2();													  // 11
		virtual bool Remove(Entity *, int, EInventoryItemRemoveType, bool);		  // 12
		virtual void InventoryUnknown13();										  // 13
		virtual void InventoryUnknown14();										  // 14

		int GetItemAmount(Entity *ItemEntity) const
		{
			const static auto ammoRTTI = RTTI::FindTypeByName("Ammo");
			const static auto scRTTI = RTTI::FindTypeByName("StackableComponent");
			const static auto uscRTTI = RTTI::FindTypeByName("UpgradableStackableComponent");

			std::lock_guard lock(ItemEntity->m_EntityAccessMutex);

			auto stackableComponent = static_cast<EntityComponent *>(ItemEntity->m_Components.FindComponentByRTTI(scRTTI));

			if (!stackableComponent)
				stackableComponent = static_cast<EntityComponent *>(ItemEntity->m_Components.FindComponentByRTTI(uscRTTI));

			if (!stackableComponent && ItemEntity->GetRTTI()->IsKindOf(ammoRTTI)) // Dedicated StackableComponent for Ammo
				stackableComponent = *reinterpret_cast<EntityComponent **>(reinterpret_cast<uintptr_t>(ItemEntity) + 0x2D0); // TODO

			if (!stackableComponent)
				return 1;

			return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(stackableComponent) + 0x58); // TODO
		}
	};
	assert_offset(Inventory, m_Items, 0x58);
}
