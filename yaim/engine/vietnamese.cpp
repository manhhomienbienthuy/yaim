//
//  vietnamese.cpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#include "vietnamese.hpp"

using namespace std;

map<Uint16, vector<vector<Uint16>>> _vowel = {
    {
        kVK_ANSI_A, {
            {kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_A, kVK_ANSI_N},
            {kVK_ANSI_A, kVK_ANSI_M},
            {kVK_ANSI_A, kVK_ANSI_U},
            {kVK_ANSI_A, kVK_ANSI_Y},
            {kVK_ANSI_A, kVK_ANSI_T},
            {kVK_ANSI_A, kVK_ANSI_P},
            {kVK_ANSI_A},
            {kVK_ANSI_A, kVK_ANSI_C},
        }
    }, {
        kVK_ANSI_O, {
            {kVK_ANSI_O, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_O, kVK_ANSI_N},
            {kVK_ANSI_O, kVK_ANSI_M},
            {kVK_ANSI_O, kVK_ANSI_I},
            {kVK_ANSI_O, kVK_ANSI_C},
            {kVK_ANSI_O, kVK_ANSI_T},
            {kVK_ANSI_O, kVK_ANSI_P},
            {kVK_ANSI_O}
        }
    }, {
        kVK_ANSI_E, {
            {kVK_ANSI_E, kVK_ANSI_N, kVK_ANSI_H},
            {kVK_ANSI_E, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_E, kVK_ANSI_C, kVK_ANSI_H},
            {kVK_ANSI_E, kVK_ANSI_C},
            {kVK_ANSI_E, kVK_ANSI_T},
            {kVK_ANSI_E, kVK_ANSI_Y},
            {kVK_ANSI_E, kVK_ANSI_U},
            {kVK_ANSI_E, kVK_ANSI_P},
            {kVK_ANSI_E, kVK_ANSI_C},
            {kVK_ANSI_E, kVK_ANSI_N},
            {kVK_ANSI_E, kVK_ANSI_M},
            {kVK_ANSI_E}
        }
    }, {
        kVK_ANSI_W, {
            {kVK_ANSI_O, kVK_ANSI_N},

            {kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_N, kVK_ANSI_G},

            {kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_N},
            {kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_I},
            {kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_C},

            {kVK_ANSI_O, kVK_ANSI_I},
            {kVK_ANSI_O, kVK_ANSI_P},
            {kVK_ANSI_O, kVK_ANSI_M},
            {kVK_ANSI_O, kVK_ANSI_A},
            {kVK_ANSI_O, kVK_ANSI_T},

            {kVK_ANSI_U, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_U, kVK_ANSI_N},
            {kVK_ANSI_U, kVK_ANSI_M},
            {kVK_ANSI_U, kVK_ANSI_C},
            {kVK_ANSI_U, kVK_ANSI_A},
            {kVK_ANSI_U, kVK_ANSI_I},
            {kVK_ANSI_U, kVK_ANSI_T},
            {kVK_ANSI_U},

            {kVK_ANSI_A, kVK_ANSI_P},
            {kVK_ANSI_A, kVK_ANSI_T},
            {kVK_ANSI_A, kVK_ANSI_M},

            {kVK_ANSI_A, kVK_ANSI_N},
            {kVK_ANSI_A},
            {kVK_ANSI_A, kVK_ANSI_C},
            {kVK_ANSI_A, kVK_ANSI_C, kVK_ANSI_H},

            {kVK_ANSI_O},
            {kVK_ANSI_U, kVK_ANSI_U}
        }
    }
};

