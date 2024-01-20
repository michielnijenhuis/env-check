if [ -d "src/lib" ]; then
    mkdir -p "src/lib"
fi

cd "src/lib"
symlink_lib "array-list" "cli" "cstring" "hash-table" "types" "utils"