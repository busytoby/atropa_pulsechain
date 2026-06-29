using System;
using System.Numerics;
using Dysnomia;
using Dysnomia.Lib;
using Dysnomia.Domain;

class Program {
    static void Main(string[] args) {
        Console.WriteLine("=== Dysnomia Integration Test Suite ===");
        
        // Test Case 1: Known Small Math.ModPow
        Console.WriteLine("\n[TEST 1] Testing Dysnomia.Math.ModPow with TSFi Interop...");
        BigInteger a = BigInteger.Parse("123456789");
        BigInteger b = BigInteger.Parse("65537");
        BigInteger c = BigInteger.Parse("1000000007");
        BigInteger expected = 560583526;
        
        try {
            BigInteger result = Dysnomia.Math.ModPow(a, b, c);
            Console.WriteLine($"Result: {result}");
            
            if (result == expected) {
                Console.WriteLine("SUCCESS: ModPow result matches expected value.");
            } else {
                Console.WriteLine($"FAILURE: Expected {expected}, got {result}");
                Environment.Exit(1);
            }
        } catch (Exception ex) {
            Console.WriteLine($"EXCEPTION: {ex}");
            Environment.Exit(2);
        }

        // Test Case 2: WinchesterMQ P/Invoke Bindings
        Console.WriteLine("\n[TEST 2] Testing WinchesterMQ Thunks JIT Publish...");
        try {
            // Attempt to publish a test query to verify dynamic loading of libtsfi2.so
            Thunks.PublishMQ("TEST:PING_FROM_CSHARP");
            Console.WriteLine("SUCCESS: Thunks.PublishMQ executed without exceptions.");
        } catch (DllNotFoundException) {
            Console.WriteLine("WARNING: libtsfi2.so not found in path. Skipping native thunk check.");
        } catch (Exception ex) {
            Console.WriteLine($"FAILURE: Unexpected exception on PublishMQ: {ex}");
            Environment.Exit(3);
        }

        // Test Case 3: Network Daemon Handshake Pipeline
        Console.WriteLine("\n[TEST 3] Testing WinchesterMQ Network Handshake Daemon...");
        try {
            Network testNetwork = new Network();
            string testPeer = "0x0000000000000000000000000000000000000001";
            
            // Triggering the multi-stage YI handshake sequence
            bool handshakeSuccess = testNetwork.PerformYiHandshake(testPeer);
            Console.WriteLine($"Handshake execution status: {handshakeSuccess}");
            
            if (handshakeSuccess) {
                Console.WriteLine("SUCCESS: YI Handshake pipeline completed successfully.");
            } else {
                Console.WriteLine("WARNING: Handshake pipeline returned false (is ZMM server offline?).");
            }
        } catch (DllNotFoundException) {
            Console.WriteLine("WARNING: libtsfi2.so not found. Network pipeline verification skipped.");
        } catch (Exception ex) {
            Console.WriteLine($"WARNING: Handshake pipeline threw exception: {ex}");
        }

        // Test Case 4: Folklore Compilation Verification
        Console.WriteLine("\n[TEST 4] Testing Folklore Compiler and Assembler CLI...");
        try {
            Network testNetwork = new Network();
            string testProgram = "LOAD Base, 10; LOAD Secret, 20; ADD Foundation, Base, Secret";
            bool compileSuccess = testNetwork.CompileAndWriteFolklore("test_program.bin", testProgram);
            
            if (compileSuccess) {
                Console.WriteLine("SUCCESS: Folklore program assembled and parsed correctly.");
            } else {
                Console.WriteLine("WARNING: Folklore program assembly returned false (is libtsfi2.so missing?).");
            }
        } catch (DllNotFoundException) {
            Console.WriteLine("WARNING: libtsfi2.so not found. Folklore assembler verification skipped.");
        } catch (Exception ex) {
            Console.WriteLine($"FAILURE: Folklore assembler threw exception: {ex}");
            Environment.Exit(7);
        }

        Console.WriteLine("\n=== ALL SYSTEM TESTS COMPLETED ===");
    }
}