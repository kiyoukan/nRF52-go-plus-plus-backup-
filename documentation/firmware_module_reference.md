# Firmware Module Reference

Date: 2026-05-19

Primary firmware:

`code\nrf_emulator\pgpemu_nrf52_plusplus_exp`

This document summarizes the firmware structure and the app-visible protocol
flows. It is a reference for the current source tree, not a development log.

## Current Protocol Flow

First pair:

1. BLE topology exposes the Plus+ service set and pairable advertising.
2. Service C/Auth writes stage and commit a pair.
3. Auth `24/25/18/16/36/28` persists reconnect-critical state.
4. Older Service D completes certification.
5. Auth `26` completes the pair and enters authenticated/reconnect-ready state.
6. First authenticated disconnect arms initial unread sleep generation.

Reconnect:

1. Boot or prior auth restores authenticated/reconnect-ready state.
2. Auth `24 01` is rebuilt from the persisted accepted nonce.
3. Auth `25` exposes reconnect auth blob material and arms compact Service D.
4. Compact Service D completes through type `4`, type `5`, and type `3-done`.
5. ServiceB char1 is left readable as `04`; reconnect-ready `01` notify is suppressed for Device B parity.
6. Sleep export is handled later over authenticated auth-service opcodes.

## Module Ownership

| Area | Primary files | Owns |
|---|---|---|
| BLE topology and dispatch | `pp_ble_topology.*`, `pp_ble_dispatch.*`, `pp_ble_refs.h` | Services, characteristics, CCCD/write/read dispatch |
| Connection routing | `pp_connection_coordinator.*` | Auth write routing, Service D entry routing |
| First-pair flow | `pp_first_pair_flow.*`, `pp_pair_first_time.*` | First-pair auth sequence, Service C burst, auth completion |
| Reconnect flow | `pp_reconnect_flow.*`, `pp_pair_reconnect.*` | Reconnect `24/25`, compact Service D entry |
| Auth protocol | `pp_auth_protocol.*`, `pp_auth_blob.*`, `pp_auth_material.*` | Auth opcode classification, reply staging, auth blob material |
| Service D protocol | `pp_service_d_protocol.*`, `pp_service_d_local.*` | Older lane, compact lane, typed write validation, Service D status |
| Sleep export and policy | `pp_sleep_export.*`, `pp_sleep_policy.*` | Authenticated sleep opcodes, unread generation policy, stream chunks |
| Pair state and persistence | `pp_pair_state.*`, `pp_persistence.*`, `pp_storage_refs.h` | Pair/reconnect state, persisted accepted nonce, Service D records |
| Runtime button/Service E | `pp_button_runtime.*` | Post-auth button/runtime behavior |
| Logging | `pp_log.h`, `pp_debug_ble.h`, `pp_debug_utils.*` | Compile-time protocol/verbose/raw BLE logging policy |

`pp_wiring.h` is a small coordinator header. Most `.ino`-local reference
construction lives in `pp_flow_refs.h` and `pp_storage_refs.h`.

## Device-Specific Material

Device-specific material is supplied by local `pp_device_material.h` and is not
included in this repository.

Required or device-derived material:

- public BLE address
- cert block / selector 2
- selector 3 key lane
- selector 6 factory seed
- persisted Service D selector/record state when available
- persisted accepted nonce after a successful first pair

Common or protocol-level behavior:

- service UUID topology
- auth opcode order and reply classes
- older Service D lane shape
- compact reconnect Service D lane shape
- authenticated sleep opcode structure
- ServiceB Device B parity behavior after compact Service D

Still inferred:

- exact semantic names for some auth worker events
- exact compact Service D type4 derivation semantics
- full meaning of every sleep selector record field

## Logging Levels

Default logs are protocol-level.

Levels are defined in `pp_log.h`:

- `PP_LOG_LEVEL_DISABLED`
- `PP_LOG_LEVEL_PROTOCOL`
- `PP_LOG_LEVEL_VERBOSE`
- `PP_LOG_LEVEL_RAW_BLE`

Verbose and raw BLE levels expose byte-level behavior.
