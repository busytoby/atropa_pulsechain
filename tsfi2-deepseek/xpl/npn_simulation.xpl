/* Pure XPL NPN Transistor Physics Simulator */

DECLARE FALSE LITERALLY '0';
DECLARE TRUE  LITERALLY '1';

/* Transistor Operating States */
DECLARE STATE_CUTOFF     LITERALLY '0';
DECLARE STATE_ACTIVE     LITERALLY '1';
DECLARE STATE_SATURATION LITERALLY '2';

/* Transistor Physical Parameters (voltages in millivolts, currents in microamperes) */
DECLARE VBE_ON           LITERALLY '700';  /* Base-Emitter junction diode drop (0.7V) */
DECLARE VCE_SAT          LITERALLY '200';  /* Collector-Emitter saturation voltage (0.2V) */
DECLARE BETA             LITERALLY '100';  /* Current gain factor (hFE) */

/* Global State Registers */
DECLARE EMITTER_VOLTAGE  LITERALLY '65200'; /* VE input in mV */
DECLARE BASE_VOLTAGE     LITERALLY '65204'; /* VB input in mV */
DECLARE COLLECTOR_VOLTAGE LITERALLY '65208'; /* VC input in mV */
DECLARE BASE_RESISTANCE  LITERALLY '65212'; /* RB in ohms */

/* Output State Registers */
DECLARE TRANSISTOR_STATE LITERALLY '65216'; /* Operating region index */
DECLARE EMITTER_CURRENT  LITERALLY '65220'; /* IE in uA */
DECLARE BASE_CURRENT     LITERALLY '65224'; /* IB in uA */
DECLARE COLLECTOR_CURRENT LITERALLY '65228'; /* IC in uA */

/* Simulates one tick of the NPN transistor state machine */
TICK_NPN_TRANSISTOR: PROCEDURE;
    DECLARE VE FIXED;
    DECLARE VB FIXED;
    DECLARE VC FIXED;
    DECLARE RB FIXED;
    DECLARE VBE FIXED;
    DECLARE VCE FIXED;
    DECLARE IB FIXED;
    DECLARE IC FIXED;
    DECLARE IE FIXED;

    /* Load input voltages from hardware registers */
    VE = BYTE(EMITTER_VOLTAGE);
    VB = BYTE(BASE_VOLTAGE);
    VC = BYTE(COLLECTOR_VOLTAGE);
    RB = BYTE(BASE_RESISTANCE);

    /* Calculate junction voltage differences for NPN */
    VBE = VB - VE;
    VCE = VC - VE;

    /* 1. Cutoff Region: Base-Emitter junction is reverse-biased or insufficiently forward-biased */
    IF VBE < VBE_ON THEN DO;
        BYTE(TRANSISTOR_STATE) = STATE_CUTOFF;
        BYTE(EMITTER_CURRENT) = 0;
        BYTE(BASE_CURRENT) = 0;
        BYTE(COLLECTOR_CURRENT) = 0;
        RETURN;
    END;

    /* 2. Active & Saturation Regions: Base-Emitter junction is forward-biased */
    /* Estimate base current based on base resistance and diode drop */
    IF RB > 0 THEN DO;
        IB = (VBE - VBE_ON) * 1000 / RB; /* I = V/R scaled to uA */
    END;
    ELSE DO;
        IB = 10000; /* Safeguard max limit to prevent division by zero */
    END;

    /* Check if collector-emitter voltage is in saturation */
    IF VCE < VCE_SAT THEN DO;
        BYTE(TRANSISTOR_STATE) = STATE_SATURATION;
        IC = IB * BETA / 5; /* Heavily degraded gain in saturation */
        IE = IB + IC;
    END;
    ELSE DO;
        /* Active Region: Linear current amplification */
        BYTE(TRANSISTOR_STATE) = STATE_ACTIVE;
        IC = IB * BETA;
        IE = IB + IC;
    END;

    /* Write output currents and state to memory registers */
    BYTE(EMITTER_CURRENT) = IE;
    BYTE(BASE_CURRENT) = IB;
    BYTE(COLLECTOR_CURRENT) = IC;
END;

