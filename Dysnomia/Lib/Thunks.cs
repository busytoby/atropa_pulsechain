using System;
using System.Runtime.InteropServices;

namespace Dysnomia.Lib
{
    public static class Thunks
    {
        // WinchesterMQ Native JIT Thunk Publish Command
        [DllImport("libtsfi2.so", EntryPoint = "tsfi_thunk_publish_mq", CallingConvention = CallingConvention.Cdecl)]
        public static extern void PublishMQ([MarshalAs(UnmanagedType.LPStr)] string cmd);

        // WinchesterMQ Native JIT Direct Transaction Submission
        [DllImport("libtsfi2.so", EntryPoint = "tsfi_pulse_rpc_send_wmq_transaction", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool SendWmqTransaction(
            [MarshalAs(UnmanagedType.LPStr)] string toAddress, 
            [MarshalAs(UnmanagedType.LPStr)] string dataHex
        );
    }
}
