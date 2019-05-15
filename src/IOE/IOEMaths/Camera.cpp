#include "Camera.h"
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOECore/IOEEngine.h>

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	IOECamera::IOECamera(ECameraStyle nStyle /*=ECameraStyle::ECS_Fixed*/)
		: m_pTarget(nullptr)
		, m_nCameraStyle(nStyle)
		, m_vUpVector(g_vUpVector)
		, m_vLookAtVec(g_vZeroVector)
	{
		m_tTransform.SetTranslation(-10.0f, 1.5f, 5.0f);
		//m_tTransform.SetRotation(20.0f, 110.0f, 0.0f);
		m_vLookAtVec.Set(-10.0f, 1.5f, 6.0f);

		m_mViewMatrix		   = g_mIdentity;
		m_mInvViewMatrix	   = g_mIdentity;
		m_mProjectionMatrix	   = g_mIdentity;
		m_mInvProjectionMatrix = g_mIdentity;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::Tick(IOE::Core::IOETimeDelta tDelta)
	{
		if (m_nCameraStyle == ECameraStyle::ECS_Defined)
		{
			return;
		}

		IOEVector vLookat(m_tTransform.GetTranslation() + g_vForwardVector);

		IOEVector vCamPos(m_tTransform.GetAxis(3));

		if (m_pTarget != nullptr)
		{
			vLookat = m_pTarget->GetTranslation();

			if (m_nCameraStyle == ECameraStyle::ECS_Following)
			{
				IOEQuaternion qTargetRot(
					g_vUpVector, m_pTarget->GetRotationQuat().GetYaw());
				IOEVector vOffsetTransformed(
					qTargetRot.Rotate(m_tTransform.GetTranslation()));
				vCamPos = vLookat + vOffsetTransformed;
			}
		}

		SetLookAt(vLookat);
		m_tTransform.SetTranslation(vCamPos);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::SetLookAt(const IOEVector &vLookAt)
	{
		m_vLookAtVec = vLookAt;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::SetTarget(IOETransform *pTarget)
	{
		m_pTarget = pTarget;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOETransform *IOECamera::GetTarget() const
	{
		return m_pTarget;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform *IOECamera::GetTarget()
	{
		return m_pTarget;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::UpdateViewMatrices()
	{
		IOEVector vDirection = m_vLookAtVec - m_tTransform.GetTranslation();
		vDirection			 = vDirection.Normalise();
		IOEQuaternion qRotation(vDirection, m_vUpVector);
		m_mViewMatrix =
			IOEMatrix::LookAt(m_tTransform.GetTranslation(), m_vLookAtVec,
							  qRotation.Rotate(m_vUpVector));
		m_mInvViewMatrix = m_mViewMatrix.Inverse();

		if (m_tParams.eProjectionMethod == EProjectionMethod::Perspective)
		{
			m_mProjectionMatrix = IOEMatrix::Perspective(
				DegreesToRadians(m_tParams.fFOV), m_tParams.fAspect,
				m_tParams.fNear, m_tParams.fFar);
		}
		else
		{
			m_mProjectionMatrix =
				IOEMatrix::Orthographic(m_tParams.fWidth, m_tParams.fHeight,
										m_tParams.fNear, m_tParams.fFar);
		}

		m_mInvProjectionMatrix = m_mProjectionMatrix.Inverse();
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEMatrix &IOECamera::GetViewMatrix() const
	{
		return m_mViewMatrix;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEMatrix &IOECamera::GetInvViewMatrix() const
	{
		return m_mInvViewMatrix;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEMatrix &IOECamera::GetProjectionMatrix() const
	{
		return m_mProjectionMatrix;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEMatrix &IOECamera::GetInvProjectionMatrix() const
	{
		return m_mInvProjectionMatrix;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::SetCameraStyle(ECameraStyle nStyle)
	{
		m_nCameraStyle = nStyle;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEVector &IOECamera::GetLookAt() const
	{
		return m_vLookAtVec;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEVector &IOECamera::GetUpVector() const
	{
		return m_vUpVector;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::SetUpVector(const IOEVector &vUp)
	{
		m_vUpVector = vUp;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOETransform &IOECamera::GetTransform() const
	{
		return m_tTransform;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform &IOECamera::GetTransform()
	{
		return m_tTransform;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::SetCameraProperties(CCameraProperties tProperties)
	{
		m_tParams = tProperties;
		UpdateViewMatrices();
	}

	//////////////////////////////////////////////////////////////////////////

	IOEVector IOECamera::ScreenSpaceDirToWorldSpaceDir(int32_t nX,
													   int32_t nY) const
	{
		std::uint32_t uWidth(0), uHeight(0);
		IOE::Core::g_pEngine->GetApplication()->GetWindowSize(uWidth, uHeight);

		// Cast to NDC
		float fX(((2.0f * nX) / uWidth) - 1.0f);
		float fY(1.0f - (2.0f * nY) / uHeight);
		float fZ(1.0f);

		// Create local ray in terms of NDC
		IOEVector vWorldRay(fX, fY, fZ);

		// Unproject it
		vWorldRay *= m_mInvProjectionMatrix;

		// Go from view space -> world space (inv view matrix)
		vWorldRay *= m_mInvViewMatrix;

		// Normalise direction
		vWorldRay.NormaliseSet();

		return vWorldRay;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOECamera::SetOrthographic(
		float fWidth /*=std::numeric_limits<float>::infinity()*/,
		float fHeight /*=std::numeric_limits<float>::infinity()*/,
		float fNear /*=0.0f*/, float fFar /*=1.0f*/)
	{
		m_tParams.eProjectionMethod = EProjectionMethod::Orthographic;
		m_tParams.fWidth			= fWidth;
		m_tParams.fHeight			= fHeight;
		m_tParams.fNear				= fNear;
		m_tParams.fFar				= fFar;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE