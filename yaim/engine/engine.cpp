//
//  engine.cpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#include "engine.hpp"

#define hBPC HookState.backspaceCount
#define hNCC HookState.newCharCount
#define hCode HookState.code
#define hData HookState.charData
#define ORD getCharacterCode
#define CHR(index) toupper((UInt16)TypingWord[index])

vKeyHookState HookState;

/**
 * data structure of each element in TypingWord (Uint64)
 * first 16 bits is character code
 * bit 17: has tone ^ or not
 * bit 18: has tone w or not
 * bit 19-23: has mark or not (sắc, huyền, hỏi, ngã, nặng)
 * bit 24: is standalone key? (w)
 */
UInt32 TypingWord[MAX_BUFF];
char _index = 0;
vector<vector<UInt32>> _typingStates;

UInt16 TypingKeys[MAX_BUFF];
char _stateIndex = 0;
vector<vector<UInt16>> _keyStates;

char vowelCount = 0;
char VSI = 0; // vowelStartIndex
char VEI = 0; // vowelEndIndex
char VWSM = 0; // vowelWillSetMark
char _spaceCount = 0;

void startNewSession() {
    _index = _stateIndex = _spaceCount = 0;
}

void* vKeyInit() {
    _typingStates.clear();
    _keyStates.clear();
    startNewSession();
    return &HookState;
}

bool isKeyIn(const char& charCode, vector<char>& charset) {
    return std::find(charset.begin(), charset.end(), charCode) != charset.end();
}

void saveTypingHistory() {
    if (_index > 0) {
        char i;
        vector<UInt32> _typingStatesData;
        for (i = 0; i < _index; i++) {
            _typingStatesData.push_back(TypingWord[i]);
        }
        _typingStatesData.push_back(_spaceCount);
        _typingStates.push_back(_typingStatesData);
        if (_typingStates.size() > MAX_WORD) {
            _typingStates.erase(_typingStates.begin());
        }

        vector<UInt16> _keyStatesData;
        for (i = 0; i < _stateIndex; i++) {
            _keyStatesData.push_back(TypingKeys[i]);
        }
        _keyStates.push_back(_keyStatesData);
        if (_keyStates.size() > MAX_WORD) {
            _keyStates.erase(_keyStates.begin());
        }
    }
}

void addToTypingWord(const UInt16& charCode) {
    TypingWord[_index++] = charCode;
}

void addToTypingKeys(const UInt16& charCode) {
    if (_stateIndex >= MAX_BUFF) {
        saveTypingHistory();
        startNewSession();
    }
    TypingKeys[_stateIndex++] = charCode;
}

void restoreLastTypingState() {
    if (_typingStates.size() > 0) {
        vector<UInt32> _typingStatesData = _typingStates.back();
        _typingStates.pop_back();
        vector<UInt16> _keyStatesData = _keyStates.back();
        _keyStates.pop_back();

        _spaceCount = _typingStatesData.back();
        for (char i = 0; i < _typingStatesData.size() - 1; i++) {
            TypingWord[i] = _typingStatesData[i];
        }
        _index = _typingStatesData.size() - 1;

        for (char i = 0; i < _keyStatesData.size(); i++) {
            TypingKeys[i] = _keyStatesData[i];
        }
        _stateIndex = _keyStatesData.size();
    }
}

bool checkCorrectVowel(vector<char>& charset, const char& markKey) {
    // ignore "qu" case
    if (_index >= 2 && CHR(_index - 1) == 'U' && CHR(_index - 2) == 'Q') {
        return false;
    }

    char s = charset.size();
    for (char j = 0; j < s; j++) {
        if (charset[j] != CHR(_index - s + j)) {
            return false;
        }
    }
    if (_index > s && CHR(_index - s - 1) == CHR(_index - s)) {
        return false;
    }

    // limit mark for end consonant: "C", "T", "P", "CH"
    if ((markKey == 'F' || markKey == 'X' || markKey == 'R') &&
        (charset.back() == 'T' || charset.back() == 'C' || charset.back() == 'P' ||
         (charset.back() == 'H' && charset[1] == 'C'))) {
        return false;
    }

    return true;
}

