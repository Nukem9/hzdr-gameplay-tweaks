#pragma once

#include <unordered_set>

namespace HRZR
{
	class RTTI;
}

namespace RTTIScanner
{
	const std::unordered_set<const HRZR::RTTI *>& GetAllTypes();

	void ScanForRTTIStructures();
	void RegisterTypeInfoRecursively(const HRZR::RTTI *Info);
	void RegisterRTTIStructures();
}
