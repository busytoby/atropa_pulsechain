# Semantic Tokenization via Accumulator Vector Fields

This document specifies how **Accumulator Vector Fields** are utilized to optimize semantic tokenization, attention weights, and context routing during database and RAG queries within the TSFi2 ecosystem.

---

## 1. Concept: Semantic Attention Accumulation

When a user query is parsed, standard tokenizers treat words as discrete, isolated indexes. By integrating **Accumulator Vector Fields**, we treat incoming query tokens as dynamic "forces" that pull or push the active context accumulator in a semantic vector space:

```
[Query Input String] --> [Token Parser] 
                              | (Tokens mapped to Attractors/Repellers)
                              v
                 [Accumulator Vector Field] ---> [Context Routing / RAG Query]
                 (Integrated Attention State)
```

### The Attention Integration Math
As each token $T_i$ is processed, it exerts a force vector $\vec{F}(T_i)$ based on its semantic embedding:
* **Attractors (Topic Anchors)**: Nouns, keywords, and explicit user goals pull the accumulator toward specific document categories (e.g., `"SID"`, `"Z-Machine"`, `"rigging"`).
* **Repellers (Negation/Exclude Tokens)**: Negation words (e.g., `"without"`, `"not"`, `"exclude"`) push the context accumulator away from matching categories.

---

## 2. Decaying Context & Slit Windows

To prevent older parts of a long query or conversation from diluting the active focus, we apply the **decaying history buffer** model to token weights:
* As new tokens are parsed, the attention force of past tokens decays at rate $\lambda$:
  $$W_{\text{attention}}(T_i, t) = W_{\text{max}}(T_i) - \lambda \cdot (t - t_{\text{parsed}})$$
* This ensures that the context vector $\vec{A}(t)$ remains highly focused on the active subject window, naturally handling topic shifts without hard context-window cutoffs.

---

## 3. Emulated Tokenizer Registers Map ($D6E0–$D6EF)

To run this semantic routing directly inside the virtual CPU memory map, we define the following interface:

| Address (Hex) | Dec Address | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D6E0` | `55008` | Read-Write | **Semantic Centroid X ($C_x$)**: Integrated topic coordinates. |
| `$D6E1` | `55009` | Read-Write | **Semantic Centroid Y ($C_y$)** |
| `$D6E2` | `55010` | Read-Only | **Active Category ID**: Resolved document namespace to query in RAG database. |
| `$D6E3` | `55011` | Write-Only | **Parse Trigger**: Writing `1` updates the centroid coordinates with the next token word. |

---

## 4. Query Routing Application

By using these accumulator coordinates:
1. **Semantic Hashing**: The final integrated coordinate $(C_x, C_y)$ is mapped to a document hash sector inside the on-chain registry, retrieving the most relevant lore files in milliseconds.
2. **Context-Aware Deduplication**: Duplicate concepts in queries cancel out dynamically (forces balancing to net zero), preventing redundant RAG retrieval loops.
