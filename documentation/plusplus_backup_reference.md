# Pokemon GO Plus + Backup Reference

Date: 2026-05-18

## Purpose

This is the current master reference for the Plus+ backup emulator work.

It replaces the older exploratory notes as the primary source of truth for:

- BLE identity and advertising behavior
- service layout and observed purpose
- first-pair and reconnect behavior
- the auth lane
- Service D
- post-pair runtime behavior
- sleep transfer placement
- the minimum device-specific material needed from a real donor

This document is written for someone who wants to build a working backup emulator device and can image the needed keys and persisted records from a real Plus+.

## Scope and confidence

Confidence is highest in these areas:

- advertising and visible scan contract
- service/characteristic layout
- first-pair auth control flow
- older Service D lane
- successful transition into authenticated state
- post-pair LED/button behavior at a protocol level
- sleep transfer happening after pairing, not during the Service D gate

Confidence is lower where noted:

- exact semantic naming for some non-auth services
- exact meaning of every Service E effect payload
- exact meaning of every sleep record field

## Current Implementation

The current working firmware model is in:

`code/nrf_emulator/pgpemu_nrf52_plusplus_exp/pgpemu_nrf52_plusplus_exp.ino`

This line has already achieved:

- real successful first pair with the app
- real reconnect success after a normal disconnect
- no low-battery icon after reconnect
- authenticated sleep descriptor and stream behavior on the expected reconnect path

The firmware module overview is `documentation\firmware_module_reference.md`.

## Current Firmware Architecture

The firmware keeps first pair and reconnect as separate flows while moving shared protocol behavior into explicit modules:

- BLE topology/dispatch: `pp_ble_topology.*`, `pp_ble_dispatch.*`
- connection routing: `pp_connection_coordinator.*`
- first-pair flow: `pp_first_pair_flow.*`, `pp_pair_first_time.*`
- reconnect flow: `pp_reconnect_flow.*`, `pp_pair_reconnect.*`
- auth protocol: `pp_auth_protocol.*`, `pp_auth_blob.*`, `pp_auth_material.*`
- Service D protocol: `pp_service_d_protocol.*`, `pp_service_d_local.*`
- sleep export/policy: `pp_sleep_export.*`, `pp_sleep_policy.*`
- logging policy: `pp_log.h`, `pp_debug_ble.h`, `pp_debug_utils.*`

Use `documentation\firmware_protocol_map.md` for byte/opcode meanings.

## What you must image from a real device

For a faithful backup emulator, the common protocol alone is not enough. You need donor-specific material.

Required:

- Public BLE address
- Cert block / selector 2 (`256` bytes)
- Live cert/session key lane / selector 3 (`16` bytes)

Strongly recommended:

- External `.SAV` selector table image, especially selectors `4`, `5`, and `6`
- Any real persisted pairing state you want to preserve for reconnect behavior

What appears common and not donor-specific in the current emulator:

- Device name: `Pokemon GO Plus +`
- advertising structure
- service UUID layout
- version string `3.900.1.115r1`
- battery level default behavior
- auth and Service D control flow

## Required visible scan contract

If the app cannot recognize the device from advertising, pairing never starts.

The current visible contract is:

- public advertiser address matching the donor device
- connectable scannable undirected advertising
- name in scan response: `Pokemon GO Plus +`
- advertising payload contains:
  - flags
  - manufacturer data
  - service data type `0x20`

Current emulator values:

- public address: user/device-specific, supplied by local `pp_device_material.h`
- manufacturer payload:
  - `53 05 01 AE DE 00 F0 BE 00 00 00 00 00 00 00 00 01` in pairable mode
  - final byte changes to `02` in reconnect/authenticated mode
- service data `0x20` payload:
  - `B6 35 8C 13`
- advertising interval:
  - `160` BLE units = `100 ms`

## Pairing stage model

The working emulator uses four top-level pairing stages:

- `discovery`
- `staged`
- `committed`
- `authenticated`

Those stages are reflected into multiple services so the app sees a coherent device state.

## Service layout

### Battery Service

Service:

- UUID `0x180F`

Characteristic:

- `0x2A19` battery level

Current emulator behavior:

- readable only
- default value `100`

This is not part of the auth gate.

### Service A

Service UUID:

- `addc3e26-4aa5-4c1a-8a6a-735db4e01c6c`

Characteristics:

- `...c6d` write-only
- `...c6e` write-only
- `...c6f` read
- `...c70` read
- `...c71` read + notify

Working role:

