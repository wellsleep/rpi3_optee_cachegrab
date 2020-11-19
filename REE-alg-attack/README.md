1. 修改 `shim-ree/ree_shim.c` 以增添 REE 自定义函数功能
2. 在 `shim-ree` 中 `make`，生成 `libree_shim.so`
3. `make` 本文件夹，生成 `ree_program`
4. 将 `libree_shim.so` 拷贝到上一层目录，即 `REE-alg-attack`；并将 `ree_program` 拷贝到 `/bin`
5. 在 cachegrab 中运行 `ree_program`
