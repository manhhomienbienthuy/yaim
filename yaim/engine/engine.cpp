//
//  engine.cpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#include "engine.hpp"

vector<Byte> _whiteSpaces = {kVK_Space, kVK_Tab, kVK_Return};
vector<Byte> _vowels = {kVK_ANSI_A, kVK_ANSI_E, kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_I, kVK_ANSI_O};

#define hBPC HookState.backspaceCount
#define hNCC HookState.newCharCount
#define hCode HookState.code
#define hData HookState.charData
#define GET getCharacterCode
#define CHR(index) (Uint16)TypingWord[index]

// data to sendback to main program
vKeyHookState HookState;

// private data
/**
 * data structure of each element in TypingWord (Uint64)
 * first 2 byte is character code or key code.
 * bit 16: has caps or not
 * bit 17: has tone ^ or not
 * bit 18: has tone w or not
 * bit 19 - > 23: has mark or not (Sắc, huyền, hỏi, ngã, nặng)
 * bit 24: is standalone key? (w, [, ])
 * bit 25: is character code or keyboard code; 1: character code; 0: keycode
 */
Uint32 TypingWord[MAX_BUFF];
Byte _index = 0;
vector<vector<Uint32>> _typingStates;

/**
 * use for restore key press esc
 */
Uint32 TypingKeys[MAX_BUFF];
Byte _stateIndex = 0;
vector<vector<Uint32>> _keyStates;

Byte vowelCount = 0;
Byte VSI = 0; // vowelStartIndex
Byte VEI = 0; // vowelEndIndex
Byte VWSM = 0; // vowelWillSetMark
Byte _spaceCount = 0;

void startNewSession() {
    _index = _stateIndex = _spaceCount = 0;
}

void* vKeyInit() {
    _typingStates.clear();
    _keyStates.clear();
    startNewSession();
    return &HookState;
}

bool isKeyIn(const Uint16& data, vector<Byte>& charset) {
    return std::find(charset.begin(), charset.end(), data) != charset.end();
}

void saveTypingHistory() {
    // save word history
    if (_index > 0) {
        char i;
        vector<Uint32> _typingStatesData;
        for (i = 0; i < _index; i++) {
            _typingStatesData.push_back(TypingWord[i]);
        }
        _typingStatesData.push_back(_spaceCount);
        _typingStates.push_back(_typingStatesData);
        if (_typingStates.size() > MAX_WORD) {
            _typingStates.erase(_typingStates.begin());
        }

        vector<Uint32> _keyStatesData;
        for (i = 0; i < _stateIndex; i++) {
            _keyStatesData.push_back(TypingKeys[i]);
        }
        _keyStates.push_back(_keyStatesData);
        if (_keyStates.size() > MAX_WORD) {
            _keyStates.erase(_keyStates.begin());
        }
    }
}

void addToTypingWord(const Uint16& keyCode, const bool& isCaps) {
    TypingWord[_index++] = keyCode | (isCaps ? CAPS_MASK : 0);
}

void addToTypingKeys(const Uint16& keyCode, const bool& isCaps) {
    if (_stateIndex >= MAX_BUFF) {
        saveTypingHistory();
        startNewSession();
    }
    TypingKeys[_stateIndex++] = keyCode | (isCaps ? CAPS_MASK : 0);
}