- identity-facing state mirror
- exposes public address bytes
- exposes high-level pairing readiness and manufacturer state

Important emulator behavior:

- `serviceAChar3` exposes the donor public address
- `serviceAChar4` mirrors readiness, committed-pair visibility, pair bytes, and manufacturer-state byte
- `serviceAChar5` notifies compact status

This service matters for the app's general device-state picture, but it is not the main auth lane.

### Service B

Service UUID:

- `2bbe7f7c-7304-4466-8407-8eaf89f8ce45`

Characteristics:

- `...ce46` read + notify
- `...ce47` read + notify
- `...ce49` read

Working role:

- status mirror for readiness and pairing progress
- post-auth ready-state support

Current state model:

- char1: readiness, active pair bytes, event class
- char2: candidate pair bytes, committed-pair visibility, readiness
- char3: small ready/event summary

This service matters after pairing succeeds and during the device's "ready" presentation to the app.

### Service C

Service UUID:

- `6b7f9c98-8588-4624-8a94-38c9bce32685`

Characteristics:

- `68282de6-e643-4caa-a6b4-e96f46ed988a` write + notify
- `290278ac-5eb9-47df-899d-e904ab62b4b3` write-only

Working role:

- pre-auth pairing side channel
- pair staging / commit signaling
- first-pair notify burst carrier

Observed top-level writes on char1:

- opcode `0x02`
  - enters transfer state
  - can stage a candidate pair
- opcode `0x03`
  - updates compact mode/state
- opcode `0x04`
  - participates in committing the staged pair

Important emulator behavior:

- it must present coherent staged/committed pair bytes
- it participates in the transition from `discovery` to `staged` to `committed`
- during first pair it sends a timed notify burst that helps the app see a realistic pre-auth device

This service is not the final auth gate, but it helps make the pre-auth device state believable enough for the app to continue.

### Service D

Service UUID:

- `bbe87709-5b89-4433-ab7f-8b8eef0d8e37`

Characteristics:

- `...e38` write
- `...e39` read + notify
- `...e3a` read

Working role:

- certification gate and final pairing blocker

This is the most important service to get right for first pair.

#### Service D lanes

There are two firmware-known lanes:

- compact late lane
- older first-pair lane

The emulator now uses the older lane for real first-pair success.

#### Compact lane

The compact lane is the `3 -> 4 -> 5 -> 3-done` family.

It is important for protocol reference and it is now also the active reconnect lane, but it is not the first-pair success target that solved the real device pairing problem.

Key truths:

- compact type `5` is a CTR-like AES-128 transform
- key source is record `4`
- auth `36` zeroes selector `4` in the observed first-pair path
- after that zero write, the compact lane remains consistent with firmware and tends to fail as a first-pair target
- on reconnect, the app uses the compact lane instead of the older first-pair lane
- reconnect compact `type 4` is dynamic and session-derived, not a fixed constant that can be hardcoded from one old sample
- reconnect compact `type 5` also varies and must be handled from live write state

#### Older first-pair lane

This is the lane that now pairs successfully.

Successful sequence:

1. Device sends status `00 00 00 00`
2. App writes type `1`, length `0x14`
3. Device sends status `01 00 00 00`
4. App writes type `2`, length `0x34`
5. Device sends status `02 00 00 00`
6. App writes type `0`, length `0x34`
7. Device sends status `04 00 23 00`
8. App continues with auth command `26`

Critical implementation details that mattered:

- the older opener must be a full generated `0x17A` payload, not a short shortcut
- the type `1` device payload must be generated, not replayed
- the type `2` device payload must be generated from the live app packet, not replayed
- the readable Service D blob must already be staged before the status notify is sent
- the read value must stay resident while the app reads it

#### Service D persistent inputs

The older lane directly consumes these selector-backed records:

- record `2`
- record `3`
- record `4`
- record `6`

The control path also depends on:

- selector `5`
- record `0x1A`

#### Service D state-changing auth commands

The critical auth commands before Service D are:

- `36`
  - writes selector `4 = 00..00`
  - writes selector `5 = 00`
- `28`
  - writes record `0x1A = 1`

For the successful first-pair path, the app still expects the older Service D lane after those writes.

### Service E

Service UUID:

- `21c50462-67cb-63a3-5c4c-82b5b9939aeb`

Detailed reference:

- `service_e_button_reference.md` is not included in the public release bundle.

Characteristics:

