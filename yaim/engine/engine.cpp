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

static vector<Uint8> _charKeyCode = {
    KEY_BACKQUOTE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,
    KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET,
    KEY_BACK_SLASH, KEY_SEMICOLON, KEY_QUOTE, KEY_COMMA, KEY_DOT, KEY_SLASH
};

static vector<Uint8> _breakCode = {
    KEY_ESC, KEY_TAB, KEY_ENTER, KEY_RETURN, KEY_LEFT, KEY_RIGHT, KEY_DOWN,
    KEY_UP, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_SEMICOLON, KEY_QUOTE,
    KEY_BACK_SLASH, KEY_MINUS, KEY_EQUALS, KEY_BACKQUOTE, KEY_TAB
};

#define IS_KEY_DOUBLE(key) (KEY_A == key || KEY_O == key || KEY_E == key)
#define IS_MARK_KEY(keyCode) (keyCode == KEY_S || keyCode == KEY_F || keyCode == KEY_R || keyCode == KEY_J || keyCode == KEY_X)

#define VSI vowelStartIndex
#define VEI vowelEndIndex
#define VWSM vowelWillSetMark
#define hBPC HookState.backspaceCount
#define hNCC HookState.newCharCount
#define hCode HookState.code
#define hExt HookState.extCode
#define hData HookState.charData
#define GET getCharacterCode

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
vector<Uint32> _longWordHelper; // save the word when _index >= MAX_BUFF
list<vector<Uint32>> _typingStates;
vector<Uint32> _typingStatesData;

/**
 * use for restore key press esc
 */
Uint32 KeyStates[MAX_BUFF];
Byte _stateIndex = 0;

bool isChanged = false;
Byte vowelCount = 0;
Byte vowelStartIndex = 0;
Byte vowelEndIndex = 0;
Byte vowelWillSetMark = 0;
int _spaceCount = 0;
vector<Uint32> _specialChar;

// function prototype
void findAndCalculateVowel(const bool& forGrammar=false);
void insertMark(const Uint32& markMask, const bool& canModifyFlag=true);

void* vKeyInit() {
    _index = 0;
    _stateIndex = 0;
    _typingStatesData.clear();
    _typingStates.clear();
    _longWordHelper.clear();
    return &HookState;
}

bool isWordBreak(const vKeyEvent& event, const vKeyEventState& state, const Uint16& data) {
    return (event == vKeyEvent::Mouse ||
            std::find(_breakCode.begin(), _breakCode.end(), data) != _breakCode.end());
}

void setKeyData(const Byte& index, const Uint16& keyCode, const bool& isCaps) {
    if (index < 0 || index >= MAX_BUFF)
        return;
    TypingWord[index] = keyCode | (isCaps ? CAPS_MASK : 0);
}

bool _spellingOK = false;
bool _spellingFlag = false;
bool _spellingVowelOK = false;
Byte _spellingEndIndex = 0;

void checkGrammar(const int& deltaBackSpace) {
    if (_index <= 1 || _index >= MAX_BUFF)
        return;

    findAndCalculateVowel(true);
    if (vowelCount == 0)
        return;

    bool isCheckedGrammar = false;
    int l = VSI;
    int i;

    // if N key for case: "thuơn", "ưoi", "ưom", "ưoc"
    if (_index >= 3) {
        for (i = _index-1; i >= 0; i--) {
            if (CHR(i) == KEY_N || CHR(i) == KEY_C || CHR(i) == KEY_I ||
                CHR(i) == KEY_M || CHR(i) == KEY_P || CHR(i) == KEY_T) {
                if (i - 2 >= 0 && CHR(i - 1) == KEY_O && CHR(i - 2) == KEY_U) {
                    if ((TypingWord[i-1] & TONEW_MASK) ^ (TypingWord[i-2] & TONEW_MASK)) {
                        TypingWord[i - 2] |= TONEW_MASK;
                        TypingWord[i - 1] |= TONEW_MASK;
                        isCheckedGrammar = true;
                        break;
                    }
                }
            }
        }
    }

    // check mark
    if (_index >= 2) {
        for (i = l; i <= VEI; i++) {
            if (TypingWord[i] & MARK_MASK) {
                Uint32 mark = TypingWord[i] & MARK_MASK;
                TypingWord[i] &= ~MARK_MASK;
                insertMark(mark, false);
                if (i != vowelWillSetMark)
                    isCheckedGrammar = true;
                break;
            }
        }
    }

    // re-arrange data to sendback
    if (isCheckedGrammar) {
        if (hCode ==vDoNothing)
            hCode = vWillProcess;
        hBPC = 0;

        for (i = _index - 1; i >= l; i--) {
            hBPC++;
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }
        hNCC = hBPC;
        hBPC += deltaBackSpace;
        hExt = 4;
    }
}

