#pragma once

#include "WorldPosition.h"
#include "RotMatrix.h"

namespace HRZR
{
	class WorldTransform final
	{
	public:
		WorldPosition Position;
		RotMatrix Orientation;
	};
	static_assert(sizeof(WorldTransform) == 0x40);
}
