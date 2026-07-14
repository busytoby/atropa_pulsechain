import urllib.request
import re
import sys
import html

def query_dblp(query_str):
    encoded_query = urllib.parse.quote(query_str)
    url = f"https://dblp.org/search/publ/api?q={encoded_query}&h=10&format=xml"
    
    print(f"Querying DBLP API: {url}\n")
    try:
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64)'}
        )
        with urllib.request.urlopen(req, timeout=10) as response:
            xml_data = response.read().decode('utf-8', errors='ignore')
    except Exception as e:
        print(f"Error querying DBLP API: {e}")
        sys.exit(1)
        
    # resiliant regex extraction
    hits = re.findall(r'<hit\b.*?>(.*?)</hit>', xml_data, re.DOTALL)
    if not hits:
        print("No publications found matching the query.")
        return
        
    print(f"Found {len(hits)} publications:\n")
    for idx, hit in enumerate(hits, 1):
        title_match = re.search(r'<title.*?>(.*?)</title>', hit)
        venue_match = re.search(r'<venue>(.*?)</venue>', hit)
        year_match = re.search(r'<year>(.*?)</year>', hit)
        authors = re.findall(r'<author.*?>(.*?)</author>', hit)
        
        title = html.unescape(title_match.group(1)) if title_match else "No Title"
        # Strip trailing period if present
        if title.endswith('.'):
            title = title[:-1]
        venue = html.unescape(venue_match.group(1)) if venue_match else "Unknown Venue"
        year = year_match.group(1) if year_match else "Unknown Year"
        authors_list = ", ".join([html.unescape(a) for a in authors])
        
        print(f"{idx}. \"{title}\"")
        print(f"   Authors: {authors_list}")
        print(f"   Venue: {venue} ({year})\n")

if __name__ == '__main__':
    query = "knowledge graph"
    if len(sys.argv) > 1:
        query = " ".join(sys.argv[1:])
    query_dblp(query)
