//
//  vietnamese.cpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#include "vietnamese.hpp"

using namespace std;

map<char, vector<vector<char>>> _rimesForTone = {
    {
        'A', {
            {'A'},
            {'A', 'C'},
            {'A', 'M'},
            {'A', 'N'},
            {'A', 'P'},
            {'A', 'T'},
            {'A', 'U'},
            {'A', 'Y'},
            {'A', 'N', 'G'},
            {'U', 'A'},
            {'U', 'A', 'N'},
            {'U', 'A', 'T'},
            {'U', 'A', 'Y'},
            {'U', 'A', 'N', 'G'},
        }
    },
    {
        'O', {
            {'O'},
            {'O', 'C'},
            {'O', 'I'},
            {'O', 'M'},
            {'O', 'N'},
            {'O', 'P'},
            {'O', 'T'},
            {'O', 'N', 'G'},
            {'U', 'O'},
            {'U', 'O', 'I'},
            {'U', 'O', 'C'},
            {'U', 'O', 'M'},
            {'U', 'O', 'N'},
            {'U', 'O', 'T'},
            {'U', 'O', 'N', 'G'},
        }
    },
    {
        'E', {
            {'E'},
            {'E', 'C'},
            {'E', 'M'},
            {'E', 'N'},
            {'E', 'P'},
            {'E', 'T'},
            {'E', 'U'},
            {'E', 'C', 'H'},
            {'E', 'N', 'H'},
            {'I', 'E'},
            {'I', 'E', 'C'},
            {'I', 'E', 'M'},
            {'I', 'E', 'N'},
            {'I', 'E', 'P'},
            {'I', 'E', 'T'},
            {'I', 'E', 'U'},
            {'I', 'E', 'N', 'G'},
            {'Y', 'E'},
            {'Y', 'E', 'N'},
            {'Y', 'E', 'M'},
            {'Y', 'E', 'T'},
            {'Y', 'E', 'U'},
            {'U', 'E'},
            {'U', 'Y', 'E'},
            {'U', 'Y', 'E', 'N'},
            {'U', 'Y', 'E', 'T'},
        }
    },
    {
        'W', {
            // Ư
            {'U'},
            {'U', 'A'},
            {'U', 'C'},
            {'U', 'I'},
            {'U', 'M'},
            {'U', 'T'},
            {'U', 'U'},
            {'U', 'N', 'G'},
            // ƯƠ
            {'U', 'O'},
            {'U', 'O', 'C'},
            {'U', 'O', 'I'},
            {'U', 'O', 'M'},
            {'U', 'O', 'N'},
            {'U', 'O', 'P'},
            {'U', 'O', 'T'},
            {'U', 'O', 'N', 'G'},
            // Ă
            {'A'},
            {'A', 'C'},
            {'A', 'M'},
            {'A', 'N'},
            {'A', 'P'},
            {'A', 'T'},
            {'A', 'N', 'G'},
            {'O', 'A'},
            {'O', 'A', 'C'},
            {'O', 'A', 'M'},
            {'O', 'A', 'N'},
            {'O', 'A', 'T'},
            {'O', 'A', 'N', 'G'},
            // Ơ
            {'O'},
            {'O', 'I'},
            {'O', 'M'},
            {'O', 'N'},
            {'O', 'P'},
            {'O', 'T'},
        }
    }
};

vector<vector<char>> _rimesForMark = {
    // 'A'
    {'A', 'N', 'G'},
    {'A', 'C', 'H'},
    {'A', 'N', 'H'},
    {'A', 'N'},
    {'A', 'M'},
    {'A', 'U'},
    {'A', 'Y'},
    {'A', 'T'},
    {'A', 'P'},
    {'A', 'C'},
    {'A', 'I'},
    {'A', 'O'},
    {'A'},
    {'O', 'A', 'N', 'G'},
    {'O', 'A', 'N', 'H'},
    {'O', 'A', 'C', 'H'},
    {'O', 'A', 'N'},
    {'O', 'A', 'M'},
    {'O', 'A', 'Y'},
    {'O', 'A', 'T'},
    {'O', 'A', 'P'},
    {'O', 'A', 'C'},
    {'O', 'A', 'I'},
    {'O', 'A'},
    {'U', 'A', 'N', 'G'},
    {'U', 'A', 'N'},
    {'U', 'A', 'Y'},
    {'U', 'A', 'T'},
    {'U', 'A'},
    {'U', 'Y', 'A'},
    // 'O'
    {'O', 'O', 'N', 'G'},
    {'O', 'N', 'G'},
    {'O', 'O', 'N'},
    {'O', 'O', 'C'},
    {'O', 'O'},
    {'O', 'N'},
    {'O', 'M'},
    {'O', 'I'},
    {'O', 'C'},
    {'O', 'T'},
    {'O', 'P'},
    {'O'},
    {'U', 'O', 'N', 'G'},
    {'U', 'O', 'N'},
    {'U', 'O', 'M'},
    {'U', 'O', 'I'},
    {'U', 'O', 'C'},
    {'U', 'O', 'T'},
    {'U', 'O', 'P'},
    {'U', 'O'},
    // 'E'
    {'E', 'N', 'H'},
    {'E', 'N', 'G'},
    {'E', 'C', 'H'},
    {'E', 'C'},
    {'E', 'T'},
    {'E', 'U'},
    {'E', 'P'},
    {'E', 'N'},
    {'E', 'M'},
    {'E', 'O'},
    {'E'},
    {'I', 'E', 'N', 'G'},
    {'I', 'E', 'C'},
    {'I', 'E', 'T'},
    {'I', 'E', 'U'},
    {'I', 'E', 'P'},
    {'I', 'E', 'N'},
    {'I', 'E', 'M'},
    {'I', 'E'},
    {'O', 'E', 'T'},
    {'O', 'E', 'N'},
    {'O', 'E'},
    {'U', 'E', 'N', 'H'},
    {'U', 'E'},
    {'Y', 'E', 'T'},
    {'Y', 'E', 'U'},
    {'Y', 'E', 'N'},
    {'Y', 'E', 'M'},
    {'Y', 'E'},
    {'U', 'Y', 'E', 'N'},
    {'U', 'Y', 'E', 'T'},
    {'U', 'Y', 'E'},
    // 'I'
    {'I', 'N', 'H'},
    {'I', 'C', 'H'},
    {'I', 'N'},
    {'I', 'T'},
    {'I', 'U'},
    {'I', 'N'},
    {'I', 'M'},
    {'I', 'P'},
    {'I', 'A'},
    {'I', 'C'},
    {'I'},
    // 'U'
    {'U', 'N', 'G'},
    {'U', 'I'},
    {'U', 'O'},
    {'U', 'T'},
    {'U', 'U'},
    {'U', 'A'},
    {'U', 'I'},
    {'U', 'C'},
    {'U', 'N'},
    {'U', 'M'},
    {'U', 'P'},
    {'U'},
    // 'Y'
    {'Y', 'T'},
    {'Y'},
    {'U', 'Y', 'N', 'H'},
    {'U', 'Y', 'N'},
    {'U', 'Y', 'T'},
    {'U', 'Y', 'P'},
    {'U', 'Y'},
};

