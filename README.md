# Real Hitbox — v0.1.0

Target:
- Geometry Dash Android 2.2.144
- Geode 1.8.0

## Current prototype

Editor only.

1. Select one or more objects.
2. Press the new `+` button on the left side of the editor.
3. A real Geometry Dash block object is created for every selected object.
4. The block is positioned, rotated and scaled to the source object's bounds.
5. Its rendering is hidden with opacity 0.
6. Collision comes from the real Geometry Dash block, not from custom player collision code.

## Important

This is the first prototype. It currently implements only BLOCK.

The next version can add:
- KILL
- a dedicated Hitbox menu
- exact object-bound calculation
- delete/replace generated hitboxes
- marking generated objects so they can be found later

## Build

Use the Geode SDK matching the target build and run:

    geode build -p android64

The resulting `.geode` file goes into the Geode Android mods directory.
