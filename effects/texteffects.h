// mostly the same as zolikas but replaces more things
class Effect_TransRights : public ChaosEffect {
public:
	Effect_TransRights() : ChaosEffect() {
		sName = "Trans Rights";
		fTimerLength = 120;
	}

	static const char* __fastcall SearchForStringHooked(void*, uint32_t inputHash) {
		return "Trans Rights";
	}

	void InitFunction() override {
		SummonChyron("Trans Rights", "Trans Rights", "Trans Rights");
		NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x56BB80, &SearchForStringHooked);
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0xEB); // remove duplicate racer name check
	}
	void TickFunction(double delta) override {
		Chyron::mArtist = "Trans Rights";
		Chyron::mTitle = "Trans Rights";
		Chyron::mAlbum = "Trans Rights";
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint64_t>(0x56BB80, 0x850091CF801D8B53);
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0x7E);
	}
	bool HasTimer() override { return true; }
} E_TransRights;