#!/bin/bash

# --- Configuration ---
OUTPUT_FILE="multicart.bin"
CHIP_SIZE_KB=256
BANK_SIZE_BYTES=8192

# --- Final List of 32 Games ---
GAMES=(
    "01-Diagnosticsv2Tandy.ccc"
    "02-DiagnosticDragon.ccc"
    "03-DungeonsofDaggorath.ccc"
    "04-Downland.ccc"
    "05-SevenCardStud.ccc"
    "06-Androne.ccc"
    "07-Backgammon4K.ccc"
    "08-Bustout4K.ccc"
    "09-CanyonClimber.ccc"
    "10-CastleGuard4K.ccc"
    "11-CheckerKing4K.ccc"
    "12-ClownsBalloons.ccc"
    "13-ColorBaseball.ccc"
    "14-ColorLogo.ccc"
    "15-ColorRobotBattle.ccc"
    "16-DemolitionDerby.ccc"
    "17-Football.ccc"
    "18-GinChampion.ccc"
    "19-MACE.rom"
    "20-MonsterMaze.ccc"
    "21-PanicButton.ccc"
    "22-Pinball.ccc"
    "23-Polaris.ccc"
    "24-ProjectNebula.ccc"
    "25-AllDreamDragon.ccc"
    "26-AstroblastDragon.ccc"
    "27-BerserkDragon.ccc"
    "28-BridgeMasterDragon.ccc"
    "29-CaveHunterDragon.ccc"
    "30-CosmicInvadersDragon.ccc"
    "31-DoodleBugDragom.ccc"
    "32-StarshipChameleonDragon.ccc"
)

echo "🚀 Starting multi-cart build..."

# Step 1: Create an empty binary file
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
