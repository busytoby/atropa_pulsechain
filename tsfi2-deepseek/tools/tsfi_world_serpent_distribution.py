import os
import time
import socket
import struct
import json
import ctypes
import numpy as np
import subprocess
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn
from PIL import Image

# --- Configuration ---
COMPOSITOR_SHM = "/tmp/tsfi_neural_out.raw"
STREAM_KEY_FILE = os.path.expanduser("~/.config/gemini/stream_key.txt")

def get_stream_key():
    try:
        with open(STREAM_KEY_FILE, "r") as f:
            return f.read().strip()
    except:
        return None

STREAM_KEY = get_stream_key()

# --- Load Native C Encoder Thunk ---
lib_path = os.path.abspath("thunks/tsfi_soft_encode.so")
lib = ctypes.CDLL(lib_path)
lib.tsfi_soft_encode_init.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)), ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)), ctypes.POINTER(ctypes.c_size_t)]
lib.tsfi_soft_encode_init.restype = ctypes.c_int
lib.tsfi_soft_encode_frame.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t), ctypes.c_uint32]

class NativePythonRTMP:
    def __init__(self, host, key, sps, pps):
        self.host = host; self.key = key; self.sock = None; self.epoch = 0; self.chunk_size = 4096
        self.sps = sps; self.pps = pps; self.bytes_recv = 0

    def amf_str(self, s): return b'\x02' + struct.pack('>H', len(s)) + s.encode()
    def amf_num(self, n): return b'\x00' + struct.pack('>d', n)

    def connect(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5.0)
            self.sock.connect((self.host, 1935))
            # Handshake
            self.sock.send(b'\x03' + b'\x00' * 1536); self.sock.recv(1537)
            self.sock.send(b'\x00' * 1536); self.sock.recv(1536)
            
            # connect
            body = self.amf_str("connect") + self.amf_num(1.0) + b'\x03'
            body += b'\x00\x03app' + self.amf_str("live2")
            body += b'\x00\x05tcUrl' + self.amf_str(f"rtmp://{self.host}/live2")
            body += b'\x00\x00\x09'
            self.send_packet(0x14, 0, body)
            
            # Set Chunk Size (Mandatory QoS)
            self.send_packet(0x01, 0, struct.pack('>I', self.chunk_size))
            
            # publish
            pub = self.amf_str("publish") + self.amf_num(2.0) + b'\x05' + self.amf_str(self.key) + self.amf_str("live")
            self.send_packet(0x14, 1, pub)
            
            # onMetaData
            meta = self.amf_str("@setDataFrame") + self.amf_str("onMetaData") + b'\x08' + struct.pack('>I', 2)
            meta += b'\x00\x05width' + self.amf_num(1280.0) + b'\x00\x06height' + self.amf_num(720.0) + b'\x00\x00\x09'
            self.send_packet(0x12, 1, meta)

            # AVC Sequence Header
            sh = b'\x17\x00\x00\x00\x00\x01' + self.sps[1:4] + b'\xFF\xE1' + struct.pack('>H', len(self.sps)) + self.sps + b'\x01' + struct.pack('>H', len(self.pps)) + self.pps
            self.send_packet(0x09, 1, sh)
            
            print(f"[RTMP] QoS Negotiated for {self.host}")
            return True
        except Exception as e:
            print(f"[FRACTURE] Connection Error: {e}")
            return False

    def send_packet(self, p_type, stream_id, payload):
        length = len(payload)
        header = struct.pack('>B', 0x03) + struct.pack('>BHB', (self.epoch >> 16) & 0xFF, (self.epoch >> 8) & 0xFFFF, self.epoch & 0xFF)
        header += struct.pack('>BH', (length >> 16) & 0xFF, length & 0xFFFF) + struct.pack('>B', p_type) + struct.pack('<I', stream_id)
        sent = 0
        while sent < length:
            chunk = payload[sent:sent+self.chunk_size]
            if sent == 0: self.sock.sendall(header + chunk)
            else: self.sock.sendall(b'\xC3' + chunk)
            sent += len(chunk)

    def send_video(self, bitstream):
        if len(bitstream) < 5: return
        is_key = (bitstream[4] & 0x1F) == 5
        self.send_packet(0x09, 1, (b'\x17' if is_key else b'\x27') + b'\x01\x00\x00\x00' + bitstream)
        self.epoch += 41
        
    def poll_qos(self):
        # Explicitly drain the socket to handle Window ACKs from YouTube
        try:
            self.sock.setblocking(False)
            data = self.sock.recv(4096)
            if data:
                self.bytes_recv += len(data)
                # Send Window Acknowledgment (Type 0x03) back to server
                ack = struct.pack('>I', self.bytes_recv)
                self.send_packet(0x03, 0, ack)
        except: pass

def broadcast_engine():
    sps_p = ctypes.POINTER(ctypes.c_uint8)(); sl = ctypes.c_size_t(); pps_p = ctypes.POINTER(ctypes.c_uint8)(); pl = ctypes.c_size_t()
    if lib.tsfi_soft_encode_init(1280, 720, ctypes.byref(sps_p), ctypes.byref(sl), ctypes.byref(pps_p), ctypes.byref(pl)) != 0: return
    sps = bytes(ctypes.string_at(sps_p, sl.value)); pps = bytes(ctypes.string_at(pps_p, pl.value))
    
    rtmp = NativePythonRTMP("b.rtmp.youtube.com", STREAM_KEY, sps, pps)
    if rtmp.connect():
        f_idx = 0; last_m = 0
        while True:
            try:
                if os.path.exists(COMPOSITOR_SHM):
                    m = os.path.getmtime(COMPOSITOR_SHM)
                    if m > last_m:
                        last_m = m
                        with open(COMPOSITOR_SHM, "rb") as f:
                            pix = f.read()
                            op = ctypes.c_void_p(); ol = ctypes.c_size_t()
                            if lib.tsfi_soft_encode_frame(pix, 1280, 720, ctypes.byref(op), ctypes.byref(ol), f_idx) == 0:
                                rtmp.send_video(bytes(ctypes.string_at(op, ol.value))); f_idx += 1
                rtmp.poll_qos() # Maintain QoS heartbeat
            except: pass
            time.sleep(0.01)

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass
class WorldSerpentHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200); self.send_header('Content-type', 'text/html'); self.end_headers()
            self.wfile.write(b"<html><body style='background:#000;color:#0ff;'><h1>TSFi QoS Hardened Broadcast Active</h1></body></html>")

if __name__ == '__main__':
    import threading
    threading.Thread(target=broadcast_engine, daemon=True).start()
    subprocess.run(["fuser", "-k", "9093/tcp"])
    server = ThreadedHTTPServer(('0.0.0.0', 9093), WorldSerpentHandler)
    server.serve_forever()
