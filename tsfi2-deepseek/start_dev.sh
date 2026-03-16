#!/bin/bash
echo "Copying local unversioned assets to build environment..."
rsync -a ../../assets/ ./assets/ --exclude="models" --exclude="sd_cpp" --exclude="*.safetensors"
echo "Development environment initialized."
