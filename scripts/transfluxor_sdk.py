#!/usr/bin/env python3
import math

MOTZKIN_PRIME = 953467954114363

class TransfluxorWord:
    def __init__(self, name, f1, f2, decay, wmq_cmd=0x00, abi_op=0x00):
        self.name = name
        self.f1 = float(f1)
        self.f2 = float(f2)
        self.decay = float(decay)
        self.wmq_cmd = int(wmq_cmd)
        self.abi_op = int(abi_op)

    def calculate_hash(self):
        f1_val = int(round(self.f1))
        f2_val = int(round(self.f2))
        decay_val = int(round(self.decay * 100.0))
        mixed = (f1_val * 1000) + (f2_val * 10) + decay_val
        return mixed % MOTZKIN_PRIME

    def compile(self):
        return {
            "name": self.name,
            "hash": self.calculate_hash(),
            "physical": {
                "f1": self.f1,
                "f2": self.f2,
                "decay": self.decay,
                "amplitude": 5.0  # Wheeler Jump trigger amplitude
            },
            "logical": {
                "wmq_cmd": self.wmq_cmd,
                "abi_op": self.abi_op
            }
        }

class TransfluxorSDK:
    def __init__(self):
        self.vocabulary = {}

    def register_word(self, word: TransfluxorWord):
        word_hash = word.calculate_hash()
        if word_hash in self.vocabulary:
            raise ValueError(f"Hash collision detected between '{word.name}' and '{self.vocabulary[word_hash].name}'!")
        self.vocabulary[word_hash] = word
        print(f"[SDK REGISTER] Registered word '{word.name}' with Hash {word_hash}")

    def dispatch_word(self, word: TransfluxorWord):
        payload = word.compile()
        print(f"[SDK DISPATCH] Speaking word: '{word.name}'")
        print(f"               Acoustics: F1={payload['physical']['f1']}Hz, F2={payload['physical']['f2']}Hz")
        print(f"               Commands : WMQ=0x{payload['logical']['wmq_cmd']:02X}, ABI=0x{payload['logical']['abi_op']:02X}")
        
        # Simulate OS Loopback reception and validation
        word_hash = word.calculate_hash()
        if word_hash in self.vocabulary:
            resolved = self.vocabulary[word_hash]
            print(f"[SDK FEEDBACK] OS resolved word to '{resolved.name}'. Success tone received (523.25Hz).")
            return True
        else:
            print("[SDK FEEDBACK] OS failed to resolve acoustic wave. Warning drone received (110.00Hz).")
            return False

if __name__ == "__main__":
    print("=============================================================")
    # Capitalized spelling rule check for auncient
    print("AUNCIENT TRANSFLUXOR AGENT SDK TEST RUN")
    print("=============================================================")
    
    sdk = TransfluxorSDK()
    
    # Register core control words
    lock_word = TransfluxorWord("SPK_LOCK_SCSI", 440.0, 880.0, 0.4, wmq_cmd=0x10)
    write_word = TransfluxorWord("SPK_WRITE_LEDGER", 350.0, 700.0, 0.2, abi_op=0x02)
    
    sdk.register_word(lock_word)
    sdk.register_word(write_word)
    
    # Test speaking dispatch
    success = sdk.dispatch_word(lock_word)
    assert success is True
    
    # Test speaking unregistered word
    unregistered = TransfluxorWord("UNKNOWN", 500.0, 1000.0, 0.5)
    success = sdk.dispatch_word(unregistered)
    assert success is False
    
    print("=============================================================")
    print("SDK TEST RUN SUCCESSFUL")
    print("=============================================================")