void restoreLastTypingState() {
    if (_typingStates.size() > 0) {
        vector<Uint32> _typingStatesData = _typingStates.back();
        _typingStates.pop_back();
        vector<Uint32> _keyStatesData = _keyStates.back();
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

bool checkCorrectVowel(vector<Uint16>& charset, const Uint16& markKey) {
    // ignore "qu" case
    if (_index >= 2 && CHR(_index - 1) == kVK_ANSI_U && CHR(_index - 2) == kVK_ANSI_Q) {
        return false;
    }

    char s = charset.size();
    for (char j = s - 1; j >= 0; j--) {
        if (charset[j] != CHR(_index - s + j)) {
            return false;
        }
    }

    // limit mark for end consonant: "C", "T"
    if (s > 1 && (markKey == kVK_ANSI_F || markKey == kVK_ANSI_X || markKey == kVK_ANSI_R)) {
        if (charset[1] == kVK_ANSI_C || charset[1] == kVK_ANSI_T) {
            return false;
        } else if (s > 2 && (charset[2] == kVK_ANSI_T)) {
            return false;
        }
    }

    if (_index > s && CHR(_index - s - 1) == CHR(_index - s)) {
        return false;
    }

    return true;
}

Uint32 getCharacterCode(const Uint32& data) {
    char capsElem = (data & CAPS_MASK) ? 0 : 1;
    Uint32 key = data & CHAR_MASK;

    if (data & MARK_MASK) { // has mark
        char markElem = -2;
        switch (data & MARK_MASK) {
            case MARK1_MASK:
                markElem = 0;
                break;
            case MARK2_MASK:
                markElem = 2;
                break;
            case MARK3_MASK:
                markElem = 4;
                break;
            case MARK4_MASK:
                markElem = 6;
                break;
            case MARK5_MASK:
                markElem = 8;
                break;
        }
        markElem += capsElem;

        if ((key == kVK_ANSI_A ||
             key == kVK_ANSI_E ||
             key == kVK_ANSI_O ||
             key == kVK_ANSI_U) &&
            !(data & (TONE_MASK | TONEW_MASK))) {
                markElem += 4;
        }

        if (data & TONE_MASK) {
            key |= TONE_MASK;
        } else if (data & TONEW_MASK) {
            key |= TONEW_MASK;
        }

        if (_codeTable.find(key) != _codeTable.end()) {
            return _codeTable[key][markElem] | CHAR_CODE_MASK;
        }
    } else if ((data & (TONE_MASK | TONEW_MASK)) &&
               (_codeTable.find(key) != _codeTable.end())) {
        return _codeTable[key][capsElem + (data & TONE_MASK ? 0 : 2)] | CHAR_CODE_MASK;
    }

    return data;
}

void findAndCalculateVowel(const bool& isCheckSpelling=false) {
    vowelCount = 0;
    VSI = VEI = 0;
    for (char i = _index - 1; i >= 0; i--) {
        if (!isKeyIn(CHR(i), _vowels)) {
            if (vowelCount > 0) {
                break;
            }
        } else { // is vowel
            if (CHR(i) == kVK_ANSI_U && i && CHR(i - 1) == kVK_ANSI_Q) {
                break;
            }
            if (!isCheckSpelling && CHR(i) == kVK_ANSI_I && i && CHR(i - 1) == kVK_ANSI_G && vowelCount > 0) {
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

void removeMark(const Uint16& data, const bool& isCaps) {
    findAndCalculateVowel(true);
    if (_index > 0) {
        for (char i = VSI; i <= VEI; i++) {
            if (TypingWord[i] & MARK_MASK) {
                TypingWord[i] &= ~MARK_MASK;
                hCode = vWillProcess;
                for (char j = _index - 1; j >= VSI; j--) {
                    hData[_index - 1 - j] = GET(TypingWord[j]);
                }
                hNCC = hBPC = _index - VSI;
                _stateIndex -= 2;
                return;
            }
        }
    }

    addToTypingWord(data, isCaps);
}

bool canHasEndConsonant() {
    vector<vector<Uint32>>& vo = _vowelCombine[CHR(VSI)];
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
        if ((CHR(i) == kVK_ANSI_E && TypingWord[i] & TONE_MASK) || (CHR(i) == kVK_ANSI_O && TypingWord[i] & TONEW_MASK)) {
            VWSM = i;
            break;
        }
    }

    hBPC = _index - VWSM;
}

void insertMark(const Uint32& markMask, const bool& canModifyFlag=true) {
    vowelCount = 0;

    if (canModifyFlag) {
        hCode = vWillProcess;
    }

    findAndCalculateVowel();
    VWSM = 0;

    // detect mark position
    if (vowelCount == 1) {
        VWSM = VEI;
        hBPC = _index - VEI;
    } else {
        calcMarkPosition();
        if (TypingWord[VEI] & (TONE_MASK | TONEW_MASK)) {
            VWSM = VEI;
        }
    }

    // send data
    if (TypingWord[VWSM] & markMask) {
        // if duplicate same mark -> restore
        TypingWord[VWSM] &= ~MARK_MASK;
        if (canModifyFlag) {
            hCode = vRestore;
        }
        for (char i = VSI; i < _index; i++) {
            TypingWord[i] &= ~MARK_MASK;
            hData[_index - 1 - i] = GET(TypingWord[i]);
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
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }

        hBPC = _index - VSI;
    }
    hNCC = hBPC;
}

void processMark(const Uint16& data, const bool& isCaps) {
    for (char i = 0; i < _vowelForMark.size(); i++) {
        vector<vector<Uint16>>& charset = _vowelForMark[i];

        for (char j = 0; j < charset.size(); j++) {
            if (_index < charset[j].size()) {
                continue;
            }
            if (checkCorrectVowel(charset[j], data)) {
                switch (data) {
                    case kVK_ANSI_S:
                        return insertMark(MARK1_MASK);
                    case kVK_ANSI_F:
                        return insertMark(MARK2_MASK);
                    case kVK_ANSI_R:
                        return insertMark(MARK3_MASK);
                    case kVK_ANSI_X:
                        return insertMark(MARK4_MASK);
                    case kVK_ANSI_J:
                        return insertMark(MARK5_MASK);
                    default:
                        return;
                }
            }
        }
    }

    addToTypingWord(data, isCaps);
}

void insertD(const Uint16& data, const bool& isCaps) {
    for (char i = 0; i < _consonantD.size(); i++) {
        if (_index < _consonantD[i].size()) {
            continue;
        }
        if (checkCorrectVowel(_consonantD[i], data)) {
            hCode = vWillProcess;
            hBPC = 0;
            for (char j = _index - 1; j >= 0; j--) {
                hBPC++;
                if (CHR(j) == kVK_ANSI_D) {
                    if (TypingWord[j] & TONE_MASK) {
                        // restore
                        hCode = vRestore;
                        TypingWord[j] &= ~TONE_MASK;
                        hData[_index - 1 - j] = TypingWord[j];
                    } else {
                        TypingWord[j] |= TONE_MASK;
                        hData[_index - 1 - j] = GET(TypingWord[j]);
                    }
                    break;
                } else {  // preresent old char
                    hData[_index - 1 - j] = GET(TypingWord[j]);
                }
            }
            hNCC = hBPC;
            return;
        }
    }

    addToTypingWord(data, isCaps);
}

void insertAOE(const Uint16& data, const bool& isCaps) {
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
        if (CHR(i) == data) {
            if (TypingWord[i] & TONE_MASK) {
                // restore
                hCode = vRestore;
                TypingWord[i] &= ~TONE_MASK;
                hData[_index - 1 - i] = TypingWord[i];
            } else {
                TypingWord[i] |= TONE_MASK;
                if (data != kVK_ANSI_D) {
                    TypingWord[i] &= ~TONEW_MASK;
                }
                hData[_index - 1 - i] = GET(TypingWord[i]);

            }
            break;
        } else { // preresent old char
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }
    }
    hNCC = hBPC;
}

void insertW(const Uint16& data, const bool& isCaps) {
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
                                              CHR(VSI + 1) == kVK_ANSI_I ||
                                              CHR(VSI + 1) == kVK_ANSI_A ||
                                              CHR(VSI + 1) == kVK_ANSI_U)) ||
            (!(TypingWord[VSI] & TONEW_MASK) && TypingWord[VSI + 1] & TONEW_MASK && VSI == _index - 2)) {
            // restore
            hCode = vRestore;
            for (i = VSI; i < _index; i++) {
                TypingWord[i] &= ~TONEW_MASK;
                hData[_index - 1 - i] = GET(TypingWord[i]) & ~STANDALONE_MASK;
            }
        } else {
            hCode = vWillProcess;

            if ((CHR(VSI) == kVK_ANSI_U && CHR(VSI + 1) == kVK_ANSI_O)) {
                TypingWord[VSI + 1] |= TONEW_MASK;
                if (!VSI || TypingWord[VSI - 1] != kVK_ANSI_H || VSI < _index - 2) {
                    TypingWord[VSI] |= TONEW_MASK;
                }
            } else if ((CHR(VSI) == kVK_ANSI_U && CHR(VSI + 1) == kVK_ANSI_A) ||
                       (CHR(VSI) == kVK_ANSI_U && CHR(VSI + 1) == kVK_ANSI_I) ||
                       (CHR(VSI) == kVK_ANSI_U && CHR(VSI + 1) == kVK_ANSI_U) ||
                       (CHR(VSI) == kVK_ANSI_O && CHR(VSI + 1) == kVK_ANSI_I)) {
                TypingWord[VSI] |= TONEW_MASK;
            } else if (CHR(VSI) == kVK_ANSI_O && CHR(VSI + 1) == kVK_ANSI_A) {
                TypingWord[VSI + 1] |= TONEW_MASK;
            } else {
                hCode = vDoNothing;
                addToTypingWord(data, isCaps);
            }

            for (i = VSI; i < _index; i++) {
                hData[_index - 1 - i] = GET(TypingWord[i]);
            }
        }

        return;
    }

    hCode = vWillProcess;

    for (i = _index - 1; i >= VSI; i--) {
        switch (CHR(i)) {
            case kVK_ANSI_A:
            case kVK_ANSI_U:
            case kVK_ANSI_O:
                if (TypingWord[i] & TONEW_MASK) {
                    // restore
                    if (TypingWord[i] & STANDALONE_MASK) {
                        hCode = vWillProcess;
                        if (CHR(i) == kVK_ANSI_U){
                            TypingWord[i] = kVK_ANSI_W | ((TypingWord[i] & CAPS_MASK) ? CAPS_MASK : 0);
                        } else if (CHR(i) == kVK_ANSI_O) {
                            hCode = vRestore;
                            TypingWord[i] = kVK_ANSI_O | ((TypingWord[i] & CAPS_MASK) ? CAPS_MASK : 0);
                        }
                    } else {
                        hCode = vRestore;
                        TypingWord[i] &= ~TONEW_MASK;
                    }
                    hData[_index - 1 - i] = TypingWord[i];
                } else {
                    TypingWord[i] |= TONEW_MASK;
                    TypingWord[i] &= ~TONE_MASK;
                    hData[_index - 1 - i] = GET(TypingWord[i]);
                }
                break;
            default:
                hData[_index - 1 - i] = GET(TypingWord[i]);
                break;
        }
    }

    hNCC = hBPC = _index - VSI;
}

