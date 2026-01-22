#!/bin/bash

# Script: create_files_with_subdirs.sh
# Purpose: Create directory structure with files and log all activities

# Define log file location
LOG_FILE="script.log"

# Array of programming languages
declare -a LANGUAGES=("Python" "Java" "C++" "JavaScript" "Ruby" "Go" "Rust" "PHP" "Swift" "Kotlin")

# Function to log messages with timestamp
log_message() {
    local message="$1"
    local timestamp=$(date '+[%Y-%m-%d %H:%M:%S]')
    echo "${timestamp} ${message}" >> "$LOG_FILE"
}

# Clear previous log file if it exists
> "$LOG_FILE"

# Log script start
log_message "Script execution started"

# Create main directory with current date and time
MAIN_DIR="$(date '+%Y%m%d_%H%M%S')"
mkdir "$MAIN_DIR"
log_message "Created main directory: $MAIN_DIR"

# Create 10 subdirectories (file101 to file110)
for i in {101..110}; do
    SUBDIR="$MAIN_DIR/file$i"
    mkdir "$SUBDIR"
    log_message "Created subdirectory: file$i"
    
    # Create 10 .txt files in each subdirectory (tuser501.txt to tuser510.txt)
    for j in {501..510}; do
        FILE="$SUBDIR/tuser$j.txt"
        
        # Get the language based on the file number (cycling through the array)
        LANGUAGE_INDEX=$(( (j - 501) % ${#LANGUAGES[@]} ))
        LANGUAGE="${LANGUAGES[$LANGUAGE_INDEX]}"
        
        # Write the programming language name to the file
        echo "$LANGUAGE" > "$FILE"
        log_message "Created file: file$i/tuser$j.txt (Content: $LANGUAGE)"
    done
done

# Log script completion
log_message "Script execution completed successfully"

echo "Script completed. Check script.log for details."
