#!/bin/bash

set -e -u -o pipefail

GREEN=$'\e[1;32m'
RED=$'\e[1;31m'
CYAN=$'\e[0;36m'
MAGENTA=$'\e[0;35m'
BLUE=$'\e[0;34m'
YELLOW=$'\e[1;33m'
NC=$'\e[0m'

in_file="LittlePrince.txt"
inflag=0
baseline=0
sleepflag=0
blocksizeflag=0

CLIENT_COMMANDS=""

SERVER_FILE="basic_server/server.cpp"
ENCODER_FILE="basic_server/encoder.cpp"
CLIENT_FILE="Client/client.cpp"
DECODER_FILE="Decoder/Decoder.cpp"

ENCODER_PID=0

function cleanup {
    echo -e "${YELLOW}Cleaning up...${NC}"
    if [ $ENCODER_PID -ne 0 ]; then
        kill $ENCODER_PID 2>/dev/null || true
    fi
    rm -f output_cpu.bin encoder client

    if [ $baseline == 0 ]; then
        rm -f decoder output_decoded.bin
    fi
}

trap cleanup ERR

for arg in $@; do
    if [ "$arg" == "-f" ]; then
	    inflag=1
    elif [ $inflag == 1 ]; then
        in_file=$arg
        inflag=0
    elif [ "$arg" == "-bl" ]; then
        baseline=1
        SERVER_FILE="Server/server.cpp"
        ENCODER_FILE="Server/encoder.cpp"
    elif [ "$arg" == "-b" ]; then
        blocksizeflag=1
    elif [ $blocksizeflag == 1 ]; then
        CLIENT_COMMANDS+="-b $arg "
        blocksizeflag=0
    elif [ "$arg" == "-s" ]; then
        sleepflag=1
    elif [ $sleepflag == 1 ]; then
        CLIENT_COMMANDS+="-s $arg "
        sleepflag=0
    else
        echo "Warning: "${arg}" is not a recognized command option"
        echo ""
    fi
done

echo -e "${YELLOW}Compiling ${ENCODER_FILE} and ${CLIENT_FILE}${NC}"
g++ ${ENCODER_FILE} ${SERVER_FILE} -o encoder
g++ ${CLIENT_FILE} -o client

echo -e "${YELLOW}Running encoder${NC}"
(./encoder 2>&1 | sed -u "s/^/${CYAN}[ENCODER] ${NC}/") & disown
ENCODER_PID=$!

sleep 0.5

echo -e "${YELLOW}Running client${NC}"
./client -f $in_file -i 127.0.0.1 $CLIENT_COMMANDS 2>&1 | sed -u "s/^/${MAGENTA}[CLIENT]  ${NC}/"

if [ $baseline == 1 ]; then
    echo ""
    echo -e "${YELLOW}Comparing output${NC}"
    if diff -q output_cpu.bin "$in_file" > /dev/null 2>&1; then
        echo -e "${GREEN}TEST PASSED ✔️${NC}"
    else
        echo -e "${RED}TEST FAILED ❌${NC}"
    fi
else
    echo -e "${YELLOW}Compiling decoder${NC}"
    g++ ${DECODER_FILE} -o decoder

    echo -e "${YELLOW}Running decoder${NC}"
    ./decoder output_cpu.bin output_decoded.bin  2>&1 | sed -u "s/^/${BLUE}[DECODER] ${NC}/"

    echo -e "${YELLOW}Comparing output${NC}"
    if diff -q output_decoded.bin "$in_file" > /dev/null 2>&1; then
        echo -e "${GREEN}TEST PASSED ✔️${NC}"
    else
        echo -e "${RED}TEST FAILED ❌${NC}"
    fi

    rm decoder output_decoded.bin
fi

rm output_cpu.bin encoder client

echo ""
echo "TEST COMPLETE"