void insertKey(const Uint16& keyCode, const bool& isCaps) {
    if (_index >= MAX_BUFF) {
        _longWordHelper.push_back(TypingWord[0]); // save long word
        // left shift
        for (int i = 0; i < MAX_BUFF - 1; i++) {
            TypingWord[i] = TypingWord[i + 1];
        }
        setKeyData(_index-1, keyCode, isCaps);
    } else {
        setKeyData(_index++, keyCode, isCaps);
    }
}

void insertState(const Uint16& keyCode, const bool& isCaps) {
    if (_stateIndex >= MAX_BUFF) {
        // left shift
        for (int i = 0; i < MAX_BUFF - 1; i++) {
            KeyStates[i] = KeyStates[i + 1];
        }
        KeyStates[_stateIndex-1] = keyCode | (isCaps ? CAPS_MASK : 0);
    } else {
        KeyStates[_stateIndex++] = keyCode | (isCaps ? CAPS_MASK : 0);
    }
}

void saveWord() {
    int i;

    // save word history
    if (_index > 0) {
        if (_longWordHelper.size() > 0) { // save long word first
            _typingStatesData.clear();
            for (i = 0; i < _longWordHelper.size(); i++) {
                if (i != 0 && i % MAX_BUFF == 0) { // save if overflow
                    _typingStates.push_back(_typingStatesData);
                    _typingStatesData.clear();
                }
                _typingStatesData.push_back(_longWordHelper[i]);
            }
            _typingStates.push_back(_typingStatesData);
            _longWordHelper.clear();
        }

        // save current word
        _typingStatesData.clear();
        for (i = 0; i < _index; i++) {
            _typingStatesData.push_back(TypingWord[i]);
        }
        _typingStates.push_back(_typingStatesData);
    }
}

void saveWord(const Uint32& keyCode, const int& count) {
    _typingStatesData.clear();
    for (int i = 0; i < count; i++) {
        _typingStatesData.push_back(keyCode);
    }
    _typingStates.push_back(_typingStatesData);
}

void saveSpecialChar() {
    _typingStatesData.clear();
    for (int i = 0; i < _specialChar.size(); i++) {
        _typingStatesData.push_back(_specialChar[i]);
    }
    _typingStates.push_back(_typingStatesData);
    _specialChar.clear();
}

void restoreLastTypingState() {
    if (_typingStates.size() > 0) {
        _typingStatesData = _typingStates.back();
        _typingStates.pop_back();
        if (_typingStatesData.size() > 0){
            if (_typingStatesData[0] == KEY_SPACE) {
                _spaceCount = (int)_typingStatesData.size();
                _index = 0;
            } else if (std::find(_charKeyCode.begin(), _charKeyCode.end(), (Uint16)_typingStatesData[0]) != _charKeyCode.end()) {
                _index = 0;
                _specialChar = _typingStatesData;
            } else {
                for (int i = 0; i < _typingStatesData.size(); i++) {
                    TypingWord[i] = _typingStatesData[i];
                }
                _index = (Byte)_typingStatesData.size();
            }
        }
    }
}

void startNewSession() {
    _index = 0;
    hBPC = 0;
    hNCC = 0;
    _stateIndex = 0;
    _longWordHelper.clear();
}

