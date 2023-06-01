//
//  datatypes.h
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#ifndef datatypes_h
#define datatypes_h

#include <vector>

using namespace std;

#define MAX_BUFF 32
#define MAX_WORD 3

enum vKeyEvent {
    Keyboard,
    Mouse
};

enum vKeyEventState {
    KeyDown,
    KeyUp,
    MouseDown,
    MouseUp
};

typedef unsigned char Byte;
typedef signed char Int8;
typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
typedef unsigned long int Uint64;

enum HoolCodeState {
    vDoNothing = 0, // do not do anything
    vWillProcess,   // will reverse
    vRestore,       // restore character to old char
};

// bytes data for main program
struct vKeyHookState {
    /*
     * 0: Do nothing
     * 1: Process
     * 2: Restore
     */
    Byte code;
    Byte backspaceCount;
    Byte newCharCount;
    Uint32 charData[MAX_BUFF];
};

#include "keycodes.h"

// internal engine data
#define CAPS_MASK                               0x10000
#define TONE_MASK                               0x20000
#define TONEW_MASK                              0x40000

/*
 * MARK MASK
 * 1: Dấu Sắc - á
 * 2: Dấu Huyền - à
 * 3: Dấu Hỏi - ả
 * 4: Dấu Ngã - ã
 * 5: dấu Nặng - ạ
 */
#define MARK1_MASK                              0x080000
#define MARK2_MASK                              0x100000
#define MARK3_MASK                              0x200000
#define MARK4_MASK                              0x400000
#define MARK5_MASK                              0x800000

// for checking has mark or not
#define MARK_MASK                               0xF80000

// mark and get first 16 bytes character
#define CHAR_MASK                               0xFFFF

// Check whether the data is create by standalone key or not (W)
#define STANDALONE_MASK                         0x1000000

// Chec whether the data is keyboard code or character code
#define CHAR_CODE_MASK                          0x2000000

#define PURE_CHARACTER_MASK                     0x80000000


#endif /* datatypes_h */
