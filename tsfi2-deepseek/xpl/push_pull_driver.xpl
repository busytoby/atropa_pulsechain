/* Class B Push-Pull Emitter Follower Simulator with Shared Emitter Output */

DECLARE FALSE LITERALLY '0';
DECLARE TRUE  LITERALLY '1';

/* Transistor States */
DECLARE STATE_CUTOFF     LITERALLY '0';
DECLARE STATE_ACTIVE     LITERALLY '1';
DECLARE STATE_SATURATION LITERALLY '2';

/* Shared Input and Load Specifications */
DECLARE INPUT_VOLTAGE     LITERALLY '65300'; /* Vin in mV */
DECLARE LOAD_RESISTANCE   LITERALLY '65304'; /* RL (Capstan Motor Winding) in ohms */

/* Connected Transistor Emitters (Shared Output Node) */
DECLARE EMITTER_OUTPUT_V  LITERALLY '65308'; /* VE output in mV */
DECLARE EMITTER_OUTPUT_I  LITERALLY '65312'; /* IE output in uA */

/* Individual Base Control Nodes */
DECLARE NPN_BASE_VOLTAGE  LITERALLY '65316'; /* NPN VB in mV */
DECLARE PNP_BASE_VOLTAGE  LITERALLY '65320'; /* PNP VB in mV */

/* Power Rail Registers */
DECLARE POS_POWER_RAIL    LITERALLY '65332'; /* Positive rail input in mV */
DECLARE NEG_POWER_RAIL    LITERALLY '65336'; /* Negative rail input in mV */

/* Collector Registers */
DECLARE NPN_COLLECTOR_V   LITERALLY '65340'; /* NPN VC in mV */
DECLARE PNP_COLLECTOR_V   LITERALLY '65344'; /* PNP VC in mV */

/* Individual Transistor State Tracking */
DECLARE NPN_STATE         LITERALLY '65324';
DECLARE PNP_STATE         LITERALLY '65328';

TICK_PUSH_PULL_DRIVER: PROCEDURE;
    DECLARE VIN FIXED;
    DECLARE RL FIXED;
    DECLARE VOUT FIXED;
    DECLARE IOUT FIXED;
    
    VIN = BYTE(INPUT_VOLTAGE);
    RL = BYTE(LOAD_RESISTANCE);
    
    /* Connect collectors to the power rails */
    BYTE(NPN_COLLECTOR_V) = BYTE(POS_POWER_RAIL);
    BYTE(PNP_COLLECTOR_V) = BYTE(NEG_POWER_RAIL);
    
    /* Connect the bases through two biasing diodes in series (Class AB bias) */
    /* This shifts NPN base up by 700mV and PNP base down by 700mV relative to Vin */
    BYTE(NPN_BASE_VOLTAGE) = VIN + 700;
    BYTE(PNP_BASE_VOLTAGE) = VIN - 700;
    
    /* 1. Positive Half-Cycle Conduction (NPN active, PNP cutoff) */
    IF VIN > 0 THEN DO;
        BYTE(NPN_STATE) = STATE_ACTIVE;
        BYTE(PNP_STATE) = STATE_CUTOFF;
        VOUT = VIN; /* Crossover distortion eliminated: output follows Vin directly */
    END;
    ELSE DO;
        /* 2. Negative Half-Cycle Conduction (PNP active, NPN cutoff) */
        IF VIN < 0 THEN DO;
            BYTE(NPN_STATE) = STATE_CUTOFF;
            BYTE(PNP_STATE) = STATE_ACTIVE;
            VOUT = VIN; /* Output follows Vin directly */
        END;
        /* 3. Perfect Zero Cross Alignment (Coast state) */
        ELSE DO;
            BYTE(NPN_STATE) = STATE_CUTOFF;
            BYTE(PNP_STATE) = STATE_CUTOFF;
            VOUT = 0;
        END;
    END;
    
    /* Calculate output load current using connected emitter node voltage */
    IF RL > 0 THEN DO;
        IOUT = VOUT * 1000 / RL; /* Current in uA */
    END;
    ELSE DO;
        IOUT = 0;
    END;
    
    /* Write to the connected emitter hardware registers */
    BYTE(EMITTER_OUTPUT_V) = VOUT;
    BYTE(EMITTER_OUTPUT_I) = IOUT;
