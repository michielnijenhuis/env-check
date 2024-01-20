if [ -d "src/lib" ]; then
    mkdir -p "src/lib"
fi

cd "src/lib"

function symlink_lib() {
    ln -sf "$HOME/dotfiles/lib/$1.h" "$(pwd)"
}

symlink_lib "array-list" "cli" "cstring" "hash-table" "types" "utils"