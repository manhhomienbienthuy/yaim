//
//  engine.hpp
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#ifndef engine_hpp
#define engine_hpp

#include "vietnamese.hpp"

extern bool isVietnamese;
extern bool isABCKeyboard;

void* vKeyInit();
void vKeyHandleEvent(const Uint16&, const bool&, const bool&);


#endif /* engine_hpp */
