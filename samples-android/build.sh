#!/usr/bin/env bash
set -eua

# List of targets to build. Interpreted as directories relative to
# this script's path.
declare targets=(           \
  CollectAllTheStarsNative  \
  Minimalist                \
  TbmpSkeletonNative        \
  Teapot                    \
  TrivialQuestNative        \
  ButtonClicker             \
)

declare -r script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# This little piece of code prepends the project name and a timestamp to every line in the
# output, which is kind of handy.
function add_timestamp {
    while IFS= read -r line; do
        echo "[${1}:$(date)] $line"
    done
}


# Build each target in turn.
declare -i status=0
declare -i pipestatus=0
declare failed="Failures: "
for target in "${targets[@]}"; do
  echo "Building ${target}..." >&2
  ${script_dir}/${target}/src/main/build.sh $@ 2>&1 \
    | add_timestamp ${target} | tee build.log
  pipestatus=${PIPESTATUS[0]}
  status=status+pipestatus
  [ ${pipestatus} -ne 0 ] && failed=${failed}.${target}
done

echo ${status}
echo ${failed}
[ ${status} -ne 0 ] && echo "Build failed" >&2 || echo "Build Succeeded" >&2
exit ${status}


