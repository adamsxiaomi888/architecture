# 浏览器核心桥接

`bridge.c` 将同一份 `er_runtime` 和虚拟舵机连接成浏览器可调用的 C API。没有安装 Emscripten 时也可作为普通 C 源码编译检查；安装 `emcc` 后可按下面命令生成 WASM：

```sh
emcc -Iinclude -Isim web/wasm/bridge.c src/runtime.c sim/sim.c \
  -O2 -s WASM=1 -s EXPORTED_FUNCTIONS='["_er_web_reset","_er_web_command","_er_web_step","_er_web_set_fault","_er_web_stop","_er_web_state"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -o web/er_core.js
```

导出的 `er_web_*` 函数只通过命令、观测和故障接口交互；浏览器不会直接修改运行时内部状态。网页现在自动加载 `er_core.js`，成功后显示“C11 核心 · WebAssembly”；加载失败则明确标识 JavaScript 演示后备模式。

本次构建使用 Emscripten 3.1.64。生成的 JS/WASM 随工程提供，Windows 浏览器无需安装工具链。开发环境工具链位于 `/tmp/er-emsdk`（临时目录，系统清理后需重新安装）；重新构建执行 `source /tmp/er-emsdk/emsdk_env.sh` 后运行 `bash web/wasm/build.sh`。
