class CHUDElement {
public:
	bool bEnabled = false;

	struct tDrawPositions {
		float fPosX;
		float fPosY;
		float fSize;
		float fSpacingX;
		float fSpacingY;
	};

	struct tDrawPositions1080p {
		int nPosX;
		int nPosY;
		float fSize;
		int nSpacingX;
		int nSpacingY;
	};

	enum eJustify {
		JUSTIFY_LEFT,
		JUSTIFY_CENTER,
		JUSTIFY_RIGHT,
		JUSTIFY_480P_LEFT,
		JUSTIFY_480P_CENTER,
		JUSTIFY_480P_RIGHT
	};

	static void DoJustify(eJustify justify, float& x, float& y) {
		switch (justify) {
			case JUSTIFY_LEFT:
			default:
				x /= 1920.0;
				y /= 1080.0;

				// aspect correction
				x *= 16.0 / 9.0;
				x /= GetAspectRatio();
				break;
			case JUSTIFY_CENTER:
				x -= 960;
				x /= 1920.0;
				y /= 1080.0;

				// aspect correction
				x *= 16.0 / 9.0;
				x /= GetAspectRatio();

				// recenter
				x += 0.5;
				break;
			case JUSTIFY_RIGHT:
				x = 1920 - x;
				x /= 1920.0;
				y /= 1080.0;

				// aspect correction
				x *= 16.0 / 9.0;
				x /= GetAspectRatio();

				// recenter
				x = 1 - x;
				break;
			case JUSTIFY_480P_LEFT:
				x /= 640.0;
				y /= 480.0;

				// aspect correction
				x *= 4.0 / 3.0;
				x /= GetAspectRatio();
				break;
			case JUSTIFY_480P_CENTER:
				x -= 320;
				x /= 640.0;
				y /= 480.0;

				// aspect correction
				x *= 4.0 / 3.0;
				x /= GetAspectRatio();

				// recenter
				x += 0.5;
				break;
			case JUSTIFY_480P_RIGHT:
				x = 640 - x;
				x /= 640.0;
				y /= 480.0;

				// aspect correction
				x *= 4.0 / 3.0;
				x /= GetAspectRatio();

				// recenter
				x = 1 - x;
				break;
		}
	}

	static bool Draw1080pSprite(eJustify justify, float left, float right, float top, float bottom, NyaDrawing::CNyaRGBA32 rgb, TEXTURE_TYPE* texture, ImVec2 uvMin = {0,0}, ImVec2 uvMax = {1,1}) {
		DoJustify(justify, left, top);
		DoJustify(justify, right, bottom);
		return DrawRectangle(left, right, top, bottom, rgb, 0, texture, 0, uvMin, uvMax);
	}

	static void Draw1080pString(eJustify justify, tNyaStringData data, const std::string& text, void(*drawFunc)(const tNyaStringData&, const std::string&)) {
		DoJustify(justify, data.x, data.y);
		return DrawString(data, text, drawFunc);
	}

	static void Draw1080pStringBottomMost(eJustify justify, tNyaStringData data, const std::string& text, void(*drawFunc)(const tNyaStringData&, const std::string&)) {
		DoJustify(justify, data.x, data.y);
		drawFunc(data, text);
	}

	static float GetFlashingAlpha(float timer) {
		return (0.75 + 0.25 * (std::cos(timer * 3.141592 * 2) + 1.0) / 2) * 255;
	}

	virtual void Init() {}
	virtual void InitHooks() {}
	virtual void Reset() {}
	virtual void Process() = 0;
};