import json
import os
import re
import sys
from datetime import datetime

# Configuration
DUMP_FILE = "creative/raw_browser_dump.json"
PROMPTS_DIR = "creative/prompts"

def sanitize_filename(text):
    s = re.sub(r'[^\w\s-]', '', text).strip().lower()
    return re.sub(r'[-\s]+', '_', s)[:50]

def parse_dump():
    if not os.path.exists(DUMP_FILE):
        print(f"Error: {DUMP_FILE} not found. Please paste browser output there first.")
        return

    try:
        with open(DUMP_FILE, 'r') as f:
            data = json.load(f)
    except json.JSONDecodeError:
        print("Error: Invalid JSON in dump file.")
        return

    # Handle different dump structures (from our JS extractor)
    projects = []
    if 'data' in data and isinstance(data['data'], list):
        # Look inside API responses
        for entry in data['data']:
            if 'projects' in entry.get('data', {}):
                projects.extend(entry['data']['projects'])
    
    # If standard structure isn't found, try to find *any* list of objects with 'prompt'
    if not projects:
        # Recursive search for objects with 'prompt' key
        def find_prompts(obj):
            found = []
            if isinstance(obj, dict):
                if 'prompt' in obj and isinstance(obj['prompt'], str):
                    found.append(obj)
                for k, v in obj.items():
                    found.extend(find_prompts(v))
            elif isinstance(obj, list):
                for item in obj:
                    found.extend(find_prompts(item))
            return found
        
        projects = find_prompts(data)

    print(f"Found {len(projects)} potential prompt entries.")

    for i, p in enumerate(projects):
        prompt_text = p.get('prompt') or p.get('text')
        if not prompt_text:
            continue

        # Generate Metadata
        timestamp = p.get('createdAt') or p.get('created_at') or datetime.now().isoformat()
        model = p.get('model') or "Veo (Unknown)"
        
        # Create Filename
        safe_title = sanitize_filename(prompt_text)
        filename = f"{PROMPTS_DIR}/{i:03d}_{safe_title}.md"
        
        # Content
        content = f"""# Project Flow Prompt: {safe_title}

**Status:** Imported
**Date:** {timestamp}
**Model:** {model}
**Source:** Browser Dump

## Prompt
> {prompt_text}

## Metadata
- **ID:** {p.get('id', 'N/A')}
- **Seed:** {p.get('seed', 'N/A')}
"""
        
        # Write
        with open(filename, "w") as out:
            out.write(content)
        print(f"Synced: {filename}")

if __name__ == "__main__":
    parse_dump()