void reserveLastStandaloneChar(const Uint32& keyCode, const bool& isCaps) {
    hCode = vWillProcess;
    hNCC = hBPC = 0;
    TypingWord[_index] = (keyCode | TONEW_MASK | STANDALONE_MASK | (isCaps ? CAPS_MASK : 0));
    hData[hNCC++] = GET(TypingWord[_index++]);
}

void checkForStandaloneW(const Uint16& data, const bool& isCaps) {
    if (_index && std::find(_standaloneWbad.begin(), _standaloneWbad.end(), CHR(_index - 1)) != _standaloneWbad.end()) {
        return addToTypingWord(data, isCaps);
    }
    reserveLastStandaloneChar(kVK_ANSI_U, isCaps);
}

void processTone(const Uint16& data, const bool& isCaps) {
    vector<vector<Uint16>>& charset = _vowel[data];

    for (char i = 0; i < charset.size(); i++) {
        if (_index < charset[i].size()) {
            continue;
        }
        if (checkCorrectVowel(charset[i], data)) {
            if (data == kVK_ANSI_W) {
                insertW(data, isCaps);
            } else {
                insertAOE(data, isCaps);
            }
            return;
        }
    }

    if (data == kVK_ANSI_W) {
        checkForStandaloneW(data, isCaps);
    } else {
        addToTypingWord(data, isCaps);
    }
}

