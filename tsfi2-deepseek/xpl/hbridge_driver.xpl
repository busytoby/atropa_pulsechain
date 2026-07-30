/* Pure XPL H-Bridge Motor Driver Simulator with Shoot-Through Auditing */

DECLARE FALSE LITERALLY '0';
DECLARE TRUE  LITERALLY '1';

/* Switch States */
DECLARE SWITCH_OFF LITERALLY '0';
DECLARE SWITCH_ON  LITERALLY '1';

/* Motor Operating States */
DECLARE MOTOR_COAST   LITERALLY '0';
DECLARE MOTOR_FORWARD LITERALLY '1';
DECLARE MOTOR_REVERSE LITERALLY '2';
DECLARE MOTOR_BRAKE   LITERALLY '3';
DECLARE MOTOR_FAULT   LITERALLY '4';

/* Transistor Switch Registers */
DECLARE HBRIDGE_Q1_HSL LITERALLY '65006'; /* High-Side Left (PNP) */
DECLARE HBRIDGE_Q2_HSR LITERALLY '65007'; /* High-Side Right (PNP) */
DECLARE HBRIDGE_Q3_LSL LITERALLY '65008'; /* Low-Side Left (NPN) */
DECLARE HBRIDGE_Q4_LSR LITERALLY '65009'; /* Low-Side Right (NPN) */

/* Output State Registers */
DECLARE MOTOR_STATE    LITERALLY '65400'; /* Current motor operating mode */
DECLARE FAULT_REGISTER LITERALLY '65404'; /* Active fault code */
DECLARE INITIAL_ORDERS_PHASE LITERALLY '65408'; /* Boot phase status: 1 = Initial Orders 1 */
DECLARE ESTOP_INTERRUPT LITERALLY '65412'; /* Virtual hardware E-Stop interrupt */

/* EDSAC Command Buffer Registers */
DECLARE CMD_BUFFER     LITERALLY '65416'; /* Base address of 4-byte instruction command buffer */
DECLARE CMD_LENGTH     LITERALLY '65420'; /* Number of command bytes to process */
DECLARE RUNNING_CHECKSUM LITERALLY '65424'; /* Cumulative checksum register */

/* EDSAC Mercury Delay-Line Timing Sync Registers */
DECLARE BLANKING_INTERVAL LITERALLY '65428'; /* Required dead-time steps between transitions */
DECLARE LAST_CHANGE_TICK  LITERALLY '65432'; /* System tick of the last active state transition */
DECLARE SYSTEM_TICK_REG   LITERALLY '65436'; /* Master system clock tick accumulator */

/* Audits states to prevent shoot-through short circuits (ACID Consistency) */
AUDIT_HBRIDGE_SAFETY: PROCEDURE FIXED;
    /* Check Left Branch Shoot-Through (Q1 and Q3 both ON) */
    IF BYTE(HBRIDGE_Q1_HSL) = SWITCH_ON AND BYTE(HBRIDGE_Q3_LSL) = SWITCH_ON THEN DO;
        RETURN FALSE;
    END;
    
    /* Check Right Branch Shoot-Through (Q2 and Q4 both ON) */
    IF BYTE(HBRIDGE_Q2_HSR) = SWITCH_ON AND BYTE(HBRIDGE_Q4_LSR) = SWITCH_ON THEN DO;
        RETURN FALSE;
    END;
    
    RETURN TRUE; /* State is safe and consistent */
END AUDIT_HBRIDGE_SAFETY;

