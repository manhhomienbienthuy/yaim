// Engine.swift
// yaim
//
//  Created by naa on 2026/09/29.
//  Modified from source code of OpenKey by TuyenVM.
//

import Foundation

final class Engine {
    var hookState = KeyHookState()

    // Current word being composed (bit-packed: base char | tone bits | mark bits)
    var typingWord = [UInt16](repeating: 0, count: maxBuff)
    var index: Int = 0  // number of elements in typingWord

    // Raw key codes typed (parallel to typingWord, for restore)
    var typingKeys = [UInt16](repeating: 0, count: maxBuff)
    var stateIndex: Int = 0

    // Undo history (up to maxWord words)
    var typingStates: [[UInt16]] = []
    var keyStates: [[UInt16]] = []

    // Vowel analysis results (populated by calcVowels)
    var vowelCount: Int = 0
    var vsi: Int = 0  // vowelStartIndex
    var vei: Int = 0  // vowelEndIndex
    var vsm: Int = 0  // vowelWillSetMark

    var spaceCount: Int = 0

    // Input mode flags (set by AppDelegate, read by EventProcessor)
    var isVietnamese: Bool = true
    var isABCKeyboard: Bool = true

    // ── Shorthand accessors ──────────────────────────────────────────────────

    private var hBPC: Int {
        get { hookState.backspaceCount }
        set { hookState.backspaceCount = newValue }
    }
    private var hNCC: Int {
        get { hookState.newCharCount }
        set { hookState.newCharCount = newValue }
    }
    private var hCode: HookCode {
        get { hookState.code }
        set { hookState.code = newValue }
    }

    private func chr(_ i: Int) -> UInt16 {
        typingWord[i] & Bitmasks.capMask
    }

    private func ord(_ i: Int) -> UInt16 {
        getCharacterCode(typingWord[i])
    }

    // ── Lifecycle ────────────────────────────────────────────────────────────

    func reset() {
        typingStates.removeAll()
        keyStates.removeAll()
        startNewSession()
    }

    private func startNewSession() {
        index = 0
        stateIndex = 0
        spaceCount = 0
    }

    // ── History ──────────────────────────────────────────────────────────────

    private func saveHistory() {
        guard index > 0 else { return }

        var stateData = Array(typingWord[0..<index])
        stateData.append(UInt16(spaceCount))
        typingStates.append(stateData)
        if typingStates.count > maxWord { typingStates.removeFirst() }

        let keyData = Array(typingKeys[0..<stateIndex])
        keyStates.append(keyData.map { $0 })
        if keyStates.count > maxWord { keyStates.removeFirst() }
    }

    private func restoreLastState() {
        guard !typingStates.isEmpty else { return }
        let stateData = typingStates.removeLast()
        let keyData = keyStates.removeLast()

        spaceCount = Int(stateData.last!)
        let wordLen = stateData.count - 1
        for i in 0..<wordLen { typingWord[i] = stateData[i] }
        index = wordLen

        for i in 0..<keyData.count { typingKeys[i] = keyData[i] }
        stateIndex = keyData.count
    }

    // ── Typing buffers ───────────────────────────────────────────────────────

    private func addToTypingWord(_ charCode: UInt16) {
        typingWord[index] = charCode
        index += 1
    }

    private func addToTypingKeys(_ charCode: UInt16) {
        if stateIndex >= maxBuff {
            saveHistory()
            startNewSession()
        }
        typingKeys[stateIndex] = charCode
        stateIndex += 1
    }

    // ── Vowel analysis ───────────────────────────────────────────────────────

    private func calcVowels() {
        vowelCount = 0; vsi = 0; vei = 0
        let vowels: Set<UInt16> = [65, 69, 85, 89, 73, 79] // A E U Y I O
        for i in stride(from: index - 1, through: 0, by: -1) {
            if !vowels.contains(chr(i)) {
                if vowelCount > 0 { break }
            } else {
                // 'U' after 'Q' is not a vowel
                if chr(i) == 85 && i > 0 && chr(i - 1) == 81 { break }
                // 'I' after 'G' when there's already a vowel with no marks
                if chr(i) == 73 && i > 0 && chr(i - 1) == 71 &&
                   vowelCount > 0 && !(typingWord[i] & Bitmasks.markMask != 0) { break }
                if vowelCount == 0 { vei = i }
                vsi = i
                vowelCount += 1
            }
        }
    }

