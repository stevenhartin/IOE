#include "IOEShader_Platform.h"

#include <IOE/IOECore/IOEManagerContainer.h>

#include <IOE/IOEExceptions/IOEException_Enum.h>
#include <IOE/IOEExceptions/IOEException_Renderer.h>

#include EXTERNAL_PLATFORM_INCLUDE(IOECore, IOEPlatformStatics_Platform.h)
#include <IOE/IOECore/Algorithms/IOEStringHelpers.h>
#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOERenderer/IOEModel.h>
#include <IOE/IOERenderer/IOERPI.h>
#include <IOE/IOERenderer/IOEShader.h>

#include <D3Dcompiler.h>

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOEShader_Platform::IOEShader_Platform(IOEShader &rShader)
		: IOEShader_PlatformBase(rShader)
		, m_uNumCBuffers(0)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::Compile(const IOEShaderCompilationOptions &rOptions)
	{
		std::cout << "Compiling shader: " << IOE::Core::Algorithm::ConvertWideToNarrow(rOptions.szPath).c_str()
#if (IOE_SHADER_DEBUG == IOE_ON)
				  << " With full debug support"
#endif
				  << std::endl;

		Cleanup();
		m_rShader.SetVertexFormat(rOptions.pVertexFormat);
		if (rOptions.pMacros != nullptr && rOptions.pMacros->size() > 0)
		{
			std::copy(std::begin(*rOptions.pMacros), std::end(*rOptions.pMacros), std::back_inserter(m_arrMacros));
		}
		D3D_SHADER_MACRO *pMacros(nullptr);
		std::vector<D3D_SHADER_MACRO> arrMacros;
		std::size_t nNumMacros(GetMacros().size());
		if (nNumMacros > 0)
		{
			// + 1 for null terminator
			arrMacros.resize(nNumMacros + 1);

			for (std::size_t nIdx(0); nIdx < nNumMacros; ++nIdx)
			{
				arrMacros[nIdx].Name	   = GetMacros().at(nIdx).m_szName.c_str();
				arrMacros[nIdx].Definition = GetMacros().at(nIdx).m_szDefinition.c_str();
			}
			// Last macro must be null terminated (HACKY DIRECTX!)
			arrMacros[nNumMacros].Name		 = nullptr;
			arrMacros[nNumMacros].Definition = nullptr;
			pMacros							 = arrMacros.data();
		}
		std::wstring szPath(IOE::Core::g_pEngine->GetMediaPath() +
							IOE::Core::IOEPlatformStatics_Platform::GetPathSplit() + rOptions.szPath);
		TComSharedPtr<ID3DBlob> pErrors;

		HRESULT tResult(D3DCompileFromFile(szPath.c_str(), pMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE,
										   rOptions.szEntryPoint.c_str(), rOptions.szProfile.c_str(),

#if (IOE_SHADER_DEBUG == IOE_ON)
										   D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION |
											   D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL0 |
#else
										   D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_AVOID_FLOW_CONTROL |
#endif
											   D3DCOMPILE_WARNINGS_ARE_ERRORS,
										   0, &m_pCompiledShader, &pErrors));
		if (FAILED(tResult))
		{
			static const std::uint32_t uBufferSize = 1024 * 4;
			char buffer[uBufferSize];
			strncpy_s<uBufferSize>(buffer,
								   pErrors != nullptr ? static_cast<char *>(pErrors->GetBufferPointer())
													  : "No output from shader compiler",
								   uBufferSize);

			THROW_IOE_RENDERER_EXCEPTION_VARGS("Failed to compile shader\n%s", E_ERROR_FAILED_TO_COMPILE_SHADER,
											   buffer);
		}

		auto pPtr = IOERPI::GetSingletonPtr();
		IOERPI_Platform &rPlatform(pPtr->GetPlatform());

		tResult = E_FAIL;

		switch (rOptions.eShaderType)
		{
		case EShaderType::Vertex:
			tResult = rPlatform.GetDevice()->CreateVertexShader(
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), nullptr,
				reinterpret_cast<ID3D11VertexShader **>(&m_pCreatedShader));
			break;
		case EShaderType::Geometry:
			// tResult =
			// rPlatform.GetDevice()->CreateGeometryShaderWithStreamOutput(m_pCompiledShader->GetBufferPointer(),
			// m_pCompiledShader->GetBufferSize(), nullptr,
			// reinterpret_cast<ID3D11GeometryShader**>(&m_pCreatedShader));
			tResult = rPlatform.GetDevice()->CreateGeometryShader(
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), nullptr,
				reinterpret_cast<ID3D11GeometryShader **>(&m_pCreatedShader));
			break;
		case EShaderType::Hull:
			tResult = rPlatform.GetDevice()->CreateHullShader(
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), nullptr,
				reinterpret_cast<ID3D11HullShader **>(&m_pCreatedShader));
			break;
		case EShaderType::Domain:
			tResult = rPlatform.GetDevice()->CreateDomainShader(
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), nullptr,
				reinterpret_cast<ID3D11DomainShader **>(&m_pCreatedShader));
			break;
		case EShaderType::Fragment:
			tResult = rPlatform.GetDevice()->CreatePixelShader(
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), nullptr,
				reinterpret_cast<ID3D11PixelShader **>(&m_pCreatedShader));
			break;
		case EShaderType::Compute:
			tResult = rPlatform.GetDevice()->CreateComputeShader(
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), nullptr,
				reinterpret_cast<ID3D11ComputeShader **>(&m_pCreatedShader));
			break;
		}
		if (FAILED(tResult))
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create shader", E_ERROR_FAILED_TO_COMPILE_SHADER);
		}

		if (rOptions.eShaderType == EShaderType::Vertex)
		{
			CreateInputLayout();
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::Cleanup()
	{
		m_pInputLayout.reset();
		m_pCreatedShader.reset();
		m_pCompiledShader.reset();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::CreateInputLayout()
	{
		auto pPtr = IOERPI::GetSingletonPtr();
		IOERPI_Platform &rPlatform(pPtr->GetPlatform());

		IOEVertexFormat *pVertexFormat(m_rShader.GetVertexFormat());

		if (FAILED(rPlatform.GetDevice()->CreateInputLayout(
				pVertexFormat->GetPlatform().GetInputDescription().data(), pVertexFormat->GetNumSemantics(),
				m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(), &m_pInputLayout)))
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create input layout for shader", E_ERROR_FAILED_TO_COMPILE_SHADER);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::FindShaderVariables(ShaderConstantBuffers &arrConstantBuffers,
												 ShaderCategories &arrDescriptions)
	{
		ID3D11ShaderReflection *pShaderVars;
		// if (FAILED(D3DReflect(m_pCompiledShader->GetBufferPointer(),
		// m_pCompiledShader->GetBufferSize(), IID_ID3D11ShaderReflection,
		// (void **)&pRShader)))
		if (FAILED(D3DReflect(m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(),
							  /*__uuidof(ID3D11ShaderReflection)*/ IID_ID3D11ShaderReflection, (void **)&pShaderVars)))
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to get shader reflectioin parameters",
										 E_ERROR_FAILED_TO_FIND_VARIABLES);
		}

		D3D11_SHADER_DESC tShaderDesc;
		pShaderVars->GetDesc(&tShaderDesc);

		m_uNumCBuffers		 = tShaderDesc.ConstantBuffers;
		m_uNumBoundResources = tShaderDesc.BoundResources;

		// Add variables from constant buffers.
		for (std::uint32_t uBufferIdx(0); uBufferIdx < tShaderDesc.BoundResources; ++uBufferIdx)
		{
			D3D11_SHADER_INPUT_BIND_DESC bufferBindDesc;
			if (FAILED(pShaderVars->GetResourceBindingDesc(uBufferIdx, &bufferBindDesc)))
			{
				THROW_IOE_RENDERER_EXCEPTION("Failed to get description for a resource",
											 E_ERROR_FAILED_TO_FIND_VARIABLES);
			}
			if (bufferBindDesc.Type == D3D_SIT_CBUFFER || bufferBindDesc.Type == D3D_SIT_TBUFFER)
			{
				// Handled below
				continue;
			}
			switch (bufferBindDesc.Type)
			{
			case D3D_SIT_TEXTURE:
			case D3D_SIT_UAV_RWTYPED:
				InsertTexture(bufferBindDesc, arrDescriptions);
				break;
			case D3D_SIT_SAMPLER:
				InsertSampler(bufferBindDesc, arrDescriptions);
				break;
			case D3D_SIT_BYTEADDRESS:
			case D3D_SIT_UAV_RWBYTEADDRESS:
				InsertRawBuffer(bufferBindDesc, arrDescriptions);
				break;
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
				InsertStructuredBuffer(bufferBindDesc, arrDescriptions);
				break;
			}
		}
		for (std::uint32_t uBufferIdx(0); uBufferIdx < tShaderDesc.ConstantBuffers; ++uBufferIdx)
		{
			ID3D11ShaderReflectionConstantBuffer *pConstantBuffer = pShaderVars->GetConstantBufferByIndex(uBufferIdx);

			D3D11_SHADER_BUFFER_DESC bufferDesc;
			if (FAILED(pConstantBuffer->GetDesc(&bufferDesc)))
			{
				THROW_IOE_RENDERER_EXCEPTION("Failed to get description for a constant buffer",
											 E_ERROR_FAILED_TO_FIND_VARIABLES);
			}

			D3D11_SHADER_INPUT_BIND_DESC bufferBindDesc;
			if (FAILED(pShaderVars->GetResourceBindingDescByName(bufferDesc.Name, &bufferBindDesc)))
			{
				THROW_IOE_RENDERER_EXCEPTION_VARGS("Failed to get resource binding for constant buffer '%s'",
												   E_ERROR_FAILED_TO_FIND_VARIABLES, bufferDesc.Name);
			}

			if (bufferBindDesc.BindPoint >= ShaderConstantBufferDescription::ms_nMaxCBufferSlots)
			{
				THROW_IOE_RENDERER_EXCEPTION_VARGS(
					"Constant buffer slot %d out of range, only able to store "
					"%d separate slots",
					E_ERROR_FAILED_TO_FIND_VARIABLES, bufferBindDesc.BindPoint,
					ShaderConstantBufferDescription::ms_nMaxCBufferSlots);
			}
			if (bufferBindDesc.Type != D3D_SIT_CBUFFER && bufferBindDesc.Type != D3D_SIT_TBUFFER)
			{
				--m_uNumCBuffers;
				/*switch (bufferBindDesc.Type)
				{
				case D3D_SIT_TEXTURE:
				case D3D_SIT_UAV_RWTYPED:
					InsertTexture(bufferBindDesc, arrDescriptions);
					break;
				case D3D_SIT_SAMPLER:
					InsertSampler(bufferBindDesc, arrDescriptions);
					break;
				case D3D_SIT_BYTEADDRESS:
				case D3D_SIT_UAV_RWBYTEADDRESS:
					InsertRawBuffer(bufferBindDesc, arrDescriptions);
					break;
				case D3D_SIT_STRUCTURED:
				case D3D_SIT_UAV_RWSTRUCTURED:
				case D3D_SIT_UAV_APPEND_STRUCTURED:
				case D3D_SIT_UAV_CONSUME_STRUCTURED:
				case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
					InsertStructuredBuffer(bufferBindDesc, arrDescriptions);
					break;
				}*/
			}
			else
			{
				ShaderConstantBufferDescription *pCBuffer = &arrConstantBuffers[bufferBindDesc.BindPoint];
				pCBuffer->nSlotIdx						  = bufferBindDesc.BindPoint;

				if (strlen(pCBuffer->szName) > 0)
				{
					THROW_IOE_RENDERER_EXCEPTION_VARGS(
						"Constant buffer %s already bound to same slot, "
						"cannot overwrite",
						E_ERROR_FAILED_TO_FIND_VARIABLES, pCBuffer->szName);
				}

				strcpy_s<64>(pCBuffer->szName, _strdup(bufferDesc.Name));
				pCBuffer->nSizeBytes = bufferDesc.Size;

				// Create the D3D11 buffer of where to store the CBuffer data
				D3D11_BUFFER_DESC tBufferDesc;

				tBufferDesc.ByteWidth	  = static_cast<UINT>(pCBuffer->nSizeBytes);
				tBufferDesc.Usage		   = D3D11_USAGE_DYNAMIC;
				tBufferDesc.BindFlags	  = D3D11_BIND_CONSTANT_BUFFER;
				tBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				tBufferDesc.MiscFlags	  = 0;

				tBufferDesc.StructureByteStride = 0;

				if (FAILED(IOERPI::GetSingletonPtr()->GetPlatform().GetDevice()->CreateBuffer(
						&tBufferDesc, nullptr, &m_arrConstantD3D11Buffers[bufferBindDesc.BindPoint])))
				{
					THROW_IOE_RENDERER_EXCEPTION_VARGS("Failed to create D3D11 Buffer to map CBuffer '%s'",
													   E_ERROR_FAILED_TO_FIND_VARIABLES, pCBuffer->szName);
				}

				for (std::uint32_t uVariableIdx(0); uVariableIdx < bufferDesc.Variables; ++uVariableIdx)
				{
					ID3D11ShaderReflectionVariable *pShaderVariable =
						pConstantBuffer->GetVariableByIndex(uVariableIdx);
					if (!pShaderVariable)
					{
						THROW_IOE_RENDERER_EXCEPTION_VARGS(
							"Failed to find shader variable at index %d for "
							"constant buffer %s",
							E_ERROR_FAILED_TO_FIND_VARIABLES, uVariableIdx, pCBuffer->szName);
					}

					D3D11_SHADER_VARIABLE_DESC tVariableDescription;
					if (FAILED(pShaderVariable->GetDesc(&tVariableDescription)))
					{
						THROW_IOE_RENDERER_EXCEPTION_VARGS(
							"Failed to find shader variable description at "
							"index %d for constant buffer %s",
							E_ERROR_FAILED_TO_FIND_VARIABLES, uVariableIdx, pCBuffer->szName);
					}

					ID3D11ShaderReflectionType *pShaderVariableType(pShaderVariable->GetType());
					if (pShaderVariableType == nullptr)
					{
						THROW_IOE_RENDERER_EXCEPTION_VARGS(
							"Failed to find shader variable type for variable "
							"at index %d for constant buffer %s",
							E_ERROR_FAILED_TO_FIND_VARIABLES, uVariableIdx, pCBuffer->szName);
					}

					D3D11_SHADER_TYPE_DESC tShaderVariableTypeDesc;
					if (FAILED(pShaderVariableType->GetDesc(&tShaderVariableTypeDesc)))
					{
						THROW_IOE_RENDERER_EXCEPTION_VARGS(
							"Failed to find shader variable type description "
							"for variable at index %d for constant buffer %s",
							E_ERROR_FAILED_TO_FIND_VARIABLES, uVariableIdx, pCBuffer->szName);
					}

					EShaderVariableType eShaderVariableType(EShaderVariableType::None);

					if (tShaderVariableTypeDesc.Type == D3D_SVT_FLOAT && tShaderVariableTypeDesc.Rows == 1 &&
						tShaderVariableTypeDesc.Columns == 1)
					{
						eShaderVariableType = EShaderVariableType::Float;
					}
					else if (tShaderVariableTypeDesc.Type == D3D_SVT_FLOAT && tShaderVariableTypeDesc.Rows == 1 &&
							 tShaderVariableTypeDesc.Columns == 2)
					{
						eShaderVariableType = EShaderVariableType::Float2;
					}
					else if (tShaderVariableTypeDesc.Type == D3D_SVT_FLOAT && tShaderVariableTypeDesc.Rows == 1 &&
							 tShaderVariableTypeDesc.Columns == 3)
					{
						eShaderVariableType = EShaderVariableType::Float3;
					}
					else if (tShaderVariableTypeDesc.Type == D3D_SVT_FLOAT && tShaderVariableTypeDesc.Rows == 1 &&
							 tShaderVariableTypeDesc.Columns == 4)
					{
						eShaderVariableType = EShaderVariableType::Float4;
					}
					else if (tShaderVariableTypeDesc.Type == D3D_SVT_FLOAT && tShaderVariableTypeDesc.Rows == 4 &&
							 tShaderVariableTypeDesc.Columns == 4)
					{
						eShaderVariableType = EShaderVariableType::Float4x4;
					}
					else if (tShaderVariableTypeDesc.Type == D3D_SVT_INT && tShaderVariableTypeDesc.Rows == 1 &&
							 tShaderVariableTypeDesc.Columns == 1)
					{
						eShaderVariableType = EShaderVariableType::Int;
					}
					else if (tShaderVariableTypeDesc.Type == D3D_SVT_UINT && tShaderVariableTypeDesc.Rows == 1 &&
							 tShaderVariableTypeDesc.Columns == 1)
					{
						eShaderVariableType = EShaderVariableType::UInt;
					}
					else
					{
						THROW_IOE_RENDERER_EXCEPTION_VARGS(
							"Unrecognised shader variable type in CBuffer "
							"'%s'",
							E_ERROR_FAILED_TO_FIND_VARIABLES, pCBuffer->szName);
					}

					if (eShaderVariableType != EShaderVariableType::None)
					{
						ShaderVariableDescription tDescription;

						strcpy_s<64>(tDescription.szName, _strdup(tVariableDescription.Name));
						tDescription.szName[63] = '\0';
						tDescription.eType		= eShaderVariableType;
						tDescription.nSlot		= tVariableDescription.StartOffset;

						pCBuffer->arrVariables.push_back(tDescription);
					}
				}
			}
		}

		ReleaseD3DObject(pShaderVars);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::FindGlobalResource(ID3D11ShaderReflection *pShaderReflection, int nInputType,
												EShaderVariableType eVariableType,
												ShaderCategories &arrGlobalDescriptions)
	{
		D3D11_SHADER_DESC tShaderDesc;
		if (FAILED(pShaderReflection->GetDesc(&tShaderDesc)))
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to find shader reflection description",
										 E_ERROR_FAILED_TO_FIND_VARIABLES);
		}

		for (std::uint32_t uResourceIdx(0); uResourceIdx < tShaderDesc.BoundResources; ++uResourceIdx)
		{
			D3D11_SHADER_INPUT_BIND_DESC tInputBindDesc;
			if (FAILED(pShaderReflection->GetResourceBindingDesc(uResourceIdx, &tInputBindDesc)))
			{
				THROW_IOE_RENDERER_EXCEPTION_VARGS(
					"Failed to get the resource binding description for "
					"resource at slot %d",
					E_ERROR_FAILED_TO_FIND_VARIABLES, uResourceIdx);
			}

			if (tInputBindDesc.Type == nInputType)
			{
				ShaderVariableDescription tDescription;

				strcpy_s<64>(tDescription.szName, _strdup(tInputBindDesc.Name));
				tDescription.szName[63] = '\0';
				if (eVariableType == EShaderVariableType::Texture1D ||
					eVariableType == EShaderVariableType::Texture2D ||
					eVariableType == EShaderVariableType::Texture3D ||
					eVariableType == EShaderVariableType::TextureCube ||
					eVariableType == EShaderVariableType::Texture1DArray ||
					eVariableType == EShaderVariableType::Texture2DArray ||
					eVariableType == EShaderVariableType::TextureCubeArray)
				{
					switch (tInputBindDesc.Dimension)
					{
					case D3D_SRV_DIMENSION_TEXTURE1D:
						tDescription.eType = EShaderVariableType::Texture1D;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2D:
						tDescription.eType = EShaderVariableType::Texture2D;
						break;
					case D3D_SRV_DIMENSION_TEXTURE3D:
						tDescription.eType = EShaderVariableType::Texture3D;
						break;
					case D3D_SRV_DIMENSION_TEXTURECUBE:
						tDescription.eType = EShaderVariableType::TextureCube;
						break;
					case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
						tDescription.eType = EShaderVariableType::Texture1DArray;
						break;
					case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
						tDescription.eType = EShaderVariableType::Texture2DArray;
						break;
					case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
						tDescription.eType = EShaderVariableType::TextureCubeArray;
						break;
					}
				}
				else
				{
					tDescription.eType = eVariableType;
				}
				tDescription.nSlot = tInputBindDesc.BindPoint;

				arrGlobalDescriptions[EShaderVariableCategories::Texture].push_back(tDescription);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::InsertTexture(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
										   ShaderCategories &arrGlobalDescriptions)
	{
		if (rDesc.Type == D3D_SIT_TEXTURE || rDesc.Type == D3D_SIT_UAV_RWTYPED)
		{
			ShaderVariableDescription tDescription;

			strcpy_s<64>(tDescription.szName, _strdup(rDesc.Name));
			tDescription.szName[63] = '\0';

			switch (rDesc.Dimension)
			{
			case D3D_SRV_DIMENSION_TEXTURE1D:
				tDescription.eType = EShaderVariableType::Texture1D;
				break;
			case D3D_SRV_DIMENSION_TEXTURE2D:
				tDescription.eType = EShaderVariableType::Texture2D;
				break;
			case D3D_SRV_DIMENSION_TEXTURE3D:
				tDescription.eType = EShaderVariableType::Texture3D;
				break;
			case D3D_SRV_DIMENSION_TEXTURECUBE:
				tDescription.eType = EShaderVariableType::TextureCube;
				break;
			case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
				tDescription.eType = EShaderVariableType::Texture1DArray;
				break;
			case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
				tDescription.eType = EShaderVariableType::Texture2DArray;
				break;
			case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
				tDescription.eType = EShaderVariableType::TextureCubeArray;
				break;
			}

			tDescription.nSlot			= rDesc.BindPoint;
			tDescription.bGPUWriteable  = rDesc.Type == D3D_SIT_UAV_RWTYPED;
			tDescription.nSizeOrSamples = rDesc.NumSamples;

			arrGlobalDescriptions[EShaderVariableCategories::Texture].push_back(tDescription);
		}
	}

	void IOEShader_Platform::InsertSampler(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
										   ShaderCategories &arrGlobalDescriptions)
	{
		if (rDesc.Type == D3D_SIT_SAMPLER)
		{
			ShaderVariableDescription tDescription;

			strcpy_s<64>(tDescription.szName, _strdup(rDesc.Name));
			tDescription.szName[63]		= '\0';
			tDescription.nSlot			= rDesc.BindPoint;
			tDescription.nSizeOrSamples = rDesc.NumSamples;

			arrGlobalDescriptions[EShaderVariableCategories::Sampler].push_back(tDescription);
		}
	}

	void IOEShader_Platform::InsertRawBuffer(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
											 ShaderCategories &arrGlobalDescriptions)
	{
		if (rDesc.Type == D3D_SIT_BYTEADDRESS || rDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
		{
			ShaderVariableDescription tDescription;

			strcpy_s<64>(tDescription.szName, _strdup(rDesc.Name));
			tDescription.szName[63]		= '\0';
			tDescription.nSlot			= rDesc.BindPoint;
			tDescription.nSizeOrSamples = rDesc.NumSamples;
			tDescription.bGPUWriteable  = rDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS;

			arrGlobalDescriptions[EShaderVariableCategories::RawBuffer].push_back(tDescription);
		}
	}

	void IOEShader_Platform::InsertStructuredBuffer(const D3D11_SHADER_INPUT_BIND_DESC &rDesc,
													ShaderCategories &arrGlobalDescriptions)
	{
		if (rDesc.Type == D3D_SIT_STRUCTURED || rDesc.Type == D3D_SIT_UAV_RWSTRUCTURED ||
			rDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED || rDesc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED ||
			rDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
		{
			ShaderVariableDescription tDescription;

			strcpy_s<64>(tDescription.szName, _strdup(rDesc.Name));
			tDescription.szName[63]		= '\0';
			tDescription.nSlot			= rDesc.BindPoint;
			tDescription.bGPUWriteable  = rDesc.Type != D3D_SIT_STRUCTURED;
			tDescription.nSizeOrSamples = rDesc.NumSamples;

			arrGlobalDescriptions[EShaderVariableCategories::StructuredBuffer].push_back(tDescription);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void *IOEShader_Platform::MapConstantBuffers(IOERPI *pRPI, std::int32_t nSlotIdx,
												 ShaderConstantBufferDescription *pBuffer)
	{
		if (m_arrConstantD3D11Buffers[pBuffer->nSlotIdx] != nullptr)
		{
			D3D11_MAPPED_SUBRESOURCE tBufferMap;
			if (SUCCEEDED(pRPI->GetPlatform().GetDeviceContext()->Map(
					m_arrConstantD3D11Buffers[pBuffer->nSlotIdx].get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &tBufferMap)))
			{
				return static_cast<void *>(static_cast<std::uint8_t *>(tBufferMap.pData) + nSlotIdx);
			}
		}
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::UnmapConstantBuffers(IOERPI *pRPI, std::int32_t nSlotIdx,
												  ShaderConstantBufferDescription *pBuffer)
	{
		if (m_arrConstantD3D11Buffers[pBuffer->nSlotIdx] != nullptr)
		{
			pRPI->GetPlatform().GetDeviceContext()->Unmap(m_arrConstantD3D11Buffers[pBuffer->nSlotIdx].get(), 0);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::Set(IOERPI *pRPI)
	{
		std::array<ID3D11Buffer *, ShaderConstantBufferDescription::ms_nMaxCBufferSlots> arrConstantD3D11Buffers;
		std::transform(std::begin(m_arrConstantD3D11Buffers), std::end(m_arrConstantD3D11Buffers),
					   std::begin(arrConstantD3D11Buffers),
					   [](TComSharedPtr<ID3D11Buffer> &rVal) { return rVal.get(); });
		ID3D11Buffer (**pBuffer)(arrConstantD3D11Buffers.data());
		switch (m_rShader.GetShaderType())
		{
		case EShaderType::Vertex:
			pRPI->GetPlatform().GetDeviceContext()->VSSetConstantBuffers(0, m_uNumCBuffers, pBuffer);
			pRPI->GetPlatform().GetDeviceContext()->VSSetShader(
				reinterpret_cast<ID3D11VertexShader *>(m_pCreatedShader.get()), nullptr, 0);
			pRPI->GetPlatform().GetDeviceContext()->IASetInputLayout(m_pInputLayout.get());
			break;
		case EShaderType::Geometry:
			pRPI->GetPlatform().GetDeviceContext()->GSSetConstantBuffers(0, m_uNumCBuffers, pBuffer);
			pRPI->GetPlatform().GetDeviceContext()->GSSetShader(
				reinterpret_cast<ID3D11GeometryShader *>(m_pCreatedShader.get()), nullptr, 0);
			break;
		case EShaderType::Hull:
			pRPI->GetPlatform().GetDeviceContext()->HSSetConstantBuffers(0, m_uNumCBuffers, pBuffer);
			pRPI->GetPlatform().GetDeviceContext()->HSSetShader(
				reinterpret_cast<ID3D11HullShader *>(m_pCreatedShader.get()), nullptr, 0);
			break;
		case EShaderType::Domain:
			pRPI->GetPlatform().GetDeviceContext()->DSSetConstantBuffers(0, m_uNumCBuffers, pBuffer);
			pRPI->GetPlatform().GetDeviceContext()->DSSetShader(
				reinterpret_cast<ID3D11DomainShader *>(m_pCreatedShader.get()), nullptr, 0);
			break;
		case EShaderType::Fragment:
			pRPI->GetPlatform().GetDeviceContext()->PSSetConstantBuffers(0, m_uNumCBuffers, pBuffer);
			pRPI->GetPlatform().GetDeviceContext()->PSSetShader(
				reinterpret_cast<ID3D11PixelShader *>(m_pCreatedShader.get()), nullptr, 0);
			break;
		case EShaderType::Compute:
			pRPI->GetPlatform().GetDeviceContext()->CSSetShader(
				reinterpret_cast<ID3D11ComputeShader *>(m_pCreatedShader.get()), nullptr, 0);
			pRPI->GetPlatform().GetDeviceContext()->CSSetConstantBuffers(0, m_uNumCBuffers, pBuffer);
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEShader_Platform::SetTextureVariable(IOERPI *pRPI, const char *szTextureName, IOETexture *pTexture,
												int arrOffset /*=0*/)
	{
		bool bGenericTexture(false);
		if (pTexture)
		{
			bGenericTexture = (pTexture->GetTextureType() == ETextureType::GenericBuffer);
		}
		else
		{

			bGenericTexture =
				m_rShader.FindGlobalVariable(szTextureName, EShaderVariableCategories::RawBuffer) != nullptr ||
				m_rShader.FindGlobalVariable(szTextureName, EShaderVariableCategories::StructuredBuffer) != nullptr;
		}
		if (bGenericTexture)
		{
			return SetBufferVariable(pRPI, szTextureName, pTexture, arrOffset);
		}
		else
		{
			return SetShaderResourceVariable(pRPI, szTextureName, pTexture, arrOffset);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEShader_Platform::SetShaderResourceVariable(IOERPI *pRPI, const char *szTextureName, IOETexture *pTexture,
													   int arrOffset /*=0*/)
	{
		ShaderVariableDescription *pDescription(
			m_rShader.FindGlobalVariable(szTextureName, EShaderVariableCategories::Texture));
		if (pDescription == nullptr)
		{
			return false;
		}
		IOE_ASSERT(!pDescription->bGPUWriteable, "Writeable textures not yet supported!");
		ID3D11ShaderResourceView *pShaderResourceView(
			pTexture ? static_cast<ID3D11ShaderResourceView *>(pTexture->GetPlatform().GetView()) : nullptr);
		switch (m_rShader.GetShaderType())
		{
		case EShaderType::Vertex:
			pRPI->GetPlatform().GetDeviceContext()->VSSetShaderResources(pDescription->nSlot, 1, &pShaderResourceView);
			return true;
		case EShaderType::Geometry:
			pRPI->GetPlatform().GetDeviceContext()->GSSetShaderResources(pDescription->nSlot, 1, &pShaderResourceView);
			return true;
		case EShaderType::Hull:
			pRPI->GetPlatform().GetDeviceContext()->HSSetShaderResources(pDescription->nSlot, 1, &pShaderResourceView);
			return true;
		case EShaderType::Domain:
			pRPI->GetPlatform().GetDeviceContext()->DSSetShaderResources(pDescription->nSlot, 1, &pShaderResourceView);
			return true;
		case EShaderType::Fragment:
			pRPI->GetPlatform().GetDeviceContext()->PSSetShaderResources(pDescription->nSlot, 1, &pShaderResourceView);
			return true;
		case EShaderType::Compute:
			pRPI->GetPlatform().GetDeviceContext()->CSSetShaderResources(pDescription->nSlot, 1, &pShaderResourceView);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEShader_Platform::SetBufferVariable(IOERPI *pRPI, const char *szTextureName, IOETexture *pTexture,
											   int arrOffset /*=0*/)
	{
		ShaderVariableDescription *pDescription(
			m_rShader.FindGlobalVariable(szTextureName, EShaderVariableCategories::RawBuffer));
		if (pDescription == nullptr)
		{
			pDescription = m_rShader.FindGlobalVariable(szTextureName, EShaderVariableCategories::StructuredBuffer);
			if (pDescription == nullptr)
			{
				return false;
			}
		}
		if (pDescription->bGPUWriteable)
		{
			EUAVSet eUAVSet(m_rShader.GetShaderType() == EShaderType::Compute ? EUAVSet::ComputeShader
																			  : EUAVSet::OutputMerger);
			if (pTexture)
			{
				IOE_ASSERT(pTexture->GetBufferDescription() == EBufferDescription::UnorderedAccess,
						   "Writeable texture must be bound as a UAV");
				pRPI->SetUAV(*pTexture, pDescription->nSlot, eUAVSet, arrOffset);
				return true;
			}
			else
			{
				pRPI->UnsetUAV(pDescription->nSlot, eUAVSet);
				return true;
			}
		}
		else
		{
			IOE_ASSERT(pTexture == nullptr || pTexture->GetBufferDescription() == EBufferDescription::ShaderResource,
					   "Read-only texture must be bound as a SRV");
			ID3D11ShaderResourceView *pShaderResourceView(
				pTexture ? static_cast<ID3D11ShaderResourceView *>(pTexture->GetPlatform().GetView()) : nullptr);
			switch (m_rShader.GetShaderType())
			{
			case EShaderType::Vertex:
				pRPI->GetPlatform().GetDeviceContext()->VSSetShaderResources(pDescription->nSlot, 1,
																			 &pShaderResourceView);
				return true;
			case EShaderType::Geometry:
				pRPI->GetPlatform().GetDeviceContext()->GSSetShaderResources(pDescription->nSlot, 1,
																			 &pShaderResourceView);
				return true;
			case EShaderType::Hull:
				pRPI->GetPlatform().GetDeviceContext()->HSSetShaderResources(pDescription->nSlot, 1,
																			 &pShaderResourceView);
				return true;
			case EShaderType::Domain:
				pRPI->GetPlatform().GetDeviceContext()->DSSetShaderResources(pDescription->nSlot, 1,
																			 &pShaderResourceView);
				return true;
			case EShaderType::Fragment:
				pRPI->GetPlatform().GetDeviceContext()->PSSetShaderResources(pDescription->nSlot, 1,
																			 &pShaderResourceView);
				return true;
			case EShaderType::Compute:
				pRPI->GetPlatform().GetDeviceContext()->CSSetShaderResources(pDescription->nSlot, 1,
																			 &pShaderResourceView);
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEShader_Platform::SetSamplerVariable(IOERPI *pRPI, const char *szSamplerName, IOESamplerState *pSamplers)
	{
		ShaderVariableDescription *pDescription(
			m_rShader.FindGlobalVariable(szSamplerName, EShaderVariableCategories::Sampler));
		if (pDescription == nullptr)
		{
			return false;
		}
		ID3D11SamplerState *pSamplerState(pSamplers ? pSamplers->GetPlatform().GetD3D11State() : nullptr);
		switch (m_rShader.GetShaderType())
		{
		case EShaderType::Vertex:
			pRPI->GetPlatform().GetDeviceContext()->VSSetSamplers(pDescription->nSlot, 1, &pSamplerState);
			return true;
		case EShaderType::Geometry:
			pRPI->GetPlatform().GetDeviceContext()->GSSetSamplers(pDescription->nSlot, 1, &pSamplerState);
			return true;
		case EShaderType::Hull:
			pRPI->GetPlatform().GetDeviceContext()->HSSetSamplers(pDescription->nSlot, 1, &pSamplerState);
			return true;
		case EShaderType::Domain:
			pRPI->GetPlatform().GetDeviceContext()->DSSetSamplers(pDescription->nSlot, 1, &pSamplerState);
			return true;
		case EShaderType::Fragment:
			pRPI->GetPlatform().GetDeviceContext()->PSSetSamplers(pDescription->nSlot, 1, &pSamplerState);
			return true;
		case EShaderType::Compute:
			pRPI->GetPlatform().GetDeviceContext()->CSSetSamplers(pDescription->nSlot, 1, &pSamplerState);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEShader_Platform::SetSamplers(IOERPI *pRPI, const vector<pair<string, IOESamplerState *> > &arrSamplers)
	{
		static const std::uint32_t s_uMaxSamplers = 8;
		std::int32_t uNumSamplers(0);
		ID3D11SamplerState *arrD3D11Samplers[s_uMaxSamplers] = { nullptr };

		bool bAnySet = false;

		for (const auto &tPair : arrSamplers)
		{
			ShaderVariableDescription *pDescription(
				m_rShader.FindGlobalVariable(tPair.first.c_str(), EShaderVariableCategories::Sampler));
			if (pDescription == nullptr)
			{
				return false;
			}
			ID3D11SamplerState *pSamplerState(tPair.second ? tPair.second->GetPlatform().GetD3D11State() : nullptr);
			arrD3D11Samplers[pDescription->nSlot] = pSamplerState;
			uNumSamplers						  = Maths::Max(uNumSamplers, pDescription->nSlot + 1);
		}
		if (bAnySet && uNumSamplers > 0)
		{
			switch (m_rShader.GetShaderType())
			{
			case EShaderType::Vertex:
				pRPI->GetPlatform().GetDeviceContext()->VSSetSamplers(0, uNumSamplers, arrD3D11Samplers);
				bAnySet = true;
				break;
			case EShaderType::Geometry:
				pRPI->GetPlatform().GetDeviceContext()->GSSetSamplers(0, uNumSamplers, arrD3D11Samplers);
				bAnySet = true;
				break;
			case EShaderType::Hull:
				pRPI->GetPlatform().GetDeviceContext()->HSSetSamplers(0, uNumSamplers, arrD3D11Samplers);
				bAnySet = true;
				break;
			case EShaderType::Domain:
				pRPI->GetPlatform().GetDeviceContext()->DSSetSamplers(0, uNumSamplers, arrD3D11Samplers);
				bAnySet = true;
				break;
			case EShaderType::Fragment:
				pRPI->GetPlatform().GetDeviceContext()->PSSetSamplers(0, uNumSamplers, arrD3D11Samplers);
				bAnySet = true;
				break;
			case EShaderType::Compute:
				pRPI->GetPlatform().GetDeviceContext()->CSSetSamplers(0, uNumSamplers, arrD3D11Samplers);
				bAnySet = true;
				break;
			}
		}
		return bAnySet;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEShader_Platform::Dispatch(IOERPI *pRPI, int32_t nNumThreadGroupsX, int32_t nNumThreadGroupsY,
									  int32_t nNumThreadGroupsZ)
	{
		pRPI->GetPlatform().GetDeviceContext()->Dispatch((UINT)nNumThreadGroupsX, (UINT)nNumThreadGroupsY,
														 (UINT)nNumThreadGroupsZ);
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE