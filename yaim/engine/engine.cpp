//
//  engine.cpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#include "engine.hpp"
#include <iostream>
#include <algorithm>
#include <string.h>
#include <list>

vector<Uint8> _whiteSpaces = {KEY_SPACE, KEY_TAB, KEY_ENTER, KEY_RETURN};
vector<Uint8> _vowels = {KEY_A, KEY_E, KEY_U, KEY_Y, KEY_I, KEY_O};

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
list<vector<Uint32>> _typingStates;

/**
 * use for restore key press esc
 */
Uint32 TypingKeys[MAX_BUFF];
Byte _stateIndex = 0;
list<vector<Uint32>> _keyStates;

Byte vowelCount = 0;
Byte VSI = 0; // vowelStartIndex
Byte VEI = 0; // vowelEndIndex
Byte VWSM = 0; // vowelWillSetMark
int _spaceCount = 0;

void* vKeyInit() {
    _index = 0;
    _stateIndex = 0;
    _typingStates.clear();
    _keyStates.clear();
    return &HookState;
}

void startNewSession() {
    _index = 0;
    hBPC = 0;
    hNCC = 0;
    _stateIndex = 0;
}

bool isKeyIn(const Uint16& data, vector<Uint8>& charset) {
    return std::find(charset.begin(), charset.end(), data) != charset.end();
}

void saveTypingHistory() {
    // save word history
    if (_index > 0) {
        int i;
        vector<Uint32> _typingStatesData;
        for (i = 0; i < _index; i++) {
            _typingStatesData.push_back(TypingWord[i]);
        }
        _typingStatesData.push_back(_spaceCount);
        _typingStates.push_back(_typingStatesData);
        if (_typingStates.size() > MAX_WORD) {
            _typingStates.pop_front();
        }

        vector<Uint32> _keyStatesData;
        for (i = 0; i < _stateIndex; i++) {
            _keyStatesData.push_back(TypingKeys[i]);
        }
        _keyStates.push_back(_keyStatesData);
        if (_keyStates.size() > MAX_WORD) {
            _keyStates.pop_front();
        }
    }
}

void addToTypingWord(const Uint16& keyCode, const bool& isCaps) {
    TypingWord[_index++] = keyCode | (isCaps ? CAPS_MASK : 0);
}

void addToTypingKeys(const Uint16& keyCode, const bool& isCaps) {
    if (_stateIndex >= MAX_BUFF) {
        hBPC = 0;
        hNCC = 0;
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
        for (int i = 0; i < _typingStatesData.size() - 1; i++) {
            TypingWord[i] = _typingStatesData[i];
        }
        _index = _typingStatesData.size() - 1;

        for (int i = 0; i < _keyStatesData.size(); i++) {
            TypingKeys[i] = _keyStatesData[i];
        }
        _stateIndex = _keyStatesData.size();
    }
}

bool checkCorrectVowel(vector<Uint16>& charset, const Uint16& markKey) {
    // ignore "qu" case
    if (_index >= 2 && CHR(_index - 1) == KEY_U && CHR(_index - 2) == KEY_Q) {
        return false;
    }

    int s = (int)charset.size();
    for (int j = s - 1; j >= 0; j--) {
        if (charset[j] != CHR(_index - s + j)) {
            return false;
        }
    }

    // limit mark for end consonant: "C", "T"
    if (s > 1 && (markKey == KEY_F || markKey == KEY_X || markKey == KEY_R)) {
        if (charset[1] == KEY_C || charset[1] == KEY_T) {
            return false;
        } else if (s > 2 && (charset[2] == KEY_T)) {
            return false;
        }
    }

    if (_index > s && CHR(_index - s - 1) == CHR(_index - s)) {
        return false;
    }

    return true;
}

Uint32 getCharacterCode(const Uint32& data) {
    int capsElem = (data & CAPS_MASK) ? 0 : 1;
    int key = data & CHAR_MASK;

    if (data & MARK_MASK) { // has mark
        int markElem = -2;
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

        if ((key == KEY_A ||
             key == KEY_E ||
             key == KEY_O ||
             key == KEY_U) &&
            !(data & (TONE_MASK | TONEW_MASK))) {
                markElem += 4;
        }

        if (data & TONE_MASK) {
            key |= TONE_MASK;
        } else if (data & TONEW_MASK) {
            key |= TONEW_MASK;
        }
        if (_codeTable.find(key) == _codeTable.end())
            return data; // not found

        return _codeTable[key][markElem] | CHAR_CODE_MASK;
    } else { // doesn't has mark
        if (_codeTable.find(key) == _codeTable.end())
            return data; // not found

        if (data & TONE_MASK) {
            return _codeTable[key][capsElem] | CHAR_CODE_MASK;
        }
        if (data & TONEW_MASK) {
            return _codeTable[key][capsElem + 2] | CHAR_CODE_MASK;
        }
        return data; // not found
    }
}

void findAndCalculateVowel(const bool& forGrammar=false) {
    vowelCount = 0;
    VSI = VEI = 0;
    for (int i = _index - 1; i >= 0; i--) {
        if (!isKeyIn(CHR(i), _vowels)) {
            if (vowelCount > 0)
                break;
        } else { // is vowel
            if (vowelCount == 0)
                VEI = i;
            if (!forGrammar) {
                if ((i - 1 >= 0 && (CHR(i) == KEY_I && CHR(i - 1) == KEY_G)) ||
                    (i - 1 >= 0 && (CHR(i) == KEY_U && CHR(i - 1) == KEY_Q))) {
                    break;
                }
            }
            VSI = i;
            vowelCount++;
        }
    }
    // don't count "u" at "q u" as a vowel
    if (VSI - 1 >= 0 && CHR(VSI) == KEY_U && CHR(VSI - 1) == KEY_Q) {
        VSI++;
        vowelCount--;
    }
}

void removeMark(const Uint16& data, const bool& isCaps) {
    findAndCalculateVowel(true);
    if (_index > 0) {
        for (int i = VSI; i <= VEI; i++) {
            if (TypingWord[i] & MARK_MASK) {
                TypingWord[i] &= ~MARK_MASK;
                hCode = vWillProcess;
                for (int j = _index - 1; j >= VSI; j--) {
                    hData[_index - 1 - j] = GET(TypingWord[j]);
                }
                hNCC = hBPC = _index - VSI;
                return;
            }
        }
    }

    addToTypingWord(data, isCaps);
}

bool canHasEndConsonant() {
    vector<vector<Uint32>>& vo = _vowelCombine[CHR(VSI)];
    for (int i = 0; i < vo.size(); i++) {
        int k = VSI;
        int j;
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
    // default
    if (vowelCount == 0 && CHR(VEI) == KEY_I)
        VWSM = VEI;
    else
        VWSM = VSI;

    // rule 2
    if (vowelCount == 3 || (VEI + 1 < _index && !isKeyIn(CHR(VEI + 1), _vowels) && canHasEndConsonant())) {
        VWSM = VSI + 1;
    }

    // rule 3
    for (int i = VSI; i <= VEI; i++) {
        if ((CHR(i) == KEY_E && TypingWord[i] & TONE_MASK) || (CHR(i) == KEY_O && TypingWord[i] & TONEW_MASK)) {
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
    } else if (TypingWord[VEI] & (TONE_MASK | TONEW_MASK)) {
        VWSM = VEI;
    } else {
        calcMarkPosition();
    }

    // send data
    if (TypingWord[VWSM] & markMask) {
        // if duplicate same mark -> restore
        TypingWord[VWSM] &= ~MARK_MASK;
        if (canModifyFlag)
            hCode = vRestore;
        for (int i = VSI; i < _index; i++) {
            TypingWord[i] &= ~MARK_MASK;
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }
    } else {
        // remove other mark
        TypingWord[VWSM] &= ~MARK_MASK;

        // add mark
        TypingWord[VWSM] |= markMask;
        for (int i = VSI; i < _index; i++) {
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
    for (int i = 0; i < _vowelForMark.size(); i++) {
        vector<vector<Uint16>>& charset = _vowelForMark[i];

        for (int j = 0; j < charset.size(); j++) {
            if (_index < charset[j].size())
                continue;
            if (checkCorrectVowel(charset[j], data)) {
                switch (data) {
                    case KEY_S:
                        return insertMark(MARK1_MASK);
                    case KEY_F:
                        return insertMark(MARK2_MASK);
                    case KEY_R:
                        return insertMark(MARK3_MASK);
                    case KEY_X:
                        return insertMark(MARK4_MASK);
                    case KEY_J:
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
    for (int i = 0; i < _consonantD.size(); i++) {
        if (_index < _consonantD[i].size())
            continue;
        if (checkCorrectVowel(_consonantD[i], data)) {
            hCode = vWillProcess;
            hBPC = 0;
            for (int j = _index - 1; j >= 0; j--) {
                hBPC++;
                if (CHR(j) == KEY_D) { // reverse unicode char
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
    int i;
    for (i = VSI; i <= VEI; i++) {
        TypingWord[i] &= ~TONEW_MASK;
    }

    hCode = vWillProcess;
    hBPC = 0;

    for (i = _index - 1; i >= 0; i--) {
        hBPC++;
        if (CHR(i) == data) { // reverse unicode char
            if (TypingWord[i] & TONE_MASK) {
                // restore
                hCode = vRestore;
                TypingWord[i] &= ~TONE_MASK;
                hData[_index - 1 - i] = TypingWord[i];
            } else {
                TypingWord[i] |= TONE_MASK;
                if (data != KEY_D)
                    TypingWord[i] &= ~TONEW_MASK;
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
    int i;
    for (i = VSI; i <= VEI; i++) {
        TypingWord[i] &= ~TONE_MASK;
    }

    if (vowelCount > 1) {
        hBPC = _index - VSI;
        hNCC = hBPC;

        if (((TypingWord[VSI] & TONEW_MASK) && (TypingWord[VSI+1] & TONEW_MASK)) ||
            ((TypingWord[VSI] & TONEW_MASK) && CHR(VSI+1) == KEY_I) ||
            ((TypingWord[VSI] & TONEW_MASK) && CHR(VSI+1) == KEY_A)){
            // restore
            hCode = vRestore;

            for (i = VSI; i < _index; i++) {
                TypingWord[i] &= ~TONEW_MASK;
                hData[_index - 1 - i] = GET(TypingWord[i]) & ~STANDALONE_MASK;
            }
        } else {
            hCode = vWillProcess;

            if ((CHR(VSI) == KEY_U && CHR(VSI+1) == KEY_O)) {
                if (VSI && TypingWord[VSI - 1] == KEY_H) {
                    TypingWord[VSI+1] |= TONEW_MASK;
                    if (VSI + 2 < _index && CHR(VSI + 2) == KEY_N) {
                        TypingWord[VSI] |= TONEW_MASK;
                    }
                } else if (VSI - 1 >= 0 && TypingWord[VSI - 1] == KEY_Q) {
                    TypingWord[VSI+1] |= TONEW_MASK;
                } else {
                    TypingWord[VSI] |= TONEW_MASK;
                    TypingWord[VSI+1] |= TONEW_MASK;
                }
            } else if ((CHR(VSI) == KEY_U && CHR(VSI+1) == KEY_A) ||
                       (CHR(VSI) == KEY_U && CHR(VSI+1) == KEY_I) ||
                       (CHR(VSI) == KEY_U && CHR(VSI+1) == KEY_U) ||
                       (CHR(VSI) == KEY_O && CHR(VSI+1) == KEY_I)) {
                TypingWord[VSI] |= TONEW_MASK;
            } else if ((CHR(VSI) == KEY_I && CHR(VSI+1) == KEY_O) ||
                       (CHR(VSI) == KEY_O && CHR(VSI+1) == KEY_A)) {
                TypingWord[VSI+1] |= TONEW_MASK;
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
            case KEY_A:
            case KEY_U:
            case KEY_O:
                if (TypingWord[i] & TONEW_MASK) {
                    // restore
                    if (TypingWord[i] & STANDALONE_MASK) {
                        hCode = vWillProcess;
                        if (CHR(i) == KEY_U){
                            TypingWord[i] = KEY_W | ((TypingWord[i] & CAPS_MASK) ? CAPS_MASK : 0);
                        } else if (CHR(i) == KEY_O) {
                            hCode = vRestore;
                            TypingWord[i] = KEY_O | ((TypingWord[i] & CAPS_MASK) ? CAPS_MASK : 0);
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
    TypingWord[_index] = (keyCode | TONEW_MASK | STANDALONE_MASK | (isCaps ? CAPS_MASK : 0));
    hData[hNCC++] = GET(TypingWord[_index++]);
}

void checkForStandaloneW(const Uint16& data, const bool& isCaps) {
    if (_index && std::find(_standaloneWbad.begin(), _standaloneWbad.end(), CHR(_index - 1)) != _standaloneWbad.end()) {
        return addToTypingWord(data, isCaps);
    }
    reserveLastStandaloneChar(KEY_U, isCaps);
}

void processTone(const Uint16& data, const bool& isCaps) {
    vector<vector<Uint16>>& charset = _vowel[data];

    for (int i = 0; i < charset.size(); i++) {
        if (_index < charset[i].size())
            continue;

        if (checkCorrectVowel(charset[i], data)) {
            if (data == KEY_W) {
                insertW(data, isCaps);
            } else {
                insertAOE(data, isCaps);
            }
            return;
        }
    }

    if (data == KEY_W) {
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
        for (int i = 0; i < _stateIndex; i++) {
            TypingWord[i] = TypingKeys[i];
            hData[_stateIndex - 1 - i] = TypingWord[i];
        }
        _index = _stateIndex;
        _spaceCount = 0;
    } else {
        hCode = vDoNothing;
    }
}

void checkGrammar(const int& deltaBackSpace) {
    if (_index <= 1 || _index >= MAX_BUFF)
        return;

    findAndCalculateVowel(true);
    if (vowelCount == 0)
        return;

    bool needCorrect = false;
    int i;

    // if N key for case: "thuơn", "ưoi", "ưom", "ưoc"
    if (_index >= 3) {
        for (i = _index - 1; i >= 0; i--) {
            if (CHR(i) == KEY_N || CHR(i) == KEY_C || CHR(i) == KEY_I ||
                CHR(i) == KEY_M || CHR(i) == KEY_P || CHR(i) == KEY_T) {
                if (i - 2 >= 0 && CHR(i - 1) == KEY_O && CHR(i - 2) == KEY_U) {
                    if ((TypingWord[i - 1] & TONEW_MASK) ^ (TypingWord[i - 2] & TONEW_MASK)) {
                        TypingWord[i - 2] |= TONEW_MASK;
                        TypingWord[i - 1] |= TONEW_MASK;
                        needCorrect = true;
                        break;
                    }
                }
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
                if (i != VWSM)
                    needCorrect = true;
                break;
            }
        }
    }

    // re-arrange data to sendback
    if (needCorrect) {
        if (hCode ==vDoNothing)
            hCode = vWillProcess;

        for (i = _index - 1; i >= VSI; i--) {
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }
        hNCC = _index - VSI;
        hBPC = _index - VSI + deltaBackSpace;
    }
}

void vKeyHandleEvent(const vKeyEvent& event,
                     const vKeyEventState& state,
                     const Uint16& data,
                     const Uint8& capsStatus,
                     const bool& otherControlKey) {

    bool _isCaps = (capsStatus == 1 || // shift
                    capsStatus == 2);  // caps lock

    if (data == KEY_ESC) {
        restoreTyping();
    } else if (otherControlKey || event == vKeyEvent::Mouse) {
        hCode = vDoNothing;
        hBPC = 0;
        hNCC = 0;
        _typingStates.clear();
        _keyStates.clear();
        startNewSession();
    } else if (isKeyIn(data, _whiteSpaces)) {
        hCode = vDoNothing;
        _spaceCount++;
    } else if (data == KEY_DELETE) {
        hCode = vDoNothing;
        if (_spaceCount > 0) { // previous char is space
            _spaceCount--;
        } else if (--_index == 0){
            startNewSession();
            restoreLastTypingState();
        } else {
            _stateIndex--;
            checkGrammar(1);
        }
    } else { // start and check key
        if (_spaceCount > 0) {
            hBPC = 0;
            hNCC = 0;
            saveTypingHistory();
            startNewSession();
            _spaceCount = 0;
        }

        addToTypingKeys(data, _isCaps); // save state
        hCode = vDoNothing;

        switch (data) {
            case KEY_Z:
                removeMark(data, _isCaps);
                break;
            case KEY_D:
                insertD(data, _isCaps);
                break;
            case KEY_S:
            case KEY_F:
            case KEY_R:
            case KEY_J:
            case KEY_X:
                processMark(data, _isCaps);
                break;
            case KEY_A:
            case KEY_E:
            case KEY_O:
            case KEY_W:
                processTone(data, _isCaps);
                break;
            default:
                hBPC = 0;
                hNCC = 0;
                addToTypingWord(data, _isCaps);
                break;
        }

        if (data != KEY_D) {
            checkGrammar(0 - (hCode == vDoNothing));
        }

        if (hCode == vRestore) {
            addToTypingWord(data, _isCaps);
            _stateIndex--;
        }
    }
}
