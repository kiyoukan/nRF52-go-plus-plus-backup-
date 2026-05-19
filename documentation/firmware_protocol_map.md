# Firmware Protocol Map

Date: 2026-05-18

This map documents the app-visible BLE and protocol values used by the current firmware:

`nRF52 Plus+ backup emulator, 2026-05-19`

For module ownership and validation notes, see `documentation\firmware_module_reference.md`.

It is intentionally conservative. Values copied from real-device behavior but not fully understood are labeled as observed behavior instead of being given invented semantics.

## Confidence Terms

| Confidence | Meaning |
|---|---|
| high | Required by the working protocol path or directly grounded in current behavior. |
| medium | Supported by current behavior, but name or deeper meaning may change. |
| low | Observed/tolerated behavior whose semantics are still uncertain. |

## BLE Topology

| Service | UUID | Characteristic | UUID | Properties | Role | Confidence |
|---|---|---|---|---|---|---|
| Battery | `0x180F` | Battery level | `0x2A19` | read | One-byte battery level, default `100`. | high |
| Service A | `addc3e26-4aa5-4c1a-8a6a-735db4e01c6c` | Char1 | `addc3e26-4aa5-4c1a-8a6a-735db4e01c6d` | write without response | App write lane for identity/pair-state interaction. | medium |
| Service A | same | Char2 | `addc3e26-4aa5-4c1a-8a6a-735db4e01c6e` | write without response | App write lane for identity/pair-state interaction. | medium |
| Service A | same | Char3 | `addc3e26-4aa5-4c1a-8a6a-735db4e01c6f` | read | Donor public address mirror. | high |
| Service A | same | Char4 | `addc3e26-4aa5-4c1a-8a6a-735db4e01c70` | read | Pairing identity/status mirror. | medium |
| Service A | same | Char5 | `addc3e26-4aa5-4c1a-8a6a-735db4e01c71` | read + notify | Compact runtime/pair status mirror; also receives button-state mirror notifications. | high |
| Service B | `2bbe7f7c-7304-4466-8407-8eaf89f8ce45` | Char1 | `2bbe7f7c-7304-4466-8407-8eaf89f8ce46` | read + notify | Readiness/status lane. Reconnect ready notify is suppressed for Device B parity. | high |
| Service B | same | Char2 | `2bbe7f7c-7304-4466-8407-8eaf89f8ce47` | read + notify | Secondary readiness/status lane. | medium |
| Service B | same | Char3 | `2bbe7f7c-7304-4466-8407-8eaf89f8ce49` | read | Fixed two-byte readable status. | medium |
| Service C | `6b7f9c98-8588-4624-8a94-38c9bce32685` | Char1 | `68282de6-e643-4caa-a6b4-e96f46ed988a` | write + notify | First-pair staging, commit signaling, and pre-auth notify burst. | high |
| Service C | same | Char2 | `290278ac-5eb9-47df-899d-e904ab62b4b3` | write without response | Companion first-pair write lane. | medium |
| Service D | `bbe87709-5b89-4433-ab7f-8b8eef0d8e37` | Char1 | `bbe87709-5b89-4433-ab7f-8b8eef0d8e38` | write | Typed Service D certificate/protocol write lane. | high |
| Service D | same | Char2 | `bbe87709-5b89-4433-ab7f-8b8eef0d8e39` | read + notify | Service D status notify and readable staged blob carrier. | high |
| Service D | same | Char3 | `bbe87709-5b89-4433-ab7f-8b8eef0d8e3a` | read | Staged Service D blob readback. | high |
| Service E | `21c50462-67cb-63a3-5c4c-82b5b9939aeb` | LED | `21c50462-67cb-63a3-5c4c-82b5b9939aec` | write | App-to-device LED/vibration command lane. | medium |
| Service E | same | Button | `21c50462-67cb-63a3-5c4c-82b5b9939aed` | notify | Runtime button/status notify lane. | high |
| Service E | same | Aux3 | `21c50462-67cb-63a3-5c4c-82b5b9939aee` | write | Auxiliary app write lane. | low |
| Service E | same | Aux4 | `21c50462-67cb-63a3-5c4c-82b5b9939aef` | write | Auxiliary app write lane. | low |
| Service E | same | Version | `21c50462-67cb-63a3-5c4c-82b5b9939af0` | read | Firmware version string, currently `3.900.1.115r1`. | high |
| Service E | same | Config | `1e9d6f80-9b71-4bbf-9ba3-1d4d97212b49` | write | One-byte config write lane. | medium |
| Auth | `138c35b6-23a0-459c-9869-acb9228be64f` | Control | `7e11e71f-97f6-4578-aa23-2f0b2f8a946e` | write without response + notify | Auth-control opcodes and framed sleep-control replies. | high |
| Auth | same | Blob | `125266b9-bcfd-4904-8be0-530ef0b93372` | write without response + notify | Auth blob staging and sleep stream data. | high |

## Auth Control Opcodes

