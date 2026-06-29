# Phase 2 Layout Engine Unit Test Scope

This plan scopes the verification checks and scenarios required to guarantee the correctness of our layout solver under Phase 2.

---

## 1. Structural Box Model Tests
Validate that margins, padding, borders, and content sizes are calculated correctly.
- **Box Size Invariance**: Verify that for a box with width $W$, padding $P$, border $B$, and margin $M$, its computed outer bounding box matches $W + 2P + 2B + 2M$.
- **Auto Width/Height Fallbacks**: Ensure elements without explicit widths fill their parent container's width, and elements without explicit heights shrink to wrap their children.

## 2. Reflow & Layout Flow Tests
Validate element positioning under standard document flow rules.
- **Block Layout (Vertical Stacking)**: Verify that subsequent block elements are positioned below previous block elements ($y_2 = y_1 + h_1 + \text{margins}$).
- **Inline Layout (Line Wrapping)**: Verify that inline text nodes flow horizontally and wrap onto a new line when they exceed the parent container's boundary.
- **Flexbox Constraints (Horizontal & Vertical)**: Check that children of elements styled with `display: flex` correctly align, distribute extra space, or shrink when constrained.

## 3. Style Cascade & Hidden Elements
Validate that computed style flags are honored during box layout tree generation.
- **Display None Exclusion**: Ensure that any DOM node with `display: none` does not generate a layout box and is excluded from reflow geometry calculations.

---

## 4. Test Harness Design
Since we have not integrated the Vulkan rasterizer yet, the unit tests for Phase 2 will execute *headless structural validations*:
- Parse HTML & style strings.
- Run the Layout Reflow solver.
- Programmatically assert coordinates ($x, y, w, h$) of DOM nodes against expected geometric values.
