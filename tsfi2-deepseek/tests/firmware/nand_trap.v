// tests/firmware/nand_trap.v
`timescale 1ns / 1ps

module nand_trap (
    input wire set_n,
    input wire reset_n,
    output wire q,
    output wire q_bar
);
    // The Physical Trap: Cross-coupled NAND gates.
    // If set_n = 0 and reset_n = 0 simultaneously, both q and q_bar are forced to 1.
    // When the inputs are released back to 1, 1, the circuit enters a metastable
    // race condition (oscillation), structurally trapping the logic synthesizer.
    
    assign q = ~(set_n & q_bar);
    assign q_bar = ~(reset_n & q);

endmodule

// Testbench to physically spring the trap
module tb_nand_trap;
    reg set_n;
    reg reset_n;
    wire q;
    wire q_bar;

    nand_trap uut (
        .set_n(set_n),
        .reset_n(reset_n),
        .q(q),
        .q_bar(q_bar)
    );

    initial begin
        $dumpfile("nand_trap.vcd");
        $dumpvars(0, tb_nand_trap);

        // Safe State
        set_n = 1; reset_n = 1;
        #10;
        
        // Set Q to 1
        set_n = 0; reset_n = 1;
        #10;

        // Reset Q to 0
        set_n = 1; reset_n = 0;
        #10;

        // Spring the Trap (The Forbidden State)
        $display("[TB] Injecting Forbidden State (0, 0)...");
        set_n = 0; reset_n = 0;
        #10;
        
        // Release the Trap (Race Condition / Metastability)
        $display("[TB] Releasing inputs. Entering metastable logic fracture...");
        set_n = 1; reset_n = 1;
        #10;

        $finish;
    end
endmodule
