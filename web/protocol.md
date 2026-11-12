# 浏览器命令与回放协议（v1）

网页默认加载已构建的 C/WASM 后端，失败时明确标注 JavaScript 演示后端。入口协议不等同于 C ABI；日志保留后端身份，不允许跨后端重播。

控制台 `window.erConsole` 提供 `reset()`、`command(c)`、`event(e)`、`step()`、`observation()`、`exportLog()`、`replay(text)`。AI 自动化调用同一接口，不修改内部状态。每个 step 固定推进 20000 us；浏览器定时器仅决定播放速度，不充当硬实时钟。

命令：`{sequence:1,timestamp_us:0,position:[1,-0.5]}`，序号严格递增，时间必须等于当前虚拟时间，位置单位 rad，允许 [-3,3]。默认速度限制 1.4 rad/s。观测含 timestamp_us、tick、position、velocity、enabled、faults、last_sequence。

事件类型：command、start、pause、sensor_fault、drop_commands。复位开始新会话。网页交互播放会自动续发命令；脚本调用 step 不自动续发，便于测试命令超时。

JSONL 首行含 schema、backend、step_us、end_tick，后续每行是含 tick 的事件。同 tick 保持记录顺序；回放拒绝乱序或不兼容日志。最多 10000 个事件、100000 个步进，防止浏览器无限执行。日志不代表真实硬件验证报告。

构建 `er_core.js`/`er_core.wasm` 后页面自动加载 C 后端并开始新会话。后端身份显示为 `c-wasm`，未构建时明确显示 `javascript-demo`，两种日志不混用。C 后端由 bridge 管理自身运行时序号；网页 sequence 是入口协议序号。故障注入在下一控制步生效。C 计算采用 float，观测按桥接精度序列化，不与 JS double 结果作逐位等价承诺。
