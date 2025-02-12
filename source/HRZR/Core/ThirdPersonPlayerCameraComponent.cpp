#include "../../ModConfiguration.h"
#include "../DebugUI/MainMenuBar.h"
#include "CameraEntity.h"

namespace HRZR
{
	class CameraMode;

	void HookedSetWorldTransform(CameraEntity *This, const WorldTransform& Transform)
	{
		if (DebugUI::MainMenuBar::m_FreeCamMode == DebugUI::MainMenuBar::FreeCamMode::Free)
			return;

		This->SetWorldTransform(Transform);
	}

	void (*OriginalGetCameraModeViewConstraints)(CameraMode *, const float&, const float&, float&, float&, float&, float&);
	void HookedGetCameraModeViewConstraints(
		CameraMode *Thisptr,
		const float& a2,
		const float& a3,
		float& HeadingMin,
		float& HeadingMax,
		float& PitchMin,
		float& PitchMax)
	{
		OriginalGetCameraModeViewConstraints(Thisptr, a2, a3, HeadingMin, HeadingMax, PitchMin, PitchMax);

		HeadingMin = -0.27925f; // 16 degrees in radians
		HeadingMax = HeadingMin;

		PitchMin = 0.0f;
		PitchMax = PitchMin;
	}

	DECLARE_HOOK_TRANSACTION(ThirdPersonPlayerCameraComponent)
	{
		Hooks::WriteCall(Offsets::Signature("E8 ? ? ? ? 48 8B 8F 98 02 00 00 E8"), &HookedSetWorldTransform);

		if (ModConfiguration.ForceLeftAlignedCamera)
		{
			Hooks::WriteJump(
				Offsets::Signature("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 81 EC B0 00 00 00 48 8B 71 48"),
				&HookedGetCameraModeViewConstraints,
				&OriginalGetCameraModeViewConstraints);

			// Disable FollowSpeedToOrbitScalar curve evaluation by returning 0.0f
			Memory::Patch(Offsets::Signature("E8 ? ? ? ? C5 FA 10 5C ? ? C5 A2 5F C8 C5 A2 5F C6"), { 0x90, 0xC5, 0xF8, 0x57, 0xC0 });
		}
	};
}
