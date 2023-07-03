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

void* vInit();
void vHandleKey(const UInt16&);


#endif /* engine_hpp */
