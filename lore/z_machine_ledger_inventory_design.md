# Z-Machine Style Ledger Inventory System (Bags & Rooms)

We can bridge Z-machine text-adventure concepts (hierarchical object trees of rooms, bags, containers, and items) with our on-chain accounting ledger. This allows ledger accounts (including user addresses) to act as dynamic physical locations or inventory containers holding ERC20 alchemical assets.

---

## 1. The Z-Machine Object Tree Model in EVM Storage

In the Z-machine, every entity (a room, a player, a chest, a gold coin) is an **Object** in a single unified tree. Objects are linked via three pointers:
1.  **Parent**: What container or room contains this object?
2.  **Sibling**: What is the next object in the same container?
3.  **Child**: What is the first object inside this container?

```
               [ Room: "Central Treasury" (1200) ]
                            | (Child)
                            v
               [ Container Bag: "Lead Vault" (1101) ]
                            | (Child)               \ (Sibling)
                            v                        v
             [ ERC20 Asset: Lead Token (Pb) ]    [ Chest Key (Item) ]
```

We can map this directly into Yul storage slots using nested hashes:
*   `parentOf(objectId) -> parentId`
*   `childOf(objectId) -> childId`
*   `siblingOf(objectId) -> siblingId`

---

## 2. On-Chain Yul Storage Layout for Containers

Inside the `ZarrellaLedger` or a wrapper contract, we define the following slots:

| Function | Storage Slot Key Formulation | Purpose |
| :--- | :--- | :--- |
| `parent(obj)` | `keccak256(obj, 90001)` | Returns the parent container object ID. |
| `child(obj)` | `keccak256(obj, 90002)` | Returns the first child object ID. |
| `sibling(obj)` | `keccak256(obj, 90003)` | Returns the next sibling object ID. |
| `quantity(obj)` | `keccak256(obj, 90004)` | If the object represents an ERC20/reagent quantity. |

---

## 3. Inventory Modification & Transfer Mechanics (Move/Insert)

When a player "mints" or "transfers" a reagent, instead of just modifying a flat mapping, we perform a Z-machine tree manipulation:

### The `move(object, new_parent)` operation:
1.  **Unlink from old parent**:
    *   Find the parent of `object`.
    *   Walk the sibling chain of the parent's `child` to find `object` and patch the sibling pointers (bypass `object`).
2.  **Link to new parent**:
    *   Set `parent(object) := new_parent`.
    *   Set `sibling(object) := child(new_parent)` (prepend to the new sibling chain).
    *   Set `child(new_parent) := object`.

```javascript
function moveObject(obj, newParent) {
    let oldParent := getParent(obj)
    if oldParent {
        // Unlink logic...
        let prev := 0
        let curr := getChild(oldParent)
        while curr {
            if eq(curr, obj) {
                if prev {
                    setSibling(prev, getSibling(obj))
                }
                if iszero(prev) {
                    setChild(oldParent, getSibling(obj))
                }
                break
            }
            prev := curr
            curr := getSibling(curr)
        }
    }
    
    // Link to new parent
    setParent(obj, newParent)
    setSibling(obj, getChild(newParent))
    setChild(newParent, obj)
}
```

---

## 4. ERC20 Compatibility Overlay

Under this architecture, when MetaMask queries `balanceOf(user)`, the ledger behaves as follows:
1.  We lookup the `user` address as a container node.
2.  We iterate through the user's `child` and `sibling` chains to look for child objects representing alchemical ERC20 tokens (e.g. Lead, Mercury, Sulfur).
3.  We sum up the quantities of those specific token objects and return the result.
4.  Standard ERC20 `transfer(to, amount)` is translated to moving/splitting quantity nodes inside the recipient's "room" or "bag" container!
