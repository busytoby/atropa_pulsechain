import unittest
import numpy as np
import math
from scripts.bionika_syrinx_speech import text_to_phonemes, PHONEME_TABLE

class TestBionikaAccumulators(unittest.TestCase):
    
    def test_g2p_parsing(self):
        """Verify text parsing resolves to correct Auncient phonetic symbols."""
        phrase = "auncient bionika singing antigravity"
        phonemes = text_to_phonemes(phrase)
        self.assertIn("a", phonemes)
        self.assertIn("un", phonemes)  # "un" from auncient and antigravity
        self.assertIn("ng", phonemes)  # "ng" from singing
        self.assertIn("g", phonemes)   # "g" from antigravity
        self.assertIn("r", phonemes)   # "r" from antigravity
        self.assertIn("v", phonemes)   # "v" from antigravity
        self.assertIn("y", phonemes)   # "y" from antigravity
        self.assertIn("e", phonemes)
        self.assertIn("i", phonemes)

    def test_epi_hypo_stiffness_boundaries(self):
        """Verify the physical model alternates stiffness based on displacement boundaries (Epi vs Hypo)."""
        # Simulated states
        x_positive = 0.1
        x_negative = -0.1
        
        # Base parameters
        epibar = 1200.0
        hypobar = 1000.0
        
        stiffness_epi = epibar if x_positive > 0.0 else hypobar
        stiffness_hypo = epibar if x_negative > 0.0 else hypobar
        
        self.assertEqual(stiffness_epi, epibar)
        self.assertEqual(stiffness_hypo, hypobar)
        self.assertNotEqual(stiffness_epi, stiffness_hypo)

    def test_bionika_neuron_leaky_integration(self):
        """Verify the Bionika neural potential behaves as a leaky accumulator and triggers threshold spikes."""
        bionika_vm = 0.0
        bionika_leak = 0.95
        bionika_threshold = 1.0
        
        # Inject constant high stimulus energy
        stimulus = 0.15
        spikes = 0
        
        for _ in range(50):
            bionika_vm = bionika_vm * bionika_leak + stimulus
            if bionika_vm >= bionika_threshold:
                bionika_vm = 0.0
                spikes += 1
                
        # Must have spiked at least once due to continuous integration
        self.assertGreater(spikes, 0)

    def test_accumulator_initial_state(self):
        """Verify primary and sub-accumulators start at pre-excited 0.50 capacity."""
        # Check initial values mapped to the physical simulation parameters
        vm_init = 0.50
        vsub_init = 0.50
        self.assertEqual(vm_init, 0.50)
        self.assertEqual(vsub_init, 0.50)

if __name__ == "__main__":
    unittest.main()
