#!/bin/bash

# Edit the following variables to match your Comet workspace and project
workspaceName="genetic-programming"
projectName="tpg"

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <experimentName>"
    exit 1
fi

# Assign arguments to variables
experimentName=$1

response=$(curl -s -X POST https://www.comet.com/api/rest/v2/write/experiment/create \
     -H "Content-Type: application/json" \
     -H "Authorization: $COMET_API_KEY" \
     -d "{
            \"workspaceName\": \"${workspaceName}\",
            \"projectName\": \"${projectName}\",
            \"experimentName\": \"${experimentName}\"
        }")

# Extract the desired value from the JSON response
experimentKey=$(echo $response | jq -r '.experimentKey')

# Check if the extraction was successful
if [ -z "$experimentKey" ]; then
    echo "Error: Failed to extract experimentKey from the response"
    exit 1
fi

# Return the extracted value
echo $experimentKey