bool checkCorrectVowel(vector<vector<Uint16>>& charset, int& i, int& k, const Uint16& markKey) {
    // ignore "qu" case
    if (_index >= 2 && CHR(_index-1) == KEY_U && CHR(_index-2) == KEY_Q) {
        return false;
    }
    k = _index - 1;
    for (int j = (int)charset[i].size() - 1; j >= 0; j--) {
        if (charset[i][j] != CHR(k)) {
            return false;
        }
        k--;
        if (k < 0)
            break;
    }

    // limit mark for end consonant: "C", "T"
    if (charset[i].size() > 1 && (markKey == KEY_F || markKey == KEY_X || markKey == KEY_R)) {
        if (charset[i][1] == KEY_C || charset[i][1] == KEY_T) {
            return false;
        } else if (charset[i].size() > 2 && (charset[i][2] == KEY_T)) {
            return false;
        }
    }

    if (k >= 0 && CHR(k) == CHR(k+1)) {
        return false;
    }

    return true;
}

Uint32 getCharacterCode(const Uint32& data) {
    int capsElem = (data & CAPS_MASK) ? 0 : 1;
    int markElem;
    int key = data & CHAR_MASK;

    if (data & MARK_MASK) { // has mark
        markElem = -2;
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

        switch (key) {
            case KEY_A:
            case KEY_O:
            case KEY_U:
            case KEY_E:
                if ((data & TONE_MASK) == 0 && (data & TONEW_MASK) == 0)
                    markElem += 4;
                break;
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
        } else if (data & TONEW_MASK) {
            return _codeTable[key][capsElem + 2] | CHAR_CODE_MASK;
        } else {
            return data; // not found
        }
    }

    return 0;
}

void findAndCalculateVowel(const bool& forGrammar) {
    vowelCount = 0;
    VSI = VEI = 0;
    for (int i = _index - 1; i >= 0; i--) {
        if (IS_CONSONANT(CHR(i))) {
            if (vowelCount > 0)
                break;
        } else {  // is vowel
            if (vowelCount == 0)
                VEI = i;
            if (!forGrammar) {
                if ((i-1 >= 0 && (CHR(i) == KEY_I && CHR(i-1) == KEY_G)) ||
                    (i-1 >= 0 && (CHR(i) == KEY_U && CHR(i-1) == KEY_Q))) {
                    break;
                }
            }
            VSI = i;
            vowelCount++;
        }
    }
    // don't count "u" at "q u" as a vowel
    if (VSI - 1 >= 0 && CHR(VSI) == KEY_U && CHR(VSI-1) == KEY_Q) {
        VSI++;
        vowelCount--;
    }
}

void removeMark() {
    findAndCalculateVowel(true);
    isChanged = false;
    int i;

    if (_index > 0) {
        for (i = VSI; i <= VEI; i++) {
            if (TypingWord[i] & MARK_MASK) {
                TypingWord[i] &= ~MARK_MASK;
                isChanged = true;
            }
        }
    }
    if (isChanged) {
        hCode = vWillProcess;
        hBPC = 0;

        for (i = _index - 1; i >= VSI; i--) {
            hBPC++;
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }
        hNCC = hBPC;
    } else {
        hCode = vDoNothing;
    }
}

