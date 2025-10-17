#!/bin/bash

# --- Configuration ---
OUTPUT_FILE="multicart_16k.bin"
CHIP_SIZE_KB=512
BANK_SIZE_BYTES=16384

# --- List of 16K games in order from Bank 0 to Bank 31 ---
#
# EDIT THIS LIST with the filenames of your 16K ROMs.
# The first file goes in Bank 0, the second in Bank 1, and so on.
#
GAMES=(
    "game_for_bank_0.rom"
    "game_for_bank_1.rom"
    # "another_game.rom"
    # ... add up to 32 game files here ...
)

echo "🚀 Starting 16K multi-cart build..."

# Step 1: Create an empty 512KB binary file filled with 0xFF
echo "Creating a blank ${CHIP_SIZE_KB}KB image file named ${OUTPUT_FILE}..."
tr '\0' '\377' < /dev/zero | dd of="$OUTPUT_FILE" bs=1k count=$CHIP_SIZE_KB status=none

# Step 2: Loop through the game list and inject each file
for i in "${!GAMES[@]}"; do
    BANK_NUM=$i
    FILENAME="${GAMES[$i]}"
    OFFSET=$(( BANK_NUM * BANK_SIZE_BYTES ))

    if [ -f "$FILENAME" ]; then
        echo "Injecting [${FILENAME}] into Bank ${BANK_NUM} at offset ${OFFSET}..."
        dd if="$FILENAME" of="$OUTPUT_FILE" bs=1 seek=$OFFSET conv=notrunc status=none
    else
        echo "⚠️  Warning: File '${FILENAME}' not found. Skipping Bank ${BANK_NUM}."
    fi
done

echo "✅ Build complete! Your file '${OUTPUT_FILE}' is ready."
