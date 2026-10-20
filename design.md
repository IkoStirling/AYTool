# AYTool command-line design

**Version:** 0.1.0
**Date:** 2026-10-21
**Status:** Active
**Owner:** Toolchain / AYResource

The tool executables remain thin process adapters. AYResource owns import
recipes, content fingerprints, conversion, staging and publication; AYTool owns
argument validation, human-readable progress and process exit codes. There is
no dependency on AYEditor or its UI.

import_tool has two mutually exclusive inputs: explicit --in/--out arguments
(single or batch), or --project plus one persisted --recipe ID.
--list-recipes is read-only and lists the shared reader's stable file IDs and
resolved sources. Recipe mode rejects command-line settings overrides and
ignores the cook environment override, retaining the complete authored recipe;
only force and destination may be explicitly overridden. Bad recipes and mixed
modes fail before conversion. Unit values must be finite, nonnegative and fully
parsed. Windows wmain converts UTF-16 argv to UTF-8 before shared APIs consume
paths; source text encoding flags alone cannot fix the old ACP argv boundary.

A successful process only confirms published files. It does not own another
Editor process, ResourceManager or Renderer and therefore does not send live GPU
reload notifications. Failure exit2 retains prior imported files under the
shared transaction; usage/settings errors use exit1, cancellation exit3.

import_tool/tests/RecipeWorkflow.cmake is a module-local process integration
test. It creates a real glTF with embedded geometry and derives a complete
recipe from the production codec's seed metadata. It checks Unicode/space paths,
IDs/namespaces, environment isolation, cache, ordinary failures, repair and
output override. Each run uses an isolated nonce directory and preserves it
for inspection. No built-in asset pack or runtime resource format is added.