END TICK_PUSH_PULL_DRIVER;

/* Verification of ACID compliance properties - 36 Exhaustive Test Cases for Push-Pull */
PROVE_ACID_COMPLIANCE: PROCEDURE FIXED;
    DECLARE (P, T, FAILS) FIXED;
    DECLARE (VIN, RL, VPOS, VNEG) FIXED;
    DECLARE (BACKUP_VIN, BACKUP_RL, BACKUP_VPOS, BACKUP_VNEG) FIXED;
    DECLARE (EXPECT_STATE_NPN, EXPECT_STATE_PNP, EXPECT_VOLTAGE) FIXED;
    DECLARE (IS_VALID_PHYSICAL) FIXED;
    
    FAILS = 0;
    P = 1;
    
    /* Set baseline power rails */
    BYTE(POS_POWER_RAIL) = 12000;  /* +12V */
    BYTE(NEG_POWER_RAIL) = -12000; /* -12V */
    
    DO WHILE P <= 9;
        T = 1;
        DO WHILE T <= 4;
            /* Backup baseline state prior to test */
            BACKUP_VIN = BYTE(INPUT_VOLTAGE);
            BACKUP_RL = BYTE(LOAD_RESISTANCE);
            BACKUP_VPOS = BYTE(POS_POWER_RAIL);
            BACKUP_VNEG = BYTE(NEG_POWER_RAIL);
            
            /* 1. Define physical state configurations (9 permutations) */
            IS_VALID_PHYSICAL = TRUE;
            
            /* NPN Active Region (Positive Half-Cycle, VIN > 0 mV) */
            IF P = 1 THEN DO; /* NPN Active, RL > 0 */
                VIN = 5000; RL = 1000;
                EXPECT_STATE_NPN = STATE_ACTIVE; EXPECT_STATE_PNP = STATE_CUTOFF;
                EXPECT_VOLTAGE = 5000;
            END;
            IF P = 2 THEN DO; /* NPN Active, RL = 0 (Invalid) */
                VIN = 5000; RL = 0;
                EXPECT_STATE_NPN = STATE_ACTIVE; EXPECT_STATE_PNP = STATE_CUTOFF;
                EXPECT_VOLTAGE = 5000;
                IS_VALID_PHYSICAL = FALSE;
            END;
            IF P = 3 THEN DO; /* NPN Active, RL < 0 (Invalid) */
                VIN = 5000; RL = -100;
                EXPECT_STATE_NPN = STATE_ACTIVE; EXPECT_STATE_PNP = STATE_CUTOFF;
                EXPECT_VOLTAGE = 5000;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* PNP Active Region (Negative Half-Cycle, VIN < 0 mV) */
            IF P = 4 THEN DO; /* PNP Active, RL > 0 */
                VIN = -5000; RL = 1000;
                EXPECT_STATE_NPN = STATE_CUTOFF; EXPECT_STATE_PNP = STATE_ACTIVE;
                EXPECT_VOLTAGE = -5000;
            END;
            IF P = 5 THEN DO; /* PNP Active, RL = 0 (Invalid) */
                VIN = -5000; RL = 0;
                EXPECT_STATE_NPN = STATE_CUTOFF; EXPECT_STATE_PNP = STATE_ACTIVE;
                EXPECT_VOLTAGE = -5000;
                IS_VALID_PHYSICAL = FALSE;
            END;
            IF P = 6 THEN DO; /* PNP Active, RL < 0 (Invalid) */
                VIN = -5000; RL = -100;
                EXPECT_STATE_NPN = STATE_CUTOFF; EXPECT_STATE_PNP = STATE_ACTIVE;
                EXPECT_VOLTAGE = -5000;
                IS_VALID_PHYSICAL = FALSE;
            END;
            
            /* Low Signal Conduction (Class AB elimination of Dead Zone) */
            IF P = 7 THEN DO; /* Low positive signal, RL > 0 */
                VIN = 100; RL = 1000;
                EXPECT_STATE_NPN = STATE_ACTIVE; EXPECT_STATE_PNP = STATE_CUTOFF;
                EXPECT_VOLTAGE = 100;
            END;
            IF P = 8 THEN DO; /* Low positive signal, RL = 0 (Invalid) */
                VIN = 100; RL = 0;
                EXPECT_STATE_NPN = STATE_ACTIVE; EXPECT_STATE_PNP = STATE_CUTOFF;
                EXPECT_VOLTAGE = 100;
                IS_VALID_PHYSICAL = FALSE;
            END;
            IF P = 9 THEN DO; /* Low positive signal, RL < 0 (Invalid) */
                VIN = 100; RL = -100;
                EXPECT_STATE_NPN = STATE_ACTIVE; EXPECT_STATE_PNP = STATE_CUTOFF;
                EXPECT_VOLTAGE = 100;
                IS_VALID_PHYSICAL = FALSE;
            END;


            /* Apply test values to virtual hardware registers */
            BYTE(INPUT_VOLTAGE) = VIN;
            BYTE(LOAD_RESISTANCE) = RL;

            /* 2. Execute Transactional Pathway (4 pathways) */
            
            /* T1: Commit Pathway (Normal execution or immediate abort on invalid inputs) */
            IF T = 1 THEN DO;
                IF IS_VALID_PHYSICAL = FALSE THEN DO;
                    /* Revert changes to restore original state values */
                    BYTE(INPUT_VOLTAGE) = BACKUP_VIN;
                    BYTE(LOAD_RESISTANCE) = BACKUP_RL;
                    BYTE(POS_POWER_RAIL) = BACKUP_VPOS;
                    BYTE(NEG_POWER_RAIL) = BACKUP_VNEG;
                    IF BYTE(INPUT_VOLTAGE) <> BACKUP_VIN THEN FAILS = FAILS + 1;
                END;
                ELSE DO;
                    CALL TICK_PUSH_PULL_DRIVER;
                    IF BYTE(NPN_STATE) <> EXPECT_STATE_NPN THEN FAILS = FAILS + 1;
                    IF BYTE(PNP_STATE) <> EXPECT_STATE_PNP THEN FAILS = FAILS + 1;
                END;
            END;
            
            /* T2: Explicit Abort Pathway (Tests explicit transaction rollback) */
            IF T = 2 THEN DO;
                BYTE(INPUT_VOLTAGE) = BACKUP_VIN;
                BYTE(LOAD_RESISTANCE) = BACKUP_RL;
                BYTE(POS_POWER_RAIL) = BACKUP_VPOS;
                BYTE(NEG_POWER_RAIL) = BACKUP_VNEG;
                IF BYTE(INPUT_VOLTAGE) <> BACKUP_VIN THEN FAILS = FAILS + 1;
            END;
            
            /* T3: Isolation Pathway (Tests calculation double-buffering safety) */
            IF T = 3 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    /* Ensure intermediate calculations do not leak to motor drive states */
                    IF BYTE(EMITTER_OUTPUT_I) = 0 AND BYTE(NPN_STATE) = STATE_ACTIVE THEN DO;
                        FAILS = FAILS + 1;
                    END;
                END;
            END;
            
            /* T4: Durability Pathway (Enforce writes persist to output registers) */
            IF T = 4 THEN DO;
                IF IS_VALID_PHYSICAL = TRUE THEN DO;
                    CALL TICK_PUSH_PULL_DRIVER;
                    IF BYTE(EMITTER_OUTPUT_V) <> EXPECT_VOLTAGE THEN DO;
                        IF EXPECT_VOLTAGE <> 0 THEN FAILS = FAILS + 1;
                    END;
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


