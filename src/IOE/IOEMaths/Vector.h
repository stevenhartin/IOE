#pragma once

#include <utility>

#include <DirectXMath.h>

#include "Math.h"

using namespace DirectX;

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	class alignas(16) IOEVector
	{
	public:
		FORCEINLINE IOEVector()
		{
		}

		FORCEINLINE IOEVector(float fValue)
			: m_vVector(XMVectorReplicate(fValue))
		{
		}

		FORCEINLINE IOEVector(float fX, float fY, float fZ, float fW = 1.0f)
		{
			Set(fX, fY, fZ, fW);
		}

		FORCEINLINE IOEVector(const XMFLOAT4 &rData)
		{
			Load(rData);
		}

		FORCEINLINE IOEVector(const XMFLOAT3 &rData)
		{
			Load(rData);
		}

		FORCEINLINE IOEVector(const XMVECTOR &rData)
			: m_vVector(rData)
		{
		}

		FORCEINLINE IOEVector(const IOEVector &rhs)
			: m_vVector(rhs.m_vVector)
		{
		}
		FORCEINLINE IOEVector(IOEVector && rhs)
			: m_vVector(std::move(rhs.m_vVector))
		{
		}

	public:
		FORCEINLINE IOEVector &operator=(const IOEVector &rhs)
		{
			m_vVector = rhs.m_vVector;
			return *this;
		}

		FORCEINLINE IOEVector &operator=(const XMFLOAT4 &rData)
		{
			Load(rData);
			return *this;
		}
		FORCEINLINE IOEVector &operator=(IOEVector && rhs)
		{
			m_vVector = std::move(rhs.m_vVector);
			return *this;
		}
		FORCEINLINE IOEVector &operator=(const XMVECTOR &rData)
		{
			m_vVector = rData;
			return *this;
		}
		FORCEINLINE operator XMFLOAT4() const
		{
			XMFLOAT4 tData;
			Store(tData);
			return tData;
		}

		FORCEINLINE operator XMFLOAT3() const
		{
			XMFLOAT3 tData;
			Store(tData);
			return tData;
		}

		FORCEINLINE operator CXMVECTOR() const
		{
			return m_vVector;
		}

		FORCEINLINE operator XMVECTOR() const
		{
			return m_vVector;
		}

	public:
		FORCEINLINE bool operator==(const IOEVector &rhs) const
		{
			return XMVector3NearEqual(m_vVector, rhs.m_vVector, g_XMEpsilon);
		}

		FORCEINLINE bool operator!=(const IOEVector &rhs) const
		{
			return !(operator==(rhs));
		}

	public:
		FORCEINLINE float GetX() const
		{
			return XMVectorGetX(m_vVector);
		}

		FORCEINLINE void SetX(float fX)
		{
			m_vVector = XMVectorSetX(m_vVector, fX);
		}

		FORCEINLINE float GetY() const
		{
			return XMVectorGetY(m_vVector);
		}

		FORCEINLINE void SetY(float fY)
		{
			m_vVector = XMVectorSetY(m_vVector, fY);
		}

		FORCEINLINE float GetZ() const
		{
			return XMVectorGetZ(m_vVector);
		}

		FORCEINLINE void SetZ(float fZ)
		{
			m_vVector = XMVectorSetZ(m_vVector, fZ);
		}

		FORCEINLINE float GetW() const
		{
			return XMVectorGetW(m_vVector);
		}

		FORCEINLINE void SetW(float fW)
		{
			m_vVector = XMVectorSetW(m_vVector, fW);
		}

		FORCEINLINE void Set(float fX, float fY, float fZ, float fW = 1.0f)
		{
			m_vVector = XMVectorSet(fX, fY, fZ, fW);
		}

		FORCEINLINE void Get(float &fX, float &fY, float &fZ) const
		{
			XMFLOAT4 tFloat;
			Store(tFloat);
			fX = tFloat.x;
			fY = tFloat.y;
			fZ = tFloat.z;
		}

		FORCEINLINE void Get(float &fX, float &fY, float &fZ, float &fW) const
		{
			XMFLOAT4 tFloat;
			Store(tFloat);
			fX = tFloat.x;
			fY = tFloat.y;
			fZ = tFloat.z;
			fW = tFloat.w;
		}

		FORCEINLINE IOEVector WithX(float fX) const
		{
			IOEVector vTemp(*this);
			vTemp.SetX(fX);
			return vTemp;
		}

		FORCEINLINE IOEVector WithY(float fY) const
		{
			IOEVector vTemp(*this);
			vTemp.SetY(fY);
			return vTemp;
		}

		FORCEINLINE IOEVector WithZ(float fZ) const
		{
			IOEVector vTemp(*this);
			vTemp.SetZ(fZ);
			return vTemp;
		}

		FORCEINLINE IOEVector WithW(float fW) const
		{
			IOEVector vTemp(*this);
			vTemp.SetW(fW);
			return vTemp;
		}

		FORCEINLINE const XMVECTOR &GetRaw() const
		{
			return m_vVector;
		}

	public:
		FORCEINLINE bool IsNormalised(float fEpsilon = 1.0e-5f) const
		{
			return Abs(GetLengthSq() - 1.0f) <= fEpsilon;
		}

		FORCEINLINE void NormaliseSet()
		{
			m_vVector = XMVector3Normalize(m_vVector);
		}

		FORCEINLINE IOEVector Normalise() const
		{
			IOEVector vOther(*this);
			vOther.m_vVector = XMVector3Normalize(m_vVector);
			return vOther;
		}

		FORCEINLINE float GetLength() const
		{
			return XMVectorGetX(XMVector3Length(m_vVector));
		}

		FORCEINLINE float GetLengthSq() const
		{
			return XMVectorGetX(XMVector3LengthSq(m_vVector));
		}

	public:
		FORCEINLINE static IOEVector Min(const IOEVector &lhs,
										 const IOEVector &rhs)
		{
			IOEVector vNew(lhs);
			vNew.MinSet(rhs);
			return vNew;
		}

		FORCEINLINE IOEVector Min(const IOEVector &rhs)
		{
			IOEVector vNew(*this);
			vNew.MinSet(rhs);
			return vNew;
		}

		FORCEINLINE void MinSet(const IOEVector &rhs)
		{
			m_vVector = XMVectorMin(m_vVector, rhs.m_vVector);
		}

		FORCEINLINE float MinElement(bool bIncludeW = false)
		{
			float fX, fY, fZ, fW;
			Get(fX, fY, fZ, fW);
			if (bIncludeW)
			{
				return Maths::Min(Maths::Min(Maths::Min(fX, fY), fZ), fW);
			}
			return Maths::Min(Maths::Min(fX, fY), fZ);
		}

		FORCEINLINE static IOEVector Max(const IOEVector &lhs,
										 const IOEVector &rhs)
		{
			IOEVector vNew(lhs);
			vNew.MaxSet(rhs);
			return vNew;
		}

		FORCEINLINE IOEVector Max(const IOEVector &rhs)
		{
			IOEVector vNew(*this);
			vNew.MaxSet(rhs);
			return vNew;
		}

		FORCEINLINE void MaxSet(const IOEVector &rhs)
		{
			m_vVector = XMVectorMax(m_vVector, rhs.m_vVector);
		}

		FORCEINLINE float MaxElement(bool bIncludeW = false)
		{
			float fX, fY, fZ, fW;
			Get(fX, fY, fZ, fW);
			if (bIncludeW)
			{
				return Maths::Max(Maths::Max(Maths::Max(fX, fY), fZ), fW);
			}
			return Maths::Max(Maths::Max(fX, fY), fZ);
		}

		FORCEINLINE static IOEVector Lerp(const IOEVector &lhs,
										  const IOEVector &rhs, float fDelta)
		{
			IOEVector vNew(lhs);
			vNew.LerpSet(rhs, fDelta);
			return vNew;
		}

		FORCEINLINE static IOEVector Lerp(const IOEVector &lhs,
										  const IOEVector &rhs,
										  const IOEVector &vDelta)
		{
			IOEVector vNew(lhs);
			vNew.LerpSet(rhs, vDelta);
			return vNew;
		}

		FORCEINLINE IOEVector Lerp(const IOEVector &rhs, float fDelta)
		{
			IOEVector vNew(*this);
			vNew.LerpSet(rhs, fDelta);
			return vNew;
		}

		FORCEINLINE IOEVector Lerp(const IOEVector &rhs,
								   const IOEVector &vDelta)
		{
			IOEVector vNew(*this);
			vNew.LerpSet(rhs, vDelta);
			return vNew;
		}

		FORCEINLINE IOEVector &LerpSet(const IOEVector &rhs, float fDelta)
		{
			m_vVector = XMVectorLerpV(m_vVector, rhs.m_vVector,
									  XMVectorReplicate(fDelta));
			return *this;
		}

		FORCEINLINE IOEVector &LerpSet(const IOEVector &rhs,
									   const IOEVector &vDelta)
		{
			m_vVector =
				XMVectorLerpV(m_vVector, rhs.m_vVector, vDelta.m_vVector);
			return *this;
		}

	public:
		FORCEINLINE static IOEVector Add(const IOEVector &lhs,
										 const IOEVector &rhs)
		{
			IOEVector vNew(lhs);
			vNew.Add(rhs);
			return vNew;
		}

		FORCEINLINE void Add(const IOEVector &rhs)
		{
			m_vVector = XMVectorAdd(m_vVector, rhs.m_vVector);
		}

		FORCEINLINE IOEVector operator+(const IOEVector &rhs) const
		{
			IOEVector vNew(*this);
			vNew += rhs;
			return vNew;
		}

		FORCEINLINE IOEVector &operator+=(const IOEVector &rhs)
		{
			Add(rhs);
			return *this;
		}

	public:
		FORCEINLINE static IOEVector Subtract(const IOEVector &lhs,
											  const IOEVector &rhs)
		{
			IOEVector vNew(lhs);
			vNew.Subtract(rhs);
			return vNew;
		}

		FORCEINLINE void Subtract(const IOEVector &rhs)
		{
			m_vVector = XMVectorSubtract(m_vVector, rhs.m_vVector);
		}

		FORCEINLINE IOEVector operator-(const IOEVector &rhs) const
		{
			IOEVector vNew(*this);
			vNew -= rhs;
			return vNew;
		}

		FORCEINLINE IOEVector operator-() const
		{
			return IOEVector(XMVectorNegate(m_vVector));
		}

		FORCEINLINE void Negate()
		{
			m_vVector = XMVectorNegate(m_vVector);
		}

		FORCEINLINE IOEVector &operator-=(const IOEVector &rhs)
		{
			Subtract(rhs);
			return *this;
		}

	public:
		FORCEINLINE static IOEVector Multiply(const IOEVector &lhs,
											  float fScalar)
		{
			IOEVector vNew(lhs);
			XMVECTOR vScalar(XMVectorReplicate(fScalar));
			vScalar		   = XMVectorSetW(vScalar, 1.0f);
			vNew.m_vVector = XMVectorMultiply(vNew.m_vVector, vScalar);
			return vNew;
		}

		FORCEINLINE void Multiply(float fScalar)
		{
			XMVECTOR vScalar(XMVectorReplicate(fScalar));
			vScalar   = XMVectorSetW(vScalar, 1.0f);
			m_vVector = XMVectorMultiply(m_vVector, vScalar);
		}

		FORCEINLINE static IOEVector Multiply(const IOEVector &lhs,
											  const IOEVector &vRHS)
		{
			IOEVector vNew(lhs);
			vNew.m_vVector = XMVectorMultiply(vNew.m_vVector, vRHS.m_vVector);
			return vNew;
		}

		FORCEINLINE void Multiply(const IOEVector &vRHS)
		{
			m_vVector = XMVectorMultiply(m_vVector, vRHS.m_vVector);
		}

		FORCEINLINE static IOEVector Divide(const IOEVector &lhs,
											float fScalar)
		{
			IOEVector vNew(lhs);
			XMVECTOR vScalar(XMVectorReplicate(1.0f / fScalar));
			vScalar		   = XMVectorSetW(vScalar, 1.0f);
			vNew.m_vVector = XMVectorMultiply(vNew.m_vVector, vScalar);
			return vNew;
		}

		FORCEINLINE static IOEVector Divide(const IOEVector &lhs,
											const IOEVector &rhs)
		{
			IOEVector vNew(lhs);
			vNew /= rhs;
			return vNew;
		}

		FORCEINLINE void Divide(float fScalar)
		{
			XMVECTOR vScalar(XMVectorReplicate(fScalar));
			m_vVector = XMVectorMultiply(m_vVector, vScalar);
		}

		FORCEINLINE void Divide(const IOEVector &rhs)
		{
			m_vVector = XMVectorDivide(m_vVector, rhs.m_vVector);
		}

		FORCEINLINE static float Dot(const IOEVector &lhs,
									 const IOEVector &rhs)
		{
			XMVECTOR vDot = XMVector3Dot(lhs.m_vVector, rhs.m_vVector);
			return XMVectorGetX(vDot);
		}

		FORCEINLINE float Dot(const IOEVector &rhs) const
		{
			XMVECTOR vDot = XMVector3Dot(m_vVector, rhs.m_vVector);
			return XMVectorGetX(vDot);
		}

		FORCEINLINE static IOEVector Cross(const IOEVector &lhs,
										   const IOEVector &rhs)
		{
			IOEVector vNew(lhs);
			vNew.CrossSet(rhs);
			return vNew;
		}

		FORCEINLINE IOEVector Cross(const IOEVector &rhs) const
		{
			IOEVector vNew(*this);
			vNew.CrossSet(rhs);
			return vNew;
		}

		FORCEINLINE void CrossSet(const IOEVector &rhs)
		{
			m_vVector = XMVector3Cross(m_vVector, rhs.m_vVector);
		}

		FORCEINLINE IOEVector operator*(const IOEVector &rhs) const
		{
			IOEVector vNew(*this);
			vNew.CrossSet(rhs);
			return vNew;
		}

		FORCEINLINE IOEVector &operator*=(const IOEVector &rhs)
		{
			CrossSet(rhs);
			return *this;
		}

		FORCEINLINE IOEVector operator*(float fScalar) const
		{
			IOEVector vNew(*this);
			vNew.Multiply(fScalar);
			return vNew;
		}

		FORCEINLINE IOEVector &operator*=(float fScalar)
		{
			Multiply(fScalar);
			return *this;
		}

		FORCEINLINE IOEVector operator/(float fScalar) const
		{
			IOEVector vNew(*this);
			vNew.Divide(fScalar);
			return vNew;
		}

		FORCEINLINE IOEVector &operator/=(float fScalar)
		{
			Divide(fScalar);
			return *this;
		}

		FORCEINLINE IOEVector operator/(const IOEVector &rhs) const
		{
			IOEVector vNew(*this);
			vNew /= rhs;
			return vNew;
		}

		FORCEINLINE IOEVector &operator/=(const IOEVector &rhs)
		{
			Divide(rhs);
			return *this;
		}

		IOEVector Multiply(const class IOEMatrix &rhs) const;
		void MultiplySet(const class IOEMatrix &rhs);
		IOEVector &operator*=(const class IOEMatrix &rhs);
		IOEVector operator*(const class IOEMatrix &rhs) const;

	private:
		FORCEINLINE void Load(const XMFLOAT4 &rRawData)
		{
			m_vVector = XMLoadFloat4(&rRawData);
		}

		FORCEINLINE void Store(XMFLOAT4 & rStore) const
		{
			XMStoreFloat4(&rStore, m_vVector);
		}

		FORCEINLINE void Load(const XMFLOAT3 &rRawData)
		{
			m_vVector = XMLoadFloat3(&rRawData);
		}

		FORCEINLINE void Store(XMFLOAT3 & rStore) const
		{
			XMStoreFloat3(&rStore, m_vVector);
		}

	private:
		XMVECTOR m_vVector;
	};

	//////////////////////////////////////////////////////////////////////////

	extern IOEVector g_vZeroVector;
	extern IOEVector g_vSideVector;
	extern IOEVector g_vUpVector;
	extern IOEVector g_vForwardVector;
	extern IOEVector g_vIdentity;
	extern IOEVector g_vOneVector;
	extern IOEVector g_vNegateVector;
	extern IOEVector g_vDegreesToRadians;
	extern IOEVector g_vRadiansToDegrees;
	extern IOEVector g_vEpsilon;
	extern IOEVector g_arrAxes[6];

	namespace ColourList // Taken from
						 // http://cloford.com/resources/colours/500col.htm
	{
		extern IOEVector Aliceblue;			   // (240, 248, 255)
		extern IOEVector Antiquewhite;		   // (250, 235, 215)
		extern IOEVector Antiquewhite1;		   // (255, 239, 219)
		extern IOEVector Antiquewhite2;		   // (238, 223, 204)
		extern IOEVector Antiquewhite3;		   // (205, 192, 176)
		extern IOEVector Antiquewhite4;		   // (139, 131, 120)
		extern IOEVector Aquamarine;		   // (127, 255, 212)
		extern IOEVector Aquamarine1;		   // (127, 255, 212)
		extern IOEVector Aquamarine2;		   // (118, 238, 198)
		extern IOEVector Aquamarine3;		   // (102, 205, 170)
		extern IOEVector Aquamarine4;		   // (69, 139, 116)
		extern IOEVector Azure;				   // (240, 255, 255)
		extern IOEVector Azure1;			   // (240, 255, 255)
		extern IOEVector Azure2;			   // (224, 238, 238)
		extern IOEVector Azure3;			   // (193, 205, 205)
		extern IOEVector Azure4;			   // (131, 139, 139)
		extern IOEVector Banana;			   // (227, 207, 87)
		extern IOEVector Beige;				   // (245, 245, 220)
		extern IOEVector Bisque;			   // (255, 228, 196)
		extern IOEVector Bisque1;			   // (255, 228, 196)
		extern IOEVector Bisque2;			   // (238, 213, 183)
		extern IOEVector Bisque3;			   // (205, 183, 158)
		extern IOEVector Bisque4;			   // (139, 125, 107)
		extern IOEVector Black;				   // (0, 0, 0)
		extern IOEVector Blanchedalmond;	   // (255, 235, 205)
		extern IOEVector Blue;				   // (0, 0, 255)
		extern IOEVector Blue2;				   // (0, 0, 238)
		extern IOEVector Blue3;				   // (0, 0, 205)
		extern IOEVector Blue4;				   // (0, 0, 139)
		extern IOEVector Blueviolet;		   // (138, 43, 226)
		extern IOEVector Brick;				   // (156, 102, 31)
		extern IOEVector Brown;				   // (165, 42, 42)
		extern IOEVector Brown1;			   // (255, 64, 64)
		extern IOEVector Brown2;			   // (238, 59, 59)
		extern IOEVector Brown3;			   // (205, 51, 51)
		extern IOEVector Brown4;			   // (139, 35, 35)
		extern IOEVector Burlywood;			   // (222, 184, 135)
		extern IOEVector Burlywood1;		   // (255, 211, 155)
		extern IOEVector Burlywood2;		   // (238, 197, 145)
		extern IOEVector Burlywood3;		   // (205, 170, 125)
		extern IOEVector Burlywood4;		   // (139, 115, 85)
		extern IOEVector Burntsienna;		   // (138, 54, 15)
		extern IOEVector Burntumber;		   // (138, 51, 36)
		extern IOEVector Cadetblue;			   // (95, 158, 160)
		extern IOEVector Cadetblue1;		   // (152, 245, 255)
		extern IOEVector Cadetblue2;		   // (142, 229, 238)
		extern IOEVector Cadetblue3;		   // (122, 197, 205)
		extern IOEVector Cadetblue4;		   // (83, 134, 139)
		extern IOEVector Cadmiumorange;		   // (255, 97, 3)
		extern IOEVector Cadmiumyellow;		   // (255, 153, 18)
		extern IOEVector Carrot;			   // (237, 145, 33)
		extern IOEVector Chartreuse;		   // (127, 255, 0)
		extern IOEVector Chartreuse1;		   // (127, 255, 0)
		extern IOEVector Chartreuse2;		   // (118, 238, 0)
		extern IOEVector Chartreuse3;		   // (102, 205, 0)
		extern IOEVector Chartreuse4;		   // (69, 139, 0)
		extern IOEVector Chocolate;			   // (210, 105, 30)
		extern IOEVector Chocolate1;		   // (255, 127, 36)
		extern IOEVector Chocolate2;		   // (238, 118, 33)
		extern IOEVector Chocolate3;		   // (205, 102, 29)
		extern IOEVector Chocolate4;		   // (139, 69, 19)
		extern IOEVector Cobalt;			   // (61, 89, 171)
		extern IOEVector Cobaltgreen;		   // (61, 145, 64)
		extern IOEVector Coldgrey;			   // (128, 138, 135)
		extern IOEVector Coral;				   // (255, 127, 80)
		extern IOEVector Coral1;			   // (255, 114, 86)
		extern IOEVector Coral2;			   // (238, 106, 80)
		extern IOEVector Coral3;			   // (205, 91, 69)
		extern IOEVector Coral4;			   // (139, 62, 47)
		extern IOEVector Cornflowerblue;	   // (100, 149, 237)
		extern IOEVector Cornsilk;			   // (255, 248, 220)
		extern IOEVector Cornsilk1;			   // (255, 248, 220)
		extern IOEVector Cornsilk2;			   // (238, 232, 205)
		extern IOEVector Cornsilk3;			   // (205, 200, 177)
		extern IOEVector Cornsilk4;			   // (139, 136, 120)
		extern IOEVector Crimson;			   // (220, 20, 60)
		extern IOEVector Cyan;				   // (0, 255, 255)
		extern IOEVector Cyan2;				   // (0, 238, 238)
		extern IOEVector Cyan3;				   // (0, 205, 205)
		extern IOEVector Cyan4;				   // (0, 139, 139)
		extern IOEVector Darkblue;			   // (0, 0, 139)
		extern IOEVector Darkcyan;			   // (0, 139, 139)
		extern IOEVector Darkgoldenrod;		   // (184, 134, 11)
		extern IOEVector Darkgoldenrod1;	   // (255, 185, 15)
		extern IOEVector Darkgoldenrod2;	   // (238, 173, 14)
		extern IOEVector Darkgoldenrod3;	   // (205, 149, 12)
		extern IOEVector Darkgoldenrod4;	   // (139, 101, 8)
		extern IOEVector Darkgray;			   // (169, 169, 169)
		extern IOEVector Darkgreen;			   // (0, 100, 0)
		extern IOEVector Darkkhaki;			   // (189, 183, 107)
		extern IOEVector Darkmagenta;		   // (139, 0, 139)
		extern IOEVector Darkolivegreen;	   // (85, 107, 47)
		extern IOEVector Darkolivegreen1;	  // (202, 255, 112)
		extern IOEVector Darkolivegreen2;	  // (188, 238, 104)
		extern IOEVector Darkolivegreen3;	  // (162, 205, 90)
		extern IOEVector Darkolivegreen4;	  // (110, 139, 61)
		extern IOEVector Darkorange;		   // (255, 140, 0)
		extern IOEVector Darkorange1;		   // (255, 127, 0)
		extern IOEVector Darkorange2;		   // (238, 118, 0)
		extern IOEVector Darkorange3;		   // (205, 102, 0)
		extern IOEVector Darkorange4;		   // (139, 69, 0)
		extern IOEVector Darkorchid;		   // (153, 50, 204)
		extern IOEVector Darkorchid1;		   // (191, 62, 255)
		extern IOEVector Darkorchid2;		   // (178, 58, 238)
		extern IOEVector Darkorchid3;		   // (154, 50, 205)
		extern IOEVector Darkorchid4;		   // (104, 34, 139)
		extern IOEVector Darkred;			   // (139, 0, 0)
		extern IOEVector Darksalmon;		   // (233, 150, 122)
		extern IOEVector Darkseagreen;		   // (143, 188, 143)
		extern IOEVector Darkseagreen1;		   // (193, 255, 193)
		extern IOEVector Darkseagreen2;		   // (180, 238, 180)
		extern IOEVector Darkseagreen3;		   // (155, 205, 155)
		extern IOEVector Darkseagreen4;		   // (105, 139, 105)
		extern IOEVector Darkslateblue;		   // (72, 61, 139)
		extern IOEVector Darkslategray;		   // (47, 79, 79)
		extern IOEVector Darkslategray1;	   // (151, 255, 255)
		extern IOEVector Darkslategray2;	   // (141, 238, 238)
		extern IOEVector Darkslategray3;	   // (121, 205, 205)
		extern IOEVector Darkslategray4;	   // (82, 139, 139)
		extern IOEVector Darkturquoise;		   // (0, 206, 209)
		extern IOEVector Darkviolet;		   // (148, 0, 211)
		extern IOEVector Deeppink;			   // (255, 20, 147)
		extern IOEVector Deeppink1;			   // (255, 20, 147)
		extern IOEVector Deeppink2;			   // (238, 18, 137)
		extern IOEVector Deeppink3;			   // (205, 16, 118)
		extern IOEVector Deeppink4;			   // (139, 10, 80)
		extern IOEVector Deepskyblue;		   // (0, 191, 255)
		extern IOEVector Deepskyblue1;		   // (0, 191, 255)
		extern IOEVector Deepskyblue2;		   // (0, 178, 238)
		extern IOEVector Deepskyblue3;		   // (0, 154, 205)
		extern IOEVector Deepskyblue4;		   // (0, 104, 139)
		extern IOEVector Dimgray;			   // (105, 105, 105)
		extern IOEVector Dodgerblue;		   // (30, 144, 255)
		extern IOEVector Dodgerblue1;		   // (30, 144, 255)
		extern IOEVector Dodgerblue2;		   // (28, 134, 238)
		extern IOEVector Dodgerblue3;		   // (24, 116, 205)
		extern IOEVector Dodgerblue4;		   // (16, 78, 139)
		extern IOEVector Eggshell;			   // (252, 230, 201)
		extern IOEVector Emeraldgreen;		   // (0, 201, 87)
		extern IOEVector Firebrick;			   // (178, 34, 34)
		extern IOEVector Firebrick1;		   // (255, 48, 48)
		extern IOEVector Firebrick2;		   // (238, 44, 44)
		extern IOEVector Firebrick3;		   // (205, 38, 38)
		extern IOEVector Firebrick4;		   // (139, 26, 26)
		extern IOEVector Flesh;				   // (255, 125, 64)
		extern IOEVector Floralwhite;		   // (255, 250, 240)
		extern IOEVector Forestgreen;		   // (34, 139, 34)
		extern IOEVector Fuchsia;			   // (255, 0, 255)
		extern IOEVector Gainsboro;			   // (220, 220, 220)
		extern IOEVector Ghostwhite;		   // (248, 248, 255)
		extern IOEVector Gold;				   // (255, 215, 0)
		extern IOEVector Gold1;				   // (255, 215, 0)
		extern IOEVector Gold2;				   // (238, 201, 0)
		extern IOEVector Gold3;				   // (205, 173, 0)
		extern IOEVector Gold4;				   // (139, 117, 0)
		extern IOEVector Goldenrod;			   // (218, 165, 32)
		extern IOEVector Goldenrod1;		   // (255, 193, 37)
		extern IOEVector Goldenrod2;		   // (238, 180, 34)
		extern IOEVector Goldenrod3;		   // (205, 155, 29)
		extern IOEVector Goldenrod4;		   // (139, 105, 20)
		extern IOEVector Green;				   // (0, 128, 0)
		extern IOEVector Green1;			   // (0, 255, 0)
		extern IOEVector Green2;			   // (0, 238, 0)
		extern IOEVector Green3;			   // (0, 205, 0)
		extern IOEVector Green4;			   // (0, 139, 0)
		extern IOEVector Greenyellow;		   // (173, 255, 47)
		extern IOEVector Grey;				   // (128, 128, 128)
		extern IOEVector Grey1;				   // (3, 3, 3)
		extern IOEVector Grey10;			   // (26, 26, 26)
		extern IOEVector Grey11;			   // (28, 28, 28)
		extern IOEVector Grey12;			   // (31, 31, 31)
		extern IOEVector Grey13;			   // (33, 33, 33)
		extern IOEVector Grey14;			   // (36, 36, 36)
		extern IOEVector Grey15;			   // (38, 38, 38)
		extern IOEVector Grey16;			   // (41, 41, 41)
		extern IOEVector Grey17;			   // (43, 43, 43)
		extern IOEVector Grey18;			   // (46, 46, 46)
		extern IOEVector Grey19;			   // (48, 48, 48)
		extern IOEVector Grey2;				   // (5, 5, 5)
		extern IOEVector Grey20;			   // (51, 51, 51)
		extern IOEVector Grey21;			   // (54, 54, 54)
		extern IOEVector Grey22;			   // (56, 56, 56)
		extern IOEVector Grey23;			   // (59, 59, 59)
		extern IOEVector Grey24;			   // (61, 61, 61)
		extern IOEVector Grey25;			   // (64, 64, 64)
		extern IOEVector Grey26;			   // (66, 66, 66)
		extern IOEVector Grey27;			   // (69, 69, 69)
		extern IOEVector Grey28;			   // (71, 71, 71)
		extern IOEVector Grey29;			   // (74, 74, 74)
		extern IOEVector Grey3;				   // (8, 8, 8)
		extern IOEVector Grey30;			   // (77, 77, 77)
		extern IOEVector Grey31;			   // (79, 79, 79)
		extern IOEVector Grey32;			   // (82, 82, 82)
		extern IOEVector Grey33;			   // (84, 84, 84)
		extern IOEVector Grey34;			   // (87, 87, 87)
		extern IOEVector Grey35;			   // (89, 89, 89)
		extern IOEVector Grey36;			   // (92, 92, 92)
		extern IOEVector Grey37;			   // (94, 94, 94)
		extern IOEVector Grey38;			   // (97, 97, 97)
		extern IOEVector Grey39;			   // (99, 99, 99)
		extern IOEVector Grey4;				   // (10, 10, 10)
		extern IOEVector Grey40;			   // (102, 102, 102)
		extern IOEVector Grey42;			   // (105, 105, 105)
		extern IOEVector Grey43;			   // (110, 110, 110)
		extern IOEVector Grey44;			   // (112, 112, 112)
		extern IOEVector Grey45;			   // (115, 115, 115)
		extern IOEVector Grey46;			   // (117, 117, 117)
		extern IOEVector Grey47;			   // (120, 120, 120)
		extern IOEVector Grey48;			   // (122, 122, 122)
		extern IOEVector Grey49;			   // (125, 125, 125)
		extern IOEVector Grey5;				   // (13, 13, 13)
		extern IOEVector Grey50;			   // (127, 127, 127)
		extern IOEVector Grey51;			   // (130, 130, 130)
		extern IOEVector Grey52;			   // (133, 133, 133)
		extern IOEVector Grey53;			   // (135, 135, 135)
		extern IOEVector Grey54;			   // (138, 138, 138)
		extern IOEVector Grey55;			   // (140, 140, 140)
		extern IOEVector Grey56;			   // (143, 143, 143)
		extern IOEVector Grey57;			   // (145, 145, 145)
		extern IOEVector Grey58;			   // (148, 148, 148)
		extern IOEVector Grey59;			   // (150, 150, 150)
		extern IOEVector Grey6;				   // (15, 15, 15)
		extern IOEVector Grey60;			   // (153, 153, 153)
		extern IOEVector Grey61;			   // (156, 156, 156)
		extern IOEVector Grey62;			   // (158, 158, 158)
		extern IOEVector Grey63;			   // (161, 161, 161)
		extern IOEVector Grey64;			   // (163, 163, 163)
		extern IOEVector Grey65;			   // (166, 166, 166)
		extern IOEVector Grey66;			   // (168, 168, 168)
		extern IOEVector Grey67;			   // (171, 171, 171)
		extern IOEVector Grey68;			   // (173, 173, 173)
		extern IOEVector Grey69;			   // (176, 176, 176)
		extern IOEVector Grey7;				   // (18, 18, 18)
		extern IOEVector Grey70;			   // (179, 179, 179)
		extern IOEVector Grey71;			   // (181, 181, 181)
		extern IOEVector Grey72;			   // (184, 184, 184)
		extern IOEVector Grey73;			   // (186, 186, 186)
		extern IOEVector Grey74;			   // (189, 189, 189)
		extern IOEVector Grey75;			   // (191, 191, 191)
		extern IOEVector Grey76;			   // (194, 194, 194)
		extern IOEVector Grey77;			   // (196, 196, 196)
		extern IOEVector Grey78;			   // (199, 199, 199)
		extern IOEVector Grey79;			   // (201, 201, 201)
		extern IOEVector Grey8;				   // (20, 20, 20)
		extern IOEVector Grey80;			   // (204, 204, 204)
		extern IOEVector Grey81;			   // (207, 207, 207)
		extern IOEVector Grey82;			   // (209, 209, 209)
		extern IOEVector Grey83;			   // (212, 212, 212)
		extern IOEVector Grey84;			   // (214, 214, 214)
		extern IOEVector Grey85;			   // (217, 217, 217)
		extern IOEVector Grey86;			   // (219, 219, 219)
		extern IOEVector Grey87;			   // (222, 222, 222)
		extern IOEVector Grey88;			   // (224, 224, 224)
		extern IOEVector Grey89;			   // (227, 227, 227)
		extern IOEVector Grey9;				   // (23, 23, 23)
		extern IOEVector Grey90;			   // (229, 229, 229)
		extern IOEVector Grey91;			   // (232, 232, 232)
		extern IOEVector Grey92;			   // (235, 235, 235)
		extern IOEVector Grey93;			   // (237, 237, 237)
		extern IOEVector Grey94;			   // (240, 240, 240)
		extern IOEVector Grey95;			   // (242, 242, 242)
		extern IOEVector Grey96;			   // (245, 245, 245)
		extern IOEVector Grey97;			   // (247, 247, 247)
		extern IOEVector Grey98;			   // (250, 250, 250)
		extern IOEVector Grey99;			   // (252, 252, 252)
		extern IOEVector Honeydew;			   // (240, 255, 240)
		extern IOEVector Honeydew1;			   // (240, 255, 240)
		extern IOEVector Honeydew2;			   // (224, 238, 224)
		extern IOEVector Honeydew3;			   // (193, 205, 193)
		extern IOEVector Honeydew4;			   // (131, 139, 131)
		extern IOEVector Hotpink;			   // (255, 105, 180)
		extern IOEVector Hotpink1;			   // (255, 110, 180)
		extern IOEVector Hotpink2;			   // (238, 106, 167)
		extern IOEVector Hotpink3;			   // (205, 96, 144)
		extern IOEVector Hotpink4;			   // (139, 58, 98)
		extern IOEVector IndianRed;			   // (176, 23, 31)
		extern IOEVector Indianred;			   // (205, 92, 92)
		extern IOEVector Indianred1;		   // (255, 106, 106)
		extern IOEVector Indianred2;		   // (238, 99, 99)
		extern IOEVector Indianred3;		   // (205, 85, 85)
		extern IOEVector Indianred4;		   // (139, 58, 58)
		extern IOEVector Indigo;			   // (75, 0, 130)
		extern IOEVector Ivory;				   // (255, 255, 240)
		extern IOEVector Ivory1;			   // (255, 255, 240)
		extern IOEVector Ivory2;			   // (238, 238, 224)
		extern IOEVector Ivory3;			   // (205, 205, 193)
		extern IOEVector Ivory4;			   // (139, 139, 131)
		extern IOEVector Ivoryblack;		   // (41, 36, 33)
		extern IOEVector Khaki;				   // (240, 230, 140)
		extern IOEVector Khaki1;			   // (255, 246, 143)
		extern IOEVector Khaki2;			   // (238, 230, 133)
		extern IOEVector Khaki3;			   // (205, 198, 115)
		extern IOEVector Khaki4;			   // (139, 134, 78)
		extern IOEVector Lavender;			   // (230, 230, 250)
		extern IOEVector Lavenderblush;		   // (255, 240, 245)
		extern IOEVector Lavenderblush1;	   // (255, 240, 245)
		extern IOEVector Lavenderblush2;	   // (238, 224, 229)
		extern IOEVector Lavenderblush3;	   // (205, 193, 197)
		extern IOEVector Lavenderblush4;	   // (139, 131, 134)
		extern IOEVector Lawngreen;			   // (124, 252, 0)
		extern IOEVector Lemonchiffon;		   // (255, 250, 205)
		extern IOEVector Lemonchiffon1;		   // (255, 250, 205)
		extern IOEVector Lemonchiffon2;		   // (238, 233, 191)
		extern IOEVector Lemonchiffon3;		   // (205, 201, 165)
		extern IOEVector Lemonchiffon4;		   // (139, 137, 112)
		extern IOEVector Lightblue;			   // (173, 216, 230)
		extern IOEVector Lightblue1;		   // (191, 239, 255)
		extern IOEVector Lightblue2;		   // (178, 223, 238)
		extern IOEVector Lightblue3;		   // (154, 192, 205)
		extern IOEVector Lightblue4;		   // (104, 131, 139)
		extern IOEVector Lightcoral;		   // (240, 128, 128)
		extern IOEVector Lightcyan;			   // (224, 255, 255)
		extern IOEVector Lightcyan1;		   // (224, 255, 255)
		extern IOEVector Lightcyan2;		   // (209, 238, 238)
		extern IOEVector Lightcyan3;		   // (180, 205, 205)
		extern IOEVector Lightcyan4;		   // (122, 139, 139)
		extern IOEVector Lightgoldenrod1;	  // (255, 236, 139)
		extern IOEVector Lightgoldenrod2;	  // (238, 220, 130)
		extern IOEVector Lightgoldenrod3;	  // (205, 190, 112)
		extern IOEVector Lightgoldenrod4;	  // (139, 129, 76)
		extern IOEVector Lightgoldenrodyellow; // (250, 250, 210)
		extern IOEVector Lightgreen;		   // (144, 238, 144)
		extern IOEVector Lightgrey;			   // (211, 211, 211)
		extern IOEVector Lightpink;			   // (255, 182, 193)
		extern IOEVector Lightpink1;		   // (255, 174, 185)
		extern IOEVector Lightpink2;		   // (238, 162, 173)
		extern IOEVector Lightpink3;		   // (205, 140, 149)
		extern IOEVector Lightpink4;		   // (139, 95, 101)
		extern IOEVector Lightsalmon;		   // (255, 160, 122)
		extern IOEVector Lightsalmon1;		   // (255, 160, 122)
		extern IOEVector Lightsalmon2;		   // (238, 149, 114)
		extern IOEVector Lightsalmon3;		   // (205, 129, 98)
		extern IOEVector Lightsalmon4;		   // (139, 87, 66)
		extern IOEVector Lightseagreen;		   // (32, 178, 170)
		extern IOEVector Lightskyblue;		   // (135, 206, 250)
		extern IOEVector Lightskyblue1;		   // (176, 226, 255)
		extern IOEVector Lightskyblue2;		   // (164, 211, 238)
		extern IOEVector Lightskyblue3;		   // (141, 182, 205)
		extern IOEVector Lightskyblue4;		   // (96, 123, 139)
		extern IOEVector Lightslateblue;	   // (132, 112, 255)
		extern IOEVector Lightslategray;	   // (119, 136, 153)
		extern IOEVector Lightsteelblue;	   // (176, 196, 222)
		extern IOEVector Lightsteelblue1;	  // (202, 225, 255)
		extern IOEVector Lightsteelblue2;	  // (188, 210, 238)
		extern IOEVector Lightsteelblue3;	  // (162, 181, 205)
		extern IOEVector Lightsteelblue4;	  // (110, 123, 139)
		extern IOEVector Lightyellow;		   // (255, 255, 224)
		extern IOEVector Lightyellow1;		   // (255, 255, 224)
		extern IOEVector Lightyellow2;		   // (238, 238, 209)
		extern IOEVector Lightyellow3;		   // (205, 205, 180)
		extern IOEVector Lightyellow4;		   // (139, 139, 122)
		extern IOEVector Lime;				   // (0, 255, 0)
		extern IOEVector Limegreen;			   // (50, 205, 50)
		extern IOEVector Linen;				   // (250, 240, 230)
		extern IOEVector Magenta;			   // (255, 0, 255)
		extern IOEVector Magenta2;			   // (238, 0, 238)
		extern IOEVector Magenta3;			   // (205, 0, 205)
		extern IOEVector Magenta4;			   // (139, 0, 139)
		extern IOEVector Manganeseblue;		   // (3, 168, 158)
		extern IOEVector Maroon;			   // (128, 0, 0)
		extern IOEVector Maroon1;			   // (255, 52, 179)
		extern IOEVector Maroon2;			   // (238, 48, 167)
		extern IOEVector Maroon3;			   // (205, 41, 144)
		extern IOEVector Maroon4;			   // (139, 28, 98)
		extern IOEVector Mediumaquamarine;	 // (102, 205, 170)
		extern IOEVector Mediumblue;		   // (0, 0, 205)
		extern IOEVector Mediumorchid;		   // (186, 85, 211)
		extern IOEVector Mediumorchid1;		   // (224, 102, 255)
		extern IOEVector Mediumorchid2;		   // (209, 95, 238)
		extern IOEVector Mediumorchid3;		   // (180, 82, 205)
		extern IOEVector Mediumorchid4;		   // (122, 55, 139)
		extern IOEVector Mediumpurple;		   // (147, 112, 219)
		extern IOEVector Mediumpurple1;		   // (171, 130, 255)
		extern IOEVector Mediumpurple2;		   // (159, 121, 238)
		extern IOEVector Mediumpurple3;		   // (137, 104, 205)
		extern IOEVector Mediumpurple4;		   // (93, 71, 139)
		extern IOEVector Mediumseagreen;	   // (60, 179, 113)
		extern IOEVector Mediumslateblue;	  // (123, 104, 238)
		extern IOEVector Mediumspringgreen;	// (0, 250, 154)
		extern IOEVector Mediumturquoise;	  // (72, 209, 204)
		extern IOEVector Mediumvioletred;	  // (199, 21, 133)
		extern IOEVector Melon;				   // (227, 168, 105)
		extern IOEVector Midnightblue;		   // (25, 25, 112)
		extern IOEVector Mint;				   // (189, 252, 201)
		extern IOEVector Mintcream;			   // (245, 255, 250)
		extern IOEVector Mistyrose;			   // (255, 228, 225)
		extern IOEVector Mistyrose1;		   // (255, 228, 225)
		extern IOEVector Mistyrose2;		   // (238, 213, 210)
		extern IOEVector Mistyrose3;		   // (205, 183, 181)
		extern IOEVector Mistyrose4;		   // (139, 125, 123)
		extern IOEVector Moccasin;			   // (255, 228, 181)
		extern IOEVector Navajowhite;		   // (255, 222, 173)
		extern IOEVector Navajowhite1;		   // (255, 222, 173)
		extern IOEVector Navajowhite2;		   // (238, 207, 161)
		extern IOEVector Navajowhite3;		   // (205, 179, 139)
		extern IOEVector Navajowhite4;		   // (139, 121, 94)
		extern IOEVector Navy;				   // (0, 0, 128)
		extern IOEVector Oldlace;			   // (253, 245, 230)
		extern IOEVector Olive;				   // (128, 128, 0)
		extern IOEVector Olivedrab;			   // (107, 142, 35)
		extern IOEVector Olivedrab1;		   // (192, 255, 62)
		extern IOEVector Olivedrab2;		   // (179, 238, 58)
		extern IOEVector Olivedrab3;		   // (154, 205, 50)
		extern IOEVector Olivedrab4;		   // (105, 139, 34)
		extern IOEVector Orange;			   // (255, 165, 0)
		extern IOEVector Orange1;			   // (255, 165, 0)
		extern IOEVector Orange2;			   // (238, 154, 0)
		extern IOEVector Orange3;			   // (205, 133, 0)
		extern IOEVector Orange4;			   // (139, 90, 0)
		extern IOEVector Orangered;			   // (255, 69, 0)
		extern IOEVector Orangered1;		   // (255, 69, 0)
		extern IOEVector Orangered2;		   // (238, 64, 0)
		extern IOEVector Orangered3;		   // (205, 55, 0)
		extern IOEVector Orangered4;		   // (139, 37, 0)
		extern IOEVector Orchid;			   // (218, 112, 214)
		extern IOEVector Orchid1;			   // (255, 131, 250)
		extern IOEVector Orchid2;			   // (238, 122, 233)
		extern IOEVector Orchid3;			   // (205, 105, 201)
		extern IOEVector Orchid4;			   // (139, 71, 137)
		extern IOEVector Palegoldenrod;		   // (238, 232, 170)
		extern IOEVector Palegreen;			   // (152, 251, 152)
		extern IOEVector Palegreen1;		   // (154, 255, 154)
		extern IOEVector Palegreen2;		   // (144, 238, 144)
		extern IOEVector Palegreen3;		   // (124, 205, 124)
		extern IOEVector Palegreen4;		   // (84, 139, 84)
		extern IOEVector Paleturquoise;		   // (174, 238, 238)
		extern IOEVector Paleturquoise1;	   // (187, 255, 255)
		extern IOEVector Paleturquoise2;	   // (174, 238, 238)
		extern IOEVector Paleturquoise3;	   // (150, 205, 205)
		extern IOEVector Paleturquoise4;	   // (102, 139, 139)
		extern IOEVector Palevioletred;		   // (219, 112, 147)
		extern IOEVector Palevioletred1;	   // (255, 130, 171)
		extern IOEVector Palevioletred2;	   // (238, 121, 159)
		extern IOEVector Palevioletred3;	   // (205, 104, 137)
		extern IOEVector Palevioletred4;	   // (139, 71, 93)
		extern IOEVector Papayawhip;		   // (255, 239, 213)
		extern IOEVector Peachpuff;			   // (255, 218, 185)
		extern IOEVector Peachpuff1;		   // (255, 218, 185)
		extern IOEVector Peachpuff2;		   // (238, 203, 173)
		extern IOEVector Peachpuff3;		   // (205, 175, 149)
		extern IOEVector Peachpuff4;		   // (139, 119, 101)
		extern IOEVector Peacock;			   // (51, 161, 201)
		extern IOEVector Peru;				   // (205, 133, 63)
		extern IOEVector Pink;				   // (255, 192, 203)
		extern IOEVector Pink1;				   // (255, 181, 197)
		extern IOEVector Pink2;				   // (238, 169, 184)
		extern IOEVector Pink3;				   // (205, 145, 158)
		extern IOEVector Pink4;				   // (139, 99, 108)
		extern IOEVector Plum;				   // (221, 160, 221)
		extern IOEVector Plum1;				   // (255, 187, 255)
		extern IOEVector Plum2;				   // (238, 174, 238)
		extern IOEVector Plum3;				   // (205, 150, 205)
		extern IOEVector Plum4;				   // (139, 102, 139)
		extern IOEVector Powderblue;		   // (176, 224, 230)
		extern IOEVector Purple;			   // (128, 0, 128)
		extern IOEVector Purple1;			   // (155, 48, 255)
		extern IOEVector Purple2;			   // (145, 44, 238)
		extern IOEVector Purple3;			   // (125, 38, 205)
		extern IOEVector Purple4;			   // (85, 26, 139)
		extern IOEVector Raspberry;			   // (135, 38, 87)
		extern IOEVector Rawsienna;			   // (199, 97, 20)
		extern IOEVector Red;				   // (255, 0, 0)
		extern IOEVector Red1;				   // (255, 0, 0)
		extern IOEVector Red2;				   // (238, 0, 0)
		extern IOEVector Red3;				   // (205, 0, 0)
		extern IOEVector Red4;				   // (139, 0, 0)
		extern IOEVector Rosybrown;			   // (188, 143, 143)
		extern IOEVector Rosybrown1;		   // (255, 193, 193)
		extern IOEVector Rosybrown2;		   // (238, 180, 180)
		extern IOEVector Rosybrown3;		   // (205, 155, 155)
		extern IOEVector Rosybrown4;		   // (139, 105, 105)
		extern IOEVector Royalblue;			   // (65, 105, 225)
		extern IOEVector Royalblue1;		   // (72, 118, 255)
		extern IOEVector Royalblue2;		   // (67, 110, 238)
		extern IOEVector Royalblue3;		   // (58, 95, 205)
		extern IOEVector Royalblue4;		   // (39, 64, 139)
		extern IOEVector Saddlebrown;		   // (139, 69, 19)
		extern IOEVector Salmon;			   // (250, 128, 114)
		extern IOEVector Salmon1;			   // (255, 140, 105)
		extern IOEVector Salmon2;			   // (238, 130, 98)
		extern IOEVector Salmon3;			   // (205, 112, 84)
		extern IOEVector Salmon4;			   // (139, 76, 57)
		extern IOEVector Sandybrown;		   // (244, 164, 96)
		extern IOEVector Sapgreen;			   // (48, 128, 20)
		extern IOEVector Seagreen;			   // (46, 139, 87)
		extern IOEVector Seagreen1;			   // (84, 255, 159)
		extern IOEVector Seagreen2;			   // (78, 238, 148)
		extern IOEVector Seagreen3;			   // (67, 205, 128)
		extern IOEVector Seagreen4;			   // (46, 139, 87)
		extern IOEVector Seashell;			   // (255, 245, 238)
		extern IOEVector Seashell1;			   // (255, 245, 238)
		extern IOEVector Seashell2;			   // (238, 229, 222)
		extern IOEVector Seashell3;			   // (205, 197, 191)
		extern IOEVector Seashell4;			   // (139, 134, 130)
		extern IOEVector Sepia;				   // (94, 38, 18)
		extern IOEVector SgiBeet;			   // (142, 56, 142)
		extern IOEVector SgiBrightgray;		   // (197, 193, 170)
		extern IOEVector SgiChartreuse;		   // (113, 198, 113)
		extern IOEVector SgiDarkgray;		   // (85, 85, 85)
		extern IOEVector SgiGrey12;			   // (30, 30, 30)
		extern IOEVector SgiGrey16;			   // (40, 40, 40)
		extern IOEVector SgiGrey32;			   // (81, 81, 81)
		extern IOEVector SgiGrey36;			   // (91, 91, 91)
		extern IOEVector SgiGrey52;			   // (132, 132, 132)
		extern IOEVector SgiGrey56;			   // (142, 142, 142)
		extern IOEVector SgiGrey72;			   // (183, 183, 183)
		extern IOEVector SgiGrey76;			   // (193, 193, 193)
		extern IOEVector SgiGrey92;			   // (234, 234, 234)
		extern IOEVector SgiGrey96;			   // (244, 244, 244)
		extern IOEVector SgiLightblue;		   // (125, 158, 192)
		extern IOEVector SgiLightgray;		   // (170, 170, 170)
		extern IOEVector SgiOlivedrab;		   // (142, 142, 56)
		extern IOEVector SgiSalmon;			   // (198, 113, 113)
		extern IOEVector SgiSlateblue;		   // (113, 113, 198)
		extern IOEVector SgiTeal;			   // (56, 142, 142)
		extern IOEVector Sienna;			   // (160, 82, 45)
		extern IOEVector Sienna1;			   // (255, 130, 71)
		extern IOEVector Sienna2;			   // (238, 121, 66)
		extern IOEVector Sienna3;			   // (205, 104, 57)
		extern IOEVector Sienna4;			   // (139, 71, 38)
		extern IOEVector Silver;			   // (192, 192, 192)
		extern IOEVector Skyblue;			   // (135, 206, 235)
		extern IOEVector Skyblue1;			   // (135, 206, 255)
		extern IOEVector Skyblue2;			   // (126, 192, 238)
		extern IOEVector Skyblue3;			   // (108, 166, 205)
		extern IOEVector Skyblue4;			   // (74, 112, 139)
		extern IOEVector Slateblue;			   // (106, 90, 205)
		extern IOEVector Slateblue1;		   // (131, 111, 255)
		extern IOEVector Slateblue2;		   // (122, 103, 238)
		extern IOEVector Slateblue3;		   // (105, 89, 205)
		extern IOEVector Slateblue4;		   // (71, 60, 139)
		extern IOEVector Slategray;			   // (112, 128, 144)
		extern IOEVector Slategray1;		   // (198, 226, 255)
		extern IOEVector Slategray2;		   // (185, 211, 238)
		extern IOEVector Slategray3;		   // (159, 182, 205)
		extern IOEVector Slategray4;		   // (108, 123, 139)
		extern IOEVector Snow;				   // (255, 250, 250)
		extern IOEVector Snow1;				   // (255, 250, 250)
		extern IOEVector Snow2;				   // (238, 233, 233)
		extern IOEVector Snow3;				   // (205, 201, 201)
		extern IOEVector Snow4;				   // (139, 137, 137)
		extern IOEVector Springgreen;		   // (0, 255, 127)
		extern IOEVector Springgreen1;		   // (0, 238, 118)
		extern IOEVector Springgreen2;		   // (0, 205, 102)
		extern IOEVector Springgreen3;		   // (0, 139, 69)
		extern IOEVector Steelblue;			   // (70, 130, 180)
		extern IOEVector Steelblue1;		   // (99, 184, 255)
		extern IOEVector Steelblue2;		   // (92, 172, 238)
		extern IOEVector Steelblue3;		   // (79, 148, 205)
		extern IOEVector Steelblue4;		   // (54, 100, 139)
		extern IOEVector Tan;				   // (210, 180, 140)
		extern IOEVector Tan1;				   // (255, 165, 79)
		extern IOEVector Tan2;				   // (238, 154, 73)
		extern IOEVector Tan3;				   // (205, 133, 63)
		extern IOEVector Tan4;				   // (139, 90, 43)
		extern IOEVector Teal;				   // (0, 128, 128)
		extern IOEVector Thistle;			   // (216, 191, 216)
		extern IOEVector Thistle1;			   // (255, 225, 255)
		extern IOEVector Thistle2;			   // (238, 210, 238)
		extern IOEVector Thistle3;			   // (205, 181, 205)
		extern IOEVector Thistle4;			   // (139, 123, 139)
		extern IOEVector Tomato;			   // (255, 99, 71)
		extern IOEVector Tomato1;			   // (255, 99, 71)
		extern IOEVector Tomato2;			   // (238, 92, 66)
		extern IOEVector Tomato3;			   // (205, 79, 57)
		extern IOEVector Tomato4;			   // (139, 54, 38)
		extern IOEVector Turquoise;			   // (64, 224, 208)
		extern IOEVector Turquoise1;		   // (0, 245, 255)
		extern IOEVector Turquoise2;		   // (0, 229, 238)
		extern IOEVector Turquoise3;		   // (0, 197, 205)
		extern IOEVector Turquoise4;		   // (0, 134, 139)
		extern IOEVector Turquoiseblue;		   // (0, 199, 140)
		extern IOEVector Violet;			   // (238, 130, 238)
		extern IOEVector Violetred;			   // (208, 32, 144)
		extern IOEVector Violetred1;		   // (255, 62, 150)
		extern IOEVector Violetred2;		   // (238, 58, 140)
		extern IOEVector Violetred3;		   // (205, 50, 120)
		extern IOEVector Violetred4;		   // (139, 34, 82)
		extern IOEVector Warmgrey;			   // (128, 128, 105)
		extern IOEVector Wheat;				   // (245, 222, 179)
		extern IOEVector Wheat1;			   // (255, 231, 186)
		extern IOEVector Wheat2;			   // (238, 216, 174)
		extern IOEVector Wheat3;			   // (205, 186, 150)
		extern IOEVector Wheat4;			   // (139, 126, 102)
		extern IOEVector White;				   // (255, 255, 255)
		extern IOEVector WhiteSmoke;		   // (245, 245, 245)
		extern IOEVector Yellow;			   // (255, 255, 0)
		extern IOEVector Yellow1;			   // (255, 255, 0)
		extern IOEVector Yellow2;			   // (238, 238, 0)
		extern IOEVector Yellow3;			   // (205, 205, 0)
		extern IOEVector Yellow4;			   // (139, 139, 0)
		extern IOEVector Yellowgreen;		   // (154, 205, 50)
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE