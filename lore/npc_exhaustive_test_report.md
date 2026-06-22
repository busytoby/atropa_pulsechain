# Exhaustive NPC Traversal & Z-Machine Testing Report

**Testing Signer/Deployer**: `0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266`
**NPC Character Address**: `0x0000000000000000000000000000000000000003`
**Z-Machine Contract Address**: `0xe6dDcC5353d6d67BDCef42430F1e3c11335823B8`
**Z-Machine Parser Address**: `0x0165eF489a48135909eA9bAbC9b263aD78aF04C8`

## Travis Map Configuration

| Room ID | Name | Exits Mapping | Custom Description |
|---|---|---|---|
| 10 | Grand Lobby | North -> 11, South -> 13, East -> 12 | You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. |
| 11 | Dusty Library | South -> 10, East -> 14 | You enter the Dusty Library. Floor-to-ceiling bookshelves are packed with ancient spellbooks. |
| 12 | Royal Garden | West -> 10 | You step into the Royal Garden. Exotic flowers bloom under the sunshine. |
| 13 | Cold Dungeon | North -> 10 | You creep into the Cold Dungeon. Chains hang from stone walls, and it smells damp. |
| 14 | Golden Treasury | West -> 11 | You discover the Golden Treasury! Shimmering coins and glowing gems pile high. |

## Exhaustive Traversal Log

| Step | Command | From Room | To Room | Description | Gas Used | Transaction Hash |
|---|---|---|---|---|---|---|
| 1 | **north** | 10 | 11 | You enter the Dusty Library. Floor-to-ceiling bookshelves are packed with ancient spellbooks. A door leads East. | 201970 | [`0x5a84da3a...`](https://etherscan.io/tx/0x5a84da3a27ef68cf664988583b4f51374ae187917ca74bf7cf6a7e60eb5b81c8) |
| 2 | **east** | 11 | 14 | You discover the Golden Treasury! Shimmering coins and glowing gems pile high. An archway leads West. | 49312 | [`0x10804c7d...`](https://etherscan.io/tx/0x10804c7d07e4cc569f111a590c82081d4980b4e7877634c3330ceda09e881e83) |
| 3 | **west** | 14 | 11 | You enter the Dusty Library. Floor-to-ceiling bookshelves are packed with ancient spellbooks. A door leads East. | 49328 | [`0xb7e29caa...`](https://etherscan.io/tx/0xb7e29caa59c70138e16a2c486a194f763734524f5022a5edc2acdf2b182872aa) |
| 4 | **south** | 11 | 10 | You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. Arches lead North, South, and East. | 49302 | [`0x8067e10c...`](https://etherscan.io/tx/0x8067e10c7285e0f0faf263807a3cb0086ec91669b940ec453a3e5e3d3a8f1ead) |
| 5 | **east** | 10 | 12 | You step into the Royal Garden. Exotic flowers bloom under the sunshine. A path leads back West. | 47148 | [`0xd037b68b...`](https://etherscan.io/tx/0xd037b68b9702e45a328cf1b35fc574afe15f371d5246ba16c9370330e713c3ec) |
| 6 | **west** | 12 | 10 | You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. Arches lead North, South, and East. | 49328 | [`0xafe10325...`](https://etherscan.io/tx/0xafe10325583d53366208fbc5ca1368dc83b353142b64806a6191f36f2551d591) |
| 7 | **south** | 10 | 13 | You creep into the Cold Dungeon. Chains hang from stone walls, and it smells damp. An exit goes North. | 49302 | [`0x12d3bcb2...`](https://etherscan.io/tx/0x12d3bcb2ce146194277fcec86bd20df759965f939752fe87ea6254fc5754e13b) |
| 8 | **north** | 13 | 10 | You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. Arches lead North, South, and East. | 49280 | [`0x9865f3e9...`](https://etherscan.io/tx/0x9865f3e9de20c58eef4ad844961a92d9177e75794308f43d44a54e3ab6a125b3) |
| 9 | **north** | 10 | 11 | You enter the Dusty Library. Floor-to-ceiling bookshelves are packed with ancient spellbooks. A door leads East. | 49280 | [`0x0e37f4fb...`](https://etherscan.io/tx/0x0e37f4fb99b67936926a1e943a98dcb7c6c130049c0f26c90a67eedd90f22407) |
| 10 | **east** | 11 | 14 | You discover the Golden Treasury! Shimmering coins and glowing gems pile high. An archway leads West. | 49312 | [`0x63af6935...`](https://etherscan.io/tx/0x63af693548bd8912fdeae671f317380cbe272d8da363294776b633d062a50021) |
| 11 | **west** | 14 | 11 | You enter the Dusty Library. Floor-to-ceiling bookshelves are packed with ancient spellbooks. A door leads East. | 49328 | [`0x4d7f7f42...`](https://etherscan.io/tx/0x4d7f7f4215f30f4fb0a2581d4151663527ed99500727e52aba840366d1cfe4d7) |
| 12 | **south** | 11 | 10 | You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. Arches lead North, South, and East. | 49302 | [`0xc175f6ba...`](https://etherscan.io/tx/0xc175f6ba802f933c77b8eecabe5bb0627975ceb7286b7ee3fee4664fb205028a) |
| 13 | **east** | 10 | 12 | You step into the Royal Garden. Exotic flowers bloom under the sunshine. A path leads back West. | 47148 | [`0x087209e9...`](https://etherscan.io/tx/0x087209e9a1b037533ba2c97a2b5b0d5ef874d17bb133b9fddbbf36627250755c) |
| 14 | **west** | 12 | 10 | You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. Arches lead North, South, and East. | 49328 | [`0x85b7587c...`](https://etherscan.io/tx/0x85b7587c6914da0d55029b7c97f7435c9812e8670ce443d0abee950b3d22692d) |
| 15 | **look** | 10 | 10 | [ZMM] No contract bound to this room.
You stand in the Grand Lobby. Tall marble pillars support a vaulted ceiling. Arches lead North, South, and East. | 45211 | [`0x5b6c5ce9...`](https://etherscan.io/tx/0x5b6c5ce9da36c6b815aced963f9b621509b1e43c0b84c498092a6bd2b2771117) |

## Test Summary

> [!NOTE]
> The Z-Machine parser successfully resolved all 15 directions and returned the corresponding room description assets from dynamic contract storage.
> The NPC successfully navigated all 5 designated room indices with no reverts or state corruption.
