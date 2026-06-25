import unittest
import numpy as np
import math

class TestVaesenBirdEmotionalAccumulator(unittest.TestCase):
    
    def setUp(self):
        # Initial emotional indices: Agitation (E_ag) and Pleasure (H_pl)
        self.E_ag = 0.1
        self.H_pl = 0.9
        self.decay = 0.92  # Leak accumulator decay factor
        
        # Syrinx Physical Parameters
        self.k_open = 1000.0
        self.k_closed = 3000.0
        self.Kc = 100.0  # Syrinx coupling
        
    def accumulate_emotion(self, dynamic_input, aesthetic_error):
        # Accumulator updates:
        # High dynamic variance raises Agitation
        self.E_ag = self.decay * self.E_ag + (1.0 - self.decay) * dynamic_input
        # High aesthetic error (dissonance) lowers Pleasure
        self.H_pl = self.decay * self.H_pl + (1.0 - self.decay) * (1.0 - aesthetic_error)
        
    def get_modulated_syrinx_params(self):
        # Map emotional state to physical syrinx dynamics:
        # 1. Agitation modulates spring asymmetry (increasing closed stiffness for hoarse screams)
        k_closed_mod = self.k_closed * (1.0 + 1.8 * self.E_ag)
        
        # 2. Pleasure modulates syrinx coupling (high pleasure reduces coupling for clean dual whistles)
        Kc_mod = self.Kc * (1.0 - 0.7 * self.H_pl)
        
        return k_closed_mod, Kc_mod

    def test_emotional_drift_to_pleasure(self):
        """Verify that positive aesthetic reinforcement converges the bird to a calm, clean singing state."""
        # Simulate 50 steps of positive self-judgement (low error, low agitation input)
        for _ in range(50):
            self.accumulate_emotion(dynamic_input=0.05, aesthetic_error=0.02)
            
        k_closed_mod, Kc_mod = self.get_modulated_syrinx_params()
        
        # Assert Pleasure index has accumulated towards maximum
        self.assertTrue(self.H_pl > 0.85)
        # Assert Agitation has leaked down
        self.assertTrue(self.E_ag < 0.15)
        # Syrinx coupling should be minimized for clean bilateral whistles
        self.assertTrue(Kc_mod < 45.0)

    def test_emotional_drift_to_agitation(self):
        """Verify that high aesthetic distress (dissonance) pushes the bird into an aggressive, hoarse cawing state."""
        # Simulate 50 steps of high aesthetic distress / low pleasure input
        for _ in range(50):
            self.accumulate_emotion(dynamic_input=0.85, aesthetic_error=0.90)
            
        k_closed_mod, Kc_mod = self.get_modulated_syrinx_params()
        
        # Assert Agitation has accumulated towards maximum
        self.assertTrue(self.E_ag > 0.75)
        # Assert Pleasure index has dropped
        self.assertTrue(self.H_pl < 0.25)
        # Closed stiffness should be highly boosted for asymmetric throat caws
        self.assertTrue(k_closed_mod > 6000.0)

if __name__ == "__main__":
    unittest.main()
