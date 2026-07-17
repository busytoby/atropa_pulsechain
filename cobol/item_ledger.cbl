       IDENTIFICATION DIVISION.
       PROGRAM-ID. ITEM-LEDGER.
       AUTHOR. ANTIGRAVITY.

       ENVIRONMENT DIVISION.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
           SELECT LEDGER-FILE ASSIGN TO "ledger.dat"
               ORGANIZATION IS SEQUENTIAL.

       DATA DIVISION.
       FILE SECTION.
       FD  LEDGER-FILE.
       01  LEDGER-RECORD.
           05  ITEM-ID          PIC 9(6).
           05  ITEM-NAME        PIC X(20).
           05  ITEM-BALANCE     PIC S9(7)V99 COMP-3.

       WORKING-STORAGE SECTION.
       01  WS-EOF-FLAG          PIC X VALUE 'N'.
       01  WS-TEMP-RECORD.
           05  WS-ITEM-ID       PIC 9(6).
           05  WS-ITEM-NAME     PIC X(20).
           05  WS-ITEM-BALANCE  PIC S9(7)V99 COMP-3.

       PROCEDURE DIVISION.
       MAIN-LOGIC.
           DISPLAY "INITIALIZING COBOL LEDGER STRATEGY...".
           OPEN OUTPUT LEDGER-FILE.
           
           * CREATE AND STORE ITEM 1
           MOVE 100101 TO ITEM-ID.
           MOVE "FED-RESERVE-A" TO ITEM-NAME.
           MOVE 12500.50 TO ITEM-BALANCE.
           WRITE LEDGER-RECORD.
           
           * CREATE AND STORE ITEM 2
           MOVE 100102 TO ITEM-ID.
           MOVE "FED-RESERVE-B" TO ITEM-NAME.
           MOVE -3400.25 TO ITEM-BALANCE.
           WRITE LEDGER-RECORD.
           
           CLOSE LEDGER-FILE.
           
           * RETRIEVE AND DISPLAY ITEMS
           DISPLAY "RETRIEVING ITEMS FROM STORE...".
           OPEN INPUT LEDGER-FILE.
           
           PERFORM UNTIL WS-EOF-FLAG = 'Y'
               READ LEDGER-FILE INTO WS-TEMP-RECORD
                   AT END
                       MOVE 'Y' TO WS-EOF-FLAG
                   NOT AT END
                       DISPLAY "RETRIEVED ID: " WS-ITEM-ID
                       DISPLAY "NAME: " WS-ITEM-NAME
                       DISPLAY "BALANCE: " WS-ITEM-BALANCE
               END-READ
           END-PERFORM.
           
           CLOSE LEDGER-FILE.
           GOBACK.