map<Uint16, vector<vector<Uint32>>> _vowelCombine = {
    {
        kVK_ANSI_A, {
            // fist elem can has end consonant or not
            {0, kVK_ANSI_A, kVK_ANSI_I},
            {0, kVK_ANSI_A, kVK_ANSI_O},
            {0, kVK_ANSI_A, kVK_ANSI_U},
            {0, kVK_ANSI_A|TONE_MASK, kVK_ANSI_U},
            {0, kVK_ANSI_A, kVK_ANSI_Y},
            {0, kVK_ANSI_A|TONE_MASK, kVK_ANSI_Y},
        }
    }, {
        kVK_ANSI_E, {
            {0, kVK_ANSI_E, kVK_ANSI_O},
            {0, kVK_ANSI_E|TONE_MASK, kVK_ANSI_U},
        }
    }, {
        kVK_ANSI_I, {
            {1, kVK_ANSI_I, kVK_ANSI_E|TONE_MASK, kVK_ANSI_U},
            {0, kVK_ANSI_I, kVK_ANSI_A},
            {1, kVK_ANSI_I, kVK_ANSI_E|TONE_MASK},
            {0, kVK_ANSI_I, kVK_ANSI_U},

        }
    }, {
        kVK_ANSI_O, {
            {0, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_I},
            {0, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_O},
            {0, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_Y},
            {0, kVK_ANSI_O, kVK_ANSI_E, kVK_ANSI_O},
            {1, kVK_ANSI_O, kVK_ANSI_A},
            {1, kVK_ANSI_O, kVK_ANSI_A|TONEW_MASK},
            {1, kVK_ANSI_O, kVK_ANSI_E},
            {0, kVK_ANSI_O, kVK_ANSI_I},
            {0, kVK_ANSI_O|TONE_MASK, kVK_ANSI_I},
            {0, kVK_ANSI_O|TONEW_MASK, kVK_ANSI_I},
            {1, kVK_ANSI_O, kVK_ANSI_O},
            {1, kVK_ANSI_O|TONE_MASK, kVK_ANSI_O|TONE_MASK},
        }
    }, {
        kVK_ANSI_U, {
            {0, kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_U},
            {1, kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_E|TONE_MASK},
            {0, kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_A},
            {0, kVK_ANSI_U|TONEW_MASK, kVK_ANSI_O|TONEW_MASK, kVK_ANSI_U},
            {0, kVK_ANSI_U|TONEW_MASK, kVK_ANSI_O|TONEW_MASK, kVK_ANSI_I},
            {0, kVK_ANSI_U, kVK_ANSI_O|TONE_MASK, kVK_ANSI_I},
            {0, kVK_ANSI_U, kVK_ANSI_A|TONE_MASK, kVK_ANSI_Y},
            {1, kVK_ANSI_U, kVK_ANSI_A, kVK_ANSI_O},
            {1, kVK_ANSI_U, kVK_ANSI_A},
            {1, kVK_ANSI_U, kVK_ANSI_A|TONEW_MASK},
            {1, kVK_ANSI_U, kVK_ANSI_A|TONE_MASK},
            {0, kVK_ANSI_U|TONEW_MASK, kVK_ANSI_A},
            {1, kVK_ANSI_U, kVK_ANSI_E|TONE_MASK},
            {0, kVK_ANSI_U, kVK_ANSI_I},
            {0, kVK_ANSI_U|TONEW_MASK, kVK_ANSI_I},
            {1, kVK_ANSI_U, kVK_ANSI_O},
            {1, kVK_ANSI_U, kVK_ANSI_O|TONE_MASK},
            {0, kVK_ANSI_U, kVK_ANSI_O|TONEW_MASK},
            {1, kVK_ANSI_U|TONEW_MASK, kVK_ANSI_O|TONEW_MASK},
            {0, kVK_ANSI_U|TONEW_MASK, kVK_ANSI_U},
            {1, kVK_ANSI_U, kVK_ANSI_Y},
        }
    }, {
        kVK_ANSI_Y, {
            {0, kVK_ANSI_Y, kVK_ANSI_E|TONE_MASK, kVK_ANSI_U},
            {1, kVK_ANSI_Y, kVK_ANSI_E|TONE_MASK},
        }
    }
};