| Value | Direction | Phase | Device reply/status | Meaning | Confidence |
|---|---|---|---|---|---|
| `0x24` | app -> device | first pair, reconnect | `24 00 00 00 00` on fresh pair; `24 00 nn nn nn` on reconnect | Indexed auth reply lane. Reconnect reply uses persisted accepted nonce bytes for index `1`; fresh pair uses the zero-tail observed shape. | high |
| `0x25` | app -> device | first pair, reconnect | `25 00`, then auth blob notify | Exposes the visible auth blob and, on reconnect, arms compact Service D. | high |
| `0x18` | app -> device | first pair | `18 00`; `18 05` on failure path | Stores parser bytes and central nonce, seeds auth worker and Service D material. | high |
| `0x16` | app -> device | first pair | delayed `16 00 00 00 00 00` | Latches donor auth worker branch and later emits completion-style marker. | medium |
| `0x36` | app -> device | first pair | `36 00` | Commits Service D selector4 default and selector5 zero into persistent runtime state. | high |
| `0x28` | app -> device | first pair | `28 00` | Commits Service D record `0x1A = 1` into persistent runtime state. | high |
| `0x26` | app -> device | first pair, authenticated sleep | first pair: `26 00` plus short blob; sleep: `26 00` plus blob stream | First-pair auth completion, or authenticated tokenized sleep export request depending on phase. | high |
| `0x21` | device -> app | first pair | `21 00 02`, `21 00 01` | Auth worker wait/ready markers. | medium |
| `0x29` | app -> device | first pair | usually no external notify | Seeds local-family runtime state. Relevant as an accepted write, but the current cleaned firmware no longer carries unused static `0x29` reply constants. | medium |

## Authenticated Sleep Opcodes

These share the Auth Control characteristic after the device is authenticated.

| Value | Direction | Phase | Device reply/status | Meaning | Confidence |
|---|---|---|---|---|---|
| `0x01` | app -> device | sleep | framed length reply on Auth Control, then Auth Blob stream | Publishes selector `0x01` unread sleep-log record. | high |
| `0x02` | app -> device | sleep | `02 00` | Updates runtime anchor. Before first authenticated disconnect, it does not create unread sleep. | high |
| `0x15` | app -> device | sleep | `15 00` | Dequeues/clears one unread sleep descriptor for selector `0x01`. | high |
| `0x16` | app -> device | sleep | framed 4-byte count reply | Returns queued unread descriptor count for selector `0x01`. | high |
| `0x26` | app -> device | sleep | `26 00`, then Auth Blob stream | Tokenized sleep payload export. | high |
| `0x27` | app -> device | sleep | `27 00` | Staged export acknowledgement. | medium |
| `0x2d` | app -> device | sleep | framed one-byte classification | Reports whether unread sleep descriptors are queued. | medium |
| `0x30` | app -> device | sleep | `30 00` | Updates incomplete-flag state. | medium |
| `0x31` | app -> device | sleep | `31 00` | Timing/config acknowledgement. | medium |

## Service C Opcodes

| Value | Direction | Phase | Device reply/status | Meaning | Confidence |
|---|---|---|---|---|---|
| `0x02` | app -> device | first pair | state refresh and possible notify burst | Sets Service C transfer state to `0x02`, moves pair event class toward staged/pairable, and stages pair if needed. | high |
| `0x03` | app -> device | first pair | state refresh | Sets transfer state to `0x01` and mirrors byte 1 into the Service C state image. | medium |
| `0x04` | app -> device | first pair | state refresh | Commits the staged pair when preconditions are met. | high |

## Service D Typed Messages

Service D typed writes use byte 0 as the message type. The status notify is four bytes on Service D Char2. The readable blob must be staged before the status notify that tells the app to read it.

### Older Lane

Used during first-time pairing.

| State | Expected type | Expected length | Direction | Device status | Meaning | Confidence |
|---|---:|---:|---|---|---|---|
| begin | n/a | n/a | device -> app | `00 00 00 00` | Opens older Service D lane. | high |
| `5` | `0x01` | `0x14` | app -> device | `01 00 00 00` | Prepare/generated type1 read blob, arm type2 expectation. | high |
| `6` | `0x02` | `0x34` | app -> device | `02 00 00 00` | Save type2 write, increment runtime record6 tail, prepare generated type2 reply. | high |
| `7` | `0x00` | `0x34` | app -> device | `04 00 23 00` | Older lane completion marker; auth `0x26` remains responsible for final pair completion. | high |

### Compact Lane

Used during authenticated reconnect after auth `0x25` arms compact Service D.

| State | Expected type | Expected length | Direction | Device status | Meaning | Confidence |
|---|---:|---:|---|---|---|---|
| begin | n/a | n/a | device -> app | `03 00 00 00` | Opens compact Service D lane. | high |
| `1` | `0x04` | `0x14` | app -> device | `04 00 01 00` | Save type4 write and arm type5 expectation. | high |
| `2` | `0x05` | `0x24` | app -> device | `05 00 00 00` | Save type5 write and derive 20-byte type5 read payload. | high |
| `3` | `0x03` | `0x05` | app -> device | `04 00 02 00` on match; `04 00 03 00` on mismatch | Completion packet check. The expected done shape is `03 00 00 00 01`. | high |