- `...9aec` write
- `...9aed` notify
- `...9aee` write
- `...9aef` write
- `...9af0` read
- `1e9d6f80-9b71-4bbf-9ba3-1d4d97212b49` write

Working role:

- LED/effect control
- button/event notification
- version/info
- simple config state
- prompt / encounter / pairing-adjacent UI lane

Practical mapping:

- `ledWriteChar`
  - app sends effect and animation programs here
- `buttonNotifyChar`
  - emulator sends button/event pulses here
- `versionChar`
  - reads as `3.900.1.115r1`
- `configChar`
  - writable mode/ready byte mirror

Important current semantics:

- `00 07` on `buttonNotifyChar` is the emulator's generic "button/event active" pulse
- `00 00` is idle/released

### Service E characteristics in the current emulator

Current emulator behavior is more specific than the older notes:

- `ledWriteChar` (`...9aec`) is write-only and is treated as the app-to-device effect/prompt input lane
- `buttonNotifyChar` (`...9aed`) is notify-only and carries the emulator's exposed button/event state
- `versionChar` (`...9af0`) is read-only and returns `3.900.1.115r1`
- `configChar` (`1e9d6f80-9b71-4bbf-9ba3-1d4d97212b49`) is a small writable config/mode byte

### BLE-visible button values we can currently justify

The only button-notify values that are strongly grounded in current behavior are:

- `00 00`
  - idle / released / no active button-style event
- `00 07`
  - short action pulse / pressed-like event pulse

At the BLE level, these are the only Service E button states that should currently be treated as established.

### Physical button mapping status

This part is still intentionally narrow.

Real-data-backed conclusions:

- the real device does expose BLE-visible Service E button/status traffic
- the only firmly established plaintext values in that visible lane are:
  - `00 00`
  - `00 07`
- `00 00` is consistent with idle / released-like state
- `00 07` is consistent with a short action / pressed-like pulse

Not yet established in the current reference:

- an exact mapping from physical main button press to a distinct BLE plaintext value
- an exact mapping from physical top button press to a distinct BLE plaintext value
- an exact mapping from simultaneous or multi-button combinations to distinct BLE plaintext values
- any separate established plaintext BLE values beyond `00 00` and `00 07`

So the current master-reference position is:

- we do not yet have real-data-backed proof that the real device publishes separate plaintext values for:
  - main button only
  - top button only
  - multi-button combinations
- if such distinctions exist on the real device, they are not yet proven in the BLE-visible plaintext lane
- the stronger firmware interpretation is that physical button handling feeds a higher-level local prompt/pairing state machine, and the BLE-visible Service E output is only a compact summary of that larger internal state
- the Device B runtime behavior also shows that the short `0x0033 = 000000010a0000` packet follows a device-led nonzero `0x0035` state, so the runtime lane should be modeled as device-driven state with app acknowledgement, not as a simple reaction to that short write

### Current emulator behavior for button output

The current emulator has three separate Service E button behaviors:

1. Short LED-triggered pulse

- when `ledWriteChar` receives a payload with leading bytes `00 00 00` and command byte `0x0F` or `0x01`, the emulator emits:
  - `00 07`
  - then roughly `80 ms` later `00 00`
- this is implemented in:
  - `maybeTriggerButtonFromLedPayload()`
  - `pulseButton()`
  - `clearButtonPulseIfNeeded()`

2. Fresh-auth confirmation pulse

- during the successful first-pair auth path, the emulator can send a dedicated auth-confirm pulse:
  - `00 07`
  - then later `00 00`
- this is separate from the ordinary LED-triggered pulse
- it exists to support the first-pair progression timing and is not just generic runtime UI traffic

3. Idle-state notification train

- after auth completes, the emulator sends repeated `00 00` button notifications
- this is the post-auth idle train seen in runtime examples:
  - `Post-auth idle button notify sent=true`
- it is intended to mirror the app's expectation that the paired device settles back to idle output state after pairing/runtime transitions

### Service A coupling

In the current emulator, Service E button output is mirrored into Service A status output:

- `notifyButtonState()` notifies both:
  - `buttonNotifyChar`
  - `serviceAChar5`
- when the emulator emits `00 07` or `00 00` on Service E, it also mirrors that compact state into Service A

This means button-visible state is not isolated to one characteristic in the emulator's current presentation.

### What the firmware evidence says about the real device

The strongest firmware-side interpretation is:

- Service E is not just a simple button byte lane
- it is the broader prompt / effect / encounter / pairing-adjacent UI cluster
- `0x0033`-style writes appear to feed a larger buffered local state image
- `0x0035`-style notifications appear to export a much smaller summary state from that larger subsystem