vector<vector<Uint16>> _consonantD = {
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_N, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_N, kVK_ANSI_G},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_C, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_M},
    {kVK_ANSI_D, kVK_ANSI_E},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_T},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_U},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_O},
    {kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_P},

    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_N, kVK_ANSI_G},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_M},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_O},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_A},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_I},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_O, kVK_ANSI_N, kVK_ANSI_G},
    {kVK_ANSI_D, kVK_ANSI_U},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_P},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_T},
    {kVK_ANSI_D, kVK_ANSI_U, kVK_ANSI_I},

    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_C, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_N, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_I},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_A},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_E},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_E, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_E, kVK_ANSI_U},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_E, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_E, kVK_ANSI_M},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_E, kVK_ANSI_P},
    {kVK_ANSI_D, kVK_ANSI_I, kVK_ANSI_T},

    {kVK_ANSI_D, kVK_ANSI_O},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_A},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_G},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_A, kVK_ANSI_M},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_E},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_I},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_P},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_N, kVK_ANSI_G},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_M},
    {kVK_ANSI_D, kVK_ANSI_O, kVK_ANSI_T},

    {kVK_ANSI_D, kVK_ANSI_A},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_T},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_Y},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_U},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_I},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_O},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_P},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_C},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_C, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_N},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_H},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_G},
    {kVK_ANSI_D, kVK_ANSI_A, kVK_ANSI_M},

    {kVK_ANSI_D}
};

map<Uint16, vector<vector<Uint16>>> _vowelForMark = {
    {
        kVK_ANSI_A, {
            {kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_A, kVK_ANSI_N},
            {kVK_ANSI_A, kVK_ANSI_N, kVK_ANSI_H},
            {kVK_ANSI_A, kVK_ANSI_M},
            {kVK_ANSI_A, kVK_ANSI_U},
            {kVK_ANSI_A, kVK_ANSI_Y},
            {kVK_ANSI_A, kVK_ANSI_T},
            {kVK_ANSI_A, kVK_ANSI_P},
            {kVK_ANSI_A},
            {kVK_ANSI_A, kVK_ANSI_C},
            {kVK_ANSI_A, kVK_ANSI_I},
            {kVK_ANSI_A, kVK_ANSI_O},
            {kVK_ANSI_A, kVK_ANSI_C, kVK_ANSI_H},
        }
    }, {
        kVK_ANSI_O, {
            {kVK_ANSI_O, kVK_ANSI_O, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_O, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_O, kVK_ANSI_O, kVK_ANSI_N},
            {kVK_ANSI_O, kVK_ANSI_O, kVK_ANSI_C},
            {kVK_ANSI_O, kVK_ANSI_O},
            {kVK_ANSI_O, kVK_ANSI_N},
            {kVK_ANSI_O, kVK_ANSI_M},
            {kVK_ANSI_O, kVK_ANSI_I},
            {kVK_ANSI_O, kVK_ANSI_C},
            {kVK_ANSI_O, kVK_ANSI_T},
            {kVK_ANSI_O, kVK_ANSI_P},
            {kVK_ANSI_O}
        }
    }, {
        kVK_ANSI_E, {
            {kVK_ANSI_E, kVK_ANSI_N, kVK_ANSI_H},
            {kVK_ANSI_E, kVK_ANSI_N, kVK_ANSI_G},
            {kVK_ANSI_E, kVK_ANSI_C, kVK_ANSI_H},
            {kVK_ANSI_E, kVK_ANSI_C},
            {kVK_ANSI_E, kVK_ANSI_T},
            {kVK_ANSI_E, kVK_ANSI_Y},
            {kVK_ANSI_E, kVK_ANSI_U},
            {kVK_ANSI_E, kVK_ANSI_P},
            {kVK_ANSI_E, kVK_ANSI_C},
            {kVK_ANSI_E, kVK_ANSI_N},
            {kVK_ANSI_E, kVK_ANSI_M},
            {kVK_ANSI_E}
        }
    }, {
        kVK_ANSI_I, {
            {kVK_ANSI_I, kVK_ANSI_N, kVK_ANSI_H},
            {kVK_ANSI_I, kVK_ANSI_C, kVK_ANSI_H},
            {kVK_ANSI_I, kVK_ANSI_N},
            {kVK_ANSI_I, kVK_ANSI_T},
            {kVK_ANSI_I, kVK_ANSI_U},
            {kVK_ANSI_I, kVK_ANSI_U, kVK_ANSI_P},
            {kVK_ANSI_I, kVK_ANSI_N},
            {kVK_ANSI_I, kVK_ANSI_M},
            {kVK_ANSI_I, kVK_ANSI_P},
            {kVK_ANSI_I, kVK_ANSI_A},
            {kVK_ANSI_I, kVK_ANSI_C},
            {kVK_ANSI_I},
        }
    }, {
        kVK_ANSI_U, {
            { kVK_ANSI_U, kVK_ANSI_N, kVK_ANSI_G},
            { kVK_ANSI_U, kVK_ANSI_I},
            { kVK_ANSI_U, kVK_ANSI_O},
            { kVK_ANSI_U, kVK_ANSI_Y},
            { kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_N},
            { kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_T},
            { kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_P},
            { kVK_ANSI_U, kVK_ANSI_Y, kVK_ANSI_N, kVK_ANSI_H},
            { kVK_ANSI_U, kVK_ANSI_T},
            { kVK_ANSI_U, kVK_ANSI_U},
            { kVK_ANSI_U, kVK_ANSI_A},
            { kVK_ANSI_U, kVK_ANSI_I},
            { kVK_ANSI_U, kVK_ANSI_C},
            { kVK_ANSI_U, kVK_ANSI_N},
            { kVK_ANSI_U, kVK_ANSI_M},
            { kVK_ANSI_U, kVK_ANSI_P},
            { kVK_ANSI_U}
        }
    }, {
        kVK_ANSI_Y, {
            {kVK_ANSI_Y}
        }
    }
};

