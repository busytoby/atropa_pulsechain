import os
import json
from pathlib import Path

CONFIG_PATH = Path.home() / ".gemini/config"

def load_config():
    """Loads configuration from ~/.gemini/config securely."""
    config = {}
    
    if not CONFIG_PATH.exists():
        print(f"Warning: Config file not found at {CONFIG_PATH}")
        return config

    try:
        # Try JSON first
        with open(CONFIG_PATH, 'r') as f:
            content = f.read().strip()
            if content.startswith('{'):
                config = json.loads(content)
            else:
                # Fallback to KEY=VALUE format
                for line in content.splitlines():
                    if '=' in line and not line.startswith('#'):
                        key, value = line.split('=', 1)
                        config[key.strip()] = value.strip().strip('"').strip("'")
    except Exception as e:
        print(f"Error reading config: {e}")
    
    return config

def get_secret(key):
    """Retrieves a secret from config or environment."""
    # Priority: Env Var > Config File
    val = os.environ.get(key)
    if val:
        return val
        
    config = load_config()
    return config.get(key)
