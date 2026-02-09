#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_DIR="${ROOT_DIR}/.venv"
LIBCLANG_DIR="${ROOT_DIR}/third_party/libclang"

echo "Setting up libclang virtual environment..."
python3 -m venv "${VENV_DIR}"
source "${VENV_DIR}/bin/activate"

python -m pip install --upgrade pip
python -m pip install libclang

export LIBCLANG_DIR
python - <<'PY'
import os
import site
import sys
from pathlib import Path
from shutil import copy2

libclang_dir = Path(os.environ["LIBCLANG_DIR"])
search_roots = []
search_roots.extend(site.getsitepackages())
search_roots.append(site.getusersitepackages())

candidates = []
for root in filter(None, search_roots):
    root_path = Path(root)
    if not root_path.exists():
        continue
    candidates.extend(root_path.rglob("libclang.*"))

preferred = [c for c in candidates if c.is_file() and c.name.startswith("libclang.")]
if not preferred:
    sys.exit("libclang shared library not found in the virtual environment.")

libclang_dir.mkdir(parents=True, exist_ok=True)
source_path = preferred[0]
destination = libclang_dir / source_path.name
copy2(source_path, destination)
print(f"Copied {source_path} -> {destination}")
PY

cat <<EOF

libclang setup complete.

Activate the environment:
  source "${VENV_DIR}/bin/activate"

Export the library path before running tooling:
  export CLANG_LIBRARY_PATH="${LIBCLANG_DIR}"

EOF