vector<Uint16> _standaloneWbad = {
    kVK_ANSI_W, kVK_ANSI_E, kVK_ANSI_Y, kVK_ANSI_F, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_Z
};

/*
 * 0: Unicode
 * 1: TCVN3
 * 2: VNI
 * 3: Unicode Compound
 * 4: Vietnamese Locale CP 1258
 */
map<Uint32, vector<Uint16>> _codeTable = {
        // {keyCode, {CAPS_CHAR,    NORMAL_CHAR,     CAPS_W_CHAR,     NORMAL_W_CHAR}}
        // kVK_ANSI_A,           {Â, â, Ă, ă, Á, á, À, à, Ả, ả, Ã, ã, Ạ, ạ
        { kVK_ANSI_A,            {0x00C2, 0x00E2, 0x0102, 0x0103, 0x00C1, 0x00E1, 0x00C0, 0x00E0, 0x1EA2, 0x1EA3, 0x00C3, 0x00E3, 0x1EA0, 0x1EA1}},
        { kVK_ANSI_O,            {0x00D4, 0x00F4, 0x01A0, 0x01A1, 0x00D3, 0x00F3, 0x00D2, 0x00F2, 0x1ECE, 0x1ECF, 0x00D5, 0x00F5, 0x1ECC, 0x1ECD}},
        { kVK_ANSI_U,            {0x0000, 0x0000, 0x01AF, 0x01B0, 0x00DA, 0x00FA, 0x00D9, 0x00F9, 0x1EE6, 0x1EE7, 0x0168, 0x0169, 0x1EE4, 0x1EE5}},
        { kVK_ANSI_E,            {0x00CA, 0x00EA, 0x0000, 0x0000, 0x00C9, 0x00E9, 0x00C8, 0x00E8, 0x1EBA, 0x1EBB, 0x1EBC, 0x1EBD, 0x1EB8, 0x1EB9}},
        { kVK_ANSI_D,            {0x0110, 0x0111}},
                            // Ấ, ấ, Ầ, ầ, Ẩ, ẩ, Ẫ, ẫ, Ậ, ậ ,
        { kVK_ANSI_A|TONE_MASK,  {0x1EA4, 0x1EA5, 0x1EA6, 0x1EA7, 0x1EA8, 0x1EA9, 0x1EAA, 0x1EAB, 0x1EAC, 0x1EAD}},
                            // Ắ, ắ, Ằ, ằ, Ẳ, ẳ, Ẵ, ẵ, Ặ, ặ
        { kVK_ANSI_A|TONEW_MASK, {0x1EAE, 0x1EAF, 0x1EB0, 0x1EB1, 0x1EB2, 0x1EB3, 0x1EB4, 0x1EB5, 0x1EB6, 0x1EB7}},
        { kVK_ANSI_O|TONE_MASK,  {0x1ED0, 0x1ED1, 0x1ED2, 0x1ED3, 0x1ED4, 0x1ED5, 0x1ED6, 0x1ED7, 0x1ED8, 0x1ED9}},
        { kVK_ANSI_O|TONEW_MASK, {0x1EDA, 0x1EDB, 0x1EDC, 0x1EDD, 0x1EDE, 0x1EDF, 0x1EE0, 0x1EE1, 0x1EE2, 0x1EE3}},
        { kVK_ANSI_U|TONEW_MASK, {0x1EE8, 0x1EE9, 0x1EEA, 0x1EEB, 0x1EEC, 0x1EED, 0x1EEE, 0x1EEF, 0x1EF0, 0x1EF1}},
        { kVK_ANSI_E|TONE_MASK,  {0x1EBE, 0x1EBF, 0x1EC0, 0x1EC1, 0x1EC2, 0x1EC3, 0x1EC4, 0x1EC5, 0x1EC6, 0x1EC7}},
        { kVK_ANSI_I,            {0x00CD, 0x00ED, 0x00CC, 0x00EC, 0x1EC8, 0x1EC9, 0x128, 0x129, 0x1ECA, 0x1ECB}},
        { kVK_ANSI_Y,            {0x00DD, 0x00FD, 0x1EF2, 0x1EF3, 0x1EF6, 0x1EF7, 0x1EF8, 0x1EF9, 0x1EF4, 0x1EF5}},
};

