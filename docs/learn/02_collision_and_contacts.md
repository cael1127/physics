# 02 Collision And Contact Manifolds

Goal: study broadphase + narrowphase and how contact points feed the solver.

## Concepts

- Broadphase quickly finds likely pairs (`naive` or `sweep-and-prune`).
- Narrowphase computes manifold normal, penetration, and contact points.
- Two-point box manifolds improve stack stability over single-point contacts.

## Code Landmarks

- `src/rigid2d/fp2_broadphase.h`
- `src/rigid2d/fp2_broadphase.c`
- `src/rigid2d/fp2_collision.h`
- `src/rigid2d/fp2_collision.c`

## Mini Lab

1. Use scene 1 (stack) and enable contacts (`C`).
2. Observe normals at each contact point.
3. Compare behavior when switching broadphase mode in `Fp2WorldDesc`.

Expected result: same physical interactions with better scalability using sweep-and-prune.
