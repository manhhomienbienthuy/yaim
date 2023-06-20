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
#include <vector>
#include <map>

using namespace std;

extern vector<char> _vowels;
extern map<char, vector<vector<char>>> _rimes;
extern map<char, vector<vector<UInt32>>> _vowelCombine;
extern vector<vector<char>> _rimesForMark;
extern vector<vector<char>> _consonantD;
extern vector<char> _standaloneWbad;
extern map<UInt32, vector<UInt16>> _codeTable;


#endif /* vietnamese_hpp */
