class ChaosUIPopup {
public:
	bool bIsVotingOption = false;
	bool bIsHint = false;
	bool bLeftSide = false;
	double fTextTimer = 0;

	bool bHasTimer = false;
	double fTimer = 0;
	double fTimerLength = 0;

	float GetOffscreenPercentage() const {
		if (fTextTimer < 1) return std::lerp(1, 0, easeInOutQuart(fTextTimer));
		return 0;
	}

	void Update(double delta, bool shouldBeOnScreen) {
		if (shouldBeOnScreen) {
			if (fTextTimer < 1) {
				fTextTimer += delta * 2;
			}
			else {
				fTextTimer = 1;
			}
		}
		else {
			if (fTextTimer > 0) {
				fTextTimer -= delta * 2;
			}
			else {
				fTextTimer = 0;
			}
		}
	}

	void Draw(std::string str, float y, bool ignoreTimers) const {
		if (fTextTimer <= 0 && !ignoreTimers) return;

		float effectY = fEffectY;
		float effectSize = fEffectSize;
		float effectSpacing = fEffectSpacing;
		float effectTextureYSpacing = fEffectTextureYSpacing;
		if (bIsVotingOption) {
			effectSize *= fEffectVotingSize;
			effectSpacing *= fEffectVotingSize;
			effectTextureYSpacing *= fEffectVotingSize;
		}
		if (bIsHint) {
			effectY = fEffectHintY;
			effectSize *= fEffectHintSize;
			effectSpacing *= fEffectHintSize;
			effectTextureYSpacing *= fEffectHintSize;
		}

		auto x = fEffectX;
		x = 1 - x;
		x *= GetAspectRatioInv();
		x = 1 - x;
		y = effectY + (effectSpacing * y);

		auto width = GetStringWidth(effectSize, str.c_str());
		float barWidth = ((bHasTimer && !ignoreTimers ? fEffectTextureXSpacing : fEffectTextureXSpacingNoTimer) * GetAspectRatioInv());
		float barTipWidth = (fEffectTextureTipX * GetAspectRatioInv());

		if (fTextTimer < 1 && !ignoreTimers) x = std::lerp(1 + width + barWidth + barTipWidth, x, easeInOutQuart(fTextTimer));

		static auto textureBarL = LoadTexture("CwoeeChaos/data/textures/effectbg_bar.png");
		static auto textureTipL = LoadTexture("CwoeeChaos/data/textures/effectbg_end.png");
		static auto textureBarD = LoadTexture("CwoeeChaos/data/textures/effectbg_dark_bar.png");
		static auto textureTipD = LoadTexture("CwoeeChaos/data/textures/effectbg_dark_end.png");
		auto textureBar = bDarkMode ? textureBarD : textureBarL;
		auto textureTip = bDarkMode ? textureTipD : textureTipL;
		if (textureBar && textureTip) {
			float barX = x - width - barWidth;
			if (bLeftSide) {
				DrawRectangle(1 - barX, 0, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureBar);
				DrawRectangle(1 - (barX - barTipWidth), 1 - barX, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureTip);
			}
			else {
				DrawRectangle(barX, 1, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureBar);
				DrawRectangle(barX - barTipWidth, barX, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureTip);
			}
			if (!ignoreTimers && bHasTimer && fTimer > 0.1) {
				float arcX = barX - (fEffectArcX * GetAspectRatioInv());
				DrawArc(arcX, y, fEffectArcSize, fEffectArcThickness, fEffectArcRotation, fEffectArcRotation - ((fTimer / fTimerLength) * std::numbers::pi * 2), {255,255,255,255});
				if (fTimer < 5 && fTimerLength > 5) {
					tNyaStringData data;
					data.x = arcX;
					data.y = y;
					data.size = fEffectTimerTextSize;
					data.XCenterAlign = true;
					data.outlinea = 255;
					data.outlinedist = 0.025;
					int timer = ((int)fTimer) + 1;
					if (timer < 1) timer = 1;
					if (timer > 5) timer = 5;
					DrawString(data, std::to_string(timer));
				}
			}
		}

		tNyaStringData data;
		data.x = x;
		data.y = y;
		data.size = effectSize;
		data.XRightAlign = true;
		if (bLeftSide) {
			data.x = 1 - x;
			data.XRightAlign = false;
		}
		if (!bDarkMode) {
			data.outlinea = 255;
			data.outlinedist = 0.025;
		}
		DrawString(data, str);
	}
};