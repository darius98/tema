#!/usr/bin/env sh

set -e

./setup-deps.sh

./check.sh ${1}