map<Uint32, Uint32> _characterMap = {
    {'a', kVK_ANSI_A}, {'A', kVK_ANSI_A|CAPS_MASK},
    {'b', kVK_ANSI_B}, {'B', kVK_ANSI_B|CAPS_MASK},
    {'c', kVK_ANSI_C}, {'C', kVK_ANSI_C|CAPS_MASK},
    {'d', kVK_ANSI_D}, {'D', kVK_ANSI_D|CAPS_MASK},
    {'e', kVK_ANSI_E}, {'E', kVK_ANSI_E|CAPS_MASK},
    {'f', kVK_ANSI_F}, {'F', kVK_ANSI_F|CAPS_MASK},
    {'g', kVK_ANSI_G}, {'G', kVK_ANSI_G|CAPS_MASK},
    {'h', kVK_ANSI_H}, {'H', kVK_ANSI_H|CAPS_MASK},
    {'i', kVK_ANSI_I}, {'I', kVK_ANSI_I|CAPS_MASK},
    {'j', kVK_ANSI_J}, {'J', kVK_ANSI_J|CAPS_MASK},
    {'k', kVK_ANSI_K}, {'K', kVK_ANSI_K|CAPS_MASK},
    {'l', kVK_ANSI_L}, {'L', kVK_ANSI_L|CAPS_MASK},
    {'m', kVK_ANSI_M}, {'M', kVK_ANSI_M|CAPS_MASK},
    {'n', kVK_ANSI_N}, {'N', kVK_ANSI_N|CAPS_MASK},
    {'o', kVK_ANSI_O}, {'O', kVK_ANSI_O|CAPS_MASK},
    {'p', kVK_ANSI_P}, {'P', kVK_ANSI_P|CAPS_MASK},
    {'q', kVK_ANSI_Q}, {'Q', kVK_ANSI_Q|CAPS_MASK},
    {'r', kVK_ANSI_R}, {'R', kVK_ANSI_R|CAPS_MASK},
    {'s', kVK_ANSI_S}, {'S', kVK_ANSI_S|CAPS_MASK},
    {'t', kVK_ANSI_T}, {'T', kVK_ANSI_T|CAPS_MASK},
    {'u', kVK_ANSI_U}, {'U', kVK_ANSI_U|CAPS_MASK},
    {'v', kVK_ANSI_V}, {'V', kVK_ANSI_V|CAPS_MASK},
    {'w', kVK_ANSI_W}, {'W', kVK_ANSI_W|CAPS_MASK},
    {'x', kVK_ANSI_X}, {'X', kVK_ANSI_X|CAPS_MASK},
    {'y', kVK_ANSI_Y}, {'Y', kVK_ANSI_Y|CAPS_MASK},
    {'z', kVK_ANSI_Z}, {'Z', kVK_ANSI_Z|CAPS_MASK},
    {'1', kVK_ANSI_1}, {'!', kVK_ANSI_1|CAPS_MASK},
    {'2', kVK_ANSI_2}, {'@', kVK_ANSI_2|CAPS_MASK},
    {'3', kVK_ANSI_3}, {'#', kVK_ANSI_3|CAPS_MASK},
    {'4', kVK_ANSI_4}, {'$', kVK_ANSI_4|CAPS_MASK},
    {'5', kVK_ANSI_5}, {'%', kVK_ANSI_5|CAPS_MASK},
    {'6', kVK_ANSI_6}, {'^', kVK_ANSI_6|CAPS_MASK},
    {'7', kVK_ANSI_7}, {'&', kVK_ANSI_7|CAPS_MASK},
    {'8', kVK_ANSI_8}, {'*', kVK_ANSI_8|CAPS_MASK},
    {'9', kVK_ANSI_9}, {'(', kVK_ANSI_9|CAPS_MASK},
    {'0', kVK_ANSI_0}, {')', kVK_ANSI_0|CAPS_MASK},
    {'`', kVK_ANSI_Grave}, {'~', kVK_ANSI_Grave|CAPS_MASK},
    {'-', kVK_ANSI_Minus}, {'_', kVK_ANSI_Minus|CAPS_MASK},
    {'=', kVK_ANSI_Equal}, {'+', kVK_ANSI_Equal|CAPS_MASK},
    {'[', kVK_ANSI_LeftBracket}, {'{', kVK_ANSI_LeftBracket|CAPS_MASK},
    {']', kVK_ANSI_RightBracket}, {'}', kVK_ANSI_RightBracket|CAPS_MASK},
    {'\\', kVK_ANSI_Backslash}, {'|', kVK_ANSI_Backslash|CAPS_MASK},
    {';', kVK_ANSI_Semicolon}, {':', kVK_ANSI_Semicolon|CAPS_MASK},
    {'\'', kVK_ANSI_Quote}, {'"', kVK_ANSI_Quote|CAPS_MASK},
    {',', kVK_ANSI_Comma}, {'<', kVK_ANSI_Comma|CAPS_MASK},
    {'.', kVK_ANSI_Period}, {'>', kVK_ANSI_Period|CAPS_MASK},
    {'/', kVK_ANSI_Slash}, {'?', kVK_ANSI_Slash|CAPS_MASK},
    {' ', kVK_Space}
};

map<Uint32, Uint32> _keyCodeToChar;

void initKeyCodeToChar() {
    _keyCodeToChar.clear();
    for (std::map<Uint32, Uint32>::iterator it = _characterMap.begin(); it != _characterMap.end(); ++it) {
        _keyCodeToChar[it->second] = it->first;
    }
}

Uint16 keyCodeToCharacter(const Uint32& keyCode) {
    if (_keyCodeToChar.size() == 0) {  // init data if it is empty
        initKeyCodeToChar();
    }
    if (_keyCodeToChar.find(keyCode) != _keyCodeToChar.end()) {
        return _keyCodeToChar[keyCode];
    }
    return 0;
}
