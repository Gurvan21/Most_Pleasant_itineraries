#!/bin/bash
# Pour chaque tests/itineraries.*.in :
#   - produit outputItineraries/itineraries.X.out (réponses aux requêtes, une ligne par requête)
#   - remplit Runtimes/itineraries.X/ avec v1.txt, v2.txt, v3.txt (une ligne par requête ; N/A si une requête > 30 s)
# Le programme n'est pas limité en temps (pas de timeout global).
# Utilise plusieurs cœurs : NPROC tests en parallèle (défaut = nombre de cœurs).
#   Exemple : NPROC=4 ./scripts/run_itineraries_with_output.sh
# Plage de tests (optionnel) : START et END = numéros de test (inclus).
#   Exemple : START=4 END=9 NPROC=6 ./scripts/run_itineraries_with_output.sh  # uniquement tests 4 à 9, 6 en parallèle
# À exécuter depuis la racine du projet (dossier contenant Makefile, tests/, scripts/).

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

# Nombre de tests à lancer en parallèle (un processus = un cœur utilisé)
NPROC=${NPROC:-$(nproc 2>/dev/null || echo 4)}

# Plage de tests (vide = tous les tests)
START=${START:-}
END=${END:-}

# Créer les dossiers de sortie
mkdir -p outputItineraries
mkdir -p Runtimes

# Compiler si nécessaire
make -q output/main 2>/dev/null || make

MAIN="./output/main"

# Extraire les temps par requête depuis un fichier log
write_query_times() {
  local log="$1"
  local start_marker="$2"
  local end_marker="$3"
  local outfile="$4"
  sed -n "/^${start_marker}$/,/^${end_marker}$/p" "$log" \
    | grep -v "^${start_marker}$" | grep -v "^${end_marker}$" \
    | awk '$1+0 >= 30000 { print "N/A"; next } { print }' \
    > "$outfile"
}

run_one_test() {
  local infile="$1"
  local base=$(basename "$infile" .in)
  local runtime_dir="$ROOT/Runtimes/${base}"
  local log=$(mktemp)
  mkdir -p "$runtime_dir"
  echo "  [$$] Début $base"
  "$MAIN" "$infile" outputItineraries > "$log" 2>&1
  write_query_times "$log" "RUNTIME_V1_QUERIES_START" "RUNTIME_V1_QUERIES_END" "$runtime_dir/v1.txt"
  grep '^RUNTIME_V2_PREPROCESSING ' "$log" | awk '{print $2}' > "$runtime_dir/preprocessing_v2.txt" || true
  write_query_times "$log" "RUNTIME_V2_QUERIES_START" "RUNTIME_V2_QUERIES_END" "$runtime_dir/v2.txt"
  grep '^RUNTIME_V3_PREPROCESSING ' "$log" | awk '{print $2}' > "$runtime_dir/preprocessing_v3.txt" || true
  write_query_times "$log" "RUNTIME_V3_QUERIES_START" "RUNTIME_V3_QUERIES_END" "$runtime_dir/v3.txt"
  sed -n '/^n = /,/Résultats identiques/p' "$log" | grep -v '^RUNTIME_' > "$runtime_dir/summary.txt" || true
  rm -f "$log"
  echo "  [$$] Fin $base"
}

# Construire la liste des tests à lancer
batch=()
if [ -n "$START" ]; then
  END=${END:-$START}
  for i in $(seq "$START" "$END"); do
    f="tests/itineraries.$i.in"
    [ -f "$f" ] && batch+=("$f")
  done
else
  for f in tests/itineraries.*.in; do
    [ -f "$f" ] || continue
    batch+=("$f")
  done
  # Trier par numéro (0, 1, 2, ... 9)
  batch=($(printf '%s\n' "${batch[@]}" | sort -V))
fi

[ ${#batch[@]} -eq 0 ] && { echo "Aucun test à lancer."; exit 0; }

echo "=== Génération des itineraries.out et des Runtimes (jusqu'à $NPROC tests en parallèle) ==="
if [ -n "$START" ]; then
  echo "    Plage : tests $START à $END (${#batch[@]} fichier(s))"
fi

if [ "$NPROC" -le 1 ]; then
  # Mode séquentiel : affichage direct (tee)
  for infile in "${batch[@]}"; do
    base=$(basename "$infile" .in)
    runtime_dir="Runtimes/${base}"
    mkdir -p "$runtime_dir"
    echo ""
    echo "--- $infile ---"
    log=$(mktemp)
    set +e
    "$MAIN" "$infile" outputItineraries 2>&1 | tee "$log"
    set -e
    write_query_times "$log" "RUNTIME_V1_QUERIES_START" "RUNTIME_V1_QUERIES_END" "$runtime_dir/v1.txt"
    grep '^RUNTIME_V2_PREPROCESSING ' "$log" | awk '{print $2}' > "$runtime_dir/preprocessing_v2.txt" || true
    write_query_times "$log" "RUNTIME_V2_QUERIES_START" "RUNTIME_V2_QUERIES_END" "$runtime_dir/v2.txt"
    grep '^RUNTIME_V3_PREPROCESSING ' "$log" | awk '{print $2}' > "$runtime_dir/preprocessing_v3.txt" || true
    write_query_times "$log" "RUNTIME_V3_QUERIES_START" "RUNTIME_V3_QUERIES_END" "$runtime_dir/v3.txt"
    sed -n '/^n = /,/Résultats identiques/p' "$log" | grep -v '^RUNTIME_' > "$runtime_dir/summary.txt" || true
    rm -f "$log"
    [ -f "outputItineraries/${base}.out" ] && echo "  -> outputItineraries/${base}.out ($(wc -l < "outputItineraries/${base}.out") lignes)"
    echo "  -> $runtime_dir/v1.txt, v2.txt, v3.txt"
  done
else
  # Mode parallèle : par lots de NPROC tests
  i=0
  while [ "$i" -lt "${#batch[@]}" ]; do
    end=$((i + NPROC))
    [ "$end" -gt "${#batch[@]}" ] && end=${#batch[@]}
    echo ""
    echo "--- Lot $((i/NPROC + 1)) : tests $i à $((end-1)) ---"
    while [ "$i" -lt "$end" ]; do
      run_one_test "${batch[$i]}" &
      i=$((i + 1))
    done
    wait
  done
fi

echo ""
echo "=== Fin. outputItineraries/ et Runtimes/ sont à jour. ==="
