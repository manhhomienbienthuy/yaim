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

// internal engine data
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
#define MARK_MASK                               0xf80000

// mark and get first 16 bits character
#define CHAR_MASK                               0xffff

// Check whether the data is create by standalone key or not (W)
#define STANDALONE_MASK                         0x1000000


#endif /* datatypes_h */
