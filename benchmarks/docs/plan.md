Common UX methods for displaying Pin Lu / IME suggestions:
1.  **Horizontal Strip (Inline/Floating):** The most common method. A horizontal bar appears either inline with the text (true inline) or floating just below/above the cursor. It displays candidates side-by-side.
    *   *Pros:* Minimal vertical space, feels native to left-to-right typing.
    *   *Cons:* Limited horizontal space restricts the number of visible candidates (usually 5-9).
2.  **Vertical List (Dropdown):** A vertical menu drops down from the cursor position.
    *   *Pros:* Can display more candidates or longer candidate strings clearly. Easier scanning for complex glyphs.
    *   *Cons:* Can obscure more of the underlying text.
3.  **Grid / Matrix:** Often used for symbol selection or when the candidate list is very large and the user requests an expanded view (e.g., pressing an "expand" button).
    *   *Pros:* High density of options.
    *   *Cons:* High cognitive load, requires two-dimensional scanning.
4.  **Split View (Composition + Selection):** One area shows the current phonetic composition (e.g., "p-i-n-l-u"), and an adjacent area shows the selectable geometric candidates.
5.  **Focus / Highlighting:** The "best guess" or first candidate is visually highlighted or pre-selected.
6.  **Pagination Indicators:** Clear visual cues (arrows, dots, "1/X") indicating that more candidates exist beyond the current view, accessible via paging keys (PageUp/PageDown, -/=, etc.).
7.  **Numeric Shortcuts:** Small numbers (1-9) placed next to or above each candidate to allow rapid keyboard selection without mouse interaction or arrow key navigation.
