; ==========================================
; WITCHES' SABBATH (VAESEN) - 6502 ASSEMBLY
; GOTHIC Retrospective for the Mythic North
; ==========================================
; Memory registers:
; $D6E0 (55008): Witch X coordinate
; $D6E1 (55009): Witch Y coordinate
; $D6E2 (55010): Ingredient X
; $D6E3 (55011): Ingredient Y
; $D6E4 (55012): Cauldron Temperature
; $D6E5 (55013): Score
; $D6E6 (55014): Energy
; $D6E7 (55015): Input Direction (1=Up, 2=Down, 3=Left, 4=Right)
; $D6E8 (55016): Physics Trigger Strobe
; $D6E9 (55017): Win State (1 = Win, 2 = Game Over)

START:
    ; Initialize player coordinate
    LDA #$A0        ; Start X = 160
    STA $D6E0
    LDA #$50        ; Start Y = 80
    STA $D6E1
    
    ; Initialize ingredient coordinate
    LDA #$C8        ; Ingredient X = 200
    STA $D6E2
    LDA #$32        ; Ingredient Y = 50
    STA $D6E3
    
    ; Initialize stats
    LDA #$00        ; Score = 0
    STA $D6E5
    LDA #$64        ; Energy = 100
    STA $D6E6
    LDA #$A5        ; Cauldron Temp = 285 - 120 = 165
    STA $D6E4

LOOP:
    ; Wait for physics trigger strobe ($D6E8 == 1)
    LDA $D6E8
    CMP #$01
    BNE LOOP
    
    ; Reset strobe
    LDA #$00
    STA $D6E8

    ; Process input direction
    LDA $D6E7
    CMP #$01        ; Up
    BEQ MOVE_UP
    CMP #$02        ; Down
    BEQ MOVE_DOWN
    CMP #$03        ; Left
    BEQ MOVE_LEFT
    CMP #$04        ; Right
    BEQ MOVE_RIGHT
    JMP PROCESS_PHYSICS

MOVE_UP:
    LDA $D6E1
    SEC
    SBC #$08
    STA $D6E1
    JMP PROCESS_PHYSICS

MOVE_DOWN:
    LDA $D6E1
    CLC
    ADC #$08
    STA $D6E1
    JMP PROCESS_PHYSICS

MOVE_LEFT:
    LDA $D6E0
    SEC
    SBC #$08
    STA $D6E0
    JMP PROCESS_PHYSICS

MOVE_RIGHT:
    LDA $D6E0
    CLC
    ADC #$08
    STA $D6E0
    JMP PROCESS_PHYSICS

PROCESS_PHYSICS:
    ; Clear input direction
    LDA #$00
    STA $D6E7

    ; Check collision with ingredient
    LDA $D6E0
    SEC
    SBC $D6E2
    BNE CHECK_DIST_Y
    JMP COLLECT_INGREDIENT

CHECK_DIST_Y:
    LDA $D6E1
    SEC
    SBC $D6E3
    BNE TEMPERATURE_FLOW
    
COLLECT_INGREDIENT:
    ; Increment score by 100
    LDA $D6E5
    CLC
    ADC #$64
    STA $D6E5
    
    ; Randomize ingredient (coords incremented)
    LDA $D6E2
    CLC
    ADC #$46
    STA $D6E2
    LDA $D6E3
    CLC
    ADC #$19
    STA $D6E3

TEMPERATURE_FLOW:
    ; Cauldron temperature decay/fluctuation simulation
    LDA $D6E4
    CLC
    ADC #$01
    STA $D6E4
    
    ; Check if temperature is out of range
    CMP #$C0        ; Temp too high
    BEQ DEDUCT_ENERGY
    JMP CHECK_VICTORY

DEDUCT_ENERGY:
    LDA $D6E6
    SEC
    SBC #$05
    STA $D6E6
    
CHECK_VICTORY:
    ; Check if score >= 500 (0x01F4)
    LDA $D6E5
    CMP #$F4
    BNE CHECK_DEFEAT
    
    ; Win game
    LDA #$01
    STA $D6E9
    JMP LOOP

CHECK_DEFEAT:
    LDA $D6E6
    CMP #$00
    BNE END_TICK
    
    ; Game Over
    LDA #$02
    STA $D6E9

END_TICK:
    JMP LOOP
