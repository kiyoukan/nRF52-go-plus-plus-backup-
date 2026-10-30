# External Flash Cheat Sheet

## Main regions

| Region | Offset | Length | Purpose |
|---|---:|---:|---|
| `FBCT` factory/auth record | `0x2000` | `0x1a2` used | donor auth identity |
| persisted `.SAV` image | `0x3000` | `0x288` | donor pairing/reconnect state |

## Required donor data

| Item | Offset | Length | Notes |
|---|---:|---:|---|
| donor public BLE address | internal `nRF52832` `UICR` `0x10001080` (separate from external flash) | `6` bytes | required; scan over BLE or see [`swd_mac_recovery.md`](swd_mac_recovery.md) |
| selector 2 / cert block | `0x2006` | `0x100` | required |
| selector 3 / key lane | `0x2106` | `0x10` | required |

## Recommended donor data

| Item | Offset | Length | Notes |
|---|---:|---:|---|
| full persisted `.SAV` image | `0x3000` | `0x288` | recommended |
| persisted selector 4 | `0x306c` | `0x10` | reconnect/finalization |
| persisted selector 5 | `0x307c` | `0x01` | reconnect/finalization |
| persisted selector 6 | `0x307d` | `0x0a` | reconnect/finalization |

## `FBCT` quick check

| Field | Value |
|---|---|
| base offset | `0x2000` |
| magic | `46 42 43 54` |

## `.SAV` quick check

| Field | Value |
|---|---|
| base offset | `0x3000` |
| length | `0x288` |
| magic at start | `18 53 41 56` |

## Short version

- required: BLE address, selector 2, selector 3
- recommended: full `.SAV`
- auth identity lives at `FBCT @ 0x2000`
- reconnect state lives at `.SAV @ 0x3000`