## Service D Status Payloads

| Constant | Payload | Phase | Meaning | Confidence |
|---|---|---|---|---|
| `PLUSPLUS_SERVICE_D_STATUS0` | `00 00 00 00` | first pair | Older lane start/status idle marker. | high |
| `PLUSPLUS_SERVICE_D_STATUS1` | `01 00 00 00` | first pair | Older lane type1 accepted; type1 read blob ready. | high |
| `PLUSPLUS_SERVICE_D_STATUS2` | `02 00 00 00` | first pair | Older lane type2 accepted; type2 read blob ready. | high |
| `PLUSPLUS_SERVICE_D_STATUS4` | `04 00 23 00` | first pair | Older lane completion/status marker observed in working flow. The `0x23` byte is observed behavior. | medium |
| `PLUSPLUS_SERVICE_D_STATUS3_START` | `03 00 00 00` | reconnect | Compact lane start marker. | high |
| `PLUSPLUS_SERVICE_D_STATUS4_STAGE1` | `04 00 01 00` | reconnect | Compact type4 accepted; type5 expected next. | high |
| `PLUSPLUS_SERVICE_D_STATUS5_STAGE2` | `05 00 00 00` | reconnect | Compact type5 accepted; type3 done expected next. | high |
| `PLUSPLUS_SERVICE_D_STATUS4_DONE` | `04 00 02 00` | reconnect | Compact Service D complete. | high |
| `PLUSPLUS_SERVICE_D_STATUS4_FAIL` | `04 00 03 00` | reconnect | Compact completion rejected; stay in completion expectation. | high |

## Protocol Data Constant Classification

| Constant | Classification | Map entry or reason |
|---|---|---|
| `REAL_PLUSPLUS_ADDR_LE` | donor/device identity | BLE identity value, not an opcode. Mirrors the donor public address in little-endian form. |
| `PLUSPLUS_SERVICE_DATA_32` | advertising protocol constant | 32-bit service-data advertising payload used by the Plus+ app discovery path. |
| `PLUSPLUS_AUTH_CTRL_REPLY_2401` | observed auth reply | Reconnect/default indexed `0x24` reply seed; live reconnect uses persisted nonce when available. |
| `PLUSPLUS_FRESH_AUTH_CTRL_REPLY_24` | auth reply | Fresh-pair `0x24` reply. |
| `PLUSPLUS_AUTH_CTRL_REPLY_25` | auth reply | `0x25` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_18` | auth reply | Successful `0x18` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_18_FAIL` | auth reply | Failure `0x18` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_16` | auth reply | Delayed `0x16` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_36` | auth reply | `0x36` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_28` | auth reply | `0x28` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_26` | auth reply | `0x26` response. |
| `PLUSPLUS_AUTH_CTRL_REPLY_21_WAIT` | auth worker status | `0x21` wait marker. |
| `PLUSPLUS_AUTH_CTRL_REPLY_21_READY` | auth worker status | `0x21` ready marker. |
| `PLUSPLUS_AUTH_BLOB_SHORT` | observed auth blob | Short blob exposed with auth `0x26`; current semantic meaning is observed but not fully named. |
| `PLUSPLUS_SERVICE_D_STATUS*` | Service D protocol statuses | Documented in Service D status table above. |
| `PLUSPLUS_OLDER_TYPE1_CONST` | Service D generation constant | Constant prefix/material for generated older-lane type1 reply; protocol role is known, deeper semantic meaning is observed. |
| `DONOR_SERVICE_D_READ_TYPE3` | donor material | Compact type3 readable payload from donor material; not a generic protocol constant. |
| `DONOR_SERVICE_D_EXPECT_WRITE3_DONE` | Service D protocol expectation | Expected compact type3 completion packet `03 00 00 00 01`. |
| `PLUSPLUS_FACTORY_CERT_BLOCK` | donor/factory material | 256-byte certificate block. Raw material, not an opcode/status value. |
| `PLUSPLUS_FACTORY_SELECTOR3` | donor/factory material | Selector/key material used by generated Service D/auth paths. |
| `PLUSPLUS_FACTORY_SELECTOR4` | donor/factory material | Selector/key material used by generated Service D/auth paths. |
| `FIRMWARE_RECORD4_DEFAULT` | runtime material default | Default record bytes used by Service D runtime state. |
| `PLUSPLUS_FACTORY_SELECTOR6` | donor/factory material | Factory record6 seed used by Service D runtime state. |

## Protocol Boundaries

- First-pair and reconnect are separate flows. The overlap is in auth and Service D primitives, not in the flow state machine.
- `04 00 23 00` is an older-lane completion/status marker with an observed `0x23` byte.
- Sleep `0x16` and first-pair auth `0x16` share an opcode byte on the Auth Control characteristic but live in different protocol phases.
- ServiceB reconnect ready notify suppression is a BLE parity behavior, not a sleep-log behavior.
