#!/bin/bash

function exitWithErrorCode() {
	[ -f "${INPUT}" ] && rm "${INPUT}"
	[ -f "${OUTPUT}" ] && rm "${OUTPUT}"
	[ -f "${GENERATED_OUTPUT}" ] && rm "${GENERATED_OUTPUT}"
	exit $1
}

if [ $# -ne 1 ] || [ ! -f "${1}" ]; then
	echo "Lucie simple testrunner"
	echo "  usage: ${0} TESTFILE"
	echo ""
	exitWithErrorCode 1
fi

# Generate needed tempfiles
INPUT=$(tempfile)
OUTPUT=$(tempfile)
GENERATED_OUTPUT=$(tempfile)

# Split the testfile
SPLITLINE=$(grep -n -- "---lucietest-output---" "${1}"|awk -F ":" '{ print $1 }')
if echo "${SPLITLINE}" | grep -v "[0-9]\+"; then
	echo "${1} is not a valid lucietest file:"
	echo "  missing '---lucietest-output---' line"
	exitWithErrorCode 2
fi

LINECOUNT=$(wc -l "${1}"|awk '{ print $1 }')
let "FROMTOP = SPLITLINE - 1"
let "FROMBOTTOM = LINECOUNT - SPLITLINE"

head -n ${FROMTOP} "${1}" >"${INPUT}"
tail -n ${FROMBOTTOM} "${1}" >"${OUTPUT}"

# Run the test
echo "" >>testlog.log
echo "---" >>testlog.log
echo "Running test ${1}" >>testlog.log
echo "---" >>testlog.log
echo "" >>testlog.log

src/lucie "${INPUT}" 1>${GENERATED_OUTPUT} 2>>testlog.log

if [ $? -ne 0 ]; then
	echo "Error lucie exited with non zero error code:"
	echo "  executed testcase: ${1}"
	exitWithErrorCode 3
fi

# Compare results
diff -u "${OUTPUT}" "${GENERATED_OUTPUT}"

if [ $? -ne 0 ]; then
	echo "Testcase failed: ${1}"
	exitWithErrorCode 4
fi

exitWithErrorCode 0
