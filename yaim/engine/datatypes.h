//
//  datatypes.h
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#ifndef datatypes_h
#define datatypes_h

#include <MacTypes.h>

#define MAX_BUFF 32
#define MAX_WORD 3

//typedef unsigned short UInt16;
//typedef unsigned int UInt32;

enum HoolCodeState {
    vDoNothing = 0,
    vWillProcess,
    vRestore,
};

// data for main program
struct vKeyHookState {
    HoolCodeState code;
    char backspaceCount;
    char newCharCount;
    UInt16 charData[MAX_BUFF];
};

enum {
    TONE_MASK       = 1 << 17, // ^ tone
    TONEW_MASK      = 1 << 18, // tone ă, ư, ơ
    MARK1_MASK      = 1 << 19, // sắc
    MARK2_MASK      = 1 << 20, // huyền
    MARK3_MASK      = 1 << 21, // hỏi
    MARK4_MASK      = 1 << 22, // ngã
    MARK5_MASK      = 1 << 23, // nặng
    MARK_MASK       = 0x1f << 19,
    STANDALONE_MASK = 1 << 24, // standalone key (w)
};


#endif /* datatypes_h */
