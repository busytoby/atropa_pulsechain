import os
import json
import re
import datetime

def get_file_date(filepath):
    filename = os.path.basename(filepath)
    match = re.search(r'(\d{4})[_-](\d{2})[_-](\d{2})', filename)
    if match:
        try:
            return datetime.datetime(int(match.group(1)), int(match.group(2)), int(match.group(3)))
        except ValueError:
            pass
            
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            for _ in range(15):
                line = f.readline()
                if not line:
                    break
                match_line = re.search(r'(\d{4})[_-](\d{2})[_-](\d{2})', line)
                if match_line:
                    try:
                        return datetime.datetime(int(match_line.group(1)), int(match_line.group(2)), int(match_line.group(3)))
                    except ValueError:
                        pass
    except Exception:
        pass
        
    return datetime.datetime.fromtimestamp(os.path.getmtime(filepath))

def build_registry():
    lore_dir = "lore"
    files = []
    for f in os.listdir(lore_dir):
        # Exclude registry itself or any other non-article files
        if f.endswith(".md") and f != "published_registry.json":
            path = os.path.join(lore_dir, f)
            date = get_file_date(path)
            files.append((date, path, f))
            
    files.sort(key=lambda x: x[0])
    
    total_files = len(files)
    vol_size = total_files // 4
    
    vols = [
        files[0:vol_size],
        files[vol_size:2*vol_size],
        files[2*vol_size:3*vol_size],
        files[3*vol_size:]
    ]
    
    registry = {}
    for idx, vol_files in enumerate(vols):
        vol_num = idx + 1
        for date, path, filename in vol_files:
            registry[filename] = {
                "published_volume": vol_num,
                "publish_date": date.strftime('%Y-%m-%d'),
                "registered_on": datetime.datetime.now().strftime('%Y-%m-%d')
            }
            
    output_path = os.path.join(lore_dir, "published_registry.json")
    with open(output_path, "w", encoding="utf-8") as out:
        json.dump(registry, out, indent=4)
        
    print(f"Published registry created successfully: {output_path}")

if __name__ == "__main__":
    build_registry()
