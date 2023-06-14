//
//  vietnamese.cpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#include "vietnamese.hpp"

using namespace std;

vector<char> _whiteSpaces = {' ', '\t', '\n'};
vector<char> _vowels = {'A', 'E', 'U', 'Y', 'I', 'O'};

map<char, vector<vector<char>>> _rimes = {
    {
        'A', {
            {'A', 'N', 'G'},
            {'A', 'N'},
            {'A', 'M'},
            {'A', 'U'},
            {'A', 'Y'},
            {'A', 'T'},
            {'A', 'P'},
            {'A'},
            {'A', 'C'},
        }
    },
    {
        'O', {
            {'O', 'N', 'G'},
            {'O', 'N'},
            {'O', 'M'},
            {'O', 'I'},
            {'O', 'C'},
            {'O', 'T'},
            {'O', 'P'},
            {'O'}
        }
    },
    {
        'E', {
            {'E', 'N', 'H'},
            {'E', 'N', 'G'},
            {'E', 'C', 'H'},
            {'E', 'C'},
            {'E', 'T'},
            {'E', 'Y'},
            {'E', 'U'},
            {'E', 'P'},
            {'E', 'C'},
            {'E', 'N'},
            {'E', 'M'},
            {'E'}
        }
    },
    {
        'W', {
            {'O', 'N'},
            {'U', 'O', 'N', 'G'},
            {'U', 'O', 'N'},
            {'U', 'O', 'I'},
            {'U', 'O', 'C'},
            {'O', 'I'},
            {'O', 'P'},
            {'O', 'M'},
            {'O', 'A'},
            {'O', 'T'},
            {'U', 'N', 'G'},
            {'A', 'N', 'G'},
            {'U', 'N'},
            {'U', 'M'},
            {'U', 'C'},
            {'U', 'A'},
            {'U', 'I'},
            {'U', 'T'},
            {'U'},
            {'A', 'P'},
            {'A', 'T'},
            {'A', 'M'},
            {'A', 'N'},
            {'A'},
            {'A', 'C'},
            {'A', 'C', 'H'},
            {'O'},
            {'U', 'U'}
        }
    }
};

map<char, vector<vector<UInt32>>> _vowelCombine = {
    {
        'A', {
            // fist elem can has end consonant or not
            {0, 'A', 'I'},
            {0, 'A', 'O'},
            {0, 'A', 'U'},
            {0, 'A'|TONE_MASK, 'U'},
            {0, 'A', 'Y'},
            {0, 'A'|TONE_MASK, 'Y'},
        }
    },
    {
        'E', {
            {0, 'E', 'O'},
            {0, 'E'|TONE_MASK, 'U'},
        }
    },
    {
        'I', {
            {1, 'I', 'E'|TONE_MASK, 'U'},
            {0, 'I', 'A'},
            {1, 'I', 'E'|TONE_MASK},
            {0, 'I', 'U'},

        }
    },
    {
        'O', {
            {0, 'O', 'A', 'I'},
            {0, 'O', 'A', 'O'},
            {0, 'O', 'A', 'Y'},
            {0, 'O', 'E', 'O'},
            {1, 'O', 'A'},
            {1, 'O', 'A'|TONEW_MASK},
            {1, 'O', 'E'},
            {0, 'O', 'I'},
            {0, 'O'|TONE_MASK, 'I'},
            {0, 'O'|TONEW_MASK, 'I'},
            {1, 'O', 'O'},
            {1, 'O'|TONE_MASK, 'O'|TONE_MASK},
        }
    },
    {
        'U', {
            {0, 'U', 'Y', 'U'},
            {1, 'U', 'Y', 'E'|TONE_MASK},
            {0, 'U', 'Y', 'A'},
            {0, 'U'|TONEW_MASK, 'O'|TONEW_MASK, 'U'},
            {0, 'U'|TONEW_MASK, 'O'|TONEW_MASK, 'I'},
            {0, 'U', 'O'|TONE_MASK, 'I'},
            {0, 'U', 'A'|TONE_MASK, 'Y'},
            {1, 'U', 'A', 'O'},
            {1, 'U', 'A'},
            {1, 'U', 'A'|TONEW_MASK},
            {1, 'U', 'A'|TONE_MASK},
            {0, 'U'|TONEW_MASK, 'A'},
            {1, 'U', 'E'|TONE_MASK},
            {0, 'U', 'I'},
            {0, 'U'|TONEW_MASK, 'I'},
            {1, 'U', 'O'},
            {1, 'U', 'O'|TONE_MASK},
            {0, 'U', 'O'|TONEW_MASK},
            {1, 'U'|TONEW_MASK, 'O'|TONEW_MASK},
            {0, 'U'|TONEW_MASK, 'U'},
            {1, 'U', 'Y'},
        }
    },
    {
        'Y', {
            {0, 'Y', 'E'|TONE_MASK, 'U'},
            {1, 'Y', 'E'|TONE_MASK},
        }
    }
};

