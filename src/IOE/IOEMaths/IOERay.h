#pragma once

#include "vector.h"

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	class IOERay
	{
	public:
		IOERay(const IOEVector &vOrigin, const IOEVector &vDir)
			: m_vOrigin(vOrigin)
			, m_vDirAndLength(vDir)
		{
			float fX, fY, fZ;
			m_vDirAndLength.Normalise().Get(fX, fY, fZ);
			fX					= 1.0f / fX;
			fY					= 1.0f / fY;
			fZ					= 1.0f / fZ;
			m_vInvNormalisedDir = IOEVector(fX, fY, fZ);
			m_arrSign[0] = (m_vInvNormalisedDir.GetX() < 0.0f);
			m_arrSign[1] = (m_vInvNormalisedDir.GetY() < 0.0f);
			m_arrSign[2] = (m_vInvNormalisedDir.GetZ() < 0.0f);
		}

		IOEVector m_vOrigin;
		IOEVector m_vDirAndLength;
		IOEVector m_vInvNormalisedDir;
		int m_arrSign[3];
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE