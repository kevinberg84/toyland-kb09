#!/usr/bin/env bash
set -euo pipefail
folder="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
chmod u+x -- "$folder/Toyland KB-09 Standalone"
exec "$folder/Toyland KB-09 Standalone" "$@"
