#!/bin/bash

echo "VoIP Demo stack/applications release"

source build/header.rc
source build/install.rc

mod_dir=$(dirname "$0")
dst_dir=${CONFIG_SYNA_SDK_REL_PATH}/application/demos
INSTALL_D ${mod_dir} ${CONFIG_SYNA_SDK_REL_PATH}/application

rm ${dst_dir}/release.sh