    private func calcMarkPosition() {
        if vowelCount == 1 ||
           typingWord[vei] & (Bitmasks.toneMask | Bitmasks.tonewMask) != 0 ||
           vei + 1 < index {
            vsm = vei
        } else {
            vsm = vei - 1
        }
    }

    // ── Lookup helpers ───────────────────────────────────────────────────────

    private func findCombination(_ charsets: [[UInt16]], size: Int, offset: Int = 0) -> Int {
        for i in 0..<charsets.count {
            if charsets[i].count != size { continue }
            var found = true
            for j in 0..<size {
                if charsets[i][j] != chr(j + offset) { found = false; break }
            }
            if found { return i }
        }
        return -1
    }

    private func isGoodSpelling(_ rimesCharset: [[UInt16]]) -> Bool {
        calcVowels()
        guard vowelCount > 0 else { return false }

        if vsi > 0 {
            let found = findCombination(VietnameseTables.beginConsonants, size: vsi, offset: 0)
            guard found >= 0 else { return false }

            let firstVowel = chr(vsi)
            // K (index 13) and NGH (index 0): only I, E, Y
            if (found == 0 || found == 13) &&
               firstVowel != 73 && firstVowel != 69 && firstVowel != 89 { return false }
            // NG (index 7): only A, U, O
            if found == 7 &&
               firstVowel != 65 && firstVowel != 85 && firstVowel != 79 { return false }
        }

        return findCombination(rimesCharset, size: index - vsi, offset: vsi) > -1
    }

    private func getCharacterCode(_ data: UInt16) -> UInt16 {
        guard data & Bitmasks.processMask != 0 else { return data }

        let key = data & (Bitmasks.capMask | Bitmasks.toneMask | Bitmasks.tonewMask)
        guard let table = VietnameseTables.codeTable[key] else { return data }

        var markIndex = 0
        switch data & Bitmasks.markMask {
        case Bitmasks.mark1Mask: markIndex = 2
        case Bitmasks.mark2Mask: markIndex = 4
        case Bitmasks.mark3Mask: markIndex = 6
        case Bitmasks.mark4Mask: markIndex = 8
        case Bitmasks.mark5Mask: markIndex = 10
        default: break
        }

        let caseOffset = Int(data >> 5 & 1)  // 1 = lowercase
        return table[markIndex + caseOffset]
    }

    // ── Mark operations ──────────────────────────────────────────────────────

    private func removeMark(_ charCode: UInt16) {
        calcVowels()
        calcMarkPosition()
        if vowelCount > 0 && typingWord[vsm] & Bitmasks.markMask != 0 {
            typingWord[vsm] &= ~Bitmasks.markMask
            hCode = .willProcess
            for i in vsm..<index { hookState.charData[i - vsm] = ord(i) }
            hNCC = index - vsm
            hBPC = index - vsm
            // remove the mark key from history
            stateIndex -= 1 + (isMarkKey(typingKeys[stateIndex - 2]) ? 1 : 0)
        } else {
            addToTypingWord(charCode)
        }
    }

    private func isMarkKey(_ k: UInt16) -> Bool {
        k == 115 || k == 102 || k == 114 || k == 120 || k == 106  // s f r x j
    }

    private func insertMark(_ markMask: UInt16, canModifyFlag: Bool = true) {
        calcMarkPosition()
        if typingWord[vsm] & markMask != 0 {
            // duplicate mark → restore
            typingWord[vsm] &= ~Bitmasks.markMask
            if canModifyFlag { hCode = .restore }
        } else {
            if canModifyFlag { hCode = .willProcess }
            if typingWord[vsm] & Bitmasks.markMask != 0 {
                typingWord[vsm] &= ~Bitmasks.markMask
                if isMarkKey(typingKeys[stateIndex - 2]) {
                    stateIndex -= 1
                }
            }
            typingWord[vsm] |= markMask
        }
        for i in vsm..<index { hookState.charData[i - vsm] = ord(i) }
        hNCC = index - vsm
        hBPC = index - vsm
    }

    private func isLimitedMark() -> Bool {
        let last = chr(index - 1)
        return last == 84 || last == 67 || last == 80 || // T C P
               (last == 72 && chr(index - 2) == 67)       // CH
    }

    private func processMark(_ charCode: UInt16) {
        if isGoodSpelling(VietnameseTables.rimesForMark) {
            switch charCode & Bitmasks.capMask {
            case 83: insertMark(Bitmasks.mark1Mask); return  // S → sắc
            case 70:                                          // F → huyền
                if !isLimitedMark() { insertMark(Bitmasks.mark2Mask); return }
            case 82:                                          // R → hỏi
                if !isLimitedMark() { insertMark(Bitmasks.mark3Mask); return }
            case 88:                                          // X → ngã
                if !isLimitedMark() { insertMark(Bitmasks.mark4Mask); return }
            case 74: insertMark(Bitmasks.mark5Mask); return  // J → nặng
            default: break
            }
        }
        addToTypingWord(charCode)
    }

    // ── D insertion (Đ) ──────────────────────────────────────────────────────

    private func insertD(_ charCode: UInt16) {
        if chr(0) == 68 && (index == 1 || isGoodSpelling(VietnameseTables.rimesForMark)) {
            if typingWord[0] & Bitmasks.toneMask != 0 {
                typingWord[0] &= ~Bitmasks.toneMask
                hCode = .restore
            } else {
                typingWord[0] |= Bitmasks.toneMask
                hCode = .willProcess
            }
            for i in 0..<index { hookState.charData[i] = ord(i) }
            hNCC = index; hBPC = index
            return
        }
        addToTypingWord(charCode)
    }

    // ── Tone insertion (A / O / E) ────────────────────────────────────────────

    private func insertAOE(_ charCode: UInt16) {
        for i in vsi..<index {
            if i <= vei { typingWord[i] &= ~Bitmasks.tonewMask }
            if chr(i) == charCode {
                if typingWord[i] & Bitmasks.toneMask != 0 {
                    typingWord[i] &= ~Bitmasks.toneMask
                    hCode = .restore
                } else {
                    typingWord[i] |= Bitmasks.toneMask
                    hCode = .willProcess
                }
            }
            hookState.charData[i - vsi] = ord(i)
        }
        hNCC = index - vsi
        hBPC = index - vsi
    }

    // ── W insertion (Ă / Ơ / Ư) ─────────────────────────────────────────────

