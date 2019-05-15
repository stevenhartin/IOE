#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOEApplication/IOEApplication.h>

#include "Matrix.h"

#include "Math.h"

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix g_mIdentity =
		IOEMatrix(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	IOEMatrix g_mZero =
		IOEMatrix(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::LookAt(const IOEVector &vEye,
								const IOEVector &vTarget,
								const IOEVector &vUp /*=g_vUpVector*/)
	{
		return IOEMatrix(
			XMMatrixLookAtLH(vEye.GetRaw(), vTarget.GetRaw(), vUp.GetRaw()));
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::Perspective(float fFOV, float fAspect /*=-1.0f*/,
									 float fNear /*=0.5f*/,
									 float fFar /*=1000.0f*/)
	{
		if (fAspect < 0.0f)
		{
			std::uint32_t uWidth(0), uHeight(0);
			IOE::Core::g_pEngine->GetApplication()->GetWindowSize(uWidth,
																  uHeight);
			fAspect = uWidth / static_cast<float>(uHeight);
		}
		return IOEMatrix(XMMatrixPerspectiveFovLH(fFOV, fAspect, fNear, fFar));
	}

	//////////////////////////////////////////////////////////////////////////

	IOE::Maths::IOEMatrix IOEMatrix::Orthographic(float fWidth /*=inf*/,
												  float fHeight /*=inf*/,
												  float fNear /*=0.0f*/,
												  float fFar /*=1.0f*/)
	{
		std::uint32_t uWidth(0), uHeight(0);
		IOE::Core::g_pEngine->GetApplication()->GetWindowSize(uWidth, uHeight);
		if (fWidth == std::numeric_limits<float>::infinity())
		{
			fWidth = static_cast<float>(uWidth);
		}
		if (fHeight == std::numeric_limits<float>::infinity())
		{
			fHeight = static_cast<float>(uHeight);
		}
		return IOEMatrix(XMMatrixOrthographicLH(fWidth, fHeight, fNear, fFar));
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::RotationX(float fRotationRad)
	{
		IOEMatrix mNew;
		mNew.m_mMatrix = XMMatrixRotationX(fRotationRad);
		return mNew;
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::RotationXDeg(float fRotationDeg)
	{
		return RotationX(DegreesToRadians(fRotationDeg));
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::RotationY(float fRotationRad)
	{
		IOEMatrix mNew;
		mNew.m_mMatrix = XMMatrixRotationY(fRotationRad);
		return mNew;
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::RotationYDeg(float fRotationDeg)
	{
		return RotationY(DegreesToRadians(fRotationDeg));
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::RotationZ(float fRotationRad)
	{
		IOEMatrix mNew;
		mNew.m_mMatrix = XMMatrixRotationZ(fRotationRad);
		return mNew;
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOEMatrix::RotationZDeg(float fRotationDeg)
	{
		return RotationZ(DegreesToRadians(fRotationDeg));
	}

	//////////////////////////////////////////////////////////////////////////

	IOE::Maths::IOEMatrix IOEMatrix::Translation(const IOEVector &vTranslation)
	{
		IOEMatrix mIdentity(g_mIdentity);
		mIdentity.SetColumn(3, vTranslation);
		return mIdentity;
	}

	//////////////////////////////////////////////////////////////////////////

	IOE::Maths::IOEMatrix IOEMatrix::Scale(const IOEVector &vScale)
	{
		IOEMatrix mIdentity(g_mIdentity);
		mIdentity.SetCell(0, 0, vScale.GetX());
		mIdentity.SetCell(1, 1, vScale.GetY());
		mIdentity.SetCell(2, 2, vScale.GetZ());
		mIdentity.SetCell(3, 3, 1.0f);
		return mIdentity;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace maths
} // namespace IOE