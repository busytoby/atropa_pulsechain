#!/bin/sh

for i in `find dysnomia -newer last_compile |grep -i sol$ |grep -v interfaces`; do ../../../solc.exe --combined-json=bin,abi --optimize --optimize-runs=200 --evm-version=shanghai $i > ../Wallet/bin/Contracts/`basename $i`.json; done; touch last_compile
