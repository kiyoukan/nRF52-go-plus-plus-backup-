# External Flash Reference

This document is a short reference for the donor device's **external flash image** and the parts of that image that matter for this project.

## What this file is

The external flash image is the most important donor file for this project. It contains:

- the factory/auth record
- the donor cert material
- the donor key material
- the persisted pairing/reconnect state

For device-material setup, this is the main file used by the public tooling.

## Important regions

The two main regions are:

- `0x2000`: `FBCT` factory/auth record
- `0x3000`: persisted `.SAV` image

## `FBCT` factory/auth record

The `FBCT` record begins at `0x2000`.

Important details:

- base offset: `0x2000`
- magic: `46 42 43 54` (`FBCT`)

This record contains the donor's factory auth identity material.

### Required `FBCT` data

These are the most important pieces to read from `FBCT`:

| Item | Offset | Length | Why it matters |
|---|---:|---:|---|
| selector 2 / cert block | `0x2006` | `0x100` | visible auth identity and cert container data |
| selector 3 / key lane | `0x2106` | `0x10` | auth and reconnect key material |

### Supporting `FBCT` data

These regions are useful context, but are not usually the first things a ready-to-go repo would ask a user for:

| Item | Offset | Length |
|---|---:|---:|
| selector 4 factory aux | `0x2116` | `0x0c` |
| selector 5 factory aux | `0x2122` | `0x80` |

## Persisted `.SAV` image

The persisted `.SAV` image begins at `0x3000`.

Important details:

- base offset: `0x3000`
- length: `0x288`
- expected magic at the start: `18 53 41 56`

This region holds donor-specific saved state used for pairing finalization and reconnect behavior.

### Recommended `.SAV` data

If the goal is to get behavior closer to the original donor device, this region is also useful:

| Item | Offset | Length | Why it matters |
|---|---:|---:|---|
| full persisted `.SAV` image | `0x3000` | `0x288` | donor pairing/reconnect state |
| persisted selector 4 | `0x306c` | `0x10` | Service D / reconnect state |
| persisted selector 5 | `0x307c` | `0x01` | Service D / reconnect state |
| persisted selector 6 | `0x307d` | `0x0a` | Service D / reconnect state |

## Minimum useful readout set

If a repo is already complete and only needs donor material plugged into it, the minimum useful readout set is:

1. donor public BLE address
2. `FBCT` selector 2 from `0x2006` length `0x100`
3. `FBCT` selector 3 from `0x2106` length `0x10`

For better reconnect fidelity, also read:

4. persisted `.SAV` image from `0x3000` length `0x288`

## Short summary

Shortest summary:

- the donor auth identity lives in external flash `FBCT @ 0x2000`
- the most important required pieces are selector 2 at `0x2006` and selector 3 at `0x2106`
- the donor reconnect/persistence state lives in external flash `.SAV @ 0x3000`
