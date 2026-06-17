#!/usr/bin/env python3
import os
import json
import re
import glob
import math
from datetime import datetime

# Path resolution for reserves
reserves_files = glob.glob(os.path.expanduser("~/.gemini/antigravity-cli/brain/*/scratch/nonukes_pulsex_reserves.json"))
default_reserves_path = reserves_files[0] if reserves_files else "nonukes_pulsex_reserves.json"

POOLS_FILE = "nonukes_pools.json"
SWAPS_FILE = "resolved_swaps.json"
PRICES_FILE = "price_cache.json"
RESERVES_FILE = os.environ.get("RESERVES_FILE_PATH", default_reserves_path)

def load_data():
    pools = {}
    if os.path.exists(POOLS_FILE):
        try:
            with open(POOLS_FILE, "r") as f:
                pools = json.load(f)
        except Exception:
            pass
            
    swaps = []
    if os.path.exists(SWAPS_FILE):
        try:
            with open(SWAPS_FILE, "r") as f:
                swaps = json.load(f)
        except Exception:
            pass
            
    prices = {}
    if os.path.exists(PRICES_FILE):
        try:
            with open(PRICES_FILE, "r") as f:
                prices = json.load(f)
        except Exception:
            pass
            
    reserves = {}
    if os.path.exists(RESERVES_FILE):
        try:
            with open(RESERVES_FILE, "r") as f:
                reserves = json.load(f)
        except Exception:
            pass
            
    return pools, swaps, prices, reserves

def get_token_price(prices, addr):
    if not isinstance(prices, dict) or not addr:
        return 0.0
    val = prices.get(addr.lower())
    if not val:
        # Check case-insensitive key search
        for k, v in prices.items():
            if k.lower() == addr.lower():
                val = v
                break
    price_val = 0.0
    if isinstance(val, dict):
        price_val = val.get("price")
    else:
        price_val = val
    if price_val is None:
        return 0.0
    try:
        return float(price_val)
    except (ValueError, TypeError):
        return 0.0

def get_adjusted_reserves(pool_addr, reserves, pool_info):
    NONUKES_ADDR = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"
    partner_addr = pool_info.get("other_addr", "").lower() if isinstance(pool_info, dict) else ""
    partner_symbol = pool_info.get("symbol", "UNKNOWN") if isinstance(pool_info, dict) else "UNKNOWN"
    
    res = reserves.get(pool_addr.lower(), {})
    if not isinstance(res, dict):
        res = {}
        
    t0 = res.get("token0", "").lower()
    t1 = res.get("token1", "").lower()
    
    r0_val = res.get("reserve0")
    r1_val = res.get("reserve1")
    try:
        r0 = float(r0_val) if r0_val is not None else 0.0
    except (ValueError, TypeError):
        r0 = 0.0
    try:
        r1 = float(r1_val) if r1_val is not None else 0.0
    except (ValueError, TypeError):
        r1 = 0.0
        
    if not t0 or not t1:
        if partner_addr < NONUKES_ADDR:
            t0, t1 = partner_addr, NONUKES_ADDR
        else:
            t0, t1 = NONUKES_ADDR, partner_addr
            
    if t0 == NONUKES_ADDR:
        nonukes_res = r0
        partner_res = r1
        partner_token_key = "1"
    elif t1 == NONUKES_ADDR:
        nonukes_res = r1
        partner_res = r0
        partner_token_key = "0"
    else:
        nonukes_res = r0
        partner_res = r1
        partner_token_key = "1"
        
    partner_decimals = 18
    raw_r_val = res.get(f"raw_reserve{partner_token_key}")
    adj_r_val = res.get(f"reserve{partner_token_key}")
    if raw_r_val is not None and adj_r_val is not None:
        try:
            raw_r = float(raw_r_val)
            adj_r = float(adj_r_val)
            if raw_r > 0.0 and adj_r > 0.0:
                derived_dec = round(math.log10(raw_r / adj_r))
                if 0 <= derived_dec <= 36:
                    partner_decimals = derived_dec
        except Exception:
            pass
            
    partner_res_adjusted = partner_res * (10 ** (18 - partner_decimals))
    
    return {
        "nonukes_reserves": nonukes_res,
        "partner_reserves": partner_res_adjusted,
        "partner_decimals": partner_decimals,
        "token0_symbol": "NoNukes",
        "token1_symbol": partner_symbol
    }

