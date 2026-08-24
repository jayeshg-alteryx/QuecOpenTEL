# FOTA Download Part A — Context Handoff

**Purpose:** provide enough context for another engineer or AI tool to continue the FOTA work without reconstructing the requirements from the conversation.

**Repository:** `QuecOpenTEL`

**Target:** Quectel EC800 / QuecOpen SDK

**Scope:** FOTA download only. Installation, flashing, deployment, decompression, and installation-log upload are Part B and are intentionally out of scope.

## Source of truth and fixed decisions

The requirements were extracted from:

- `FotaRequirements.xlsx`
- `FOTA Requirements Minutes of Meeting.pdf`
- `Cloud-Task-Sheet-Polarion-Requirements.xlsx`, especially the **FOTA & portal** tab

The Minutes of Meeting override contradictory workbook text. The job-document schema is taken from the workbook. The URL validity epoch is the quoted `url_expiry` field inside each package entry; it is not inferred from the URL or from download time.

Fixed decisions:

- Remove `notify-next`.
- Do not maintain default MQTT subscriptions. Create MQTT sessions on demand.
- Poll opportunities are delivered exclusively as queued `RTC_POLL_DUE` events. FOTA does not receive ignition information.
- The upstream scheduler decides when polling is permitted; FOTA still enforces the six-hour guard and daily budget.
- Job documents, MQTT response payloads, and status payloads use the QuecOpen cJSON library.
- All package URLs in one document share the job validity window for local behavior. The earliest package `url_expiry` is the local document expiry.
- Compression must be exactly `"none"`; decompression is not implemented.
- The expected hash is parsed and retained as a 32-bit unsigned value, but Part A deliberately bypasses hash calculation/comparison. This must be clearly treated as a temporary product decision.
- Installation is represented only by a future handoff boundary and identity-checked completion events.

## Current implementation map

The Part A implementation is under `TEL/APP/FOTA/`:

- `FOTAManager.c/.h` — owns the bounded event queue and is the only task that consumes `FOTAEvent_T`.
- `FOTA_SM.c/.h` — owns the FOTA context, persisted lifecycle, cloud orchestration, package selection, reporting, recovery, and installer boundary.
- `FOTADownloadSM.c/.h` — isolated 4 KB HTTP range-transfer and resume logic.
- `FOTAJobParser.c/.h` — cJSON job-document parsing and semantic validation.
- `FOTATypes.h` — public states, events, errors, package/job types, platform callbacks, storage identifiers, and limits.
- `FOTAInstallSM.c` — existing placeholder only; it is not compiled for Part A.
- `FOTAInstallSM.h` — reserved Part B boundary.
- `README.md` — concise implementation/product-decision summary.

Integration changes:

- `TEL/CMakeLists.txt` includes the FOTA sources and the QuecOpen `cJSON.c` source, and adds the SDK cJSON include directory.
- `TEL/SERVICES/PASSIVE/FS/FS_Types.h` and `FS_Manager.c` define the managed UFS identifiers listed below.
- `TEL/SYSTEM/SysInit.c/.h` initializes FOTA after filesystem initialization. Current startup calls `ge_FOTA_Init(NULL)`; production should pass/wire real platform adapters.

## Public entry points and ownership

The manager API is:

```c
TelSysResponse_E ge_FOTA_Init(const FOTAPlatformOps_T *ops);
TelSysResponse_E ge_FOTA_PostEvent(const FOTAEvent_T *event);
FOTAState_E ge_FOTA_GetState(void);
void gv_FOTA_GetStatus(FOTAStatus_T *status);
```

`FOTAManager` creates a queue of two `FOTAEvent_T` entries and a task with a 1536-byte configured stack. Services must never mutate the state machine directly; asynchronous completions are posted through `ge_FOTA_PostEvent`.

`FOTAPlatformOps_T` supplies these external dependencies:

- RTC epoch and local calendar day: `get_time`.
- AppMonitor claim/release.
- On-demand MQTT poll (`mqtt_poll_next`), AWS execution validation (`mqtt_validate_job`), fresh document request (`mqtt_request_document`), and status publication (`mqtt_publish_status`). Poll/document callbacks complete on their defined AWS response. Status publication completes on broker PUBACK and must not wait for AWS `update/accepted`.
- Asynchronous HTTPS range request (`https_get_range`).
- Manifest version lookup (`manifest_get_version`).
- Storage read/write/delete/exists/size/free-space operations.
- Future crypto verification seam (`crypto_verify`).
- Future installer handoff (`installer_handoff`).

