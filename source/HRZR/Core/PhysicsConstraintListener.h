#pragma once

namespace HRZR
{
	class PhysicsConstraintListener
	{
	public:
		virtual ~PhysicsConstraintListener();		  // 0
		virtual void OnConstraintObjectRemoved() = 0; // 1
		virtual void OnConstraintBroken() = 0;		  // 2
	};
}