def calculate_pool_metrics(pools, swaps, prices, reserves):
    NONUKES_PRICE = 1.74
    NONUKES_ADDR = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"
    nonukes_price = get_token_price(prices, NONUKES_ADDR) or NONUKES_PRICE
    
    pool_metrics = {}
    for pool_addr, info in pools.items():
        pool_addr_lower = pool_addr.lower()
        res_info = get_adjusted_reserves(pool_addr_lower, reserves, info)
        
        # Calculate volume
        vol_usd = 0.0
        swap_cnt = 0
        for swap in swaps:
            if swap.get("pool_address", "").lower() == pool_addr_lower:
                swap_cnt += 1
                vol_usd += float(swap.get("usd_value", 0.0))
                
        # Liquidity
        liq_usd = res_info["nonukes_reserves"] * nonukes_price * 2.0
        
        pool_metrics[pool_addr_lower] = {
            "address": pool_addr_lower,
            "symbol": info.get("symbol", "UNKNOWN"),
            "name": info.get("name", "Unknown Token"),
            "target_group": info.get("target_group", "NoNukes"),
            "version": info.get("version", "V1"),
            "other_addr": info.get("other_addr", "").lower(),
            "nonukes_reserves": res_info["nonukes_reserves"],
            "partner_reserves": res_info["partner_reserves"],
            "volume_usd": vol_usd,
            "swap_count": swap_cnt,
            "liquidity_usd": liq_usd
        }
    return pool_metrics

