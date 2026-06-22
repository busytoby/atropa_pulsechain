# On-Chain ZIL Parser Specification

To fully support ZIL-like parser mechanics on-chain, we will implement a command parser in Yul that reads the player's text command, parses it against the game's dictionary, and routes it to object interactions.

---

## 1. ZIL Command Parsing Flow

```
User Input: "take gold"
    |
    v
Tokenization (Split by space/punctuation)
    |
    v
Dictionary Lookup (Check if word is in Dictionary Table)
    |
    v
Grammar Matching (Resolve Verb + Object)
    |
    v
Object Interaction (Check properties, attributes, and ERC-20 balances)
```

---

## 2. Byte-Level Dictionary Parsing in Yul

To resolve input words:
1.  **Word Splitting**: The parser walks the input string in memory, splitting at space `0x20` and punctuation characters.
2.  **Z-Encoding**: For each parsed word, we compress it into the 5-bit ZSCII format (representing up to 6 characters packed into 4 bytes) so it can be compared directly to the dictionary entries.
3.  **Dictionary Lookup**: We binary search the Dictionary table base pointer for the matching Z-encoded word.
4.  **Grammar Matching**:
    *   If a Verb is matched, we identify the syntax.
    *   If a Noun is matched, we search the surrounding objects in the room to find the corresponding Object ID.

---

## 3. ERC-20 Integration Route

Once a Verb and Noun are successfully matched:
*   Action: `take coin` (where `coin` is Object 50 bound to an ERC-20 contract).
*   The parser triggers the transaction wrapper:
    *   Verifies that the object has the `TAKEABLE` attribute.
    *   Verifies/transfers the corresponding ERC-20 token rewards or fees.
    *   Updates the sibling/parent pointers in the on-chain Object tree state.

---

## 4. ZIL & WinchesterMQ Integration for 4D DNA Assets

ZIL commands compile directly into state mutation transactions that govern 4D DNA assets. By mapping packed Z-encoded noun properties to WinchesterMQ register offsets:
1. **DNA Codification**: Tesseract collection cards store their 4D vertices and rotation vectors in the ZIL object property table.
2. **MQ Interaction Routing**: Command verbs (e.g., `sabotage`, `spin`) write payload data directly to WinchesterMQ register interfaces (such as `yulStorage[100]` for SCSI phases), initiating low-level hardware state-machine changes and visual deformation feedback loops.

---

## Next Steps
1.  **Implement `parseCommand` in `zmachine.yul`**: Add string parsing and dictionary matching loops.
2.  **Build Integration Test**: Verify inputs like "take coin" on-chain and watch ERC-20 balances update dynamically.