The default operations in `FOTA_SM.c` are target scaffolding only. Notably, default free-space reporting returns `UINT32_MAX` and the default installer handoff returns success. Production adapters must replace these behaviors.

## State machine

States declared in `FOTATypes.h`:

```text
WAIT_MANIFEST
IDLE
POLLING
ACQUIRE_JOB_DOCUMENT
VALIDATE_JOB
SELECT_PACKAGE
REPORT_SKIPPED
DOWNLOADING
DOWNLOAD_COMPLETED
VERIFY
VERIFICATION_COMPLETED
WAITING_FOR_INSTALLER
REPORT_FAILURE
FATAL
```

Boot and manifest behavior:

1. Load poll-budget and recovery records.
2. Wait for `MANIFEST_READY`. `MANIFEST_INVALID` makes FOTA unavailable until the next power cycle.
3. If no valid persisted job exists, enter `IDLE` without polling.
4. If a persisted job exists, validate/reparse its persisted document. Production recovery must call `mqtt_validate_job` to confirm the AWS execution still exists before resuming.
5. If the persisted document is expired, discard its artifacts and request a fresh document immediately after manifest readiness, subject to the poll budget.

Scheduling and polling:

- Accept `RTC_POLL_DUE` only in `IDLE` with a ready manifest.
- Reject events before six hours have elapsed since the last `$next/get` request.
- Permit four valid poll responses per local RTC calendar day; reset at local midnight.
- Persist request time when polling begins.
- Increment the daily count only after an accepted/valid AWS response, including a valid “no job” response. Failed/malformed responses do not consume budget.
- Claim AppMonitor before polling. If another application owns it, drop the event without consuming budget.
- Ignore scheduler events while a job or cloud operation is active.
- Retain AppMonitor ownership from accepted-job discovery through `WAITING_FOR_INSTALLER`; release it after no-job or terminal cleanup.

MQTT operations use QoS 1 and one initial attempt plus three retries. Failed status reports are ignored after the fourth attempt so the local state machine can continue. Authoritative canceled/not-found/invalid-state responses immediately clean the job, release AppMonitor, and return to `IDLE`.

## Job-document parsing

The expected document wrapper is `jobDocument` containing:

- `operation`
- `logs_url`
- `files[]`

Each file/package must contain:

- `ecu_type`
- `ecu_id`
- `min_ver`
- `target_ver`
- `seq_id`
- `url`
- `url_expiry`
- `compression`
- `hash`
- `package_size`
- `package_id`

Parser rules:

- Serialized document length is bounded to 24 KiB; package count is bounded to eight.
- Unknown fields are ignored for forward compatibility.
- Root/wrapper/required fields must have the expected object/string/array types.
- `operation` must be exactly `"fota"`.
- Package URLs must begin with `https://`.
- `compression` must be exactly `"none"`.
- `seq_id`, `package_size`, `url_expiry`, and `hash` must be quoted unsigned decimal strings with overflow checking.
- `package_size` must be nonzero and fit the 32-bit filesystem interface.
- `hash` is temporarily limited to an unsigned 32-bit value.
- Sequence IDs must be unique and form exactly `1..N`; the parser sorts the package array by sequence ID.
- The earliest `url_expiry` is stored as `u64_EarliestExpiry` and compared against the RTC epoch.
- Missing, malformed, unsupported, expired, or semantically invalid documents are recoverable acquisition failures. Request a completely fresh document up to four total attempts. Do not persist a document until validation succeeds.

### cJSON lifetime rule

`FOTAJob_T` stores an opaque `pv_JsonRoot`. The `FOTAStringView_T` fields point to cJSON-owned `valuestring` buffers, so the cJSON tree must remain alive while the validated job is active. Call `gv_FOTA_ReleaseJob()` before replacing or discarding a job. The state machine does this before a fresh parse and during terminal cleanup. Recovery reparses the persisted raw JSON and creates a new tree.

