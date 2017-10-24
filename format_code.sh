#!/bin/bash

set -e

cd `dirname $0`

for ext in "*.h" "*.cpp"; do
	for f in `find . -name $ext`; do
		echo "Formatting $f"
		clang-format -style=file -i $f
	done
done
