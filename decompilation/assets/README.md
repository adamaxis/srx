# Direct Shadowrun assets

Generated from the US Genesis ROM by shadowrun_graphics_extract --all-assets.

The native runtime consumes these individually named BMPs and native/state.bin and resources.tsv directly. This tree deliberately contains semantic folders: menu, intro, field, and item categories. Static intro plates and independently extracted intro sprites remain separate; it does not contain full-screen flipbook samples or an aggregate serializer. Item art is grouped under items/weapons, items/first-aid, items/miscellaneous, items/magic, and items/armor. Separately traced cinematic resources belong under movies/<scene>; they must not be mixed into intro or field assets.

Genesis audio is not emitted here until its Z80 event format is decoded into complete usable files. Raw ROM slices and split event fragments are analysis inputs, not runtime assets.
