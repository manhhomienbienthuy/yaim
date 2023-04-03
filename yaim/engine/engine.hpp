//
//  engine.hpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#ifndef engine_hpp
#define engine_hpp

#include <locale>
#include <codecvt>

#include "datatypes.h"
#include "vietnamese.hpp"

#define GET_BOOL(data) (data ? 1 : 0)

//define these variable in your application
//API
/*
 * 0: English
 * 1: Vietnamese
 */
extern bool isVietnamese;

/**
 * Call this function first to receive data pointer
 */
void* vKeyInit();

/**
 * Convert engine character to real character
 */
Uint32 getCharacterCode(const Uint32& data);

/**
 * MAIN entry point for each key
 * event: mouse or keyboard event
 * state: additional state for event
 * data: key code
 * isCaps: caplock is on or shift key is pressing
 * otherControlKey: ctrl, option,... is pressing
 */
void vKeyHandleEvent(const vKeyEvent& event,
                     const vKeyEventState& state,
                     const Uint16& data,
                     const Uint8& capsStatus=0,
                     const bool& otherControlKey=false);

/**
 * Start a new word
 */
void startNewSession();


#endif /* engine_hpp */