void restoreTyping() {
    if (_index < _stateIndex) {
        // NOTE: it should be another status, but this one is good enough
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

    findAndCalculateVowel(true);
    if (vowelCount == 0) {
        return;
    }

    bool needCorrect = false;
    char i = 0;

    // check for case: "uơn", "ưoi", "ưom", "ưoc", "uơu"
    if (_index >= 3) {
        for (i = _index - 1; i >= 0; i--) {
            if (CHR(i) == kVK_ANSI_U && i < _index - 2 && CHR(i + 1) == kVK_ANSI_O &&
                (CHR(i + 2) == kVK_ANSI_N || CHR(i + 2) == kVK_ANSI_C || CHR(i + 2) == kVK_ANSI_I ||
                 CHR(i + 2) == kVK_ANSI_M || CHR(i + 2) == kVK_ANSI_P || CHR(i + 2) == kVK_ANSI_T ||
                 CHR(i + 2) == kVK_ANSI_U) &&
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
                Uint32 mark = TypingWord[i] & MARK_MASK;
                TypingWord[i] &= ~MARK_MASK;
                insertMark(mark, false);
                if (i != VWSM) {
                    needCorrect = true;
                }
                break;
            }
        }
    }

    // re-arrange data to sendback
    if (needCorrect) {
        if (hCode ==vDoNothing) {
            hCode = vWillProcess;
        }
        char right = VSI < i ? VSI : i;
        for (char j = _index - 1; j >= right; j--) {
            hData[_index - 1 - j] = GET(TypingWord[j]);
        }
        hNCC = _index - right;
        hBPC = _index - right + deltaBackSpace;
    }
}

void vKeyHandleEvent(const Uint16& data, const bool& isCaps, const bool& isModifier) {
    if (data == kVK_Escape) {
        restoreTyping();
    } else if (isModifier) {
        hCode = vDoNothing;
        vKeyInit();
    } else if (isKeyIn(data, _whiteSpaces)) {
        hCode = vDoNothing;
        _spaceCount++;
    } else if (data == kVK_Delete) {
        hCode = vDoNothing;
        if (_spaceCount > 0) { // previous char is space
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
    } else { // start and check key
        if (_spaceCount > 0) {
            saveTypingHistory();
            startNewSession();
        }

        addToTypingKeys(data, isCaps); // save state
        hCode = vDoNothing;

        switch (data) {
            case kVK_ANSI_Z:
                removeMark(data, isCaps);
                break;
            case kVK_ANSI_D:
                insertD(data, isCaps);
                break;
            case kVK_ANSI_S:
            case kVK_ANSI_F:
            case kVK_ANSI_R:
            case kVK_ANSI_J:
            case kVK_ANSI_X:
                processMark(data, isCaps);
                break;
            case kVK_ANSI_A:
            case kVK_ANSI_E:
            case kVK_ANSI_O:
            case kVK_ANSI_W:
                processTone(data, isCaps);
                break;
            default:
                addToTypingWord(data, isCaps);
                break;
        }

        if (data != kVK_ANSI_D) {
            checkSpelling(0 - (hCode == vDoNothing));
        }

        if (hCode == vRestore) {
            addToTypingWord(data, isCaps);
            _stateIndex--;
        }
    }
}
