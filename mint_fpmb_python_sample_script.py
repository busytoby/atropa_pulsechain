from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware
import hashlib

# Connect to PulseChain node
web3 = Web3(HTTPProvider("https://rpc.pulsechain.com"))
web3.middleware_onion.inject(geth_poa_middleware, layer=0)

# Function to get function selector
def get_function_selector(function_signature):
    return web3.keccak(text=function_signature)[:4]

# Your wallet details
my_address = '0xYourAddress'
private_key = 'YourPrivateKey'

# Contract addresses
token_contracts = {
    'token_7': '0xToken7ContractAddress',
    'token_b': '0xTokenBContractAddress',
    'token_c': '0xTokenCContractAddress',
    # ... Add other token addresses here
}

# Mint contract address
mint_contract_address = '0xMintContractAddress'

# Function selectors
approve_selector = get_function_selector('approve(address,uint256)')
mint_selector = get_function_selector('mint()')  # Replace with actual mint function signature

# Approve each token infinitely
for token, address in token_contracts.items():
    nonce = web3.eth.getTransactionCount(my_address)
    txn = {
        'to': address,
        'value': 0,
        'gas': 200000,
        'gasPrice': web3.toWei('10', 'gwei'),
        'nonce': nonce,
        'data': approve_selector + web3.toBytes(hexstr=mint_contract_address[2:]).rjust(32, b'\0') + web3.toBytes(hexstr=hex(2**256 - 1)[2:]).rjust(32, b'\0')
    }
    signed_txn = web3.eth.account.signTransaction(txn, private_key)
    tx_token = web3.eth.sendRawTransaction(signed_txn.rawTransaction)
    print(f"Approved {token}, tx hash: {tx_token.hex()}")

# Directly call mint function
nonce = web3.eth.getTransactionCount(my_address) + len(token_contracts)
mint_txn = {
    'to': mint_contract_address,
    'value': 0,
    'gas': 200000,
    'gasPrice': web3.toWei('10', 'gwei'),
    'nonce': nonce,
    'data': mint_selector
}
signed_mint_txn = web3.eth.account.signTransaction(mint_txn, private_key)
tx_mint = web3.eth.sendRawTransaction(signed_mint_txn.rawTransaction)
print(f"Mint transaction sent, tx hash: {tx_mint.hex()}")
