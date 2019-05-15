#pragma once

#include <memory>
#include <cstdint>

#include <IOE/IOECore/IOEHighResolutionTime.h>

#include "Vector.h"
#include "Matrix.h"
#include "Transform.h"
#include "Math.h"

using std::weak_ptr;
using std::int32_t;

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	class IOECamera
	{
	public:
		enum class EProjectionMethod
		{
			Perspective,
			Orthographic
		};
		struct CCameraProperties
		{
			CCameraProperties()
				: eProjectionMethod(EProjectionMethod::Perspective)
				, fFOV(45)
				, fAspect(2.0f)
				, fNear(0.015f)
				, fFar(5000.0f)
			{
			}

			EProjectionMethod eProjectionMethod;

			union
			{
				struct
				{
					float fFOV;
					float fAspect;
				};

				struct
				{
					float fWidth;
					float fHeight;
				};
			};
			float fNear;
			float fFar;
		};

		enum class ECameraStyle : std::uint16_t
		{
			// Camera is fixed at the world transform of this entity
			ECS_Fixed,

			// Camera follows the target using the world transform as an offset
			ECS_Following,

			// Camera targets a defined look at point
			ECS_Defined,

			ECS_MAX
		};

		IOECamera(ECameraStyle nStyle = ECameraStyle::ECS_Following);

		virtual void Tick(IOE::Core::IOETimeDelta TimeDelta);

		void SetCameraStyle(ECameraStyle nStyle);
		void SetCameraProperties(CCameraProperties tProperties);
		FORCEINLINE const CCameraProperties& GetCameraProperties() const
		{
			return m_tParams;
		}

		void
		SetOrthographic(float fWidth = std::numeric_limits<float>::infinity(),
						float fHeight = std::numeric_limits<float>::infinity(),
						float fNear = 0.0f, float fFar = 1.0f);

		void SetTarget(IOETransform* pTarget);
		const IOETransform* GetTarget() const;
		IOETransform* GetTarget();

		virtual void UpdateViewMatrices();

		const IOEVector& GetUpVector() const;
		void SetUpVector(const IOEVector& vUp);

		const IOEVector& GetLookAt() const;
		void SetLookAt(const IOEVector& vLookAt);

		const IOEMatrix& GetViewMatrix() const;
		const IOEMatrix& GetInvViewMatrix() const;
		const IOEMatrix& GetProjectionMatrix() const;
		const IOEMatrix& GetInvProjectionMatrix() const;

		const IOETransform& GetTransform() const;
		IOETransform& GetTransform();

		IOEVector ScreenSpaceDirToWorldSpaceDir(int32_t nX, int32_t nY) const;

	private:
		IOETransform m_tTransform;

		IOEMatrix m_mViewMatrix;
		IOEMatrix m_mInvViewMatrix;

		IOEMatrix m_mProjectionMatrix;
		IOEMatrix m_mInvProjectionMatrix;

		IOEVector m_vUpVector;
		IOEVector m_vLookAtVec;

		IOETransform* m_pTarget;

		CCameraProperties m_tParams;

		ECameraStyle m_nCameraStyle;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE