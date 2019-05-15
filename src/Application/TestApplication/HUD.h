//////////////////////////////////////////////////////////////////////////

template <typename _T>
struct HUDValue
{
	std::string ToString()
	{
		std::stringstream sstream;
		sstream << Value;
		return sstream.str();
	}

	_T Value;
};

template <>
struct HUDValue<float>
{
	std::string ToString()
	{
		char buffer[32];
		sprintf_s<32>(buffer, "%.2f", Value);
		return buffer;
	}

	float Value;
};

template <>
struct HUDValue<bool>
{
	std::string ToString()
	{
		return (Value ? "Yes" : "No");
	}

	bool Value;
};

//////////////////////////////////////////////////////////////////////////

template <typename _T>
struct HUDElement
{
	HUDElement(const std::string &szLabel_, float fX_, float fY_)
		: szLabel(szLabel_)
		, pCurrentValue(nullptr)
		, fX(fX_)
		, fY(fY_)
	{
	}

	void SetScreenSize(float fScreenX_, float fScreenY_)
	{
		fScreenX = fScreenX_;
		fScreenY = fScreenY_;
		if (m_pScreenText)
		{
			m_pScreenText->UpdatePosition(fX * fScreenX, fY * fScreenY);

			float fScaleX(fScreenX / 1920.0f);
			float fScaleY(fScreenY / 1080.0f);

			m_pScreenText->UpdateScale(fScaleX, fScaleY);
		}
	}

	void SetDataSource(_T *pSource)
	{
		pCurrentValue = pSource;
		tOldValue	 = *pSource;
		__InternalUpdate();
	}

	void Update()
	{
		if (Changed())
		{
			__InternalUpdate();
		}
	}

	bool Changed()
	{
		if (pCurrentValue && *pCurrentValue != tOldValue)
		{
			tOldValue = *pCurrentValue;
			return true;
		}
		return false;
	}

	void GenerateText(std::shared_ptr<IOE::Renderer::IOEFont> pFont)
	{
		m_pScreenText =
			::IOE::Renderer::IOETextManager::GetSingletonPtr()->CreateText(
				szLabel, fX * fScreenX, fY * fScreenY, pFont);
		__InternalUpdate();
	}

	std::shared_ptr<IOE::Renderer::IOEText> m_pScreenText;

	std::string szLabel;
	HUDValue<_T> tValue;
	_T *pCurrentValue;
	_T tOldValue;
	float fX, fY;
	float fScreenX, fScreenY;

private:
	void __InternalUpdate()
	{
		if (m_pScreenText && pCurrentValue)
		{
			if (pCurrentValue)
			{
				tValue.Value = *pCurrentValue;
				m_pScreenText->UpdateText(szLabel + ": " + tValue.ToString());
			}
			else
			{
				m_pScreenText->UpdateText(szLabel);
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////

struct HUD
{
	HUD()
		: m_tFrameDelta("FPS", 0.0f, 0.45f)
		, m_tCameraControlsLabel("Camera Controls:", -0.5f, 0.f)
		, m_tCameraControlsLabel_Line1("W: Forward, S: Backwards", -0.5f,
									   -0.05f)
		, m_tCameraControlsLabel_Line2("A: Left,  D: Right", -0.5f, -0.1f)
		, m_tCameraControlsLabel_Line3("Up: +Pitch,  Down: -Pitch", -0.5f,
									   -0.15f)
		, m_tCameraControlsLabel_Line4("Left: -Yaw,  Right: +Yaw", -0.5f,
									   -0.2f)

		, m_tDebugControlsLabel("Debug Controls:", 0.3f, 0.f)
		, m_tDebugControlsLabel_Line1("F1: Next Debug Mode", 0.25f, -0.05f)
		, m_tDebugControlsLabel_Line2("F2: Prev Debug Mode", 0.25f, -0.1f)
		, m_tDebugControlsLabel_Line3("F3: Physical Based Shader", 0.25f,
									  -0.15f)
		, m_tDebugControlsLabel_Line4("F4: Phong Based Shader", 0.25f, -0.2f)
		, m_tDebugControlsLabel_Line5("F5/F6: Change Roughness", 0.25f, -0.25f)
		, m_tDebugControlsLabel_Line6("F7/F8: Change Intensity", 0.25f, -0.3f)
		, m_tDebugControlsLabel_Line8("O/P: Cycle Setup", 0.25f, -0.35f)
		, m_tDebugControlsLabel_Line7("Space: Toggle HUD", 0.25f, -0.45f)

		, m_tIndirectDirectLight("Lighting: ", 0.25f, 0.4f)
		, m_tIndirectDirectLightToggle("Toggle: I", 0.25f, 0.35f)

		, m_tIntensity("Intensity", -0.5f, 0.4f)
		, m_tRoughness("Roughness", -0.5f, 0.45f)
		, m_tSpecular("Specular", -0.5f, 0.35f)

		, m_tDebugMode("Debug Mode", 0.0f, -0.45f)
	{
	}

	void GenerateText(std::shared_ptr<IOE::Renderer::IOEFont> pFont)
	{
		m_tFrameDelta.GenerateText(pFont);
		m_tIntensity.GenerateText(pFont);
		m_tRoughness.GenerateText(pFont);
		m_tSpecular.GenerateText(pFont);

		m_tCameraControlsLabel.GenerateText(pFont);
		m_tCameraControlsLabel_Line1.GenerateText(pFont);
		m_tCameraControlsLabel_Line2.GenerateText(pFont);
		m_tCameraControlsLabel_Line3.GenerateText(pFont);
		m_tCameraControlsLabel_Line4.GenerateText(pFont);

		m_tDebugControlsLabel.GenerateText(pFont);
		m_tDebugControlsLabel_Line1.GenerateText(pFont);
		m_tDebugControlsLabel_Line2.GenerateText(pFont);
		m_tDebugControlsLabel_Line3.GenerateText(pFont);
		m_tDebugControlsLabel_Line4.GenerateText(pFont);
		m_tDebugControlsLabel_Line5.GenerateText(pFont);
		m_tDebugControlsLabel_Line6.GenerateText(pFont);
		m_tDebugControlsLabel_Line7.GenerateText(pFont);
		m_tDebugControlsLabel_Line8.GenerateText(pFont);

		m_tIndirectDirectLight.GenerateText(pFont);
		m_tIndirectDirectLightToggle.GenerateText(pFont);

		m_tDebugMode.GenerateText(pFont);
	}

	void Update()
	{
		m_tFrameDelta.Update();
		m_tIntensity.Update();
		m_tRoughness.Update();
		m_tSpecular.Update();
		m_tDebugMode.Update();
		m_tIndirectDirectLight.Update();
	}

	void SetScreenSize(float fWidth, float fHeight)
	{
		m_tFrameDelta.SetScreenSize(fWidth, fHeight);

		m_tIntensity.SetScreenSize(fWidth, fHeight);
		m_tRoughness.SetScreenSize(fWidth, fHeight);
		m_tSpecular.SetScreenSize(fWidth, fHeight);

		m_tCameraControlsLabel.SetScreenSize(fWidth, fHeight);
		m_tCameraControlsLabel_Line1.SetScreenSize(fWidth, fHeight);
		m_tCameraControlsLabel_Line2.SetScreenSize(fWidth, fHeight);
		m_tCameraControlsLabel_Line3.SetScreenSize(fWidth, fHeight);
		m_tCameraControlsLabel_Line4.SetScreenSize(fWidth, fHeight);

		m_tDebugControlsLabel.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line1.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line2.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line3.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line4.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line5.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line6.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line7.SetScreenSize(fWidth, fHeight);
		m_tDebugControlsLabel_Line8.SetScreenSize(fWidth, fHeight);

		m_tIndirectDirectLight.SetScreenSize(fWidth, fHeight);
		m_tIndirectDirectLightToggle.SetScreenSize(fWidth, fHeight);

		m_tDebugMode.SetScreenSize(fWidth, fHeight);
	}

	// Frame time
	HUDElement<float> m_tFrameDelta;

	// Properties
	HUDElement<float> m_tIntensity;
	HUDElement<float> m_tRoughness;
	HUDElement<float> m_tSpecular;

	// Camera Controls
	HUDElement<std::string> m_tCameraControlsLabel;
	HUDElement<std::string> m_tCameraControlsLabel_Line1;
	HUDElement<std::string> m_tCameraControlsLabel_Line2;
	HUDElement<std::string> m_tCameraControlsLabel_Line3;
	HUDElement<std::string> m_tCameraControlsLabel_Line4;

	// Debug Controls
	HUDElement<std::string> m_tDebugControlsLabel;
	HUDElement<std::string> m_tDebugControlsLabel_Line1;
	HUDElement<std::string> m_tDebugControlsLabel_Line2;
	HUDElement<std::string> m_tDebugControlsLabel_Line3;
	HUDElement<std::string> m_tDebugControlsLabel_Line4;
	HUDElement<std::string> m_tDebugControlsLabel_Line5;
	HUDElement<std::string> m_tDebugControlsLabel_Line6;
	HUDElement<std::string> m_tDebugControlsLabel_Line7;
	HUDElement<std::string> m_tDebugControlsLabel_Line8;

	HUDElement<std::string> m_tIndirectDirectLight;
	HUDElement<std::string> m_tIndirectDirectLightToggle;

	HUDElement<std::string> m_tDebugMode;
};