vector<vector<char>> _beginConsonants = {
    {'N', 'G', 'H'},
    {'P', 'H'},
    {'T', 'H'},
    {'T', 'R'},
    {'G', 'I'},
    {'C', 'H'},
    {'N', 'H'},
    {'N', 'G'},
    {'K', 'H'},
    {'G', 'H'},
    {'Q', 'U'},
    {'G'},
    {'C'},
    {'K'},
    {'T'},
    {'R'},
    {'H'},
    {'B'},
    {'M'},
    {'V'},
    {'N'},
    {'L'},
    {'X'},
    {'P'},
    {'S'},
    {'D'},
};

map<UInt32, vector<UInt16>> _codeTable = {
    // 'A',           {A, a, Á, á, À, à, Ả, ả, Ã, ã, Ạ, ạ
    {'A',            {0x0000, 0x0000, 0x00c1, 0x00e1, 0x00c0, 0x00e0, 0x1ea2, 0x1ea3, 0x00c3, 0x00e3, 0x1ea0, 0x1ea1}},
    {'O',            {0x0000, 0x0000, 0x00d3, 0x00f3, 0x00d2, 0x00f2, 0x1ece, 0x1ecf, 0x00d5, 0x00f5, 0x1ecc, 0x1ecd}},
    {'U',            {0x0000, 0x0000, 0x00da, 0x00fa, 0x00d9, 0x00f9, 0x1ee6, 0x1ee7, 0x0168, 0x0169, 0x1ee4, 0x1ee5}},
    {'E',            {0x0000, 0x0000, 0x00c9, 0x00e9, 0x00c8, 0x00e8, 0x1eba, 0x1ebb, 0x1ebc, 0x1ebd, 0x1eb8, 0x1eb9}},
    {'D'|TONE_MASK,  {0x0110, 0x0111}},
    {'A'|TONE_MASK,  {0x00c2, 0x00e2, 0x1ea4, 0x1ea5, 0x1ea6, 0x1ea7, 0x1ea8, 0x1ea9, 0x1eaa, 0x1eab, 0x1eac, 0x1ead}},
    {'A'|TONEW_MASK, {0x0102, 0x0103, 0x1eae, 0x1eaf, 0x1eb0, 0x1eb1, 0x1eb2, 0x1eb3, 0x1eb4, 0x1eb5, 0x1eb6, 0x1eb7}},
    {'O'|TONE_MASK,  {0x00d4, 0x00f4, 0x1ed0, 0x1ed1, 0x1ed2, 0x1ed3, 0x1ed4, 0x1ed5, 0x1ed6, 0x1ed7, 0x1ed8, 0x1ed9}},
    {'O'|TONEW_MASK, {0x01a0, 0x01a1, 0x1eda, 0x1edb, 0x1edc, 0x1edd, 0x1ede, 0x1edf, 0x1ee0, 0x1ee1, 0x1ee2, 0x1ee3}},
    {'U'|TONEW_MASK, {0x01af, 0x01b0, 0x1ee8, 0x1ee9, 0x1eea, 0x1eeb, 0x1eec, 0x1eed, 0x1eee, 0x1eef, 0x1ef0, 0x1ef1}},
    {'E'|TONE_MASK,  {0x00ca, 0x00ea, 0x1ebe, 0x1ebf, 0x1ec0, 0x1ec1, 0x1ec2, 0x1ec3, 0x1ec4, 0x1ec5, 0x1ec6, 0x1ec7}},
    {'I',            {0x0000, 0x0000, 0x00cd, 0x00ed, 0x00cc, 0x00ec, 0x1ec8, 0x1ec9, 0x0128, 0x0129, 0x1eca, 0x1ecb}},
    {'Y',            {0x0000, 0x0000, 0x00dd, 0x00fd, 0x1ef2, 0x1ef3, 0x1ef6, 0x1ef7, 0x1ef8, 0x1ef9, 0x1ef4, 0x1ef5}},
};
