#!/bin/bash
echo "Downloading precompiled NCNN for rapid C++ linking..."
wget -q https://github.com/Tencent/ncnn/releases/download/20240102/ncnn-20240102-ubuntu-2204.zip
unzip -q ncnn-20240102-ubuntu-2204.zip
mv ncnn-20240102-ubuntu-2204 thirdparty/ncnn
rm ncnn-20240102-ubuntu-2204.zip
echo "Done."
