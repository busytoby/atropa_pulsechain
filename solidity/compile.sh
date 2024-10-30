#!/bin/sh

for i in `find dysnomia -newer last_compile |grep -i sol$ |grep -v interfaces`; do ../../../solc.exe --bin --abi --optimize --optimize-runs=200 -o C:\\Users\\mariarahel\\source\\repos\\atropa_pulsechain\\Wallet\\bin\\Contracts --evm-version=shanghai $i --overwrite; done; touch last_compile
