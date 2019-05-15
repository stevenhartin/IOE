#pragma once

#include <limits>
#include <utility>

#include <DirectXMath.h>
#include "Vector.h"

using namespace DirectX;

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	class alignas(16) IOEMatrix
	{
	public:
		FORCEINLINE IOEMatrix()
		{
		}

		FORCEINLINE IOEMatrix(const IOEMatrix &rhs)
		{
			m_mMatrix = rhs.m_mMatrix;
		}

		FORCEINLINE IOEMatrix(IOEMatrix && rhs)
		{
			m_mMatrix = std::move(rhs.m_mMatrix);
		}

		FORCEINLINE IOEMatrix(const XMMATRIX &mMatrix)
			: m_mMatrix(mMatrix)
		{
		}

		FORCEINLINE IOEMatrix(XMMATRIX && mMatrix)
			: m_mMatrix(std::move(mMatrix))
		{
		}

		FORCEINLINE IOEMatrix(const IOEVector &vCol0, const IOEVector &vCol1,
							  const IOEVector &vCol2, const IOEVector &vCol3)
		{
			Set(vCol0, vCol1, vCol2, vCol3);
		}

		FORCEINLINE IOEMatrix(float f00, float f01, float f02, float f03,
							  float f10, float f11, float f12, float f13,
							  float f20, float f21, float f22, float f23,
							  float f30, float f31, float f32, float f33)
		{
			Set(f00, f01, f02, f03, f10, f11, f12, f13, f20, f21, f22, f23,
				f30, f31, f32, f33);
		}

		FORCEINLINE IOEMatrix &operator=(const IOEMatrix &rhs)
		{
			m_mMatrix = rhs.m_mMatrix;
			return *this;
		}

		FORCEINLINE IOEMatrix &operator=(IOEMatrix && rhs)
		{
			m_mMatrix = std::move(rhs.m_mMatrix);
			return *this;
		}

		FORCEINLINE IOEMatrix &operator=(const XMMATRIX &mMatrix)
		{
			m_mMatrix = mMatrix;
			return *this;
		}

		FORCEINLINE IOEMatrix &operator=(XMMATRIX && mMatrix)
		{
			m_mMatrix = std::move(mMatrix);
			return *this;
		}

		const XMMATRIX &GetRaw() const
		{
			return m_mMatrix;
		}

	public:
		FORCEINLINE void Set(const IOEVector &vCol0, const IOEVector &vCol1,
							 const IOEVector &vCol2, const IOEVector &vCol3)
		{
			m_mMatrix = XMMATRIX(vCol0.GetRaw(), vCol1.GetRaw(),
								 vCol2.GetRaw(), vCol3.GetRaw());
		}

		FORCEINLINE void Set(float f00, float f01, float f02, float f03,
							 float f10, float f11, float f12, float f13,
							 float f20, float f21, float f22, float f23,
							 float f30, float f31, float f32, float f33)
		{
			m_mMatrix = XMMatrixSet(f00, f01, f02, f03, f10, f11, f12, f13,
									f20, f21, f22, f23, f30, f31, f32, f33);
		}

		FORCEINLINE void SetColumn(std::int32_t nColIdx,
								   const IOEVector &vColumn)
		{
			IOE_ASSERT(nColIdx >= 0 && nColIdx < 4,
					   "Column must be between 0 and 4");
			m_mMatrix.r[nColIdx] = vColumn.GetRaw();
		}

		FORCEINLINE void SetCell(std::int32_t nColIdx, std::int32_t nRowIdx,
								 float fValue)
		{
			m_mMatrix.r[nColIdx] =
				XMVectorSetByIndex(m_mMatrix.r[nColIdx], fValue, nRowIdx);
		}

		FORCEINLINE float GetCell(std::int32_t nColIdx, std::int32_t nRowIdx)
			const
		{
			return XMVectorGetByIndex(GetColumn(nColIdx).GetRaw(), nRowIdx);
		}

		FORCEINLINE void Get(float &f00, float &f01, float &f02, float &f03,
							 float &f10, float &f11, float &f12, float &f13,
							 float &f20, float &f21, float &f22, float &f23,
							 float &f30, float &f31, float &f32, float &f33)
			const
		{
			GetColumn(0).Get(f00, f01, f02, f03);
			GetColumn(1).Get(f10, f11, f12, f13);
			GetColumn(2).Get(f20, f21, f22, f23);
			GetColumn(3).Get(f30, f31, f32, f33);
		}

		FORCEINLINE void GetColumns(IOEVector & vCol0, IOEVector & vCol1,
									IOEVector & vCol2, IOEVector & vCol3) const
		{
			vCol0 = m_mMatrix.r[0];
			vCol1 = m_mMatrix.r[1];
			vCol2 = m_mMatrix.r[2];
			vCol3 = m_mMatrix.r[3];
		}

		FORCEINLINE IOEVector GetColumn(std::int32_t nColIdx) const
		{
			IOE_ASSERT(nColIdx >= 0 && nColIdx < 4,
					   "Column must be between 0 and 4");
			return IOEVector(m_mMatrix.r[nColIdx]);
		}

	public:
		FORCEINLINE IOEMatrix Inverse() const
		{
			return IOEMatrix(XMMatrixInverse(nullptr, m_mMatrix));
		}

		FORCEINLINE IOEMatrix &InverseSet()
		{
			m_mMatrix = XMMatrixInverse(nullptr, m_mMatrix);
			return *this;
		}

		FORCEINLINE IOEMatrix Transpose() const
		{
			return IOEMatrix(XMMatrixTranspose(m_mMatrix));
		}

		FORCEINLINE IOEMatrix &TransposeSet()
		{
			m_mMatrix = XMMatrixTranspose(m_mMatrix);
			return *this;
		}

	public:
		FORCEINLINE static IOEMatrix Multiply(const IOEMatrix &lhs,
											  const IOEMatrix &rhs)
		{
			IOEMatrix mNew(lhs);
			mNew.MultiplySet(rhs);
			return mNew;
		}

		FORCEINLINE IOEMatrix Multiply(const IOEMatrix &rhs) const
		{
			IOEMatrix mNew(*this);
			mNew.MultiplySet(rhs);
			return mNew;
		}

		FORCEINLINE void MultiplySet(const IOEMatrix &rhs)
		{
			m_mMatrix = XMMatrixMultiply(m_mMatrix, rhs.m_mMatrix);
		}

		FORCEINLINE static IOEMatrix MultiplyTranspose(const IOEMatrix &lhs,
													   const IOEMatrix &rhs)
		{
			IOEMatrix mNew(lhs);
			mNew.MultiplyTransposeSet(rhs);
			return mNew;
		}

		FORCEINLINE IOEMatrix MultiplyTranspose(const IOEMatrix &rhs) const
		{
			IOEMatrix mNew;
			mNew.m_mMatrix =
				XMMatrixMultiplyTranspose(m_mMatrix, rhs.m_mMatrix);
			return mNew;
		}

		FORCEINLINE void MultiplyTransposeSet(const IOEMatrix &rhs)
		{
			m_mMatrix = XMMatrixMultiplyTranspose(m_mMatrix, rhs.m_mMatrix);
		}

		FORCEINLINE IOEMatrix operator*(const IOEMatrix &rhs) const
		{
			return Multiply(rhs);
		}

		FORCEINLINE IOEMatrix &operator*=(const IOEMatrix &rhs)
		{
			MultiplySet(rhs);
			return *this;
		}

		FORCEINLINE IOEVector Multiply(const IOEVector &rhs) const
		{
			return IOEVector(XMVector4Transform(rhs.GetRaw(), m_mMatrix));
		}

		FORCEINLINE IOEVector operator*(const IOEVector &rhs) const
		{
			return Multiply(rhs);
		}

	public:
		FORCEINLINE static IOEMatrix Add(const IOEMatrix &lhs,
										 const IOEMatrix &rhs)
		{
			return IOEMatrix(lhs.m_mMatrix + rhs.m_mMatrix);
		}

		FORCEINLINE IOEMatrix Add(const IOEMatrix &rhs)
		{
			return IOEMatrix(m_mMatrix + rhs.m_mMatrix);
		}

		FORCEINLINE IOEMatrix &AddSet(const IOEMatrix &rhs)
		{
			m_mMatrix += rhs.m_mMatrix;
			return *this;
		}

		FORCEINLINE IOEMatrix operator+(const IOEMatrix &rhs)
		{
			return Add(rhs);
		}

		FORCEINLINE IOEMatrix &operator+=(const IOEMatrix &rhs)
		{
			return AddSet(rhs);
		}

	public:
		FORCEINLINE static IOEMatrix Subtract(const IOEMatrix &lhs,
											  const IOEMatrix &rhs)
		{
			return IOEMatrix(lhs.m_mMatrix - rhs.m_mMatrix);
		}

		FORCEINLINE IOEMatrix Subtract(const IOEMatrix &rhs)
		{
			return IOEMatrix(m_mMatrix - rhs.m_mMatrix);
		}

		FORCEINLINE IOEMatrix &SubtractSet(const IOEMatrix &rhs)
		{
			m_mMatrix -= rhs.m_mMatrix;
			return *this;
		}

		FORCEINLINE IOEMatrix operator-(const IOEMatrix &rhs)
		{
			return Subtract(rhs);
		}

		FORCEINLINE IOEMatrix &operator-=(const IOEMatrix &rhs)
		{
			return SubtractSet(rhs);
		}

	public:
		static IOEMatrix LookAt(const IOEVector &vEye,
								const IOEVector &vTarget,
								const IOEVector &vUp = g_vUpVector);
		static IOEMatrix Perspective(float fFOV, float fAspect = -1.0f,
									 float fNear = 0.5f, float fFar = 1000.0f);
		static IOEMatrix Orthographic(
			float fWidth  = std::numeric_limits<float>::infinity(),
			float fHeight = std::numeric_limits<float>::infinity(),
			float fNear = 0.0f, float fFar = 1.0f);

		static IOEMatrix Translation(const IOEVector &vTranslation);
		static IOEMatrix Scale(const IOEVector &vScale);

		static IOEMatrix RotationX(float fRotationRad);
		static IOEMatrix RotationXDeg(float fRotationDeg);

		static IOEMatrix RotationY(float fRotationRad);
		static IOEMatrix RotationYDeg(float fRotationDeg);

		static IOEMatrix RotationZ(float fRotationRad);
		static IOEMatrix RotationZDeg(float fRotationDeg);

	private:
		XMMATRIX m_mMatrix;
	};

	//////////////////////////////////////////////////////////////////////////

	extern IOEMatrix g_mIdentity;
	extern IOEMatrix g_mZero;

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE