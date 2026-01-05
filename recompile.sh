#!/bin/bash

cd /home/lotus/PROJETOS/pcg729
# bin/spc del-download psampler
rm -rf source/php-src/ext/bcg729
rm -rf downloads/bcg729
rm /home/lotus/PROJETOS/bcg729/php




# bin/spc download psampler
cp -r /home/lotus/PROJETOS/bcg729 /home/lotus/PROJETOS/pcg729/downloads/bcg729
bin/spc build --build-cli "swoole,bcg729" --debug --enable-zts


cp buildroot/bin/php /home/lotus/PROJETOS/bcg729/php