//
//  vietnamese.hpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#ifndef vietnamese_hpp
#define vietnamese_hpp

#include "datatypes.h"
#include <Carbon/Carbon.h>
#include <vector>
#include <map>

using namespace std;

extern map<Uint16, vector<vector<Uint16>>> _vowel;
extern map<Uint16, vector<vector<Uint32>>> _vowelCombine;
extern map<Uint16, vector<vector<Uint16>>> _vowelForMark;
extern vector<vector<Uint16>> _consonantD;
extern vector<Uint16> _standaloneWbad;
extern vector<vector<Uint16>> _doubleWAllowed;
extern map<Uint32, vector<Uint16>> _codeTable;
extern map<Uint32, Uint32> _characterMap;

Uint16 keyCodeToCharacter(const Uint32&);


#endif /* vietnamese_hpp */