UInt16 getCharacterCode(const UInt32& data) {
    if (!(data & (MARK_MASK | TONE_MASK | TONEW_MASK))) {
        return data;
    }

    UInt32 key = data & (toupper((char)data) | TONE_MASK | TONEW_MASK);
    char markIndex = 1;

    switch (data & MARK_MASK) {
        case MARK1_MASK:
            markIndex = 3;
            break;
        case MARK2_MASK:
            markIndex = 5;
            break;
        case MARK3_MASK:
            markIndex = 7;
            break;
        case MARK4_MASK:
            markIndex = 9;
            break;
        case MARK5_MASK:
            markIndex = 11;
            break;
    }

    return _codeTable[key][markIndex - ((char)data >= 'A' && (char)data <= 'Z')];
}

void findAndCalculateVowel() {
    vowelCount = 0;
    VSI = VEI = 0;
    for (char i = _index - 1; i >= 0; i--) {
        if (!isKeyIn(CHR(i), _vowels)) {
            if (vowelCount > 0) {
                break;
            }
        } else { // is vowel
            if ((CHR(i) == 'U' && i && CHR(i - 1) == 'Q') ||
                (CHR(i) == 'I' && i && CHR(i - 1) == 'G' && vowelCount && !(TypingWord[i] & MARK_MASK))) {
                break;
            }
            if (vowelCount == 0) {
                VEI = i;
            }
            VSI = i;
            vowelCount++;
        }
    }
}

void removeMark(const char& charCode) {
    findAndCalculateVowel();
    if (_index > 0) {
        for (char i = VSI; i <= VEI; i++) {
            if (TypingWord[i] & MARK_MASK) {
                TypingWord[i] &= ~MARK_MASK;
                hCode = vWillProcess;
                for (char j = _index - 1; j >= VSI; j--) {
                    hData[_index - 1 - j] = ORD(TypingWord[j]);
                }
                hNCC = hBPC = _index - VSI;
                _stateIndex -= 2;
                return;
            }
        }
    }

    addToTypingWord(charCode);
}

bool canHasEndConsonant() {
    vector<vector<UInt32>>& vo = _vowelCombine[CHR(VSI)];
    for (char i = 0; i < vo.size(); i++) {
        char k = VSI;
        char j;
        for (j = 1; j < vo[i].size(); j++) {
            if (k > VEI || ((CHR(k) | (TypingWord[k] & (TONE_MASK | TONEW_MASK))) != vo[i][j])) {
                break;
            }
            k++;
        }
        if (j >= vo[i].size()) {
            return vo[i][0] == 1;
        }
    }
    return false;
}

void calcMarkPosition() {
    VWSM = VSI;

    // rule 2
    if (vowelCount == 3 || (VEI + 1 < _index && !isKeyIn(CHR(VEI + 1), _vowels) && canHasEndConsonant())) {
        VWSM = VSI + 1;
    }

    // rule 3
    for (char i = VSI; i <= VEI; i++) {
        if ((CHR(i) == 'E' && TypingWord[i] & TONE_MASK) || (CHR(i) == 'O' && TypingWord[i] & TONEW_MASK)) {
            VWSM = i;
            break;
        }
    }

    hBPC = _index - VWSM;
}

