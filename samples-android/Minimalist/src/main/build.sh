#!/usr/bin/env bash
set -eua

declare -r script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "${script_dir}"
source ../../../build_sample.sh "$@"

