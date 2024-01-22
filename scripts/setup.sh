if [ ! -d "src/lib" ]; then
    mkdir -p "src/lib"
fi

cd "src/lib"

function symlink_lib() {
    for lib in $@; do
        ln -sf "$HOME/dotfiles/lib/$lib.h" "$(pwd)";
    done
}

symlink_lib "array-list" "cli" "cstring" "hash-table" "types" "utils"