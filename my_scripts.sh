#!/usr/bin/bash

NC='\033[0m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
BOLD_YELLOW='\033[1;33m'
BOLD_RED='\033[1;31m'

echo -e "${CYAN}Source SDK         -> ${BOLD_YELLOW}source_sdk()${NC}"
source_sdk () {
    source /home/tarnished/.espressif/tools/activate_idf_v6.0.1.sh
}

echo -e "${CYAN}Build ESP project  -> ${BOLD_YELLOW}build_esp()${NC}"
build_esp() {
    idf.py build
}

echo -e "${CYAN}Flash build        -> ${BOLD_YELLOW}flash_esp()${NC}"
export PORT=""

is_port_exported() {
    # FIXED: Added missing space before the closing bracket ']'
    if [ -z "$PORT" ]; then
        echo -e "${BOLD_YELLOW}Enter port name:${NC}"
        read PORT
    else
        echo -e "${GREEN}Port of device is -> ${PORT}${NC}"
    fi
}

flash_esp() {
    is_port_exported
    idf.py -p "${PORT}" flash
}

echo -e "${CYAN}BUILD, FLASH & MONITOR -> ${BOLD_YELLOW}esp_build_flash_monitor()${NC}"
esp_build_flash_monitor() {
    build_esp && is_port_exported && idf.py -p "${PORT}" flash || true

    echo -e "${CYAN}Waiting 3 seconds for port to settle...${NC}"
    sleep 3

    echo -e "${CYAN}Starting Monitor...${NC}"
    idf.py monitor
}
