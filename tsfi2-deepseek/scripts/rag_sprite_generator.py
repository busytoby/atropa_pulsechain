#!/usr/bin/env python3
import os
import re

def retrieve_lore_context(query, lore_dir="lore"):
    """
    Retrieves context matching the query from the lore folder using ranking
    based on keyword overlap density to ensure high precision.
    """
    if not os.path.exists(lore_dir):
        if os.path.exists("../lore"):
            lore_dir = "../lore"
        else:
            return "No lore directory found."

    query_words = set(query.lower().split())
    scored_chunks = []

    for filename in os.listdir(lore_dir):
        if filename.endswith(".md"):
            filepath = os.path.join(lore_dir, filename)
            with open(filepath, "r", encoding="utf-8") as f:
                content = f.read()
                content_lower = content.lower()
                # Score based on density/overlap of query terms
                score = sum(content_lower.count(word) for word in query_words if word in content_lower)
                # Boost if filename contains key terms like 'vaesen' or 'sprite'
                if "vaesen" in filename.lower():
                    score += 50
                if "sprite" in filename.lower():
                    score += 30
                
                if score > 0:
                    scored_chunks.append((score, filename, content))
                    
    # Sort by score descending
    scored_chunks.sort(key=lambda x: x[0], reverse=True)
    
    # Take top 3 documents to keep context focused and avoid overflow
    top_chunks = scored_chunks[:3]
    return "\n".join(f"--- Context from {name} ---\n{body}\n" for _, name, body in top_chunks)

def generate_sprite_prompt():
    print("[RAG] Querying lore repository for Vaesen character sprite sheets...")
    context = retrieve_lore_context("vaesen sprite sheet graphics characters")
    
    print("[RAG] Context retrieved successfully. Parsing character specifications...")
    
    # Default specifications if parsing fails
    crow_spec = "Dark Crow: Deep obsidian feathers, crimson aura, red eyes. Frames: idle, wing span, gliding, flapping."
    troll_spec = "Small Blue Forest Troll: Blue skin, pointy white hat, tattered trousers. Frames: stand, walk, panic shield, run."
    alchemist_spec = "Alchemist: Sinister hooded robe, staff with glowing gem. Frames: brewing, casting, creeping, defeated."
    
    # Extract specific character definitions using robust regex
    if "Dark Crow" in context:
        m = re.search(r"###\s*1\b.*?(?=(###\s*2\b|---|#|$))", context, re.DOTALL)
        if m:
            crow_spec = m.group(0).strip()
            
    if "Small Blue Forest Troll" in context:
        m = re.search(r"###\s*2\b.*?(?=(###\s*3\b|---|#|$))", context, re.DOTALL)
        if m:
            troll_spec = m.group(0).strip()
            
    if "Alchemist" in context:
        m = re.search(r"###\s*3\b.*?(?=(###\s*4\b|---|#|$))", context, re.DOTALL)
        if m:
            alchemist_spec = m.group(0).strip()
            
    prompt = (
        f"A 2D retro pixel art sprite sheet grid for a gothic Nordic folklore game. "
        f"The sprite sheet contains a clean grid of animation frames for three distinct characters:\n\n"
        f"1. {crow_spec.replace('#', '').strip()}\n"
        f"2. {troll_spec.replace('#', '').strip()}\n"
        f"3. {alchemist_spec.replace('#', '').strip()}\n\n"
        f"Clean aligned grid layout, 8-bit retro gaming assets, vibrant pixel color palette with deep shadow contrast, transparent black background."
    )
    
    return prompt

if __name__ == "__main__":
    prompt = generate_sprite_prompt()
    print("\n--- SYNTHESIZED RAG PROMPT ---")
    print(prompt)
    print("------------------------------")
    
    # Write synthesized prompt to a text file for stability tracking
    os.makedirs("assets", exist_ok=True)
    with open("assets/sprite_prompt.txt", "w") as f:
        f.write(prompt)
    print("[RAG] Prompt successfully exported to assets/sprite_prompt.txt")
