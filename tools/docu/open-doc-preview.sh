#!/bin/bash
# Open documentation preview for current .rst file

FILE="$1"

# Check if file is in docs folder
if [[ "$FILE" != */docs/* ]]; then
    echo "❌ Not a docs file: $FILE"
    exit 1
fi

# Extract relative path and convert .rst to .html
REL_PATH="${FILE#*/docs/}"
HTML_FILE="${REL_PATH%.rst}.html"
URL="http://localhost:8000/$HTML_FILE"

echo "📖 Opening documentation preview: $URL"

# Try to open in browser
if command -v xdg-open &> /dev/null; then
    xdg-open "$URL" 2>/dev/null &
elif [[ "$BROWSER" ]]; then
    $BROWSER "$URL" &
else
    # Fallback: print URL for user to open
    echo ""
    echo "✅ URL ready - open in your browser:"
    echo "   $URL"
    echo ""
fi
