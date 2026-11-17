#!/bin/sh
# Comprueba la logica de SITRAC. La interfaz es la aplicacion de Windows.
set -eu
cd "$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
make test
