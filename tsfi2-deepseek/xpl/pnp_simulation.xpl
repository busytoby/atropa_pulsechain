/* Pure XPL PNP Transistor Physics Simulator */

DECLARE FALSE LITERALLY '0';
DECLARE TRUE  LITERALLY '1';

/* Transistor Operating States */
DECLARE STATE_CUTOFF     LITERALLY '0';
DECLARE STATE_ACTIVE     LITERALLY '1';
DECLARE STATE_SATURATION LITERALLY '2';

/* Transistor Physical Parameters (voltages in millivolts, currents in microamperes) */
DECLARE VBE_ON           LITERALLY '700';  /* Emitter-Base junction diode drop (0.7V) */
DECLARE VEC_SAT          LITERALLY '200';  /* Emitter-Collector saturation voltage (0.2V) */
DECLARE BETA             LITERALLY '100';  /* Current gain factor (hFE) */

/* Global State Registers */
DECLARE EMITTER_VOLTAGE  LITERALLY '65100'; /* VE input in mV */
DECLARE BASE_VOLTAGE     LITERALLY '65104'; /* VB input in mV */
DECLARE COLLECTOR_VOLTAGE LITERALLY '65108'; /* VC input in mV */
DECLARE BASE_RESISTANCE  LITERALLY '65112'; /* RB in ohms */

/* Output State Registers */
DECLARE TRANSISTOR_STATE LITERALLY '65116'; /* Operating region index */
DECLARE EMITTER_CURRENT  LITERALLY '65120'; /* IE in uA */
DECLARE BASE_CURRENT     LITERALLY '65124'; /* IB in uA */
DECLARE COLLECTOR_CURRENT LITERALLY '65128'; /* IC in uA */

/* Simulates one tick of the PNP transistor state machine */
TICK_PNP_TRANSISTOR: PROCEDURE;
    DECLARE VE FIXED;
    DECLARE VB FIXED;
    DECLARE VC FIXED;
    DECLARE RB FIXED;
    DECLARE VEB FIXED;
    DECLARE VEC FIXED;
    DECLARE IB FIXED;
    DECLARE IC FIXED;
    DECLARE IE FIXED;

    /* Load input voltages from hardware registers */
    VE = BYTE(EMITTER_VOLTAGE);
    VB = BYTE(BASE_VOLTAGE);
    VC = BYTE(COLLECTOR_VOLTAGE);
    RB = BYTE(BASE_RESISTANCE);

    /* Calculate junction voltage differences */
    VEB = VE - VB;
    VEC = VE - VC;

    /* 1. Cutoff Region: Emitter-Base junction is reverse-biased or insufficiently forward-biased */
    IF VEB < VBE_ON THEN DO;
        BYTE(TRANSISTOR_STATE) = STATE_CUTOFF;
        BYTE(EMITTER_CURRENT) = 0;
        BYTE(BASE_CURRENT) = 0;
        BYTE(COLLECTOR_CURRENT) = 0;
        RETURN;
    END;

    /* 2. Active & Saturation Regions: Emitter-Base junction is forward-biased */
    /* Estimate base current based on base resistance and diode drop */
    IF RB > 0 THEN DO;
        IB = (VEB - VBE_ON) * 1000 / RB; /* I = V/R scaled to uA */
    END;
    ELSE DO;
        IB = 10000; /* Safeguard max limit to prevent division by zero */
    END;

    /* Check if collector-base junction is forward-biased (Saturation) */
    IF VEC < VEC_SAT THEN DO;
        BYTE(TRANSISTOR_STATE) = STATE_SATURATION;
        /* Saturation current is limited by collector load resistance (simplified) */
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