    private func insertW(_ charCode: UInt16) {
        if vowelCount > 1 {
            if (typingWord[vsi] & Bitmasks.tonewMask != 0 &&
                (typingWord[vsi + 1] & Bitmasks.tonewMask != 0 ||
                 chr(vsi + 1) == 73 || chr(vsi + 1) == 65 || chr(vsi + 1) == 85)) ||
               (typingWord[vsi] & Bitmasks.tonewMask == 0 &&
                typingWord[vsi + 1] & Bitmasks.tonewMask != 0 &&
                vsi == index - 2) {
                hCode = .restore
                typingWord[vsi] &= ~(Bitmasks.tonewMask | Bitmasks.standaloneMask)
                typingWord[vsi + 1] &= ~(Bitmasks.tonewMask | Bitmasks.standaloneMask)
            } else {
                hCode = .willProcess
                if chr(vsi) == 85 && chr(vsi + 1) == 79 {  // UO
                    typingWord[vsi + 1] |= Bitmasks.tonewMask
                    if vsi == 0 || chr(vsi - 1) != 72 || vsi < index - 2 {
                        typingWord[vsi] |= Bitmasks.tonewMask
                    }
                } else if (chr(vsi) == 85 && chr(vsi + 1) == 65) ||  // UA
                          (chr(vsi) == 85 && chr(vsi + 1) == 73) ||  // UI
                          (chr(vsi) == 85 && chr(vsi + 1) == 85) ||  // UU
                          (chr(vsi) == 79 && chr(vsi + 1) == 73) {   // OI
                    typingWord[vsi] |= Bitmasks.tonewMask
                } else if chr(vsi) == 79 && chr(vsi + 1) == 65 {     // OA
                    typingWord[vsi + 1] |= Bitmasks.tonewMask
                } else {
                    hCode = .doNothing
                    addToTypingWord(charCode)
                    return
                }
            }
        } else {
            if typingWord[vsi] & Bitmasks.tonewMask != 0 {
                if typingWord[vsi] & Bitmasks.standaloneMask != 0 {
                    // standalone W → convert to literal W/w
                    hCode = .willProcess
                    typingWord[vsi] = UInt8(truncatingIfNeeded: typingWord[vsi]) == 85 ? 87 : 119  // U→W, u→w
                } else {
                    hCode = .restore
                    typingWord[vsi] &= ~Bitmasks.tonewMask
                }
            } else {
                hCode = .willProcess
                typingWord[vsi] |= Bitmasks.tonewMask
                typingWord[vsi] &= ~Bitmasks.toneMask
            }
        }

        for i in vsi..<index {
            if i <= vei { typingWord[i] &= ~Bitmasks.toneMask }
            hookState.charData[i - vsi] = ord(i)
        }
        hNCC = index - vsi
        hBPC = index - vsi
    }

    // ── Standalone W ─────────────────────────────────────────────────────────

    private func processStandalone(_ charCode: UInt16) {
        // W typed at start of syllable → becomes Ư/ư placeholder
        if (charCode == 87 || charCode == 119) &&   // W or w
           (index == 0 || findCombination(VietnameseTables.beginConsonants, size: index, offset: 0) > -1) {
            hCode = .willProcess
            let base: UInt16 = charCode == 87 ? 85 : 117  // W→U, w→u
            typingWord[index] = base | Bitmasks.tonewMask | Bitmasks.standaloneMask
            hookState.charData[hNCC] = ord(index)
            hNCC += 1
            index += 1
            return
        }
        addToTypingWord(charCode)
    }

    // ── Tone processing (A / O / E / W) ─────────────────────────────────────

    private func processTone(_ charCode: UInt16) {
        let key = charCode & Bitmasks.capMask
        if let rimes = VietnameseTables.rimesForTone[key], isGoodSpelling(rimes) {
            if charCode == 87 || charCode == 119 {  // W/w
                insertW(charCode)
            } else {
                insertAOE(key)
            }
            return
        }
        processStandalone(charCode)
    }

    // ── Restore typing (Option+ESC) ───────────────────────────────────────────

    private func restoreTyping() {
        hCode = .doNothing
        for i in 0..<index {
            if typingWord[i] & Bitmasks.processMask != 0 && hCode == .doNothing {
                vsm = i
                hCode = .willProcess
                hBPC = index - vsm + spaceCount
                hNCC = stateIndex - vsm
                index = stateIndex
                spaceCount = 0
            }
            if hCode == .willProcess {
                hookState.charData[i - vsm] = typingWord[i]
                typingWord[i] = typingKeys[i]
            }
        }
    }

    // ── Regulate spelling after each key ─────────────────────────────────────

