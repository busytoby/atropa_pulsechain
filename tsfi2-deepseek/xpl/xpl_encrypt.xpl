/* Auncient XPL Telemetry Connection Encryption Toggle (xpl_encrypt.xpl) */

/* 1. Global Encryption Parameters */
DECLARE CRYPTO_KEY         LITERALLY '165'; /* XOR key 0xA5 */

/* Encryption Modes */
DECLARE MODE_PLAINTEXT     LITERALLY '0';
DECLARE MODE_ENCRYPTED     LITERALLY '1';

/* 2. In-Place Payload Encryption/Decryption Procedure */
PROCESS_PAYLOAD: PROCEDURE(DATA_ADDR, LENGTH, ENCRYPT_MODE) FIXED;
    DECLARE (DATA_ADDR, LENGTH, ENCRYPT_MODE, I) FIXED;
    
    IF ENCRYPT_MODE == MODE_ENCRYPTED THEN
        /* Simple Auncient XOR cipher transformation */
        I = 0;
        DO WHILE I < LENGTH;
            BYTE(DATA_ADDR + I) = BYTE(DATA_ADDR + I) XOR CRYPTO_KEY;
            I = I + 1;
        END;
    END;
    
    RETURN ENCRYPT_MODE;
END PROCESS_PAYLOAD;