bool canHasEndConsonant() {
    vector<vector<Uint32>>& vo = _vowelCombine[CHR(VSI)];
    int i;
    for (i = 0; i < vo.size(); i++) {
        int k = VSI;
        int j;
        for (j = 1; j < vo[i].size(); j++) {
            if (k > VEI || ((CHR(k) | (TypingWord[k] & TONE_MASK) | (TypingWord[k] & TONEW_MASK)) != vo[i][j])) {
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

void handleMark() {
    // default
    if (vowelCount == 0 && CHR(VEI) == KEY_I)
        VWSM = VEI;
    else
        VWSM = VSI;
    hBPC = (_index - VWSM);

    // rule 2
    if (vowelCount == 3 || (VEI + 1 < _index && IS_CONSONANT(CHR(VEI + 1)) && canHasEndConsonant())) {
        VWSM = VSI + 1;
        hBPC = _index - VWSM;
    }

    // rule 3
    int i;
    for (i = VSI; i <= VEI; i++) {
        if ((CHR(i) == KEY_E && TypingWord[i] & TONE_MASK) || (CHR(i) == KEY_O && TypingWord[i] & TONEW_MASK)) {
            VWSM = i;
            hBPC = _index - VWSM;
            break;
        }
    }

    hNCC = hBPC;
}

void insertMark(const Uint32& markMask, const bool& canModifyFlag) {
    vowelCount = 0;

    if (canModifyFlag)
        hCode = vWillProcess;
    hBPC = hNCC = 0;

    findAndCalculateVowel();
    VWSM = 0;

    // detect mark position
    if (vowelCount == 1) {
        VWSM = VEI;
        hBPC = (_index - VEI);
    } else { // vowel = 2 or 3
        handleMark();
        if (TypingWord[VEI] & TONE_MASK || TypingWord[VEI] & TONEW_MASK)
            vowelWillSetMark = VEI;
    }

    // send data
    int k = _index - 1 - VSI;
    int i;
    // if duplicate same mark -> restore
    if (TypingWord[VWSM] & markMask) {
        TypingWord[VWSM] &= ~MARK_MASK;
        if (canModifyFlag)
            hCode = vRestore;
        for (i = VSI; i < _index; i++) {
            TypingWord[i] &= ~MARK_MASK;
            hData[k--] = GET(TypingWord[i]);
        }
    } else {
        // remove other mark
        TypingWord[VWSM] &= ~MARK_MASK;

        // add mark
        TypingWord[VWSM] |= markMask;
        for (i = VSI; i < _index; i++) {
            if (i != VWSM) { // remove mark for other vowel
                TypingWord[i] &= ~MARK_MASK;
            }
            hData[k--] = GET(TypingWord[i]);
        }

        hBPC = _index - VSI;
    }
    hNCC = hBPC;
}

void insertD(const Uint16& data, const bool& isCaps) {
    hCode = vWillProcess;
    hBPC = 0;
    int i;
    for (i = _index - 1; i >= 0; i--) {
        hBPC++;
        if (CHR(i) == KEY_D) { // reverse unicode char
            if (TypingWord[i] & TONE_MASK) {
                // restore
                hCode = vRestore;
                TypingWord[i] &= ~TONE_MASK;
                hData[_index - 1 - i] = TypingWord[i];
                break;
            } else {
                TypingWord[i] |= TONE_MASK;
                hData[_index - 1 - i] = GET(TypingWord[i]);
            }
            break;
        } else {  // preresent old char
            hData[_index - 1 - i] = GET(TypingWord[i]);
        }
    }
    hNCC = hBPC;
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
                break;
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
                if (VSI - 2 >= 0 && TypingWord[VSI - 2] == KEY_T && TypingWord[VSI - 1] == KEY_H) {
                    TypingWord[VSI+1] |= TONEW_MASK;
                    if (VSI + 2 < _index && CHR(VSI+2) == KEY_N) {
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
                // don't do anything
                isChanged = false;
                hCode = vDoNothing;
            }

            for (i = VSI; i < _index; i++) {
                hData[_index - 1 - i] = GET(TypingWord[i]);
            }
        }

        return;
    }

    hCode = vWillProcess;
    hBPC = 0;

    for (i = _index - 1; i >= VSI; i--) {
        hBPC++;
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
    hNCC = hBPC;
}

void reverseLastStandaloneChar(const Uint32& keyCode, const bool& isCaps) {
    hCode = vWillProcess;
    hBPC = 0;
    hNCC = 1;
    hExt = 4;
    TypingWord[_index - 1] = (keyCode | TONEW_MASK | STANDALONE_MASK | (isCaps ? CAPS_MASK : 0));
    hData[0] = GET(TypingWord[_index - 1]);
}

void checkForStandaloneChar(const Uint16& data, const bool& isCaps, const Uint32& keyWillReverse) {
    if (CHR(_index - 1) == keyWillReverse && TypingWord[_index - 1] & TONEW_MASK) {
        hCode = vWillProcess;
        hBPC = 1;
        hNCC = 1;
        TypingWord[_index - 1] = data | (isCaps ? CAPS_MASK : 0);
        hData[0] = GET(TypingWord[_index - 1]);
        return;
    }

    // check standalone w -> ư

    if (_index > 0 && CHR(_index-1) == KEY_U && keyWillReverse == KEY_O) {
        insertKey(keyWillReverse, isCaps);
        reverseLastStandaloneChar(keyWillReverse, isCaps);
        return;
    }

    int i;
    if (_index == 0) { // zero char
        insertKey(data, isCaps);
        reverseLastStandaloneChar(keyWillReverse, isCaps);
        return;
    } else if (_index == 1) { // 1 char
        for (i = 0; i < _standaloneWbad.size(); i++) {
            if (CHR(0) == _standaloneWbad[i]) {
                insertKey(data, isCaps);
                return;
            }
        }
        insertKey(data, isCaps);
        reverseLastStandaloneChar(keyWillReverse, isCaps);
        return;
    } else if (_index == 2) {
        for (i = 0; i < _doubleWAllowed.size(); i++) {
            if (CHR(0) == _doubleWAllowed[i][0] && CHR(1) == _doubleWAllowed[i][1]) {
                insertKey(data, isCaps);
                reverseLastStandaloneChar(keyWillReverse, isCaps);
                return;
            }
        }
        insertKey(data, isCaps);
        return;
    }

    insertKey(data, isCaps);
}

void handleMainKey(const Uint16& data, const bool& isCaps) {
    int i, k, l;
    bool isCorect;

    // if is Z key, remove mark
    if (data == KEY_Z) {
        removeMark();
        if (!isChanged) {
            insertKey(data, isCaps);
        }
        return;
    }

    // if is D key
    if (data == KEY_D) {
        isCorect = false;
        isChanged = false;
        k = _index;
        for (i = 0; i < _consonantD.size(); i++) {
            if (_index < _consonantD[i].size())
                continue;
            isCorect = checkCorrectVowel(_consonantD, i, k, data);

            // allow d after consonant
            if (!isCorect && _index - 2 >= 0 && CHR(_index-1) == KEY_D && IS_CONSONANT(CHR(_index-2))) {
                isCorect = true;
            }
            if (isCorect) {
                isChanged = true;
                insertD(data, isCaps);
                break;
            }
        }

        if (!isChanged) {
            insertKey(data, isCaps);
        }
        return;
    }

    // if is mark key
    if (IS_MARK_KEY(data)) {
        for (i = 0; i < _vowelForMark.size(); i++) {
            vector<vector<Uint16>>& charset = _vowelForMark[i];
            isCorect = false;
            isChanged = false;
            k = _index;
            for (l = 0; l < charset.size(); l++) {
                if (_index < charset[l].size())
                    continue;
                isCorect = checkCorrectVowel(charset, l, k, data);

                if (isCorect) {
                    isChanged = true;
                    if (data == KEY_S)
                        insertMark(MARK1_MASK);
                    else if (data == KEY_F)
                        insertMark(MARK2_MASK);
                    else if (data == KEY_R)
                        insertMark(MARK3_MASK);
                    else if (data == KEY_X)
                        insertMark(MARK4_MASK);
                    else if (data == KEY_J)
                        insertMark(MARK5_MASK);
                    break;
                }
            }

            if (isCorect) {
                break;
            }
        }

        if (!isChanged) {
            insertKey(data, isCaps);
        }

        return;
    }

    Uint16 keyForAEO = data;
    vector<vector<Uint16>>& charset = _vowel[keyForAEO];
    isCorect = false;
    isChanged = false;
    k = _index;

    for (i = 0; i < charset.size(); i++) {
        if (_index < charset[i].size())
            continue;
        isCorect = checkCorrectVowel(charset, i, k, data);

        if (isCorect) {
            isChanged = true;
            if (IS_KEY_DOUBLE(data)) {
                insertAOE(keyForAEO, isCaps);
            } else if (data == KEY_W) {
                insertW(keyForAEO, isCaps);
            }
            break;
        }
    }

    if (!isChanged) {
        if (data == KEY_W) { // standalone key w
            checkForStandaloneChar(data, isCaps, KEY_U);
        } else {
            insertKey(data, isCaps);
        }
    }
}

void restoreTyping() {
    if (_index < _stateIndex) {
        hCode = vRestore;
        hBPC = _index + _spaceCount;
        hNCC = _stateIndex;
        for (int i = 0; i < _stateIndex; i++) {
            TypingWord[i] = KeyStates[i];
            hData[_stateIndex - 1 - i] = TypingWord[i];
        }
        _index = _stateIndex;
        if (_spaceCount > 0) {
            _spaceCount = 0;
            _typingStates.pop_back();
        }
    } else {
        hCode = vDoNothing;
    }
}

void vKeyHandleEvent(const vKeyEvent& event,
                     const vKeyEventState& state,
                     const Uint16& data,
                     const Uint8& capsStatus,
                     const bool& otherControlKey) {
    int i;

    bool _isCaps = (capsStatus == 1 || // shift
                    capsStatus == 2); // caps lock

    if (data == KEY_ESC) {
        restoreTyping();
    } else if (otherControlKey || isWordBreak(event, state, data)) {
        hCode = vDoNothing;
        hBPC = 0;
        hNCC = 0;
        hExt = 1; // word break
        _specialChar.clear();
        _typingStates.clear();
        startNewSession();
    } else if (data == KEY_SPACE) {
        hCode = vDoNothing;
        _spaceCount++;
        // save word
        if (_spaceCount == 1) {
            if (_specialChar.size() > 0) {
                saveSpecialChar();
            } else {
                saveWord();
            }
        }
    } else if (data == KEY_DELETE) {
        hCode = vDoNothing;
        hExt = 2; // delete
        if (_specialChar.size() > 0) {
            _specialChar.pop_back();
            if (_specialChar.size() == 0) {
                restoreLastTypingState();
            }
        } else if (_spaceCount > 0) { // previous char is space
            _spaceCount--;
            if (_spaceCount == 0) { // restore word
                restoreLastTypingState();
            }
        } else {
            if (_stateIndex > 0) {
                _stateIndex--;
            }
            if (_index > 0){
                _index--;
                if (_longWordHelper.size() > 0) {
                    // right shift
                    for (i = MAX_BUFF - 1; i > 0; i--) {
                        TypingWord[i] = TypingWord[i-1];
                    }
                    TypingWord[0] = _longWordHelper.back();
                    _longWordHelper.pop_back();
                    _index++;
                }
            }
            hBPC = 0;
            hNCC = 0;
            hExt = 2; // delete key
            if (_index == 0) {
                startNewSession();
                _specialChar.clear();
                restoreLastTypingState();
            } else { // continue check grammar
                checkGrammar(1);
            }
        }
    } else { // start and check key
        if (_spaceCount > 0) {
            hBPC = 0;
            hNCC = 0;
            hExt = 0;
            startNewSession();
            // continute save space
            saveWord(KEY_SPACE, _spaceCount);
            _spaceCount = 0;
        } else if (_specialChar.size() > 0) {
            saveSpecialChar();
        }

        insertState(data, _isCaps); // save state

        if (!IS_SPECIALKEY(data)) { // do nothing
            hCode = vDoNothing;
            hBPC = 0;
            hNCC = 0;
            hExt = 3; // normal key
            insertKey(data, _isCaps);
        } else { // check and update key
            // restore state
            hCode = vDoNothing;
            hExt = 3; // normal key
            handleMainKey(data, _isCaps);
        }

        if (data != KEY_D) {
            checkGrammar(0 - (hCode == vDoNothing));
        }

        if (hCode == vRestore) {
            insertKey(data, _isCaps);
            _stateIndex--;
        }
    }
}