vector<vector<char>> _consonantD = {
    {'D', 'E', 'N', 'H'},
    {'D', 'E', 'N', 'G'},
    {'D', 'E', 'C', 'H'},
    {'D', 'E', 'N'},
    {'D', 'E', 'C'},
    {'D', 'E', 'M'},
    {'D', 'E'},
    {'D', 'E', 'T'},
    {'D', 'E', 'U'},
    {'D', 'E', 'O'},
    {'D', 'E', 'P'},
    {'D', 'U', 'N', 'G'},
    {'D', 'U', 'N'},
    {'D', 'U', 'M'},
    {'D', 'U', 'C'},
    {'D', 'U', 'O'},
    {'D', 'U', 'A'},
    {'D', 'U', 'O', 'I'},
    {'D', 'U', 'O', 'C'},
    {'D', 'U', 'O', 'N'},
    {'D', 'U', 'O', 'N', 'G'},
    {'D', 'U'},
    {'D', 'U', 'P'},
    {'D', 'U', 'T'},
    {'D', 'U', 'I'},
    {'D', 'I', 'C', 'H'},
    {'D', 'I', 'C'},
    {'D', 'I', 'N', 'H'},
    {'D', 'I', 'N'},
    {'D', 'I'},
    {'D', 'I', 'A'},
    {'D', 'I', 'E'},
    {'D', 'I', 'E', 'C'},
    {'D', 'I', 'E', 'U'},
    {'D', 'I', 'E', 'N'},
    {'D', 'I', 'E', 'M'},
    {'D', 'I', 'E', 'P'},
    {'D', 'I', 'T'},
    {'D', 'O'},
    {'D', 'O', 'A'},
    {'D', 'O', 'A', 'N'},
    {'D', 'O', 'A', 'N', 'G'},
    {'D', 'O', 'A', 'N', 'H'},
    {'D', 'O', 'A', 'M'},
    {'D', 'O', 'E'},
    {'D', 'O', 'I'},
    {'D', 'O', 'P'},
    {'D', 'O', 'C'},
    {'D', 'O', 'N'},
    {'D', 'O', 'N', 'G'},
    {'D', 'O', 'M'},
    {'D', 'O', 'T'},
    {'D', 'A'},
    {'D', 'A', 'T'},
    {'D', 'A', 'Y'},
    {'D', 'A', 'U'},
    {'D', 'A', 'I'},
    {'D', 'A', 'O'},
    {'D', 'A', 'P'},
    {'D', 'A', 'C'},
    {'D', 'A', 'C', 'H'},
    {'D', 'A', 'N'},
    {'D', 'A', 'N', 'H'},
    {'D', 'A', 'N', 'G'},
    {'D', 'A', 'M'},
    {'D'}
};

vector<vector<char>> _vowelForMark = {
    // 'A'
    {'A', 'N', 'G'},
    {'A', 'N'},
    {'A', 'N', 'H'},
    {'A', 'M'},
    {'A', 'U'},
    {'A', 'Y'},
    {'A', 'T'},
    {'A', 'P'},
    {'A'},
    {'A', 'C'},
    {'A', 'I'},
    {'A', 'O'},
    {'A', 'C', 'H'},
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
    // 'E'
    {'E', 'N', 'H'},
    {'E', 'N', 'G'},
    {'E', 'C', 'H'},
    {'E', 'C'},
    {'E', 'T'},
    {'E', 'Y'},
    {'E', 'U'},
    {'E', 'P'},
    {'E', 'C'},
    {'E', 'N'},
    {'E', 'M'},
    {'E'},
    // 'I'
    {'I', 'N', 'H'},
    {'I', 'C', 'H'},
    {'I', 'N'},
    {'I', 'T'},
    {'I', 'U'},
    {'I', 'U', 'P'},
    {'I', 'N'},
    {'I', 'M'},
    {'I', 'P'},
    {'I', 'A'},
    {'I', 'C'},
    {'I'},
    // 'U'
    { 'U', 'N', 'G'},
    { 'U', 'I'},
    { 'U', 'O'},
    { 'U', 'Y'},
    { 'U', 'Y', 'N'},
    { 'U', 'Y', 'T'},
    { 'U', 'Y', 'P'},
    { 'U', 'Y', 'N', 'H'},
    { 'U', 'T'},
    { 'U', 'U'},
    { 'U', 'A'},
    { 'U', 'I'},
    { 'U', 'C'},
    { 'U', 'N'},
    { 'U', 'M'},
    { 'U', 'P'},
    { 'U'},
    // 'Y'
    {'Y'}
};

vector<char> _standaloneWbad = {'Q', 'W', 'E', 'Y', 'I', 'F', 'J', 'K', 'Z'};

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
