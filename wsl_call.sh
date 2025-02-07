#!/bin/bash

# Initialize an array to store processed arguments
processed_args=()
add_plugin=false

# Iterate over each input argument
for arg in "$@"; do
    # Step 1: Replace \ with / in the argument
    arg="${arg//\\//}"

    # Step 2: Replace any single character followed by :/ with /mnt/ followed by the character (lowercase) and /
    # Use sed for regex replacement
    arg=$(echo "$arg" | sed -E 's/([a-zA-Z]):\//\/mnt\/\L\1\//g')

    # Step 3: If the argument starts with @ and ends with .args
    if [[ "$arg" == @*.args ]]; then
        # Remove the leading @ symbol
        filename="${arg:1}"

        # Use sed to replace content in the file
        sed -i -E 's/([a-zA-Z]):\//\/mnt\/\L\1\//g' "$filename"

        if grep -q "\-c" "$filename"; then
            add_plugin=true
        fi
    fi

    # Add the processed argument to the array
    processed_args+=("$arg")
done

# If -c was found, add the plugin argument
if $add_plugin; then
    processed_args+=("-fplugin=gccsection13")
fi

# Call wsl and pass the processed arguments
wsl "${processed_args[@]}"