    private func regulateSpelling(deltaBackSpace: Int) {
        guard index >= 2 else { return }

        let vowels: Set<UInt16> = [65, 69, 85, 89, 73, 79]
        let consonantsAfterVowel: Set<UInt16> = [67, 77, 78, 80, 84]
        let last = chr(index - 1)
        let prev = chr(index - 2)

        guard vowels.contains(last) ||
              (consonantsAfterVowel.contains(last) && vowels.contains(prev)) else { return }

        calcVowels()
        guard vowelCount > 0 else { return }

        var needCorrect = false
        var left = index - hNCC

        // Fix mismatched TONEW on UO clusters: "uơn", "ưoi", "ưom", "ưoc", "uơu"
        if index >= 3 {
            for i in stride(from: index - 1, through: 0, by: -1) {
                if chr(i) == 85 && i < index - 2 && chr(i + 1) == 79 {
                    let next2 = chr(i + 2)
                    if next2 == 78 || next2 == 67 || next2 == 73 ||
                       next2 == 77 || next2 == 80 || next2 == 84 || next2 == 85 {
                        let mismatch = (typingWord[i] & Bitmasks.tonewMask == 0) !=
                                       (typingWord[i + 1] & Bitmasks.tonewMask == 0)
                        if mismatch {
                            typingWord[i] |= Bitmasks.tonewMask
                            typingWord[i + 1] |= Bitmasks.tonewMask
                            needCorrect = true
                            left = min(i, left)
                            break
                        }
                    }
                }
            }
        }

        // Move mark to correct position
        if index >= 2 {
            for i in vsi...vei {
                if typingWord[i] & Bitmasks.markMask != 0 {
                    let mark = typingWord[i] & Bitmasks.markMask
                    typingWord[i] &= ~Bitmasks.markMask
                    calcVowels()
                    calcMarkPosition()
                    if i != vsm {
                        needCorrect = true
                        left = min(i, left)
                        insertMark(mark, canModifyFlag: false)
                    } else {
                        typingWord[i] |= mark
                    }
                    break
                }
            }
        }

        if needCorrect {
            if hCode == .doNothing { hCode = .willProcess }
            hNCC = index - left
            hBPC = index - left + deltaBackSpace
            for i in left..<index { hookState.charData[i - left] = ord(i) }
        }
    }

    // ── Main entry point ──────────────────────────────────────────────────────

    func handleKey(_ charCode: UInt16) {
        if charCode == 0x00 {  // Option + ESC
            restoreTyping()

        } else if charCode == 0x7f {  // Option + Backspace
            hCode = .doNothing
            startNewSession()
            while !typingStates.isEmpty && spaceCount == 0 {
                restoreLastState()
            }

        } else if charCode == 0x08 {  // Backspace
            hCode = .doNothing
            if spaceCount > 0 {
                spaceCount -= 1
            } else if index > 0 {
                index -= 1
                if index == 0 {
                    stateIndex = 0
                    restoreLastState()
                } else {
                    let removed = typingWord[index]
                    var delta = 1
                    if removed & Bitmasks.toneMask != 0 { delta += 1 }
                    if chr(index) == 79 && removed & Bitmasks.tonewMask != 0 { delta += 1 }
                    if removed & Bitmasks.markMask != 0 { delta += 1 }
                    stateIndex -= delta
                    if stateIndex < index { stateIndex = index }
                    regulateSpelling(deltaBackSpace: 1)
                }
            }

        } else if (charCode & Bitmasks.capMask) < 65 || (charCode & Bitmasks.capMask) > 90 {
            // Not a letter
            hCode = .doNothing
            if charCode != 0x1b { spaceCount += 1 }  // ESC doesn't count as space

        } else {
            // Check if we need to start a new word
            if spaceCount > 0 ||
               (charCode >= 65 && charCode <= 90 &&        // uppercase letter
                index > 0 &&
                UInt8(truncatingIfNeeded: typingWord[index - 1]) >= 97 &&  // prev was lowercase
                UInt8(truncatingIfNeeded: typingWord[index - 1]) <= 122) {
                saveHistory()
                startNewSession()
            }

            addToTypingKeys(charCode)
            hCode = .doNothing
            hNCC = 0; hBPC = 0

            switch charCode & Bitmasks.capMask {
            case 90: removeMark(charCode)                        // Z
            case 68: insertD(charCode)                           // D
            case 83, 70, 82, 74, 88: processMark(charCode)      // S F R J X
            case 65, 69, 79, 87: processTone(charCode)          // A E O W
            default: addToTypingWord(charCode)
            }

            regulateSpelling(deltaBackSpace: (hCode == .doNothing || hBPC == 0) ? -1 : 0)

            if hCode == .restore {
                addToTypingWord(charCode)
                stateIndex -= 1
            }
        }
    }
}
