#!/bin/bash

# --- Configuration ---
OUTPUT_FILE="multicart_16k_020.bin"
export CHIP_SIZE_KB=256
BANK_SIZE_BYTES=16384

# --- List of 16K games in order from Bank 0 to Bank 15 ---
#
# EDIT THIS LIST with the filenames of your 16K ROMs.
# DriveWire should be one of them.
#
GAMES=(
01-DiagnosticDragon.ccc
02-DP50PSR-DW4.ccc
03-hdblbadgn.ccc
04-JDOS.ccc
05-ColorScripsit2.ccc
06-ColorFile2.ccc
07-EDTASM.ccc
08-Arkanoid.ccc
09-SuperLogo.ccc
10-DragonViewdata.rom
11-Silpheed.ccc
12-Spectaculator.ccc
13-DungeonsofDaggorath.ccc
14-Downland.ccc
15-Tetris.ccc
16-DoodleBug.ccc
)

echo "🚀 Starting 16K (SST39SF020) multi-cart build..."

# Step 1: Create an empty 256KB binary file (Mac-compatible)
echo "Creating a blank ${CHIP_SIZE_KB}KB image file named ${OUTPUT_FILE}..."
perl -e 'print "\xff" x (1024 * $ENV{CHIP_SIZE_KB})' > "$OUTPUT_FILE"

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
