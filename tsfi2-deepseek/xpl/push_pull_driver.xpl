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
    
    /* Connect the bases to the shared input voltage node */
    BYTE(NPN_BASE_VOLTAGE) = VIN;
    BYTE(PNP_BASE_VOLTAGE) = VIN;

    
    /* 1. NPN Emitter Follower Conduction (Positive Half-Cycle) */
    IF VIN > 700 THEN DO;
        BYTE(NPN_STATE) = STATE_ACTIVE;
        BYTE(PNP_STATE) = STATE_CUTOFF;
        VOUT = VIN - 700; /* Vout follows Vin minus diode drop */
    END;
    ELSE DO;
        /* 2. PNP Emitter Follower Conduction (Negative Half-Cycle) */
        IF VIN < -700 THEN DO;
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