def rule_based_query(query, pools, swaps, prices, reserves):
    query_lower = query.lower()
    
    # Calculate fresh metrics
    pool_metrics = calculate_pool_metrics(pools, swaps, prices, reserves)
    NONUKES_PRICE = 1.74
    NONUKES_ADDR = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"
    nonukes_price = get_token_price(prices, NONUKES_ADDR) or NONUKES_PRICE

    # 1. Check for address search
    addr_match = re.search(r"0x[a-fA-F0-9]{40}", query_lower)
    if addr_match:
        addr = addr_match.group(0)
        # Check if it's a pool
        if addr in pool_metrics:
            m = pool_metrics[addr]
            other_price = get_token_price(prices, m["other_addr"])
            res = f"### Pool Details: `{addr}`\n"
            res += f"- **Token Pair:** NoNukes / {m['symbol']} ({m['name']})\n"
            res += f"- **Version:** {m['version']} | **Group:** {m['target_group']}\n"
            res += f"- **Reserves:** {m['nonukes_reserves']:,.4f} NoNukes | {m['partner_reserves']:,.4f} {m['symbol']}\n"
            res += f"- **Liquidity (USD):** ${m['liquidity_usd']:,.2f}\n"
            res += f"- **24h Volume (USD):** ${m['volume_usd']:,.2f}\n"
            res += f"- **Swap Count:** {m['swap_count']}\n"
            if other_price > 0:
                res += f"- **Partner Token Price:** ${other_price:,.8f}\n"
            return res
        # Check if it's a token
        token_pools = [p for p in pool_metrics.values() if p["other_addr"] == addr]
        if token_pools:
            t_price = get_token_price(prices, addr)
            sym = token_pools[0]["symbol"]
            name = token_pools[0]["name"]
            res = f"### Token Details: `{addr}` ({sym})\n"
            res += f"- **Name:** {name}\n"
            res += f"- **Price:** ${t_price:,.8f}\n"
            res += f"- **Total Pools:** {len(token_pools)}\n"
            tot_vol = sum(p["volume_usd"] for p in token_pools)
            tot_swaps = sum(p["swap_count"] for p in token_pools)
            tot_liq = sum(p["liquidity_usd"] for p in token_pools)
            res += f"- **Total Liquidity (USD):** ${tot_liq:,.2f}\n"
            res += f"- **Total Volume (USD):** ${tot_vol:,.2f}\n"
            res += f"- **Total Swaps:** {tot_swaps}\n"
            return res

    # 2. Top pools or tokens queries
    if "top" in query_lower or "best" in query_lower or "highest" in query_lower or "largest" in query_lower:
        limit = 5
        limit_match = re.search(r"\b(10|5|3)\b", query_lower)
        if limit_match:
            limit = int(limit_match.group(0))
            
        if "volume" in query_lower:
            sorted_pools = sorted(pool_metrics.values(), key=lambda x: x["volume_usd"], reverse=True)[:limit]
            res = f"### Top {limit} Pools by Volume (USD)\n\n"
            res += "| Pool Address | Token Pair | Version | Volume | Swaps | Liquidity |\n"
            res += "| --- | --- | --- | --- | --- | --- |\n"
            for p in sorted_pools:
                res += f"| `{p['address'][:8]}...` | NoNukes/{p['symbol']} | {p['version']} | ${p['volume_usd']:,.2f} | {p['swap_count']} | ${p['liquidity_usd']:,.2f} |\n"
            return res
        elif "liquidity" in query_lower or "reserves" in query_lower or "size" in query_lower:
            sorted_pools = sorted(pool_metrics.values(), key=lambda x: x["liquidity_usd"], reverse=True)[:limit]
            res = f"### Top {limit} Pools by Liquidity\n\n"
            res += "| Pool Address | Token Pair | Version | Liquidity | Reserves (NoNukes / Partner) |\n"
            res += "| --- | --- | --- | --- | --- |\n"
            for p in sorted_pools:
                res += f"| `{p['address'][:8]}...` | NoNukes/{p['symbol']} | {p['version']} | ${p['liquidity_usd']:,.2f} | {p['nonukes_reserves']:,.2f} / {p['partner_reserves']:,.2f} |\n"
            return res
        elif "swaps" in query_lower or "active" in query_lower or "trades" in query_lower:
            sorted_pools = sorted(pool_metrics.values(), key=lambda x: x["swap_count"], reverse=True)[:limit]
            res = f"### Top {limit} Pools by Swap Count\n\n"
            res += "| Pool Address | Token Pair | Version | Swaps | Volume | Liquidity |\n"
            res += "| --- | --- | --- | --- | --- | --- |\n"
            for p in sorted_pools:
                res += f"| `{p['address'][:8]}...` | NoNukes/{p['symbol']} | {p['version']} | {p['swap_count']} | ${p['volume_usd']:,.2f} | ${p['liquidity_usd']:,.2f} |\n"
            return res

    # 3. Specific symbol search (e.g., "price of USDC" or "stats of PLSX")
    # Extract any uppercase symbol looking word or check matches
    words = re.findall(r"\b[a-zA-Z0-9\$\-\_]{2,15}\b", query)
    for w in words:
        w_lower = w.lower()
        if w_lower in ["pool", "token", "swap", "volume", "price", "stats", "info", "query", "what", "show", "list", "about"]:
            continue
        # Find matching token pools
        matched_pools = [p for p in pool_metrics.values() if p["symbol"].lower() == w_lower or p["name"].lower() == w_lower or w_lower in p["symbol"].lower()]
        if matched_pools:
            # Aggregate metrics for this symbol
            sym = matched_pools[0]["symbol"]
            name = matched_pools[0]["name"]
            other_addr = matched_pools[0]["other_addr"]
            t_price = get_token_price(prices, other_addr)
            
            tot_vol = sum(p["volume_usd"] for p in matched_pools)
            tot_swaps = sum(p["swap_count"] for p in matched_pools)
            tot_liq = sum(p["liquidity_usd"] for p in matched_pools)
            
            res = f"### Telemetry Report for Token: **{sym}** ({name})\n"
            if t_price > 0:
                res += f"- **Current Cached Price:** ${t_price:,.8f}\n"
            else:
                res += f"- **Price:** N/A (No active external price feed)\n"
            res += f"- **NoNukes Pools Count:** {len(matched_pools)}\n"
            res += f"- **Aggregated Liquidity:** ${tot_liq:,.2f} USD\n"
            res += f"- **Aggregated 24h Volume:** ${tot_vol:,.2f} USD\n"
            res += f"- **Aggregated Swaps:** {tot_swaps}\n\n"
            res += "**Active Pools List:**\n"
            for p in matched_pools[:5]:
                res += f"- Pool `{p['address']}` ({p['version']}): Reserves: {p['nonukes_reserves']:,.2f} NoNukes / {p['partner_reserves']:,.2f} {sym}\n"
            if len(matched_pools) > 5:
                res += f"- *And {len(matched_pools) - 5} more pools...*\n"
            return res

    # 4. Global statistics
    if "total" in query_lower or "summary" in query_lower or "global" in query_lower or "all" in query_lower or "how many" in query_lower:
        tot_pools = len(pool_metrics)
        tot_vol = sum(p["volume_usd"] for p in pool_metrics.values())
        tot_swaps = sum(p["swap_count"] for p in pool_metrics.values())
        tot_liq = sum(p["liquidity_usd"] for p in pool_metrics.values())
        groups = set(p["target_group"] for p in pool_metrics.values())
        
        res = "### NoNukes Ecosystem Global Telemetry Summary\n"
        res += f"- **Total Tracked Pools:** {tot_pools}\n"
        res += f"- **Active Groups:** {len(groups)} ({', '.join(groups)})\n"
        res += f"- **Aggregated Liquidity:** ${tot_liq:,.2f} USD\n"
        res += f"- **Total Swap Volume:** ${tot_vol:,.2f} USD\n"
        res += f"- **Total Trade/Swap Count:** {tot_swaps}\n"
        return res

    # 5. Default generic message if no rule matches
    return ("I processed your query, but could not map it to a specific telemetry calculation.\n\n"
            "Here are some examples of what you can ask me:\n"
            "- *'What is the price of USDC?'*\n"
            "- *'Show the top 5 pools by volume'* \n"
            "- *'Get details for pool 0x326f7e37bef85c7f74a5a2b8c10b99a47e575cda'* \n"
            "- *'Show global summary'*")