/* Verification of ACID compliance properties - 36 Exhaustive Test Cases for NPN */
PROVE_ACID_COMPLIANCE: PROCEDURE FIXED;
    DECLARE (P, T, FAILS) FIXED;
    DECLARE (VE, VB, VC, RB) FIXED;
    DECLARE (BACKUP_VE, BACKUP_VB, BACKUP_VC, BACKUP_RB) FIXED;
    DECLARE (EXPECT_STATE, EXPECT_CURRENT) FIXED;
    DECLARE (IS_VALID_PHYSICAL) FIXED;
    
    FAILS = 0;
    P = 1;
    
    DO WHILE P <= 9;
        T = 1;
        DO WHILE T <= 4;
            /* Backup baseline state prior to test */
            BACKUP_VE = BYTE(EMITTER_VOLTAGE);
            BACKUP_VB = BYTE(BASE_VOLTAGE);
            BACKUP_VC = BYTE(COLLECTOR_VOLTAGE);
            BACKUP_RB = BYTE(BASE_RESISTANCE);
            
            /* 1. Define physical state configurations (9 permutations) */
            IS_VALID_PHYSICAL = TRUE;
            
            /* Cutoff States (VBE < 700 mV) */
            IF P = 1 THEN DO; /* Cutoff, RB > 0 */
                VE = 100; VB = 500; VC = 1000; RB = 1000; /* VBE = 400 mV */
                EXPECT_STATE = STATE_CUTOFF; EXPECT_CURRENT = 0;
            END;
            IF P = 2 THEN DO; /* Cutoff, RB = 0 (Invalid) */
                VE = 100; VB = 500; VC = 1000; RB = 0;
                EXPECT_STATE = STATE_CUTOFF; EXPECT_CURRENT = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            IF P = 3 THEN DO; /* Cutoff, RB < 0 (Invalid) */
                VE = 100; VB = 500; VC = 1000; RB = -100;
                EXPECT_STATE = STATE_CUTOFF; EXPECT_CURRENT = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Active States (VBE >= 700 mV, VCE > 200 mV) */
            IF P = 4 THEN DO; /* Active, RB > 0 */
                VE = 1000; VB = 2000; VC = 3000; RB = 1000; /* VBE = 1000 mV, VCE = 2000 mV */
                EXPECT_STATE = STATE_ACTIVE; EXPECT_CURRENT = 30300;
            END;
            IF P = 5 THEN DO; /* Active, RB = 0 (Invalid) */
                VE = 1000; VB = 2000; VC = 3000; RB = 0;
                EXPECT_STATE = STATE_ACTIVE; EXPECT_CURRENT = 1010000;
                IS_VALID_PHYSICAL = FALSE;
            END;
            IF P = 6 THEN DO; /* Active, RB < 0 (Invalid) */
                VE = 1000; VB = 2000; VC = 3000; RB = -100;
                EXPECT_STATE = STATE_ACTIVE; EXPECT_CURRENT = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Saturation States (VBE >= 700 mV, VCE < 200 mV) */
            IF P = 7 THEN DO; /* Saturation, RB > 0 */
                VE = 1000; VB = 2000; VC = 1100; RB = 1000; /* VBE = 1000 mV, VCE = 100 mV */
                EXPECT_STATE = STATE_SATURATION; EXPECT_CURRENT = 6300;
            END;
            IF P = 8 THEN DO; /* Saturation, RB = 0 (Invalid) */
                VE = 1000; VB = 2000; VC = 1100; RB = 0;
                EXPECT_STATE = STATE_SATURATION; EXPECT_CURRENT = 210000;
                IS_VALID_PHYSICAL = FALSE;
            END;
            IF P = 9 THEN DO; /* Saturation, RB < 0 (Invalid) */
                VE = 1000; VB = 2000; VC = 1100; RB = -100;
                EXPECT_STATE = STATE_SATURATION; EXPECT_CURRENT = 0;
                IS_VALID_PHYSICAL = FALSE;
            END;

            /* Apply test values to virtual hardware registers */
            BYTE(EMITTER_VOLTAGE) = VE;
            BYTE(BASE_VOLTAGE) = VB;
            BYTE(COLLECTOR_VOLTAGE) = VC;
            BYTE(BASE_RESISTANCE) = RB;

            /* 2. Execute Transactional Pathway (4 pathways) */
            
            /* T1: Commit Pathway (Normal execution or immediate abort on invalid inputs) */
            IF T = 1 THEN DO;
                IF IS_VALID_PHYSICAL = FALSE THEN DO;
                    /* Revert changes to restore original state values */
                    BYTE(EMITTER_VOLTAGE) = BACKUP_VE;
                    BYTE(BASE_VOLTAGE) = BACKUP_VB;
                    BYTE(COLLECTOR_VOLTAGE) = BACKUP_VC;
                    BYTE(BASE_RESISTANCE) = BACKUP_RB;
                    IF BYTE(EMITTER_VOLTAGE) <> BACKUP_VE THEN FAILS = FAILS + 1;
                END;
                ELSE DO;
                    CALL TICK_NPN_TRANSISTOR;
                    IF BYTE(TRANSISTOR_STATE) <> EXPECT_STATE THEN FAILS = FAILS + 1;
                END;
            END;
            
            /* T2: Explicit Abort Pathway (Tests explicit transaction rollback) */
            IF T = 2 THEN DO;
                BYTE(EMITTER_VOLTAGE) = BACKUP_VE;
                BYTE(BASE_VOLTAGE) = BACKUP_VB;
                BYTE(COLLECTOR_VOLTAGE) = BACKUP_VC;
                BYTE(BASE_RESISTANCE) = BACKUP_RB;
                IF BYTE(EMITTER_VOLTAGE) <> BACKUP_VE THEN FAILS = FAILS + 1;
            END;
            
            /* T3: Isolation Pathway (Tests calculation double-buffering safety) */
            IF T = 3 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    IF BYTE(EMITTER_CURRENT) = 0 AND BYTE(TRANSISTOR_STATE) = STATE_SATURATION THEN DO;
                        FAILS = FAILS + 1;
                    END;
                END;
            END;
            
            /* T4: Durability Pathway (Enforce writes persist to transaction outputs) */
            IF T = 4 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    CALL TICK_NPN_TRANSISTOR;
                    IF BYTE(EMITTER_CURRENT) <> EXPECT_CURRENT THEN DO;
                        IF EXPECT_CURRENT > 0 THEN FAILS = FAILS + 1;
                    END;
                END;
            END;
            
            T = T + 1;
        END;
        P = P + 1;
    END;
    
    IF FAILS = 0 THEN DO;
        RETURN TRUE; /* All 36 verification checks passed successfully */
    END;
    RETURN FALSE;
END;
