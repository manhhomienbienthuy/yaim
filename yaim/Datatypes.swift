// Datatypes.swift
// yaim
//
//  Created by naa on 2026/09/29.
//  Modified from source code of OpenKey by TuyenVM.
//

let maxBuff = 16
let maxWord = 3

enum HookCode: Int {
    case doNothing = 0
    case willProcess
    case restore
}

struct KeyHookState {
    var code: HookCode = .doNothing
    var backspaceCount: Int = 0
    var newCharCount: Int = 0
    var charData: [UInt16] = Array(repeating: 0, count: maxBuff)
}

enum Bitmasks {
    static let capMask:      UInt16 = 0x00df  // strips bit 5 → uppercase
    static let toneMask:     UInt16 = 1 << 8  // ^ tone (Â/Ê/Ô)
    static let tonewMask:    UInt16 = 1 << 9  // w tone (Ă/Ơ/Ư)
    static let mark1Mask:    UInt16 = 1 << 10 // sắc
    static let mark2Mask:    UInt16 = 1 << 11 // huyền
    static let mark3Mask:    UInt16 = 1 << 12 // hỏi
    static let mark4Mask:    UInt16 = 1 << 13 // ngã
    static let mark5Mask:    UInt16 = 1 << 14 // nặng
    static let standaloneMask: UInt16 = 1 << 15
    static let markMask:     UInt16 = 0x1f << 10
    static let processMask:  UInt16 = 0xff << 8
}
