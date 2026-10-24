# What is reused, translated, and new

## Compiled original source

src/math/vec3.c and src/math/vec3_utils.c are copied from the original miniRT src/ files, with their original authorship headers and function bodies. The include changes from h_minirt.h to vec3.h to remove MLX/X11 dependencies.

These are C translation units, compiled as C and linked into the Windows C++ application. Their vec_add, vec_sub, vec_scale, vec_dot, vec_cross, vec_magnitude, and vec_norm functions are used in game physics and camera calculations. They are not unused reference copies.

src/math/vector.hpp adds small C++ conveniences plus a zero-length normalization guard. The original vec_norm implementation is retained.

## GPU adaptations of the original pipeline

The original draw_handler.c flow is recognizable as ray direction -> find_hit_normal -> phong -> output pixel. The GPU dispatch replaces the CPU pixel loops.

- shaders/direction.hlsli: rectilinear uses the R + U + forward construction from src/direction.c. This camera uses a vertical FOV; the aspect ratio is applied horizontally.
- shaders/intersections.hlsli: hit_sphere uses the stable q-form quadratic-root selection from src/discriminant_sphere.c. It uses GPU float arithmetic and does not port the CPU exponent-rescaling helper.
- shaders/phong.hlsli: named ambient_term, diffuse_term, and specular_term follow the structure and reflection formula in src/phong.c. The game uses a directional light, rather than the original linked list of point lights.
- shaders/ray_hit.hlsli: find_hit_normal selects the nearest hit and returns local per-ray normal/material values, rather than writing temporary hit data onto shared objects.

These are adaptations, not verbatim ports of the full renderer. Shader structures, triangle BVH traversal, message-card intersections, and presentation are new.

## Reused from the archived prototype

The tested D3D11 resource setup, framebuffer presentation, basic HUD drawing, mesh/BVH construction, and gameplay ideas were retained where useful, then separated and formatted. This is not a claim that every low-level graphics call was reinvented.

## New in this version

The application flow, content model, JSON loader, dynamic message IDs/history, per-course WAV loading and playback, asset reload, modular physics, and editing documentation.

## Not ported

MLX/window hooks, the .rt parser, original linked object lists and function-pointer dispatch, cylinders/cones/caps, bump maps, alternate camera projections, and the original interactive scene editor. Your original project and the archived attempt remain available alongside this version.
