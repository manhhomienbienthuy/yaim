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

#define MAX_BUFF 16
#define MAX_WORD 3

//typedef unsigned short UInt16;

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
    CAP_MASK        = 0xdf, // 0xff & ~(1 << 5)
    TONE_MASK       = 1 << 8, // ^ tone
    TONEW_MASK      = 1 << 9, // tone ă, ư, ơ
    MARK1_MASK      = 1 << 10, // sắc
    MARK2_MASK      = 1 << 11, // huyền
    MARK3_MASK      = 1 << 12, // hỏi
    MARK4_MASK      = 1 << 13, // ngã
    MARK5_MASK      = 1 << 14, // nặng
    STANDALONE_MASK = 1 << 15, // standalone key (w)
    MARK_MASK       = 0x1f << 10,
    PROCESS_MASK    = 0xff << 8,
};


#endif /* datatypes_h */
