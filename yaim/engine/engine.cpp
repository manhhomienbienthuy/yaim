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
#define ORD(index) getCharacterCode(TypingWord[index])
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

void* vInit() {
    _typingStates.clear();
    _keyStates.clear();
    startNewSession();
    return &HookState;
}

void saveHistory() {
    if (_index) {
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
        saveHistory();
        startNewSession();
    }
    TypingKeys[_stateIndex++] = charCode;
}

void restoreLastState() {
    if (_typingStates.size()) {
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

void calcVowels() {
    vowelCount = VSI = VEI = 0;
    for (char i = _index - 1; i >= 0; i--) {
        if (CHR(i) != 'A' &&
            CHR(i) != 'E' &&
            CHR(i) != 'U' &&
            CHR(i) != 'Y' &&
            CHR(i) != 'I' &&
            CHR(i) != 'O') {
            if (vowelCount) {
                break;
            }
        } else { // is vowel
            if ((CHR(i) == 'U' && i && CHR(i - 1) == 'Q') ||
                (CHR(i) == 'I' && i && CHR(i - 1) == 'G' &&
                 vowelCount && !(TypingWord[i] & MARK_MASK))) {
                break;
            }
            if (!vowelCount) {
                VEI = i;
            }
            VSI = i;
            vowelCount++;
        }
    }
}

void calcMarkPosition() {
    if (vowelCount == 1 ||
        TypingWord[VEI] & (TONE_MASK | TONEW_MASK) ||
        VEI + 1 < _index) {
        VWSM = VEI;
    } else {
        VWSM = VEI - 1;
    }
}

vector<char> findCombination(vector<vector<char>>& charsets,
                             const char& size,
                             const char& offset = 0) {
    for (char i = 0; i < charsets.size(); i++) {
        char s = charsets[i].size();
        if (s != size) {
            continue;
        }

        bool found = true;
        for (char j = 0; j < s; j++) {
            if (charsets[i][j] != CHR(j + offset)) {
                found = false;
                break;
            }
        }

        if (found) {
            return charsets[i];
        }
    }

    return {};
}

bool isGoodBeginning(const char& size) {
    vector<char> found = findCombination(_beginConsonants, size, 0);
    char s = found.size();
    if (s) {
        if (s < _index) {
            if (((vector<char>){'K'} == found ||
                 (vector<char>){'N', 'G', 'H'} == found) &&
                CHR(s) != 'I' && CHR(s) !='E' && CHR(s) != 'Y') {
                return false;
            }

            if ((vector<char>){'N', 'G'} == found &&
                CHR(s) != 'A' && CHR(s) != 'U' && CHR(s) != 'O') {
                return false;
            }

            return true;
        }

        return true;
    }

    return false;
}

bool isGoodSpelling(vector<vector<char>>& rimesCharset, const char& markKey) {
    calcVowels();

    if (!vowelCount) {
        return false;
    }

    if (VSI && !isGoodBeginning(VSI)) {
        return false;
    }

    vector<char> found = findCombination(rimesCharset, _index - VSI, VSI);
    char s = found.size();
    if (s) {
        char backChar = found.back();
        if ((markKey == 'F' || markKey == 'X' || markKey == 'R') &&
            (backChar == 'T' || backChar == 'C' || backChar == 'P' ||
             (backChar == 'H' && found[s - 2] == 'C'))) {
            return false;
        }

        return true;
    }
    
    return false;
}

UInt16 getCharacterCode(const UInt32& data) {
    if (!(data & PROCESS_MASK)) {
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

void removeMark(const char& charCode) {
    calcVowels();
    calcMarkPosition();

    if (vowelCount && TypingWord[VWSM] & MARK_MASK) {
        TypingWord[VWSM] &= ~MARK_MASK;
        hCode = vWillProcess;
        for (char i = VWSM; i < _index; i++) {
            hData[i - VWSM] = ORD(i);
        }
        hNCC = hBPC = _index - VWSM;
        _stateIndex -= 1 + (TypingKeys[_stateIndex - 2] == 's' ||
                            TypingKeys[_stateIndex - 2] == 'f' ||
                            TypingKeys[_stateIndex - 2] == 'r' ||
                            TypingKeys[_stateIndex - 2] == 'x' ||
                            TypingKeys[_stateIndex - 2] == 'j');
    } else {
        addToTypingWord(charCode);
    }
}

void insertMark(const UInt32& markMask, const bool& canModifyFlag=true) {
    calcMarkPosition();

    if (TypingWord[VWSM] & markMask) {
        // if duplicate same mark -> restore
        TypingWord[VWSM] &= ~MARK_MASK;
        if (canModifyFlag) {
            hCode = vRestore;
        }
    } else {
        if (canModifyFlag) {
            hCode = vWillProcess;
        }
        // remove other mark
        if (TypingWord[VWSM] & MARK_MASK) {
            TypingWord[VWSM] &= ~MARK_MASK;
            if (TypingKeys[_stateIndex - 2] == 's' ||
                TypingKeys[_stateIndex - 2] == 'f' ||
                TypingKeys[_stateIndex - 2] == 'r' ||
                TypingKeys[_stateIndex - 2] == 'x' ||
                TypingKeys[_stateIndex - 2] == 'j') {
                TypingKeys[_stateIndex - 1] = TypingKeys[--_stateIndex];
            }
        }
        // add mark
        TypingWord[VWSM] |= markMask;
    }

    for (char i = VWSM; i < _index; i++) {
        hData[i - VWSM] = ORD(i);
    }
    hNCC = hBPC = _index - VWSM;
}

void processMark(const char& charCode) {
    if (isGoodSpelling(_rimesForMark, toupper(charCode))) {
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

    addToTypingWord(charCode);
}

void insertD(const char& charCode) {
    if (CHR(0) == 'D' &&
        (_index == 1 ||
         isGoodSpelling(_rimesForMark, 'D'))) {
        if (TypingWord[0] & TONE_MASK) {
            // restore
            TypingWord[0] &= ~TONE_MASK;
            hCode = vRestore;
        } else {
            TypingWord[0] |= TONE_MASK;
            hCode = vWillProcess;
        }

        for (char i = 0; i < _index; i++) {
            hData[i] = ORD(i);
        }

        hNCC = hBPC = _index;
        return;
    }

    addToTypingWord(charCode);
}

void insertAOE(const char& charCode) {
    for (char i = VSI; i < _index; i++) {
        if (i <= VEI) {
            TypingWord[i] &= ~TONEW_MASK;
        }

        if (CHR(i) == toupper(charCode)) {
            if (TypingWord[i] & TONE_MASK) {
                TypingWord[i] &= ~TONE_MASK;
                hCode = vRestore;
            } else {
                TypingWord[i] |= TONE_MASK;
                hCode = vWillProcess;
            }
        }
        hData[i - VSI] = ORD(i);
    }
    hNCC = hBPC = _index - VSI;
}

void insertW(const char& charCode) {
    if (vowelCount > 1) {
        if ((TypingWord[VSI] & TONEW_MASK &&
             (TypingWord[VSI + 1] & TONEW_MASK ||
              CHR(VSI + 1) == 'I' ||
              CHR(VSI + 1) == 'A' ||
              CHR(VSI + 1) == 'U')) ||
            (!(TypingWord[VSI] & TONEW_MASK) &&
             TypingWord[VSI + 1] & TONEW_MASK &&
             VSI == _index - 2)) {
            hCode = vRestore;
            TypingWord[VSI] &= ~(TONEW_MASK | STANDALONE_MASK);
            TypingWord[VSI + 1] &= ~(TONEW_MASK | STANDALONE_MASK);
        } else {
            hCode = vWillProcess;
            if ((CHR(VSI) == 'U' && CHR(VSI + 1) == 'O')) {
                TypingWord[VSI + 1] |= TONEW_MASK;
                if (!VSI || CHR(VSI - 1) != 'H' || VSI < _index - 2) {
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
                return;
            }
        }
    } else {
        if (TypingWord[VSI] & TONEW_MASK) {
            if (TypingWord[VSI] & STANDALONE_MASK) {
                hCode = vWillProcess;
                TypingWord[VSI] = (char)TypingWord[VSI] == 'U' ? 'W' : 'w';
            } else {
                hCode = vRestore;
                TypingWord[VSI] &= ~TONEW_MASK;
            }
        } else {
            hCode = vWillProcess;
            TypingWord[VSI] |= TONEW_MASK;
            TypingWord[VSI] &= ~TONE_MASK;
        }
    }

    for (char i = VSI; i < _index; i++) {
        if (i <= VEI) {
            TypingWord[i] &= ~TONE_MASK;
        }
        hData[i - VSI] = ORD(i);
    }
    hNCC = hBPC = _index - VSI;
}

void processStandalone(const char& charCode) {
    if (toupper(charCode) == 'W' &&
        (!_index || isGoodBeginning(_index))) {
        hCode = vWillProcess;
        TypingWord[_index] = (charCode == 'W' ? 'U' : 'u') | TONEW_MASK |
            STANDALONE_MASK;
        hData[hNCC++] = ORD(_index++);
        return;
    }

    addToTypingWord(charCode);
}

void processTone(const char& charCode) {
    vector<vector<char>>& charset = _rimesForTone[toupper(charCode)];

    if (isGoodSpelling(charset, toupper(charCode))) {
        if (toupper(charCode) == 'W') {
            insertW(charCode);
        } else {
            insertAOE(charCode);
        }
        return;
    }

    processStandalone(charCode);
}

void restoreTyping() {
    hCode = vDoNothing;

    for (char i = 0; i < _index; i++) {
        if (TypingWord[i] & PROCESS_MASK && hCode == vDoNothing) {
            VWSM = i;
            hCode = vWillProcess;
            hBPC = _index - VWSM + _spaceCount;
            hNCC = _stateIndex - VWSM;
            _index = _stateIndex;
            _spaceCount = 0;
        }

        if (hCode == vWillProcess) {
            hData[i - VWSM] = TypingWord[i] = TypingKeys[i];
        }
    }
}

void regulateSpelling(const int& deltaBackSpace) {
    if (_index <= 1) {
        return;
    }

    calcVowels();
    if (!vowelCount) {
        return;
    }

    bool needCorrect = false;
    char i, left = _index - hNCC;

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
                left = i < left ? i : left;
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
                calcVowels();
                calcMarkPosition();
                if (i != VWSM) {
                    needCorrect = true;
                    left = i < left ? i : left;
                    insertMark(mark, false);
                } else {
                    TypingWord[i] |= mark;
                }
                break;
            }
        }
    }

    if (needCorrect) {
        if (hCode == vDoNothing) {
            hCode = vWillProcess;
        }
        hNCC = _index - left;
        hBPC = _index - left + deltaBackSpace;
        for (i = left; i < _index; i++ ) {
            hData[i - left] = ORD(i);
        }
    }
}

void vHandleKey(const UInt16& charCode) {
    if (charCode == 0x1b) { // ESC
        restoreTyping();
    } else if (charCode == 0x08) { // Backspace
        hCode = vDoNothing;
        if (_spaceCount) {
            _spaceCount--;
        } else if (_index) {
            if (!--_index) {
                _stateIndex = 0;
                restoreLastState();
            } else {
                _stateIndex -= 1 +
                    (bool)(TypingWord[_index] & TONE_MASK) +
                    (CHR(_index) == 'O' && TypingWord[_index] & TONEW_MASK) +
                    (bool)(TypingWord[_index] & MARK_MASK);
                if (_stateIndex < _index) {
                    _stateIndex = _index;
                }
                regulateSpelling(1);
            }
        }
    } else if (toupper(charCode) < 'A' || toupper(charCode) > 'Z') {
        hCode = vDoNothing;
        _spaceCount++;
    } else {
        if (_spaceCount ||
            (charCode >= 'A' && charCode <= 'Z' &&
             (char)TypingWord[_index - 1] >= 'a' &&
             (char)TypingWord[_index - 1] <= 'z')) {
            saveHistory();
            startNewSession();
        }

        addToTypingKeys(charCode);
        hCode = vDoNothing;
        hNCC = hBPC = 0;

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

        if (toupper(charCode) >= 'A' && toupper(charCode) <= 'Z' &&
            toupper(charCode) != 'D') {
            regulateSpelling(0 - (hCode == vDoNothing));
        }

        if (hCode == vRestore) {
            addToTypingWord(charCode);
            _stateIndex--;
        }
    }
}
