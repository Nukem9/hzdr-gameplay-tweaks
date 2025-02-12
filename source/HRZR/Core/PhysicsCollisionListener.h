#pragma once

namespace HRZR
{
	class PhysicsCollisionListener
	{
	public:
		virtual ~PhysicsCollisionListener();	   // 0
		virtual bool OnPhysicsContactValidate();   // 1
		virtual void OnPhysicsContactAdded();	   // 2
		virtual void OnPhysicsContactProcess();	   // 3
		virtual void OnPhysicsContactRemoved();	   // 4
		virtual void OnPhysicsOutsideBroadPhase(); // 5
	};
}