def query_telemetry(query_str):
    pools, swaps, prices, reserves = load_data()
    
    # Check if langchain keys exist
    has_keys = any(os.environ.get(k) for k in ["GOOGLE_API_KEY", "GEMINI_API_KEY", "OPENAI_API_KEY"])
    
    if has_keys:
        try:
            # Calculate all fresh pool metrics
            pool_metrics = calculate_pool_metrics(pools, swaps, prices, reserves)
            NONUKES_PRICE = 1.74
            NONUKES_ADDR = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"
            nonukes_price = get_token_price(prices, NONUKES_ADDR) or NONUKES_PRICE

            # Define tools
            from langchain_core.tools import tool
            from langchain_core.messages import SystemMessage, HumanMessage, ToolMessage
            
            @tool
            def get_global_summary() -> str:
                """Get the global summary statistics of the NoNukes ecosystem (total pools, total liquidity, total volume, total swaps)."""
                tot_pools = len(pool_metrics)
                tot_vol = sum(p["volume_usd"] for p in pool_metrics.values())
                tot_swaps = sum(p["swap_count"] for p in pool_metrics.values())
                tot_liq = sum(p["liquidity_usd"] for p in pool_metrics.values())
                groups = set(p["target_group"] for p in pool_metrics.values())
                return json.dumps({
                    "total_pools": tot_pools,
                    "active_groups": list(groups),
                    "total_liquidity_usd": tot_liq,
                    "total_volume_usd": tot_vol,
                    "total_swaps": tot_swaps
                }, indent=2)

            @tool
            def search_pools_by_token(query: str) -> str:
                """Search for pools or tokens in the ecosystem by symbol, name, or contract address. Useful for finding what pools exist for a given token."""
                q_low = query.lower().strip()
                matched_pools = []
                for p in pool_metrics.values():
                    if (p["symbol"].lower() == q_low or 
                        p["name"].lower() == q_low or 
                        q_low in p["symbol"].lower() or 
                        p["other_addr"] == q_low or
                        p["address"] == q_low):
                        matched_pools.append({
                            "address": p["address"],
                            "symbol": p["symbol"],
                            "name": p["name"],
                            "version": p["version"],
                            "target_group": p["target_group"],
                            "liquidity_usd": p["liquidity_usd"],
                            "volume_usd": p["volume_usd"],
                            "swap_count": p["swap_count"]
                        })
                return json.dumps(matched_pools[:15], indent=2)

            @tool
            def get_pool_details(pool_address: str) -> str:
                """Get comprehensive metrics (reserves, volume, liquidity, swap count) for a specific pool contract address."""
                addr = pool_address.lower().strip()
                if addr in pool_metrics:
                    m = pool_metrics[addr]
                    other_price = get_token_price(prices, m["other_addr"])
                    return json.dumps({
                        "pool_address": addr,
                        "token_pair": f"NoNukes / {m['symbol']}",
                        "token_name": m["name"],
                        "version": m["version"],
                        "target_group": m["target_group"],
                        "nonukes_reserves": m["nonukes_reserves"],
                        "partner_reserves": m["partner_reserves"],
                        "liquidity_usd": m["liquidity_usd"],
                        "volume_usd": m["volume_usd"],
                        "swap_count": m["swap_count"],
                        "partner_token_price": other_price
                    }, indent=2)
                return f"Pool {pool_address} not found."

            @tool
            def get_top_pools(metric: str, limit: int = 5) -> str:
                """Get top pools sorted by a metric ('volume', 'liquidity', or 'swaps'). Limit defaults to 5."""
                metric_clean = metric.lower().strip()
                if "vol" in metric_clean:
                    key_fn = lambda x: x["volume_usd"]
                elif "liq" in metric_clean or "reserve" in metric_clean:
                    key_fn = lambda x: x["liquidity_usd"]
                else:
                    key_fn = lambda x: x["swap_count"]
                
                sorted_pools = sorted(pool_metrics.values(), key=key_fn, reverse=True)[:limit]
                simplified = [{
                    "address": p["address"],
                    "symbol": p["symbol"],
                    "version": p["version"],
                    "liquidity_usd": p["liquidity_usd"],
                    "volume_usd": p["volume_usd"],
                    "swap_count": p["swap_count"]
                } for p in sorted_pools]
                return json.dumps(simplified, indent=2)

            tools = [get_global_summary, search_pools_by_token, get_pool_details, get_top_pools]
            
            # Select LLM
            llm = None
            if os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY"):
                from langchain_google_genai import ChatGoogleGenerativeAI
                api_key = os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY")
                llm = ChatGoogleGenerativeAI(model="gemini-1.5-flash", google_api_key=api_key)
            elif os.environ.get("OPENAI_API_KEY"):
                from langchain_openai import ChatOpenAI
                llm = ChatOpenAI(model="gpt-4o-mini")
                
            if llm:
                llm_with_tools = llm.bind_tools(tools)
                
                system_instruction = (
                    "You are the NoNukes Dashboard AI Assistant, a telemetry advisor for the NoNukes PulseChain ecosystem. "
                    "You can answer questions about pools, token details, volumes, swaps, and reserves. "
                    "Use the provided tools to fetch precise data. Always present the results in clear, concise, "
                    "well-formatted Markdown. If a tool returns no data, explain that clearly."
                )
                
                messages = [
                    SystemMessage(content=system_instruction),
                    HumanMessage(content=query_str)
                ]
                
                response = llm_with_tools.invoke(messages)
                
                if response.tool_calls:
                    messages.append(response)
                    tool_map = {t.name: t for t in tools}
                    for tool_call in response.tool_calls:
                        tool_func = tool_map.get(tool_call["name"])
                        if tool_func:
                            tool_result = tool_func.invoke(tool_call["args"])
                            messages.append(ToolMessage(content=str(tool_result), tool_call_id=tool_call["id"]))
                    
                    final_response = llm_with_tools.invoke(messages)
                    return final_response.content
                else:
                    return response.content
        except Exception as e:
            # Fall back to rule-based on error
            pass
            
    # Fallback to rule-based system
    return rule_based_query(query_str, pools, swaps, prices, reserves)

if __name__ == "__main__":
    import sys
    query = " ".join(sys.argv[1:]) if len(sys.argv) > 1 else "global summary"
    print(query_telemetry(query))
