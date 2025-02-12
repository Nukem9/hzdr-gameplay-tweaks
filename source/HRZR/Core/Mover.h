#pragma once

#include "EntityComponent.h"
#include "WorldTransform.h"

namespace HRZR
{
	class Mover : public EntityComponent
	{
	public:
		virtual const RTTI *GetRTTI() const override;											  // 0
		virtual ~Mover() override;																  // 1
		virtual bool IsActive();																  // 10
		virtual void SetActive(bool Active);													  // 11
		virtual void OverrideMovement(const WorldTransform& Transform, float MoveDuration, bool); // 12
		virtual void UpdateOverrideMovementTarget(const WorldTransform& Transform);				  // 13
		virtual bool IsMovementOverridden();													  // 14
		virtual void StopOverrideMovement();													  // 15
		virtual float GetOverrideMovementDuration();											  // 16
		virtual float GetOverrideMovementTime();												  // 17
		virtual float GetOverrideMovementSpeed();												  // 18
		// ...
	};
}
