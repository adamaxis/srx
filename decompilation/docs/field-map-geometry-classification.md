# Field map geometry labels

The viewer's **Terrain / event bounds** layer describes source movement and
interaction behavior. It is not a visual object classifier. Building, road,
wall, crosswalk, shadow, sign, and decal outlines belong to reviewed entity
parts; collision rectangles alone do not establish those visible identities.

## ROM behavior used for the labels

- `0xE7C8` samples terrain and classifies the selected value. A value with the
  high bit set enters the map-interaction route; `0xB7B8` derives the event
  number from its low five bits.
- Values `0x60..0x7D` select directional terrain checks. `0xEAD2` chooses one
  of four edge relations from bits `0x38`, so the viewer calls these
  **Directional terrain edge** regions instead of guessing that they are
  walls or roads.
- `0x7E` takes the source's always-blocked branch. `0x7F` is returned as a
  blocked result and reaches the event-31 movement continuation.
- Low terrain values `1..0x1F` use `0xE9EE` to resolve a record in the terrain
  collision list and test the actor against that record's bounds. They are
  labeled **Object-bounds collision area**; they do not name a visible sprite.
- `0xEE10` rasterizes records in 32-unit steps with do-while loops. The first
  cell is written before the inclusive endpoint is checked. The viewer snaps
  each start to the same cell grid and preserves a one-cell span when an end
  precedes its start. This retains all 5,163 records, including the 13 records
  with at least one reversed endpoint, across all 56 maps.
- Against each composed map bitmap, 5,010 of those records are fully inside,
  104 cross a bitmap edge, and 49 lie entirely outside it. The viewer keeps
  the ROM-derived coordinates for all of them; it does not move collision
  bounds onto nearby artwork or treat an image edge as a ROM map boundary.
- For high-bit interaction records, the viewer joins the ROM record address to
  `field-map-trigger-records.tsv` and the scene's `*-site-routes.tsv`. A
  profiled name and destination are shown when present, with guide-only or
  candidate names explicitly marked.

The numeric source value, rectangle coordinates, record offset, and ROM
routine addresses remain in the selected-region details for audit work. These
labels make the collision behavior readable without treating a table index as
an object name. They do not yet classify every decorative pixel or replace
manual visual review of the 56 map images.