void insertMark(const UInt32& markMask, const bool& canModifyFlag=true) {
    vowelCount = 0;

    if (canModifyFlag) {
        hCode = vWillProcess;
    }

    findAndCalculateVowel();
    VWSM = 0;

    if (vowelCount == 1) {
        VWSM = VEI;
        hBPC = _index - VEI;
    } else {
        calcMarkPosition();
        if (TypingWord[VEI] & (TONE_MASK | TONEW_MASK)) {
            VWSM = VEI;
        }
    }

    if (TypingWord[VWSM] & markMask) {
        // if duplicate same mark -> restore
        TypingWord[VWSM] &= ~MARK_MASK;
        if (canModifyFlag) {
            hCode = vRestore;
        }
        for (char i = VSI; i < _index; i++) {
            TypingWord[i] &= ~MARK_MASK;
            hData[_index - 1 - i] = ORD(TypingWord[i]);
        }
    } else {
        // remove other mark
        if (TypingWord[VWSM] & MARK_MASK) {
            TypingWord[VWSM] &= ~MARK_MASK;
            TypingKeys[_stateIndex - 1] = TypingKeys[--_stateIndex];
        }

        // add mark
        TypingWord[VWSM] |= markMask;
        for (char i = VSI; i < _index; i++) {
            if (i != VWSM) { // remove mark for other vowel
                TypingWord[i] &= ~MARK_MASK;
            }
            hData[_index - 1 - i] = ORD(TypingWord[i]);
        }

        hBPC = _index - VSI;
    }
    hNCC = hBPC;
}

void processMark(const char& charCode) {
    for (char i = 0; i < _rimesForMark.size(); i++) {
        if (_index < _rimesForMark[i].size()) {
            continue;
        }
        if (checkCorrectVowel(_rimesForMark[i], toupper(charCode))) {
            switch (toupper(charCode)) {
                case 'S':
                    return insertMark(MARK1_MASK);
                case 'F':
                    return insertMark(MARK2_MASK);
                case 'R':
                    return insertMark(MARK3_MASK);
                case 'X':
                    return insertMark(MARK4_MASK);
                case 'J':
                    return insertMark(MARK5_MASK);
                default:
                    return;
            }
        }
    }

    addToTypingWord(charCode);
}

void insertD(const char& charCode) {
    for (char i = 0; i < _consonantD.size(); i++) {
        if (_index < _consonantD[i].size()) {
            continue;
        }
        if (checkCorrectVowel(_consonantD[i], toupper(charCode))) {
            hCode = vWillProcess;
            hBPC = 0;
            for (char j = _index - 1; j >= 0; j--) {
                hBPC++;
                if (CHR(j) == 'D') {
                    if (TypingWord[j] & TONE_MASK) {
                        // restore
                        hCode = vRestore;
                        TypingWord[j] &= ~TONE_MASK;
                        hData[_index - 1 - j] = TypingWord[j];
                    } else {
                        TypingWord[j] |= TONE_MASK;
                        hData[_index - 1 - j] = ORD(TypingWord[j]);
                    }
                    break;
                } else {  // preresent old char
                    hData[_index - 1 - j] = ORD(TypingWord[j]);
                }
            }
            hNCC = hBPC;
            return;
        }
    }

    addToTypingWord(charCode);
}

void insertAOE(const char& charCode) {
    findAndCalculateVowel();

    // remove W tone
    char i;
    for (i = VSI; i <= VEI; i++) {
        TypingWord[i] &= ~TONEW_MASK;
    }

    hCode = vWillProcess;
    hBPC = 0;

    for (i = _index - 1; i >= 0; i--) {
        hBPC++;
        if (CHR(i) == toupper(charCode)) {
            if (TypingWord[i] & TONE_MASK) {
                hCode = vRestore;
                TypingWord[i] &= ~TONE_MASK;
                hData[_index - 1 - i] = TypingWord[i];
            } else {
                TypingWord[i] |= TONE_MASK;
                if (toupper(charCode) != 'D') {
                    TypingWord[i] &= ~TONEW_MASK;
                }
                hData[_index - 1 - i] = ORD(TypingWord[i]);

            }
            break;
        } else {
            hData[_index - 1 - i] = ORD(TypingWord[i]);
        }
    }
    hNCC = hBPC;
}

