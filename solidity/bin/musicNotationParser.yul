object "MusicNotationParser" {
    // ========================================================================
    // DEPLOYMENT INITIALIZATION BLOCK (INITCODE)
    // ========================================================================
    code {
        if callvalue() { revert(0, 0) }

        // Copy the core runtime bytecode into memory at slot 0x00
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        
        // Return the runtime bytecode
        return(0x00, datasize("runtime"))
    }
    
    // ========================================================================
    // MAIN RUNTIME ENVIRONMENT
    // ========================================================================
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // parseNote(string) -> selector: 0x4f1fe20d
            if eq(selector, 0x4f1fe20d) {
                let offset := calldataload(4)
                let length := calldataload(add(4, offset))
                
                if lt(length, 2) { revert(0, 0) }

                // Read note string characters left-aligned in a word
                let dataWord := calldataload(add(36, offset))

                let char0 := byte(0, dataWord) // Note letter (C, D, E, F, G, A, B)
                let char1 := byte(1, dataWord) // Accidental (# or b) or Octave (0-8)

                let noteIndex := 0
                let hasAccidental := 0
                let octaveChar := char1
                let waveChar := byte(2, dataWord)

                // Check note letter
                switch char0
                case 67 { noteIndex := 0 }    // 'C'
                case 99 { noteIndex := 0 }    // 'c'
                case 68 { noteIndex := 2 }    // 'D'
                case 100 { noteIndex := 2 }   // 'd'
                case 69 { noteIndex := 4 }    // 'E'
                case 101 { noteIndex := 4 }   // 'e'
                case 70 { noteIndex := 5 }    // 'F'
                case 102 { noteIndex := 5 }   // 'f'
                case 71 { noteIndex := 7 }    // 'G'
                case 103 { noteIndex := 7 }   // 'g'
                case 65 { noteIndex := 9 }    // 'A'
                case 97 { noteIndex := 9 }    // 'a'
                case 66 { noteIndex := 11 }   // 'B'
                case 98 { noteIndex := 11 }   // 'b'
                default { revert(0, 0) }

                // Check for accidental (char1 is '#' (35) or 'b' (98/flat))
                // Note: we must distinguish between note B ('b') and flat accidental ('b').
                // In C64 music editors, flats are usually lowercase 'b'.
                let isFlat := eq(char1, 98)
                let isSharp := eq(char1, 35)

                if or(isSharp, isFlat) {
                    hasAccidental := 1
                    octaveChar := byte(2, dataWord)
                    waveChar := byte(3, dataWord)
                    
                    if isSharp {
                        noteIndex := add(noteIndex, 1)
                    }
                    if isFlat {
                        // If flat, subtract 1. Handle wrap-under (Db4 becomes C#4, i.e. index 1)
                        if iszero(noteIndex) {
                            noteIndex := 12
                        }
                        noteIndex := sub(noteIndex, 1)
                    }
                }

                // Parse Octave (ASCII 48 to 56)
                if or(lt(octaveChar, 48), gt(octaveChar, 56)) { revert(0, 0) }
                let octave := sub(octaveChar, 48)

                // Base frequencies for Octave 4 (C4 to B4)
                let freq := 0
                switch noteIndex
                case 0 { freq := 4392 }   // C4
                case 1 { freq := 4653 }   // C#4
                case 2 { freq := 4930 }   // D4
                case 3 { freq := 5223 }   // D#4
                case 4 { freq := 5534 }   // E4
                case 5 { freq := 5863 }   // F4
                case 6 { freq := 6211 }   // F#4
                case 7 { freq := 6581 }   // G4
                case 8 { freq := 6972 }   // G#4
                case 9 { freq := 7386 }   // A4
                case 10 { freq := 7825 }  // A#4
                case 11 { freq := 8291 }  // B4
                default { revert(0, 0) }

                // Scale frequency based on octave difference from 4
                if gt(octave, 4) {
                    freq := shl(sub(octave, 4), freq)
                }
                if lt(octave, 4) {
                    freq := shr(sub(4, octave), freq)
                }

                // Parse Waveform char: T -> 0x11, S -> 0x21, P -> 0x41, N -> 0x81
                let waveform := 0x11 // Default to Triangle
                switch waveChar
                case 84 { waveform := 0x11 } // 'T'
                case 116 { waveform := 0x11 } // 't'
                case 83 { waveform := 0x21 } // 'S'
                case 115 { waveform := 0x21 } // 's'
                case 80 { waveform := 0x41 } // 'P'
                case 112 { waveform := 0x41 } // 'p'
                case 78 { waveform := 0x81 } // 'N'
                case 110 { waveform := 0x81 } // 'n'

                // Return (frequency, waveform)
                mstore(0x00, freq)
                mstore(0x20, waveform)
                return(0x00, 64)
            }

            revert(0, 0)
        }
    }
}
