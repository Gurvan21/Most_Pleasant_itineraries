#!/bin/bash
# Lance les trois méthodes (itineraries_v1, v2, v3) sur chaque fichier tests/itineraries.*.in
# À exécuter depuis la racine du projet (dossier contenant Makefile et output/).

set -e
cd "$(dirname "$0")/.."
make -q output/main 2>/dev/null || make

echo "=== Exécution des tests sur tous les fichiers tests/itineraries.*.in ==="
for f in tests/itineraries.*.in; do
  if [ -f "$f" ]; then
    echo ""
    ./output/main "$f" || true
  fi
done
echo ""
echo "=== Fin ==="