void insertW(const char& charCode) {
    findAndCalculateVowel();

    // remove ^ tone
    char i;
    for (i = VSI; i <= VEI; i++) {
        TypingWord[i] &= ~TONE_MASK;
    }

    if (vowelCount > 1) {
        hBPC = _index - VSI;
        hNCC = hBPC;

        if ((TypingWord[VSI] & TONEW_MASK && (TypingWord[VSI + 1] & TONEW_MASK ||
                                              CHR(VSI + 1) == 'I' ||
                                              CHR(VSI + 1) == 'A' ||
                                              CHR(VSI + 1) == 'U')) ||
            (!(TypingWord[VSI] & TONEW_MASK) && TypingWord[VSI + 1] & TONEW_MASK && VSI == _index - 2)) {
            // restore
            hCode = vRestore;
            for (i = VSI; i < _index; i++) {
                TypingWord[i] &= ~TONEW_MASK;
                hData[_index - 1 - i] = ORD(TypingWord[i]) & ~STANDALONE_MASK;
            }
        } else {
            hCode = vWillProcess;

            if ((CHR(VSI) == 'U' && CHR(VSI + 1) == 'O')) {
                TypingWord[VSI + 1] |= TONEW_MASK;
                if (!VSI || TypingWord[VSI - 1] != 'H' || VSI < _index - 2) {
                    TypingWord[VSI] |= TONEW_MASK;
                }
            } else if ((CHR(VSI) == 'U' && CHR(VSI + 1) == 'A') ||
                       (CHR(VSI) == 'U' && CHR(VSI + 1) == 'I') ||
                       (CHR(VSI) == 'U' && CHR(VSI + 1) == 'U') ||
                       (CHR(VSI) == 'O' && CHR(VSI + 1) == 'I')) {
                TypingWord[VSI] |= TONEW_MASK;
            } else if (CHR(VSI) == 'O' && CHR(VSI + 1) == 'A') {
                TypingWord[VSI + 1] |= TONEW_MASK;
            } else {
                hCode = vDoNothing;
                addToTypingWord(charCode);
            }

            for (i = VSI; i < _index; i++) {
                hData[_index - 1 - i] = ORD(TypingWord[i]);
            }
        }

        return;
    }

    hCode = vWillProcess;

    for (i = _index - 1; i >= VSI; i--) {
        switch (CHR(i)) {
            case 'A':
            case 'U':
            case 'O':
                if (TypingWord[i] & TONEW_MASK) {
                    if (TypingWord[i] & STANDALONE_MASK) {
                        hCode = vWillProcess;
                        TypingWord[i] = (char)TypingWord[i] == 'U' ? 'W' : 'w';
                    } else {
                        hCode = vRestore;
                        TypingWord[i] &= ~TONEW_MASK;
                    }
                    hData[_index - 1 - i] = TypingWord[i];
                } else {
                    TypingWord[i] |= TONEW_MASK;
                    TypingWord[i] &= ~TONE_MASK;
                    hData[_index - 1 - i] = ORD(TypingWord[i]);
                }
                break;
            default:
                hData[_index - 1 - i] = ORD(TypingWord[i]);
                break;
        }
    }

    hNCC = hBPC = _index - VSI;
}

void reserveLastStandaloneChar(const char& charCode) {
    hCode = vWillProcess;
    hNCC = hBPC = 0;
    TypingWord[_index] = (charCode | TONEW_MASK | STANDALONE_MASK);
    hData[hNCC++] = ORD(TypingWord[_index++]);
}

void checkForStandaloneW(const char& charCode) {
    if (_index && std::find(_standaloneWbad.begin(), _standaloneWbad.end(), CHR(_index - 1)) != _standaloneWbad.end()) {
        return addToTypingWord(charCode);
    }
    reserveLastStandaloneChar(charCode == 'W' ? 'U' : 'u');
}

void processTone(const char& charCode) {
    vector<vector<char>>& charset = _rimes[toupper(charCode)];

    for (char i = 0; i < charset.size(); i++) {
        if (_index < charset[i].size()) {
            continue;
        }
        if (checkCorrectVowel(charset[i], toupper(charCode))) {
            if (toupper(charCode) == 'W') {
                insertW(charCode);
            } else {
                insertAOE(charCode);
            }
            return;
        }
    }

    if (toupper(charCode) == 'W') {
        checkForStandaloneW(charCode);
    } else {
        addToTypingWord(charCode);
    }
}

