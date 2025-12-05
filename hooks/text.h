namespace TextHook {
	bool bReverseText = false;
	bool bRandomText = false;
	const char* pReplaceText = nullptr;
	bool bShuffledText = false;
	const char* pInterspersedText = nullptr;
	const char* pInterspersedTextUpper = nullptr;
	const char* pInterspersedTextProper = nullptr;

	bool CanStringBeReversed(const std::string& str) {
		for (auto& c : str) {
			if (c == '%') return false;
		}
		return true;
	}

	bool CanStringBeRandomized(const std::string& str) {
		return CanStringBeReversed(str);
	}

	bool CanStringBeShuffled(const std::string& str) {
		return CanStringBeReversed(str);
	}

	bool CanStringBeInterspersed(const std::string& str) {
		return CanStringBeReversed(str) && str != ">";
	}

	struct tRandomizeTextAssoc {
		uint32_t origHash;
		uint32_t newHash;
		const char* newString;
	};
	std::vector<tRandomizeTextAssoc> RandomTextCache;
	bool RandomTextNoRepeats = true;

	struct tReverseTextAssoc {
		uint32_t origHash;
		char* newString;
	};
	std::vector<tReverseTextAssoc> ReverseTextCache;
	std::vector<tReverseTextAssoc> ShuffledTextCache;
	std::vector<tReverseTextAssoc> InterspersedTextCache;

	bool IsRandomTextHashUsed(uint32_t hash) {
		if (!RandomTextNoRepeats) return false;

		auto str = SearchForString(nullptr, hash);
		for (auto& cache : RandomTextCache) {
			if (cache.newHash == hash) return true;
			if (!strcmp(str, cache.newString)) return true;
		}
		return false;
	}

	uint32_t GetRandomizedText(uint32_t hash) {
		for (auto& cache : RandomTextCache) {
			if (cache.origHash == hash) return cache.newHash;
		}

		uint32_t newHash;
		auto str = SearchForString(nullptr, hash);
		auto len = strlen(str);
		do {
			auto id = rand() % NumStringRecords;
			newHash = RecordTable[id].Hash;
			str = RecordTable[id].PackedString;
		} while (!str || !CanStringBeRandomized(str) || strlen(str) > len || IsRandomTextHashUsed(newHash));
		RandomTextCache.push_back({hash, newHash, SearchForString(nullptr, newHash)});
		return newHash;
	}

	const char* GetReversedText(uint32_t hash) {
		for (auto& cache : ReverseTextCache) {
			if (cache.origHash == hash) return cache.newString;
		}

		std::string reversed = SearchForString(nullptr, hash);
		std::reverse(reversed.begin(), reversed.end());

		auto text = new char[reversed.length()+1];
		strcpy_s(text, reversed.length()+1, reversed.c_str());
		ReverseTextCache.push_back({hash, text});
		return text;
	}

	std::vector<std::string> SplitStringIntoWords(const std::string& in) {
		std::string tmp = in;
		std::vector<std::string> words;

		size_t pos = 0;
		while ((pos = tmp.find(" ")) != std::string::npos) {
			words.push_back(tmp.substr(0, pos));
			tmp.erase(0, pos + 1);
		}
		if (!tmp.empty()) words.push_back(tmp);
		return words;
	}

	const char* GetShuffledText(uint32_t hash) {
		for (auto& cache : ShuffledTextCache) {
			if (cache.origHash == hash) return cache.newString;
		}

		auto words = SplitStringIntoWords(SearchForString(nullptr, hash));
		std::string str;
		for (auto& word : words) {
			if (CanStringBeShuffled(word)) {
				std::random_shuffle(word.begin(), word.end());
			}
			str += word;
			if (&word != &words[words.size()-1]) str += " ";
		}

		auto text = new char[str.length()+1];
		strcpy_s(text, str.length()+1, str.c_str());
		ShuffledTextCache.push_back({hash, text});
		return text;
	}

	std::string DoIntersperseWord(std::string origWord) {
		std::string str = pInterspersedText;
		std::string ending;
		while (origWord.ends_with('.') || origWord.ends_with(',') || origWord.ends_with(':') || origWord.ends_with('\n')) {
			ending = origWord[origWord.length()-1] + ending;
			origWord.pop_back();
		}
		if (std::all_of(origWord.begin(), origWord.end(), [](unsigned char c){ return std::isdigit(c); })) {
			str = rand() % 100 > 50 ? pInterspersedTextUpper : pInterspersedText;
		}
		else if (std::all_of(origWord.begin(), origWord.end(), [](unsigned char c){ return std::isupper(c); })) {
			str = pInterspersedTextUpper;
		}
		else if (std::isupper(origWord[0])) {
			str = pInterspersedTextProper;
		}
		str += ending;
		return str;
	}

	const char* GetInterspersedText(uint32_t hash) {
		for (auto& cache : InterspersedTextCache) {
			if (cache.origHash == hash) return cache.newString;
		}

		auto words = SplitStringIntoWords(SearchForString(nullptr, hash));
		int numWordsChanged = 0;
		int numWordsChangeable = 0;
		std::string str;
		for (auto& word : words) {
			if (CanStringBeInterspersed(word)) {
				if (rand() % 100 < 10) {
					word = DoIntersperseWord(word);
					numWordsChanged++;
				}
				numWordsChangeable++;
			}
		}
		if (!numWordsChanged && numWordsChangeable > 0) {
			bool changed = false;
			while (!changed) {
				auto &word = words[rand() % words.size()];
				if (CanStringBeInterspersed(word)) {
					word = DoIntersperseWord(word);
					numWordsChanged++;
					changed = true;
				}
			}
		}

		// don't always replace one-word strings
		if (words.size() == 1 && rand() % 100 > 25) {
			str = SearchForString(nullptr, hash);
		}
		else for (auto& word : words) {
			str += word;
			if (&word != &words[words.size()-1]) str += " ";
		}

		auto text = new char[str.length()+1];
		strcpy_s(text, str.length()+1, str.c_str());
		InterspersedTextCache.push_back({hash, text});
		return text;
	}

	const char* __fastcall SearchForStringHooked(void* a1, uint32_t a2) {
		auto str = SearchForString(a1, a2);
		if (!str) return nullptr;
		if (bRandomText && CanStringBeRandomized(str)) {
			str = SearchForString(a1, GetRandomizedText(a2));
		}
		if (pReplaceText) str = pReplaceText;
		if (pInterspersedText) {
			str = GetInterspersedText(a2);
		}
		if (bReverseText && CanStringBeReversed(str)) {
			str = GetReversedText(a2);
		}
		if (bShuffledText) {
			str = GetShuffledText(a2);
		}
		return str;
	}

	ChloeHook Hook_Text([]() {
		uintptr_t addresses[] = {
				0x56BC04,
				0x56BC14,
				0x56BC34,
				0x56BC42,
				0x56BC64,
				0x56BCF0,
				0x56DA95,
				0x56DAA3,
				0x56DAB7,
				0x56DAC5,
				0x57A50E,
				0x57A51C,
				0x57A56B,
				0x57A579,
				0x57A594,
				0x57A5A2,
				0x57AD35,
				0x57AD43,
				0x57B830,
				0x57B83E,
				0x57B959,
				0x57B967,
				0x57D7DA,
				0x57D7E8,
				0x57D813,
				0x57D821,
				0x57E924,
				0x57F505,
				0x58BE68,
				0x58BE76,
				0x58BE94,
				0x58BEA4,
				0x58BEB0,
				0x58BEBE,
				0x58C083,
				0x58C091,
				0x58C2B7,
				0x58C2C5,
				0x58C2EC,
				0x58C2FA,
				0x58C357,
				0x58C365,
				0x58E543,
				0x58E551,
				0x58E581,
				0x58E58F,
				0x5911E2,
				0x594B4C,
				0x594B5A,
				0x594C0B,
				0x594C19,
				0x595073,
				0x595081,
				0x595322,
				0x595330,
				0x595AB9,
				0x595AC7,
				0x595B27,
				0x595B39,
				0x597E9F,
				0x597EAD,
				0x597EC1,
				0x597ECF,
				0x597FF7,
				0x598005,
				0x598065,
				0x598073,
				0x5985CA,
				0x5985D8,
				0x59870A,
				0x598718,
				0x59885A,
				0x598868,
				0x5989BA,
				0x5989C8,
				0x598A6A,
				0x598A78,
				0x598C0A,
				0x598C18,
				0x598C9A,
				0x598CA8,
				0x598DDA,
				0x598DE8,
				0x598FAA,
				0x598FB8,
				0x59B0D6,
				0x59B0E4,
				0x59B1E3,
				0x59B1F1,
				0x5A2944,
				0x5A2952,
				0x5A2B3A,
				0x5A2B48,
				0x5A2B78,
				0x5A2B88,
				0x5A2D08,
				0x5A2D18,
				0x5A2D24,
				0x5A2D32,
				0x5A2D86,
				0x5A2D94,
				0x5A2E39,
				0x5A2E49,
				0x5A2E55,
				0x5A2E63,
				0x5A2ED7,
				0x5A2EE7,
				0x5A2EF3,
				0x5A2F01,
				0x5A2F6F,
				0x5A2F7D,
				0x5A5FE8,
				0x5A5FF8,
				0x5A9ABD,
				0x5A9ACB,
				0x5AB499,
				0x5AB4A7,
				0x5AB4E2,
				0x5AB4F0,
		};
		for (auto& addr : addresses) {
			NyaHookLib::PatchRelative(NyaHookLib::CALL, addr, &SearchForStringHooked);
		}
		NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x56BC22, &SearchForStringHooked);
		NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x56BC72, &SearchForStringHooked);
		NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x56BCFE, &SearchForStringHooked);
	});
}