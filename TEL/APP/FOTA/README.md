# FOTA Download Part A

This directory contains the download-only FOTA application for EC800/QuecOpen.
`FOTAManager` owns the event queue and `FOTA_SM` owns the persisted lifecycle;
`FOTADownloadSM` performs one 4 KB HTTPS range request at a time.

Job documents, cloud responses, and status payloads use the QuecOpen cJSON
library. The parsed cJSON tree is retained only for the lifetime of the active
validated job and released during terminal cleanup.

Installation, flashing, deployment, decompression, and installation-log upload
are intentionally not implemented. `FOTAInstallSM.h` reserves the identity-
checked completion boundary for Part B.

## Product decisions

- Poll opportunities arrive as `RTC_POLL_DUE` events. FOTA enforces the six-hour
  guard and four-valid-responses-per-local-day budget.
- Validated job documents and recovery records use inactive-slot writes with
  CRC-protected versioned records. `current.fota` is resumed from its actual
  file size only when matching resume metadata is valid.
- Package URLs must be HTTPS and use `compression: "none"`. The earliest
  `url_expiry` controls local job-document validity.
- **Temporary verification bypass:** the quoted 32-bit `hash` is parsed and
  retained, but no hash algorithm is available in Part A. Every completed file
  therefore advances to `VERIFICATION_COMPLETED` and is handed to the
  installer seam. A future Crypto implementation can replace this branch
  without changing the download state machine (`FOTA_ENABLE_CRYPTO_VERIFY`
  enables the retained mismatch/redownload seam).

Production MQTT, HTTPS, Manifest, RTC, Crypto, and installer adapters are
provided through `FOTAPlatformOps_T`. The EC800 target harness should supply
those adapters and use the managed UFS identifiers in `FS_Types.h`.

The manager queue is deliberately bounded and the task stack is configured for
the EC800 memory budget; release validation must still measure the linker map
and task watermark to enforce the sub-32 KB Part A working-RAM target.