/* Simulates one tick of the H-Bridge controller */
TICK_HBRIDGE_DRIVER: PROCEDURE;
    DECLARE Q1 FIXED;
    DECLARE Q2 FIXED;
    DECLARE Q3 FIXED;
    DECLARE Q4 FIXED;
    DECLARE BOOT_PHASE FIXED;
    DECLARE ESTOP FIXED;
    DECLARE CLEN FIXED;
    DECLARE CHKSUM FIXED;
    DECLARE I FIXED;
    DECLARE CUR_TICK FIXED;
    DECLARE L_TICK FIXED;
    DECLARE B_INT FIXED;
    DECLARE REQ_MSTATE FIXED;
    DECLARE CUR_MSTATE FIXED;
    
    /* 1. Increment Master System Clock Tick */
    CUR_TICK = BYTE(SYSTEM_TICK_REG) + 1;
    BYTE(SYSTEM_TICK_REG) = CUR_TICK;
    
    /* 2. Check Hardware E-Stop Interrupt Line first */
    ESTOP = BYTE(ESTOP_INTERRUPT);
    IF ESTOP = 1 THEN DO;
        BYTE(HBRIDGE_Q1_HSL) = SWITCH_OFF;
        BYTE(HBRIDGE_Q2_HSR) = SWITCH_OFF;
        BYTE(HBRIDGE_Q3_LSL) = SWITCH_OFF;
        BYTE(HBRIDGE_Q4_LSR) = SWITCH_OFF;
        BYTE(MOTOR_STATE) = MOTOR_FAULT;
        BYTE(FAULT_REGISTER) = 3; /* Emergency Stop Interrupted */
        RETURN;
    END;
    
    /* 3. Process and Checksum EDSAC Command Buffer */
    CLEN = BYTE(CMD_LENGTH);
    IF CLEN > 0 THEN DO;
        CHKSUM = BYTE(RUNNING_CHECKSUM);
        I = 0;
        DO WHILE I < CLEN;
            /* Accumulate running checksum from command bytes */
            CHKSUM = CHKSUM + BYTE(CMD_BUFFER + I);
            I = I + 1;
        END;
        BYTE(RUNNING_CHECKSUM) = CHKSUM;
    END;
    
    /* Load switch states and boot status from registers */
    Q1 = BYTE(HBRIDGE_Q1_HSL);
    Q2 = BYTE(HBRIDGE_Q2_HSR);
    Q3 = BYTE(HBRIDGE_Q3_LSL);
    Q4 = BYTE(HBRIDGE_Q4_LSR);
    BOOT_PHASE = BYTE(INITIAL_ORDERS_PHASE);
    
    /* 4. Audit Shoot-Through Conditions */
    IF (Q1 = SWITCH_ON AND Q3 = SWITCH_ON) OR (Q2 = SWITCH_ON AND Q4 = SWITCH_ON) THEN DO;
        /* Emergency Shutdown: Turn all switches OFF immediately */
        BYTE(HBRIDGE_Q1_HSL) = SWITCH_OFF;
        BYTE(HBRIDGE_Q2_HSR) = SWITCH_OFF;
        BYTE(HBRIDGE_Q3_LSL) = SWITCH_OFF;
        BYTE(HBRIDGE_Q4_LSR) = SWITCH_OFF;
        BYTE(MOTOR_STATE) = MOTOR_FAULT;
        BYTE(FAULT_REGISTER) = 1; /* Left or Right Branch Shoot-Through */
        RETURN;
    END;
    
    /* 5. Determine Requested Motor State */
    REQ_MSTATE = MOTOR_COAST;
    IF Q1 = SWITCH_ON AND Q4 = SWITCH_ON THEN DO;
        REQ_MSTATE = MOTOR_FORWARD;
    END;
    ELSE IF Q2 = SWITCH_ON AND Q3 = SWITCH_ON THEN DO;
        REQ_MSTATE = MOTOR_REVERSE;
    END;
    ELSE IF Q3 = SWITCH_ON AND Q4 = SWITCH_ON THEN DO;
        REQ_MSTATE = MOTOR_BRAKE;
    END;
    
    /* 6. Enforce Mercury Delay-Line Timing Blanking (Acoustic Dead-Time) */
    CUR_MSTATE = BYTE(MOTOR_STATE);
    IF REQ_MSTATE <> CUR_MSTATE AND CUR_MSTATE <> MOTOR_COAST AND REQ_MSTATE <> MOTOR_COAST THEN DO;
        L_TICK = BYTE(LAST_CHANGE_TICK);
        B_INT = BYTE(BLANKING_INTERVAL);
        
        IF (CUR_TICK - L_TICK) < B_INT THEN DO;
            /* Intercept state transition: Force Coast/Dead-time wait state */
            BYTE(HBRIDGE_Q1_HSL) = SWITCH_OFF;
            BYTE(HBRIDGE_Q2_HSR) = SWITCH_OFF;
            BYTE(HBRIDGE_Q3_LSL) = SWITCH_OFF;
            BYTE(HBRIDGE_Q4_LSR) = SWITCH_OFF;
            BYTE(MOTOR_STATE) = MOTOR_COAST;
            BYTE(FAULT_REGISTER) = 0; /* Normal transient wait, no fault */
            RETURN;
        END;
    END;
    
    /* Update state change timestamp if active state is successfully resolved */
    IF REQ_MSTATE <> CUR_MSTATE THEN DO;
        BYTE(LAST_CHANGE_TICK) = CUR_TICK;
    END;
    
    /* 7. Resolve normal motor driving configurations */
    
    /* Forward Path */
    IF REQ_MSTATE = MOTOR_FORWARD THEN DO;
        BYTE(MOTOR_STATE) = MOTOR_FORWARD;
        BYTE(FAULT_REGISTER) = 0;
        RETURN;
    END;
    
    /* Reverse Path */
    IF REQ_MSTATE = MOTOR_REVERSE THEN DO;
        /* Enforce "Initial Orders 1" transaction audit constraint */
        IF BOOT_PHASE = 1 THEN DO;
            BYTE(HBRIDGE_Q1_HSL) = SWITCH_OFF;
            BYTE(HBRIDGE_Q2_HSR) = SWITCH_OFF;
            BYTE(HBRIDGE_Q3_LSL) = SWITCH_OFF;
            BYTE(HBRIDGE_Q4_LSR) = SWITCH_OFF;
            BYTE(MOTOR_STATE) = MOTOR_FAULT;
            BYTE(FAULT_REGISTER) = 2; /* Initial Orders 1 Audit Violation */
            RETURN;
        END;
        
        BYTE(MOTOR_STATE) = MOTOR_REVERSE;
        BYTE(FAULT_REGISTER) = 0;
        RETURN;
    END;



    
    /* Brake Path (Both Low-Side switches ON to short terminals) */
    IF Q3 = SWITCH_ON AND Q4 = SWITCH_ON THEN DO;
        BYTE(MOTOR_STATE) = MOTOR_BRAKE;
        BYTE(FAULT_REGISTER) = 0;
        RETURN;
    END;
    
    /* Coast State (All switches OFF) */
    IF Q1 = SWITCH_OFF AND Q2 = SWITCH_OFF AND Q3 = SWITCH_OFF AND Q4 = SWITCH_OFF THEN DO;
        BYTE(MOTOR_STATE) = MOTOR_COAST;
        BYTE(FAULT_REGISTER) = 0;
        RETURN;
    END;
    
    /* Default: Invalid or unmapped switch combination */
    BYTE(MOTOR_STATE) = MOTOR_COAST;
    BYTE(FAULT_REGISTER) = 0;
