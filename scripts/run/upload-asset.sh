#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <experimentKey> <fileName>"
    exit 1
fi

# Assign arguments to variables
experimentKey=$1
fileName=$2

curl -X POST "https://www.comet.com/api/rest/v2/write/experiment/upload-asset?experimentKey=${experimentKey}&fileName=${fileName}" \
     -H "Authorization: $COMET_API_KEY" \
     -F "file=@${fileName}"