The most important historical firmware findings that still hold are:

- `0x0033` is best treated as prompt/effect input
- `0x0035` is best treated as button/status output
- `0000` and `0007` are real BLE-visible states
- the real pairing effect is likely controlled by a higher-level local pairing mode/state machine, not by a single standalone BLE-visible button code

### Waina / pairing-mode interpretation

Firmware notes point to a dedicated `Waina` pairing mode and coordinator layer.

Current best interpretation:

- physical button action probably changes higher-level local pairing state
- that higher-level state then influences auth progression
- the BLE-visible `00 07` pulse by itself is not the whole meaning of "button pressed during pairing"

This is why the current evidence does not justify claiming that Service E plaintext alone gives a full one-to-one physical button map.

This matters because earlier experiments already showed that simply replaying:

- `00 07`
- then `00 00`

does not recreate the real device's pairing behavior on its own.

### What is established vs still unknown

Established:

- Service E is the button/prompt/effect family
- `ledWriteChar` is app-to-device effect/prompt input
- `buttonNotifyChar` is device-to-app button/status output
- `00 00` and `00 07` are real button-visible states
- the emulator currently derives many button pulses from incoming LED/effect writes
- a higher-level pairing/prompt state machine exists above the visible `00 07` byte

Still unknown:

- exact mapping of physical buttons to BLE-visible values
- whether the real device has distinct plaintext values for:
  - main button only
  - top button only
  - multi-button combinations
- exact meaning of every `ledWriteChar` effect payload family
- exact internal producer that chooses `0000` vs `0007` in the real firmware
- exact bridge from Waina pairing mode into BLE-visible Service E output

### Practical implementation guidance for Service E

If building from donor material, treat Service E as follows:

- do not model it as "just a fake button code"
- implement `ledWriteChar` as a stateful prompt/effect input lane
- implement `buttonNotifyChar` with at least:
  - `00 07` pulse behavior
  - `00 00` idle behavior
- preserve the post-auth idle notification behavior because the app expects a live settled runtime state after pairing
- keep confidence labels clear: current emulator behavior here is functional, but not all real firmware semantics are fully closed

This service becomes noisy after pairing succeeds because the app starts driving the live effect patterns.

### Auth Service

Service UUID:

- `138c35b6-23a0-459c-9869-acb9228be64f`

Characteristics:

- `7e11e71f-97f6-4578-aa23-2f0b2f8a946e` write + notify
- `125266b9-bcfd-4904-8be0-530ef0b93372` write + notify

Working role:

- primary auth control lane before Service D
- staged auth blob transport

This service is just as important as Service D for first pair.

## First-pair auth sequence

The current successful sequence is:

1. App writes `24 01`
2. Device replies `24 00 00 00 00`
3. App writes `25`
4. Device replies `25 00`
5. Device sends visible auth blob on `authBlob`
6. App writes `18 01 xx xx xx 01`
7. Device replies `18 00`
8. App writes `16 01`
9. Device replies `21 00 02`
10. Device replies delayed `16 00 00 00 00 00`
11. Device replies `21 00 01`
12. App writes `36`
13. Device replies `36 00`
14. App writes `28`
15. Device replies `28 00`
16. App enables Service D and enters the older Service D lane
17. After Service D completion, app writes `26 ...`
18. Device replies `26 00`
19. Device sends short auth blob
20. Device marks auth complete and enters reconnect-ready mode

## Generated auth blob

The successful emulator builds a generated initial auth blob rather than replaying a stale packet.

Important ingredients:

- donor public address
- selector `2` cert block
- selector `3` key lane
- current auth/session state

In the current implementation:

- visible auth blob length is `262` bytes
- the larger generated challenge structure behind it is `378` bytes

## Reconnect behavior

Once authenticated, the device should stop advertising as a fresh pair target and instead behave like a reconnecting known device.

Current reconnect markers:

- manufacturer last byte changes from pairable `01` to reconnect `02`
- `24` reply for index `1` changes to carry the persisted accepted nonce
- pairing stage becomes `authenticated`
- ServiceB reconnect-ready `01` notify is suppressed after compact Service D for Device B parity; char1 remains readable as `04`

### Reconnect wire shape

Reconnect is a separate branch from first pair.

It does not redo the full first-pair `18 / 16 / 36 / 28` lane.

Current working reconnect shape:

1. App enables button/auth/auth-blob CCCDs
2. App writes `24 01`
3. Device replies `24 00 xx xx xx`
4. Those `xx xx xx` bytes are the high three bytes of the previously accepted first-pair `18` nonce
5. App writes `25`
6. Device replies `25 00`
7. Device sends a visible auth blob on `authBlob`
8. App enables Service D CCCD
9. Device enters the compact Service D reconnect lane
10. Device sends compact status `03 00 00 00`
11. App writes compact type `4`
12. Device sends `04 00 01 00`
13. App writes compact type `5`
14. Device sends `05 00 00 00`
15. App writes compact `3-done`
16. Device sends `04 00 02 00`
17. Device primes ServiceB char1 readable value `04`
18. Device suppresses the older reconnect-ready `01` notify for Device B parity
19. Device returns to authenticated idle runtime

### Reconnect truths now established

- reconnect uses persisted accepted state from the earlier real successful pair
- the reconnect `24` reply must be dynamic, not static
- reconnect enters the compact Service D lane, not the older first-pair lane
- replaying a static reconnect auth blob from another device is not firmware-faithful and fails
- replaying one private filed reconnect compact `type 4` is not firmware-faithful and fails
- reconnect Service D writes must be handled by the typed Service D state machine, not by the first-pair-only gate
- the no-low-battery reconnect fix is ServiceB parity behavior, not sleep behavior

### Reconnect implementation notes

What must be true for a donor-faithful reconnect emulator:

- accepted first-pair nonce must be persisted after real auth completion
- reconnect advertising/manufacturer state must change to reconnect mode
- auth `24` index `1` must return the persisted accepted nonce bytes
- reconnect `25` must expose the local donor-cert identity on `authBlob`, not the blob from some other device
- after reconnect Service D compact completion, firmware must return to authenticated idle instead of waiting for the first-pair `26` tail
- after reconnect Service D compact completion, firmware must leave ServiceB char1 readable as `04` and suppress the old reconnect-ready `01` notify

### Reconnect confidence

Confidence is now high in:

- reconnect branch selection
- reconnect `24` nonce persistence behavior
- reconnect use of compact Service D
- reconnect compact Service D completion path

Confidence is still lower in:

- the exact internal derivation that makes each reconnect compact `type 4` vary from session to session
- the exact firmware-side relation between reconnect `25` session material and the app's next compact `type 4` write

## Sleep transfer placement

Sleep transfer is not part of the Service D pairing gate.

Current placement:

- pairing/auth must already be settled
- Waina data notifications must be enabled
- unread sleep data must exist
- then the app requests sleep data
- firmware handles that through command `0x26` in the sleep path

Pairing behavior is independent of sleep-log payload handling.

## Emulator requirements by priority

### Absolutely required

- correct public BLE address
- correct advertising bytes and timing
- correct service/characteristic layout
- correct first-pair auth sequence
- correct older Service D lane
- correct transition to authenticated/reconnect state

### Required for donor-faithful auth

- selector `2` cert block
- selector `3` key lane
- realistic persisted state handling for selectors `4`, `5`, and `6`

### Required for a polished post-pair emulator

- Service E effect handling
- button/event notify timing
- Service B ready-state behavior
- reconnect nonce persistence
- reconnect compact Service D fidelity
- sleep transfer support

## Practical build guidance

If someone is building a new working backup emulator, the safest implementation order is:

1. Match scan-visible identity exactly
2. Implement the full service/characteristic table
3. Mirror the discovery/staged/committed/authenticated state model
4. Implement auth `24`, `25`, `18`, `16`, `36`, `28`, `26`
5. Implement the older Service D lane with generated payloads
6. Persist accepted nonce and switch to reconnect state after auth complete
7. Implement the reconnect compact Service D lane from live persisted state
8. Add post-pair LED/button/runtime polish
9. Add sleep transfer only after the pairing path is already stable

## What is intentionally not treated as current truth

These older ideas should not be used as the primary implementation target anymore:

- "compact Service D is the first-pair success lane"
- "replaying a static Device B Service D blob is good enough"
- "replaying a reconnect auth blob or reconnect compact packet from another device is good enough"
- "sleep transfer is the pairing blocker"
- "ROM .SAV defaults are the normal boot state"

Those were useful intermediate hypotheses, but they are not the final working model.

## Historical notes

Older exploratory and partial-truth notes have been moved under:

Historical raw research notes are not included in the public release bundle.

They are still useful for historical context, but this master document should be treated as the current reference first.
