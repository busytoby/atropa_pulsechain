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
    
    /* Connect the bases to the shared input voltage node */
    BYTE(NPN_BASE_VOLTAGE) = VIN;
    BYTE(PNP_BASE_VOLTAGE) = VIN;
    
    /* 1. NPN Emitter Follower Conduction (Positive Half-Cycle) */
    IF VIN > 700 THEN DO;
        BYTE(NPN_STATE) = STATE_ACTIVE;
        BYTE(PNP_STATE) = STATE_CUTOFF;
        VOUT = VIN - 700; /* Vout follows Vin minus diode drop */
    END;
    /* 2. PNP Emitter Follower Conduction (Negative Half-Cycle) */
    ELSE IF VIN < -700 THEN DO;
        BYTE(NPN_STATE) = STATE_CUTOFF;
        BYTE(PNP_STATE) = STATE_ACTIVE;
        VOUT = VIN + 700; /* Vout follows Vin plus diode drop */
    END;
    /* 3. Dead Zone (Both transistors cut off) */
    ELSE DO;
        BYTE(NPN_STATE) = STATE_CUTOFF;
        BYTE(PNP_STATE) = STATE_CUTOFF;
        VOUT = 0;
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
END;
