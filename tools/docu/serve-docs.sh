echo "Serving documentation at http://localhost:8000 ..."
#!/bin/bash
# Serve Sphinx documentation locally
# Usage: ./serve-docs.sh
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/docs/_build/html"
echo "Serving documentation at http://localhost:8000 from $BUILD_DIR ..."
cd "$BUILD_DIR"
python3 -m http.server 8000
