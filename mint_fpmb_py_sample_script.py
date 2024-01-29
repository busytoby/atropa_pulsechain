from web3 import Web3, HTTPProvider
import time

# Connect to the PulseChain network
w3 = Web3(HTTPProvider('https://rpc.pulsechain.com'))
assert w3.is_connected()

# Your account details
private_key = 'YOUR_PRIVATE_KEY'
my_account = w3.eth.account.privateKeyToAccount(private_key)

# FPM Bond contract address
fpmb_contract_address = '0x25d53961a27791b9d8b2d74fb3e937c8eaeadc38'

# Token contract addresses and amounts
tokens = {
    '0x0b1307dc5D90a0B60Be18D2634843343eBc098AF': 1,  # LEGAL
    '0xFa4d9C6E012d946853386113ACbF166deC5465Bb': 500, # ㈝
    '0x271197EFe41073681577CdbBFD6Ee1DA259BAa3c': 900, # 籯
    '0xA63F8061A67ecdbf147Cd1B60f91Cf95464E868D': 999, # ލ
    '0xCc78A0acDF847A2C1714D2A925bB4477df5d48a6': 313, # Atropa
    '0x463413c579D29c26D59a65312657DFCe30D545A1': 100000, # TREASURY BILL ㉾
    '0x2959221675bdF0e59D0cC3dE834a998FA5fFb9F4': 196.774066874365847631 # BULLION ⑧
}

# Function to send transaction
def send_transaction(w3, account, to, data, value=0):
    tx = {
        'from': account.address,
        'to': to,
        'value': value,
        'gas': 500000,
        'gasPrice': w3.eth.gasPrice,
        'nonce': w3.eth.getTransactionCount(account.address),
        'data': data,
        'chainId': w3.eth.chainId
    }
    signed_tx = account.signTransaction(tx)
    tx_hash = w3.eth.sendRawTransaction(signed_tx.rawTransaction)
    receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
    return receipt

# Infinite approval amount
infinite_approval = (2**256 - 1).to_bytes(32, byteorder='big')

# ERC-20 Approve and Transfer function selectors
approve_selector = '0x095ea7b3'
transfer_selector = '0xa9059cbb'

# Approve each token infinitely
for token_address in tokens.keys():
    data = approve_selector + w3.toBytes(hexstr=fpmb_contract_address[2:]).rjust(32, b'\x00') + infinite_approval
    send_transaction(w3, my_account, token_address, data)

# Transfer tokens to FPM Bond contract and call mint function
mint_selector = '0x467c4e68'
for token_address, amount in tokens.items():
    # Transfer tokens
    transfer_data = transfer_selector + w3.toBytes(hexstr=fpmb_contract_address[2:]).rjust(32, b'\x00') + w3.toBytes(amount).rjust(32, b'\x00')
    send_transaction(w3, my_account, token_address, transfer_data)

# Call mint function
mint_receipt = send_transaction(w3, my_account, fpmb_contract_address, mint_selector)

print("All transactions processed. Minting transaction receipt:", mint_receipt.transactionHash.hex())
