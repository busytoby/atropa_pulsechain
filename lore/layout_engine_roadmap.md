# Custom Browser Layout Engine Roadmap

This roadmap outlines the steps to build a native, lightweight HTML/CSS layout and rendering engine integrated with V8 and our Vulkan presenter (`rooted_frame_presenter`).

---

## Phase 1: Core Parsing & DOM Tree (C/C++)
Build the foundations to ingest HTML and CSS into structured memory.

1. **HTML Parser & DOM Builder** [COMPLETED]
   - Implement a robust, non-backtracking tokenizer/parser for HTML5 subsets.
   - Construct the Document Object Model (DOM) tree with parent/child links and attributes.
2. **CSS Parser & Cascade Engine**
   - Parse CSS rules, properties, and values (colors, units like `px`, `%`, `em`, `auto`).
   - Implement selector matching (tag, class, ID, hierarchical selectors) and cascade/specificity resolution.
3. **DOM & Style Integration**
   - Compute final CSS styles for each DOM node (inheritable properties, defaults).

---

## Phase 2: Layout & Box Model Calculations
Calculate spatial coordinates for all visual elements.

1. **Render/Box Tree Construction** [COMPLETED]
   - Generate layout boxes (block, inline, inline-block) from the styled DOM tree.
   - Handle hidden elements (`display: none`).
2. **Reflow & Geometry Solver** [COMPLETED]
   - Calculate box coordinates ($x, y$) and sizes ($w, h$) using CSS box model rules.
   - Implement basic Flexbox and Flow layouts.
3. **Text Wrapping & Font Shaping** [COMPLETED]
   - Wrap inline text within box bounds.
   - Integrate with the existing text and font parser/rasterizer (e.g. `tsfi_font_engine.o`) to calculate character advancements.

---

## Phase 3: Visual Compositing & Rasterization (Vulkan)
Paint the layout tree directly into the presenter's scanout buffer.

1. **Paint Tree Generation** [COMPLETED]
   - Gather visual drawing operations (background color, border, text, JPEG/PNG images) sorted by `z-index`.
2. **2D Rendering Pipeline** [COMPLETED]
   - Implement pixel-blitting functions for colored rectangles, borders, and text drawing.
   - Integrate existing JPEG image loading for thumbnails and assets.
3. **Vulkan Presenter Integration** [COMPLETED]
   - Replace the Puppeteer frame loop in `rooted_frame_presenter` with our custom page layout + render engine pipeline.

---

## Phase 4: V8 Binding & DOM API Runtime
Expose the DOM to JavaScript so YouTube's frontend scripts can run.

1. **V8 DOM Bindings** [COMPLETED]
   - Wrap C++ DOM and style objects into V8 JavaScript classes.
   - Expose core APIs: `document.getElementById`, `window.addEventListener`, etc.
2. **Event Dispatch Loop** [COMPLETED]
   - Map low-level input events (mouse move, clicks, keyboard) from our presenter's input loop into JavaScript events (`mousedown`, `keydown`).
3. **HTTPS Client & Network Stack**
   - Create a clean C/C++ network transport helper to handle HTTPS requests and feed resources directly to the browser.

---

## Phase 5: Media Stream Rendering
Support video playback and audio playback.

1. **HTML5 `<video>` and `<audio>` Nodes**
   - Implement rendering boxes that hold video frame buffers and sync with audio streams.
2. **Streaming Protocol Support (DASH/HLS)**
   - Parse and fetch dynamic adaptive media segments.
3. **Native Media Decoding**
   - Parse and decode H.264/VP9/AV1 video frames and AAC/Opus audio packets natively using our libraries.

---

## Phase 6: Security, Storage & Sandbox
Handle browser storage, security, and sessions.

1. **Storage APIs**
   - Implement cookies, `localStorage`, and `sessionStorage` backing stores.
2. **Secure HTTPS & TLS**
   - Enforce TLS certificate validation, cookie security flags, and cross-origin resource sharing (CORS) rules.
3. **User Authentication**
   - Ensure secure sign-in states are maintained for YouTube subscriptions and user data.
