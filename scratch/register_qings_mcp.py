import json
import os
import re

def main():
    status_path = "config/nonukes_qings_status.json"
    mcp_server_path = "tsfi2-deepseek/src/tsfi_mcp_server.c"

    if not os.path.exists(status_path):
        print("Status file not found")
        return
    if not os.path.exists(mcp_server_path):
        print("MCP server file not found")
        return

    with open(status_path, "r") as f:
        status_data = json.load(f)

    # Collect all QING contract addresses that exist
    qings_to_register = []
    for item in status_data:
        if item.get("exists") and item.get("qing"):
            qings_to_register.append((item["symbol"], item["qing"]))

    print(f"Generating MCP server registration code for {len(qings_to_register)} QINGs...")

    with open(mcp_server_path, "r") as f:
        content = f.read()

    # Define the static list header and standard contracts
    contracts_c = [
        '    { "vmreq",        "../solidity/dysnomia/00b_vmreq.sol",              "0x10" }',
        '    { "shafactory",   "../solidity/dysnomia/02c_shafactory.sol",          "0x11" }',
        '    { "shiofactory",  "../solidity/dysnomia/03c_shiofactory.sol",         "0x12" }',
        '    { "yi",           "../solidity/dysnomia/04_yi.sol",                  "0x83a918056aB9316837Dc48a216119D679D561d91" }',
        '    { "zheng",        "../solidity/dysnomia/05_zheng.sol",               "0x14" }',
        '    { "zhou",         "../solidity/dysnomia/06_zhou.sol",                "0xe5d3A6e88590fc2A8037D9CCbd816C05B1ff5f11" }',
        '    { "yau",          "../solidity/dysnomia/07_yau.sol",                 "0xb9A44De20f26a027e467CB6c2F98766F01904189" }',
        '    { "yang",         "../solidity/dysnomia/08_yang.sol",                "0xFE9b99eCC43cb423408b975cc5ff439e5ABaCb61" }',
        '    { "siu",          "../solidity/dysnomia/09_siu.sol",                 "0xb4C1248812dAbF72cb2e82175b4c0aCffE4D2b10" }',
        '    { "void",         "../solidity/dysnomia/10_void.sol",                "0xCd19062a6d3019b02A676D72e51D8de7A398dE25" }',
        '    { "laufactory",   "../solidity/dysnomia/11c_laufactory.sol",         "0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3" }',
        '    { "lau",          "../solidity/dysnomia/11_lau.sol",                  "0x25" }',
        '    { "cho",          "../solidity/dysnomia/domain/dan/01_cho.sol",      "0x1b" }',
        '    { "map",          "../solidity/dysnomia/domain/map.sol",             "0x1c" }'
    ]

    # Add all qings
    for sym, addr in qings_to_register:
        # Sanitize symbol for contract name key
        clean_sym = re.sub(r'[^a-zA-Z0-9_]', '', sym)
        if not clean_sym:
            clean_sym = f"qing_{addr[2:8]}"
        name_key = f"qing_{clean_sym.lower()}_{addr.lower()}"
        contracts_c.append(f'    {{ "{name_key}", "../solidity/dysnomia/domain/dan/03_qing.sol", "{addr.lower()}" }}')

    # Add remaining system contracts
    contracts_c.extend([
        '    { "qi",           "../solidity/dysnomia/domain/soeng/01_qi.sol",     "0x1b8F9E19360D1dc94295D984b7Ca7eA9b810D9ee" }',
        '    { "mai",          "../solidity/dysnomia/domain/soeng/02_mai.sol",     "0xf69e9f943674027Cedf05564A8D5A01041d07c62" }',
        '    { "xia",          "../solidity/dysnomia/domain/soeng/03_xia.sol",     "0x347BC40503E0CE23fE0F5587F232Cd2D07D4Eb89" }',
        '    { "xie",          "../solidity/dysnomia/domain/soeng/04_xie.sol",     "0x2556F7f8d82EbcdD7b821b0981C38D9dA9439CdD" }',
        '    { "chan",         "../solidity/dysnomia/domain/sky/01_chan.sol",     "0x7674516ad438dd67A057fBc1119168d9A7d2a9B1" }',
        '    { "sei",          "../solidity/dysnomia/domain/tang/01_sei.sol",     "0x8B090509eAe0fEB4A0B934de1b4345161fA9a62d" }',
        '    { "choa",         "../solidity/dysnomia/domain/sky/02_choa.sol",     "0xA63F8061A67ecdbf147Cd1B60f91Cf95464E868D" }',
        '    { "cheon",        "../solidity/dysnomia/domain/tang/02_cheon.sol",    "0x840CBD20A70774BECAc4e932Fff6fb1f5417997F" }',
        '    { "cabsMarketMachine", "../solidity/dysnomia/domain/sky/CABSMarketMachine.sol", "0x74ef2B06A1D2035C33244A4a263FF00B84504865" }',
        '    { "zmachine",       "../solidity/bin/zmachine.yul",             "5" }',
        '    { "zmachineParser", "../solidity/bin/zmachineParser.yul",       "6" }'
    ])

    array_code = "static const DysnomiaContract g_dysnomia_system[] = {\n" + ",\n".join(contracts_c) + "\n};"

    # Replace in file using regex matching from static const DysnomiaContract g_dysnomia_system[] = { ... };
    new_content = re.sub(
        r'static const DysnomiaContract g_dysnomia_system\[\].*?};',
        array_code,
        content,
        flags=re.DOTALL
    )

    with open(mcp_server_path, "w") as f:
        f.write(new_content)

    print("Successfully generated and updated tsfi_mcp_server.c.")

if __name__ == "__main__":
    main()