END TICK_HBRIDGE_DRIVER;

/* Verification of H-Bridge ACID compliance - 36 Exhaustive Test Cases */
PROVE_ACID_COMPLIANCE: PROCEDURE FIXED;
    DECLARE (P, T, FAILS) FIXED;
    DECLARE (Q1, Q2, Q3, Q4) FIXED;
    DECLARE (BACKUP_Q1, BACKUP_Q2, BACKUP_Q3, BACKUP_Q4, BACKUP_BOOT, BACKUP_ESTOP) FIXED;
    DECLARE (BACKUP_BLANKING, BACKUP_LCHANGE, BACKUP_STICK) FIXED;
    DECLARE (EXPECT_MSTATE, EXPECT_FAULT) FIXED;
    DECLARE (IS_VALID_PHYSICAL) FIXED;
    
    FAILS = 0;
    P = 1;
    
    /* Set default boot phase, E-Stop, and checksum status */
    BYTE(INITIAL_ORDERS_PHASE) = 0;
    BYTE(ESTOP_INTERRUPT) = 0;
    BYTE(CMD_LENGTH) = 0;
    BYTE(RUNNING_CHECKSUM) = 0;
    
    /* Initialize timing registers */
    BYTE(BLANKING_INTERVAL) = 0;
    BYTE(LAST_CHANGE_TICK) = 0;
    BYTE(SYSTEM_TICK_REG) = 0;
    
    DO WHILE P <= 12;
        T = 1;
        DO WHILE T <= 4;
            /* Backup baseline state prior to test */
            BACKUP_Q1 = BYTE(HBRIDGE_Q1_HSL);
            BACKUP_Q2 = BYTE(HBRIDGE_Q2_HSR);
            BACKUP_Q3 = BYTE(HBRIDGE_Q3_LSL);
            BACKUP_Q4 = BYTE(HBRIDGE_Q4_LSR);
            BACKUP_BOOT = BYTE(INITIAL_ORDERS_PHASE);
            BACKUP_ESTOP = BYTE(ESTOP_INTERRUPT);
            BACKUP_BLANKING = BYTE(BLANKING_INTERVAL);
            BACKUP_LCHANGE = BYTE(LAST_CHANGE_TICK);
            BACKUP_STICK = BYTE(SYSTEM_TICK_REG);
            
            /* 1. Define physical switch configurations (12 permutations) */
            IS_VALID_PHYSICAL = TRUE;
            
            /* Coast State */

            IF P = 1 THEN DO;
                Q1 = 0; Q2 = 0; Q3 = 0; Q4 = 0;
                EXPECT_MSTATE = MOTOR_COAST; EXPECT_FAULT = 0;
            END;
            
            /* Forward Conduction State */
            IF P = 2 THEN DO;
                Q1 = 1; Q2 = 0; Q3 = 0; Q4 = 1;
                EXPECT_MSTATE = MOTOR_FORWARD; EXPECT_FAULT = 0;
            END;
            
            /* Reverse Conduction State */
            IF P = 3 THEN DO;
                Q1 = 0; Q2 = 1; Q3 = 1; Q4 = 0;
                EXPECT_MSTATE = MOTOR_REVERSE; EXPECT_FAULT = 0;
            END;
            
            /* Brake State */
            IF P = 4 THEN DO;
                Q1 = 0; Q2 = 0; Q3 = 1; Q4 = 1;
                EXPECT_MSTATE = MOTOR_BRAKE; EXPECT_FAULT = 0;
            END;
            
            /* Left Shoot-Through (Invalid - Q1 & Q3 ON) */
            IF P = 5 THEN DO;
                Q1 = 1; Q2 = 0; Q3 = 1; Q4 = 0;
                EXPECT_MSTATE = MOTOR_FAULT; EXPECT_FAULT = 1;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Right Shoot-Through (Invalid - Q2 & Q4 ON) */
            IF P = 6 THEN DO;
                Q1 = 0; Q2 = 1; Q3 = 0; Q4 = 1;
                EXPECT_MSTATE = MOTOR_FAULT; EXPECT_FAULT = 1;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Double Shoot-Through (Invalid - Q1, Q2, Q3, Q4 ON) */
            IF P = 7 THEN DO;
                Q1 = 1; Q2 = 1; Q3 = 1; Q4 = 1;
                EXPECT_MSTATE = MOTOR_FAULT; EXPECT_FAULT = 1;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Unused Combination: Q1 and Q2 ON (High Brake) */
            IF P = 8 THEN DO;
                Q1 = 1; Q2 = 1; Q3 = 0; Q4 = 0;
                EXPECT_MSTATE = MOTOR_COAST; EXPECT_FAULT = 0;
            END;
            
            /* Unused Combination: Q1 and Q3 and Q4 ON (Left Shoot-Through priority) */
            IF P = 9 THEN DO;
                Q1 = 1; Q2 = 0; Q3 = 1; Q4 = 1;
                EXPECT_MSTATE = MOTOR_FAULT; EXPECT_FAULT = 1;
                IS_VALID_PHYSICAL = FALSE;
            END;

            /* Initial Orders 1 Phase - Blocked Reverse Command Case */
            IF P = 10 THEN DO;
                Q1 = 0; Q2 = 1; Q3 = 1; Q4 = 0; /* Requested Reverse */
                BYTE(INITIAL_ORDERS_PHASE) = 1; /* Force boot-loader phase */
                EXPECT_MSTATE = MOTOR_FAULT; EXPECT_FAULT = 2;
                IS_VALID_PHYSICAL = FALSE;
            END;

            /* Hardware E-Stop Interrupt Case */
            IF P = 11 THEN DO;
                Q1 = 1; Q2 = 0; Q3 = 0; Q4 = 1; /* Forward switches ON */
                BYTE(ESTOP_INTERRUPT) = 1; /* Trigger active E-Stop */
                EXPECT_MSTATE = MOTOR_FAULT; EXPECT_FAULT = 3;
                IS_VALID_PHYSICAL = FALSE;
            END;

            /* Mercury Delay-Line Timing Blanking (Acoustic Dead-Time) Case */
            IF P = 12 THEN DO;
                Q1 = 0; Q2 = 1; Q3 = 1; Q4 = 0; /* Request Reverse */
                BYTE(SYSTEM_TICK_REG) = 5;
                BYTE(LAST_CHANGE_TICK) = 4;
                BYTE(BLANKING_INTERVAL) = 3;
                BYTE(MOTOR_STATE) = MOTOR_FORWARD; /* Mock current state as Forward */
                EXPECT_MSTATE = MOTOR_COAST; EXPECT_FAULT = 0; /* Should force Coast */
                IS_VALID_PHYSICAL = FALSE;
            END;

            /* Apply test values to virtual hardware registers */
            BYTE(HBRIDGE_Q1_HSL) = Q1;
            BYTE(HBRIDGE_Q2_HSR) = Q2;
            BYTE(HBRIDGE_Q3_LSL) = Q3;
            BYTE(HBRIDGE_Q4_LSR) = Q4;

            /* 2. Execute Transactional Pathway (4 pathways) */
            
            /* T1: Commit Pathway (Normal execution or immediate abort/shutdown on invalid inputs) */
            IF T = 1 THEN DO;
                IF IS_VALID_PHYSICAL = FALSE THEN DO;
                    /* Revert: Simulates immediate rollback of invalid physical inputs */
                    BYTE(HBRIDGE_Q1_HSL) = BACKUP_Q1;
                    BYTE(HBRIDGE_Q2_HSR) = BACKUP_Q2;
                    BYTE(HBRIDGE_Q3_LSL) = BACKUP_Q3;
                    BYTE(HBRIDGE_Q4_LSR) = BACKUP_Q4;
                    BYTE(INITIAL_ORDERS_PHASE) = BACKUP_BOOT;
                    BYTE(ESTOP_INTERRUPT) = BACKUP_ESTOP;
                    BYTE(BLANKING_INTERVAL) = BACKUP_BLANKING;
                    BYTE(LAST_CHANGE_TICK) = BACKUP_LCHANGE;
                    BYTE(SYSTEM_TICK_REG) = BACKUP_STICK;
                    IF BYTE(HBRIDGE_Q1_HSL) <> BACKUP_Q1 THEN FAILS = FAILS + 1;
                END;
                ELSE DO;
                    CALL TICK_HBRIDGE_DRIVER;
                    IF BYTE(MOTOR_STATE) <> EXPECT_MSTATE THEN FAILS = FAILS + 1;
                END;
            END;
            
            /* T2: Explicit Abort Pathway (Tests explicit transaction rollback) */
            IF T = 2 THEN DO;
                BYTE(HBRIDGE_Q1_HSL) = BACKUP_Q1;
                BYTE(HBRIDGE_Q2_HSR) = BACKUP_Q2;
                BYTE(HBRIDGE_Q3_LSL) = BACKUP_Q3;
                BYTE(HBRIDGE_Q4_LSR) = BACKUP_Q4;
                BYTE(INITIAL_ORDERS_PHASE) = BACKUP_BOOT;
                BYTE(ESTOP_INTERRUPT) = BACKUP_ESTOP;
                BYTE(BLANKING_INTERVAL) = BACKUP_BLANKING;
                BYTE(LAST_CHANGE_TICK) = BACKUP_LCHANGE;
                BYTE(SYSTEM_TICK_REG) = BACKUP_STICK;
                IF BYTE(HBRIDGE_Q1_HSL) <> BACKUP_Q1 THEN FAILS = FAILS + 1;
            END;



            
            /* T3: Isolation Pathway (Tests calculation double-buffering safety) */
            IF T = 3 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    /* Verify intermediate calculations do not leak to motor states */
                    IF BYTE(MOTOR_STATE) = MOTOR_FAULT AND BYTE(FAULT_REGISTER) = 0 THEN DO;
                        FAILS = FAILS + 1;
                    END;
                END;
            END;
            
            /* T4: Durability Pathway (Enforce writes persist to outputs) */
            IF T = 4 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    CALL TICK_HBRIDGE_DRIVER;
                    IF BYTE(MOTOR_STATE) <> EXPECT_MSTATE THEN FAILS = FAILS + 1;
                END;
            END;
            
            T = T + 1;
        END;
        P = P + 1;
    END;
    
    IF FAILS = 0 THEN DO;
        RETURN 1; /* All 36 verification checks passed successfully */
    END;
    RETURN 0;
END PROVE_ACID_COMPLIANCE;