void restoreTyping() {
    if (_index < _stateIndex) {
        hCode = vWillProcess;
        hBPC = _index + _spaceCount;
        hNCC = _stateIndex;
        for (char i = 0; i < _stateIndex; i++) {
            TypingWord[i] = TypingKeys[i];
            hData[_stateIndex - 1 - i] = TypingWord[i];
        }
        _index = _stateIndex;
        _spaceCount = 0;
    } else {
        hCode = vDoNothing;
    }
}

void checkSpelling(const int& deltaBackSpace) {
    if (_index <= 1 || _index >= MAX_BUFF) {
        return;
    }

    findAndCalculateVowel();
    if (vowelCount == 0) {
        return;
    }

    bool needCorrect = false;
    char i = 0;

    // check for case: "uơn", "ưoi", "ưom", "ưoc", "uơu"
    if (_index >= 3) {
        for (i = _index - 1; i >= 0; i--) {
            if (CHR(i) == 'U' && i < _index - 2 && CHR(i + 1) == 'O' &&
                (CHR(i + 2) == 'N' || CHR(i + 2) == 'C' || CHR(i + 2) == 'I' ||
                 CHR(i + 2) == 'M' || CHR(i + 2) == 'P' || CHR(i + 2) == 'T' ||
                 CHR(i + 2) == 'U') &&
                ((TypingWord[i] & TONEW_MASK) ^ (TypingWord[i + 1] & TONEW_MASK))) {
                TypingWord[i] |= TONEW_MASK;
                TypingWord[i + 1] |= TONEW_MASK;
                needCorrect = true;
                break;
            }
        }
    }

    // check mark
    if (_index >= 2) {
        for (i = VSI; i <= VEI; i++) {
            if (TypingWord[i] & MARK_MASK) {
                UInt32 mark = TypingWord[i] & MARK_MASK;
                TypingWord[i] &= ~MARK_MASK;
                insertMark(mark, false);
                if (i != VWSM) {
                    needCorrect = true;
                }
                break;
            }
        }
    }

    if (needCorrect) {
        if (hCode ==vDoNothing) {
            hCode = vWillProcess;
        }
        char right = VSI < i ? VSI : i;
        for (char j = _index - 1; j >= right; j--) {
            hData[_index - 1 - j] = ORD(TypingWord[j]);
        }
        hNCC = _index - right;
        hBPC = _index - right + deltaBackSpace;
    }
}

void vKeyHandleEvent(const UInt16& charCode) {
    if (charCode == 0x1b) { // ESC
        restoreTyping();
    } else if (charCode == 0x08) { // Backspace
        hCode = vDoNothing;
        if (_spaceCount > 0) {
            _spaceCount--;
        } else if (_index > 0) {
            if(--_index == 0){
                _stateIndex = 0;
                restoreLastTypingState();
            } else {
                _stateIndex--;
                checkSpelling(1);
            }
        }
    } else if (toupper(charCode) < 'A' || toupper(charCode) > 'Z') {
        hCode = vDoNothing;
        _spaceCount++;
    } else {
        if (_spaceCount > 0) {
            saveTypingHistory();
            startNewSession();
        }

        addToTypingKeys(charCode);
        hCode = vDoNothing;

        switch (toupper(charCode)) {
            case 'Z':
                removeMark(charCode);
                break;
            case 'D':
                insertD(charCode);
                break;
            case 'S':
            case 'F':
            case 'R':
            case 'J':
            case 'X':
                processMark(charCode);
                break;
            case 'A':
            case 'E':
            case 'O':
            case 'W':
                processTone(charCode);
                break;
            default:
                addToTypingWord(charCode);
                break;
        }

        if (toupper(charCode) >= 'A' && toupper(charCode) <= 'Z' && toupper(charCode) != 'D') {
            checkSpelling(0 - (hCode == vDoNothing));
        }

        if (hCode == vRestore) {
            addToTypingWord(charCode);
            _stateIndex--;
        }
    }
}
