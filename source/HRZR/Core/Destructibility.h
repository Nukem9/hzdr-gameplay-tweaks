#pragma once

#include "EntityComponent.h"
#include "PhysicsConstraintListener.h"

namespace HRZR
{
	class Destructibility : public EntityComponent, public PhysicsConstraintListener
	{
	public:
		virtual const RTTI *GetRTTI() const override; // 0
		virtual ~Destructibility() override;		  // 1

		bool m_Invulnerable;	// 0x60
		bool m_DieAtZeroHealth; // 0x61
	};
	assert_offset(Destructibility, m_Invulnerable, 0x60);
}
