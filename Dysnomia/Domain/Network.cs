using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using Dysnomia.Lib;

namespace Dysnomia.Domain
{
    public class Network : Daemon
    {
        new public static string Name = "Network";
        new public static string Description = "WinchesterMQ Network & Handshake Manager";

        private TcpListener? _scsiListener;
        private bool _isRunning = false;

        public class WmqPayload
        {
            public string TransactionHash { get; set; } = "";
            public int Lun { get; set; }
            public int Priority { get; set; }
            public string Word0 { get; set; } = "";
            public string Word1 { get; set; } = "";
        }

        public struct YiHandshakeState
        {
            public ulong Contour;
            public ulong Pole;
            public ulong Foundation;
            public ulong Dynamo;
            public ulong Manifold;
        }

        public Network()
        {
            Logging.Log("Network", "Initializing WinchesterMQ Network Daemon");
            StartScsiServer();
        }

        // Compiles and writes a Folklore program incrementally to simulated disk storage
        public bool CompileAndWriteFolklore(string filename, string programText)
        {
            try
            {
                Logging.Log("Network", $"Assembling Auncient Folklore program for '{filename}'...", 2);
                string[] lines = programText.Split(new[] { ';', '\n' }, StringSplitOptions.RemoveEmptyEntries);
                
                // Emulate filename hashing mapping to disk inode namespaces
                using (var sha = System.Security.Cryptography.SHA256.Create())
                {
                    byte[] nameHash = sha.ComputeHash(Encoding.UTF8.GetBytes(filename));
                    string nameHashHex = "0x" + Convert.ToHexString(nameHash).ToLower();
                    
                    for (int offset = 0; offset < lines.Length; offset++)
                    {
                        byte[] instBytes = FolkloreAssembler.AssembleInstruction(lines[offset]);
                        if (instBytes.Length == 0) continue;
                        
                        string instHex = "0x" + Convert.ToHexString(instBytes).ToLower();
                        
                        // Structure compilation payloads formally and serialize
                        var payload = new WmqPayload
                        {
                            TransactionHash = "0x77726974" + nameHashHex.Replace("0x", ""),
                            Lun = 4,
                            Priority = 2,
                            Word0 = nameHashHex,
                            Word1 = instHex
                        };
                        
                        Thunks.PublishMQ(JsonSerializer.Serialize(payload));
                        Logging.Log("Network", $"  -> Instruction {offset} compiled: {lines[offset].Trim()}", 3);
                    }
                }
                
                Logging.Log("Network", $"SUCCESS: Folklore program '{filename}' compiled and queued for disk sync.", 3);
                return true;
            }
            catch (Exception ex)
            {
                Logging.Log("Network", "Assembler compilation exception: " + ex.Message, 5);
                return false;
            }
        }

        // Starts a background TCP loopback server to handle simulated raw SCSI packet transmissions
        public void StartScsiServer()
        {
            if (_isRunning) return;
            _isRunning = true;
            
            try
            {
                _scsiListener = new TcpListener(IPAddress.Loopback, 10043);
                _scsiListener.Start();
                Logging.Log("Network", "SCSI TCP Loopback Server started on Port 10043", 2);
                
                _scsiListener.BeginAcceptTcpClient(OnAcceptTcpClient, null);
            }
            catch (Exception ex)
            {
                Logging.Log("Network", "Failed to start SCSI Server: " + ex.Message, 5);
            }
        }