`cJSON_ParseWithLengthOpts(..., false)` is used for bounded, potentially non-NUL-terminated input. The implementation separately rejects non-whitespace trailing data. Status objects are created with cJSON and rendered into the fixed `FOTA_STATUS_BUFFER_BYTES` buffer using `cJSON_PrintPreallocated`.

## Package selection and version rules

Packages are evaluated in `seq_id` order with a fresh Manifest lookup for each package:

- `min_ver == "0"` means no lower bound.
- Otherwise require lexical, case-sensitive `min_ver <= current < target_ver`.
- `current < min_ver` fails the whole job.
- `current >= target_ver` reports `IN_PROGRESS`/`SKIPPED`, advances immediately, and does not wait for AWS acknowledgement.
- If every package is skipped, report `SUCCEEDED`/`SUCCESS`, clean all artifacts, release AppMonitor, and return to `IDLE`.

## Download behavior

`FOTADownloadSM` owns one `current.fota` file and one fixed 4096-byte transfer buffer.

For every block it requests:

```text
start = downloaded_bytes
end   = min(start + 4095, package_size - 1)
```

The HTTPS adapter/result must report:

- HTTP status exactly `206`.
- `Content-Range` start/end/total exactly matching the requested range and package size.
- Actual body length exactly equal to the requested byte count.
- No oversized response, partial write, or unexpected EOF.

Each range gets four total attempts. After a successful response, write at the exact offset and derive the authoritative resume offset from actual file size. Persist resume metadata after every range and before publishing crossed 25%, 50%, 75%, and 100% checkpoints.

Before starting a package, verify free space. Critical filesystem operations get four total attempts. Exhaustion is terminal failure (or `FATAL` when there is no active job).

If the package URL is expired before a request, persist `DOWNLOADING`, acquire a fresh document, delete the partial file and resume metadata, and restart the package at byte zero. Do not continue a partial file with a new signed URL.

On abrupt recovery, the actual `current.fota` size is authoritative only when matching resume metadata is valid and `0 <= file_size <= package_size`. Missing/corrupt/impossible metadata causes deletion and restart.

On `SHUTDOWN_PREPARE`, cancel or finish the in-flight range, persist the exact committed size and recovery context, acknowledge the preparation, and issue no further requests.

Persist `DOWNLOAD_COMPLETED` before entering `VERIFY`.

## Verification and installer boundary

Part A currently parses/retains the expected 32-bit hash but bypasses calculation and comparison. Every completed file advances unconditionally to `VERIFICATION_COMPLETED`, publishes that status, and is passed to `installer_handoff`.

`FOTA_ENABLE_CRYPTO_VERIFY` retains a future crypto/mismatch/redownload seam. Do not turn on a real hash algorithm without defining the production algorithm and acceptance tests.

After successful handoff, persist `WAITING_FOR_INSTALLER` and retain:

- `current.fota`
- resume metadata
- job/document context
- AppMonitor ownership

`INSTALLATION_COMPLETED` and `INSTALLATION_FAILED` events are reserved for Part B. Completion must contain matching job/package identity before Part B may authorize cleanup and select the next package. No installation behavior is implemented in Part A.

## Persistence and recovery

Managed UFS identifiers currently map to:

```text
eFS_FILE_FOTA_FILE             UFS:current.fota
eFS_FILE_FOTA_JOB_DOCUMENT_A   UFS:FOTAJobA.json
eFS_FILE_FOTA_JOB_DOCUMENT_B   UFS:FOTAJobB.json
eFS_FILE_FOTA_CONTEXT_A        UFS:FOTAContextA.bin
eFS_FILE_FOTA_CONTEXT_B        UFS:FOTAContextB.bin
eFS_FILE_FOTA_RESUME           UFS:FOTAResume.bin
eFS_FILE_FOTA_POLL_BUDGET      UFS:FOTAPoll.bin
```

Document and recovery records use inactive-slot writes. Records contain magic, version, serialized length, generation where applicable, and CRC. The inactive slot is written and read back/verified before becoming current. Recovery selects the newest valid context record.

The persisted recovery context includes state, current package index, document slot, downloaded/package size, and job identity. Resume metadata includes generation, downloaded bytes, expected size, sequence ID, and package identity. Poll metadata includes local calendar day, count, and last request epoch.

## Failure/reporting rules

For terminal download or validation failure:

1. Publish `IN_PROGRESS` with package-level `step=FAILED`, package identifiers, and error code.
2. Publish AWS `FAILED` with overall `step=FAILED`.
3. Ignore either report after its retry budget, delete active artifacts, release AppMonitor, and return to `IDLE`.

Authoritative AWS cancellation/not-found/invalid-state responses skip normal retry continuation and immediately perform cleanup.

## Required EC800 test matrix

Use target UFS and fakes for MQTT, HTTPS, Manifest, RTC, Crypto, and installer adapters. Include power-cycle tests.

Parser tests:

- Missing fields, wrong types, decimal overflow, more than eight packages, oversized documents.
- Unsupported operation/compression, invalid URL scheme, duplicate/gapped/out-of-order sequences.
- Invalid versions, zero/overflow package size, expired URL windows.
- Unknown-field forward compatibility.

Polling/cloud tests:

- Six-hour guard, four-per-day budget, midnight reset.
- Valid no-job response, malformed/failed poll not consuming budget.
- AppMonitor contention and expired-job boot polling.
- QoS 1 retry behavior, status-report exhaustion being ignored.
- Immediate cancellation cleanup.
- Package failure report followed by overall job failure.
- All-packages-skipped success.

Range/persistence tests:

- Aligned and final partial 4 KiB blocks.
- Exact Range headers and exact 206/Content-Range/body validation.
- Per-range retries, HTTP failures, partial writes, unexpected EOF.
- Insufficient space and filesystem retry exhaustion.
- Power loss after every persisted state.
- File-size-authoritative resume, corrupt record fallback, graceful-shutdown exact resume.
- URL-expiry fresh-document restart from byte zero.
- Persistence before progress/status checkpoints and before `VERIFY`.
- Explicit temporary verification bypass: hash mismatch must not block `VERIFICATION_COMPLETED`.

Measure linker allocation and task stack watermark. The product target is less than 32 KiB total FOTA working RAM, including the 24 KiB document arena, 4 KiB transfer buffer, parser/context memory, and task stack. The retained cJSON tree adds SDK heap usage and must be measured on the real target.

## Current validation and limitations

Completed locally:

- Strict C99 syntax compilation with warnings-as-errors for the FOTA/state-machine sources and related FS files.
- Syntax validation with the expected four-argument `cJSON_ParseWithLengthOpts` API shape.
- FOTA range-download smoke test.
- `git diff --check`.

Not completed locally:

- Full EC800 build, because the external QuecOpen `LinkSDK` and target headers are not present in this checkout.
- Runtime cJSON parser/status tests against the actual QuecOpen cJSON binary.
- Production MQTT, HTTPS, Manifest, RTC, Crypto, and installer integration.
- Real hash verification and Part B installation behavior.

## Known implementation caveats to review

- The current `FOTAJob_T` retains cJSON-owned strings for all validated package entries while the job is active. This is simple and keeps the state machine views stable, but it is more memory-intensive than the original design goal of retaining only the current package metadata and reloading later entries from the persisted raw document.
- cJSON uses the SDK allocator for its parse tree. The 32 KiB working-RAM target therefore cannot be considered met until the real target heap/linker map is measured. If necessary, introduce a bounded cJSON allocator or compact/copy only the metadata needed by the active package.
- `mqtt_validate_job` is optional in the current scaffolding. Production recovery must provide it; otherwise the current fallback waits for manifest readiness and resumes locally without AWS execution confirmation.
- `ge_FOTA_Init(NULL)` intentionally exercises default scaffolding. It is not a production cloud/HTTPS integration.
- Event payload pointers are borrowed. The adapter that posts an event must keep `pc_Payload` valid until the manager consumes the queued event, or change the event-ownership design before production integration.

## Recommended continuation order

1. Obtain the exact EC800/QuecOpen SDK revision and build the target CMake configuration.
2. Confirm the SDK cJSON header/API and run parser/status tests against the real library.
3. Implement production `FOTAPlatformOps_T` adapters, especially asynchronous event correlation and payload lifetime ownership.
4. Build the EC800 fake-adapter harness and execute the complete matrix above, including power-cycle/UFS corruption cases.
5. Measure cJSON heap usage, linker RAM, and task stack watermark; adjust the representation or allocator if the 32 KiB target is exceeded.
6. Only after Part A acceptance, implement Part B installer authorization, completion cleanup, and next-package progression.