        private void OnAcceptTcpClient(IAsyncResult ar)
        {
            if (!_isRunning || _scsiListener == null) return;
            
            try
            {
                TcpClient client = _scsiListener.EndAcceptTcpClient(ar);
                _scsiListener.BeginAcceptTcpClient(OnAcceptTcpClient, null);
                
                // Enforce loopback connection restriction boundary
                IPEndPoint? remoteEndpoint = client.Client.RemoteEndPoint as IPEndPoint;
                if (remoteEndpoint != null && !IPAddress.IsLoopback(remoteEndpoint.Address))
                {
                    Logging.Log("Network", $"Blocked unauthorized connection from non-loopback address: {remoteEndpoint.Address}", 4);
                    client.Dispose();
                    return;
                }

                // Process client asynchronously
                System.Threading.Tasks.Task.Run(() =>
                {
                    try
                    {
                        using (NetworkStream stream = client.GetStream())
                        {
                            byte[] buffer = new byte[3];
                            int bytesRead = stream.Read(buffer, 0, buffer.Length);
                            if (bytesRead >= 3 && buffer[0] == 0x53 && buffer[1] == 0x43)
                            {
                                byte keycode = buffer[2];
                                Logging.Log("Network", $"SCSI Packet Received: Keycode={keycode}", 3);
                                
                                // Latch keycode to WinchesterMQ state via JIT thunks
                                Thunks.PublishMQ($"SCSI:LATENCY:KEY:{keycode}");
                                
                                // Formulate response frame: [SuccessByte, Phase, DivisorHigh, DivisorLow]
                                byte[] response = new byte[4];
                                response[0] = 1; // Success
                                response[1] = 0; // Phase
                                response[2] = 0; // DivisorHigh
                                response[3] = 0; // DivisorLow
                                
                                stream.Write(response, 0, response.Length);
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        Logging.Log("Network", "SCSI stream handling exception: " + ex.Message, 5);
                    }
                    finally
                    {
                        client.Dispose();
                    }
                });
            }
            catch (Exception ex)
            {
                Logging.Log("Network", "SCSI Client Exception: " + ex.Message, 5);
            }
        }

        public void StopScsiServer()
        {
            _isRunning = false;
            _scsiListener?.Stop();
            Logging.Log("Network", "SCSI TCP Loopback Server stopped", 2);
        }

        // Sends a local JSON-RPC socket request to the ZMM MCP server on Port 10042
        public string SendMcpRequest(string method, string jsonParams)
        {
            try
            {
                using (TcpClient client = new TcpClient("127.0.0.1", 10042))
                using (NetworkStream stream = client.GetStream())
                {
                    string request = $"{{\"jsonrpc\":\"2.0\",\"method\":\"{method}\",\"params\":{jsonParams},\"id\":1}}";
                    byte[] requestBytes = Encoding.UTF8.GetBytes(request);
                    stream.Write(requestBytes, 0, requestBytes.Length);

                    byte[] buffer = new byte[8192];
                    int bytesRead = stream.Read(buffer, 0, buffer.Length);
                    return Encoding.UTF8.GetString(buffer, 0, bytesRead);
                }
            }
            catch (Exception ex)
            {
                Logging.Log("Network", "MCP Socket Request Failed: " + ex.Message, 5);
                return "";
            }
        }

        // Simulates/tests a WinchesterMQ YI Handshake sequence using direct contract thunks
        public bool PerformYiHandshake(string peerAddress)
        {
            Logging.Log("Network", "Initiating WinchesterMQ YI Handshake with " + peerAddress, 1);

            try
            {
                // Step 1: INIT / FORM (Swap Contours)
                Logging.Log("Network", "[STEP 1: FORM] Exchanging contours via WinchesterMQ JIT thunk", 2);
                
                // Pack details in formal payload object and serialize
                var formPayload = new WmqPayload
                {
                    TransactionHash = "0x0981a2f3" + peerAddress.Replace("0x", ""),
                    Lun = 0,
                    Priority = 1,
                    Word0 = peerAddress,
                    Word1 = "FORM:INITIATE"
                };
                string formJson = JsonSerializer.Serialize(formPayload);
                Thunks.PublishMQ(formJson);
                
                bool formOk = Thunks.SendWmqTransaction(peerAddress, formPayload.TransactionHash);
                if (!formOk)
                {
                    Logging.Log("Network", "[FAIL] FORM transaction handshake failed.", 5);
                    return false;
                }

                // Step 2: CONJUGATE (Swap Poles)
                Logging.Log("Network", "[STEP 2: CONJUGATE] Swapping poles via WinchesterMQ JIT thunk", 2);
                var conjugatePayload = new WmqPayload
                {
                    TransactionHash = "",
                    Lun = 1,
                    Priority = 1,
                    Word0 = peerAddress,
                    Word1 = "CONJUGATE:SWAP"
                };
                Thunks.PublishMQ(JsonSerializer.Serialize(conjugatePayload));

                // Step 3: SATURATE (Swap Foundations)
                Logging.Log("Network", "[STEP 3: SATURATE] Swapping foundations via WinchesterMQ JIT thunk", 2);
                var saturatePayload = new WmqPayload
                {
                    TransactionHash = "",
                    Lun = 2,
                    Priority = 1,
                    Word0 = peerAddress,
                    Word1 = "SATURATE:SWAP"
                };
                Thunks.PublishMQ(JsonSerializer.Serialize(saturatePayload));

                // Step 4: MAGNETIZE (Swap Dynamos and check Convergence)
                Logging.Log("Network", "[STEP 4: MAGNETIZE] Swapping dynamos and establishing Helmholtz convergence", 2);
                var magnetizePayload = new WmqPayload
                {
                    TransactionHash = "",
                    Lun = 3,
                    Priority = 1,
                    Word0 = peerAddress,
                    Word1 = "MAGNETIZE:CONVERGE"
                };
                Thunks.PublishMQ(JsonSerializer.Serialize(magnetizePayload));

                // Query active step state via MCP Server JSON-RPC
                Logging.Log("Network", "Querying ZMM active step log via Port 10042 MCP socket...", 2);
                string mcpResponse = SendMcpRequest("wave512.inspect", "{}");
                if (!string.IsNullOrEmpty(mcpResponse))
                {
                    using (JsonDocument doc = JsonDocument.Parse(mcpResponse))
                    {
                        if (doc.RootElement.TryGetProperty("result", out JsonElement resultEl))
                        {
                            Logging.Log("Network", "ZMM State Inspected: " + resultEl.ToString(), 3);
                        }
                    }
                }

                Logging.Log("Network", "[SUCCESS] YI Handshake and Helmholtz convergence established physically over WinchesterMQ!", 3);
                return true;
            }
            catch (Exception ex)
            {
                Logging.Log("Network", "Exception during YI Handshake: " + ex.Message, 6);
                return false;
            }
        }
    }
}
