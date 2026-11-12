(() => {
  'use strict';
  const $ = id => document.getElementById(id), canvas = $('chart'), ctx = canvas.getContext('2d');
  const DT = 20000, MAX_EVENTS = 10000;
  let state, events, lastCommand, wasm=null;
  const backend=()=>wasm?'c-wasm':'javascript-demo';
  function syncC(){const o=JSON.parse(wasm.ccall('er_web_state','string',[],[]));state.time_us=o.time_us;state.position=o.position;state.velocity=o.velocity;state.faults=o.faults;state.enabled=!!o.enabled;if(o.faults)state.running=false;}
  function sendC(){if(wasm){wasm.ccall('er_web_command','number',['number','number','number'],[state.target[0],state.target[1],1.4]);syncC();}}
  function reset() {
    state = { tick:0, time_us:0, running:false, enabled:false, faults:0, position:[0,0], velocity:[0,0], target:[1,-.5], sequence:0, command_us:0, drop:false, history:[] };
    events = []; lastCommand = null; if(wasm)wasm.ccall('er_web_reset',null,[],[]); publish();
  }
  function observation() {
    return {schema:1, backend:backend(), tick:state.tick, timestamp_us:state.time_us, position:[...state.position], velocity:[...state.velocity], enabled:wasm?state.enabled:(state.running && !state.faults), faults:state.faults, last_sequence:state.sequence};
  }
  function command(input) {
    if (!input || !Number.isSafeInteger(input.sequence) || input.sequence <= state.sequence || input.timestamp_us !== state.time_us || !Array.isArray(input.position) || input.position.length !== 2 || input.position.some(v => !Number.isFinite(v) || Math.abs(v)>3)) throw Error('命令无效：需要递增序号、当前虚拟时间和两轴 [-3,3] rad 位置');
    state.sequence=input.sequence; state.command_us=input.timestamp_us; state.target=[...input.position]; lastCommand={schema:1,sequence:input.sequence,timestamp_us:input.timestamp_us,position:[...input.position]};if(state.running)sendC();
  }
  function apply(event, record=true) {
    if (!event || !['command','start','pause','sensor_fault','drop_commands'].includes(event.type)) throw Error('未知事件');
    if (events.length >= MAX_EVENTS && record) throw Error('事件记录已满，请复位或导出');
    if(event.type==='command') command(event.command);
    if(event.type==='start' && !state.faults) {state.running=true;sendC();}
    if(event.type==='pause') {state.running=false;state.velocity=[0,0];if(wasm){wasm.ccall('er_web_stop',null,[],[]);syncC();}}
    if(event.type==='sensor_fault') {if(wasm)wasm.ccall('er_web_set_fault',null,['number','number','number'],[1,0,0]);else {state.faults|=4;state.running=false;state.velocity=[0,0];}}
    if(event.type==='drop_commands') state.drop=true;
    if(record) events.push({tick:state.tick,...JSON.parse(JSON.stringify(event))});
    publish();
  }
  function step() {
    state.tick++;
    if(wasm){wasm.ccall('er_web_step','number',['number'],[DT]);syncC();}
    else {state.time_us+=DT;
    if(state.running && !state.faults) {
      if(state.time_us-state.command_us>100000) {state.faults|=1;state.running=false;state.velocity=[0,0];}
      else state.position=state.position.map((p,i)=>{const v=Math.max(-1.4,Math.min(1.4,(state.target[i]-p)*10));state.velocity[i]=v;return p+v*DT/1e6;});
    }
    }
    state.history.push({position:[...state.position],target:[...state.target]});if(state.history.length>120)state.history.shift();publish();return observation();
  }
  function render() {
    const w=canvas.width,h=canvas.height;ctx.clearRect(0,0,w,h);
    const y=v=>25+(3-v)/6*(h-50);
    for(let v=-3;v<=3;v++){ctx.strokeStyle='#eee';ctx.beginPath();ctx.moveTo(42,y(v));ctx.lineTo(w-12,y(v));ctx.stroke();ctx.fillStyle='#777';ctx.fillText(v,10,y(v)+4);}
    for(let axis=0;axis<2;axis++)for(const field of ['target','position']){ctx.strokeStyle=field==='target'?'#777':axis?'#f0a07d':'#e4572e';ctx.setLineDash(field==='target'?[4,4]:[]);ctx.beginPath();state.history.forEach((p,i)=>{const x=42+i/119*(w-54);i?ctx.lineTo(x,y(p[field][axis])):ctx.moveTo(x,y(p[field][axis]));});ctx.stroke();}ctx.setLineDash([]);
  }
  function publish() {
    for(let i=0;i<2;i++){$('target'+i).value=state.target[i];$('target'+i+'Out').textContent=state.target[i].toFixed(2);}
    $('backend').textContent=wasm?'C11 核心 · WebAssembly':'JavaScript 演示模型（未接入 C 核心）';$('pos0').textContent=state.position[0].toFixed(3)+' rad';$('pos1').textContent=state.position[1].toFixed(3)+' rad';$('enabled').textContent=observation().enabled?'启用':'禁用';$('faultCode').textContent='0x'+state.faults.toString(16).padStart(8,'0');$('clock').textContent='t = '+(state.time_us/1e6).toFixed(2)+' s';$('runState').textContent=state.faults?'故障锁存':state.running?'运行中':'已停止';$('json').textContent=JSON.stringify(observation(),null,2);$('commandJson').textContent=JSON.stringify(lastCommand,null,2);render();
  }
  function sendTarget(){apply({type:'command',command:{sequence:state.sequence+1,timestamp_us:state.time_us,position:state.target}});}
  function exportLog(){return JSON.stringify({schema:1,backend:backend(),step_us:DT,end_tick:state.tick})+'\n'+events.map(e=>JSON.stringify(e)).join('\n')+'\n';}
  function replay(text) {
    const lines=text.trim().split('\n');if(lines.length>MAX_EVENTS+1)throw Error('回放事件过多');
    const header=JSON.parse(lines.shift()), parsed=lines.filter(Boolean).map(s=>JSON.parse(s));
    if(header.schema!==1||header.backend!==backend()||header.step_us!==DT||!Number.isSafeInteger(header.end_tick)||header.end_tick<0||header.end_tick>100000)throw Error('不兼容的回放头或运行过长');
    let prior=0;for(const e of parsed){if(!Number.isSafeInteger(e.tick)||e.tick<prior||e.tick>header.end_tick)throw Error('事件时间无效');prior=e.tick;}
    reset();let index=0;while(state.tick<=header.end_tick){while(index<parsed.length&&parsed[index].tick===state.tick){const {tick,...e}=parsed[index++];apply(e);}if(state.tick===header.end_tick)break;step();}return observation();
  }
  $('start').onclick=()=>{if(!state.running){sendTarget();apply({type:'start'});}else apply({type:'pause'});};
  $('reset').onclick=reset;$('fault').onclick=()=>apply({type:'sensor_fault'});
  ['0','1'].forEach(i=>{$('target'+i).min=-3;$('target'+i).max=3;$('target'+i).oninput=()=>{state.target[i]=+$('target'+i).value;$('target'+i+'Out').textContent=state.target[i].toFixed(2);sendTarget();};});
  $('scenario').onchange=()=>{const type=$('scenario').value;reset();sendTarget();apply({type:'start'});for(let i=0;i<100;i++){if(i===20&&type==='sensor')apply({type:'sensor_fault'});if(i===20&&type==='timeout')apply({type:'drop_commands'});if(!state.drop&&!state.faults)sendTarget();step();}apply({type:'pause'});};
  $('export').onclick=()=>{const url=URL.createObjectURL(new Blob([exportLog()],{type:'application/x-ndjson'}));const a=document.createElement('a');a.href=url;a.download='servo-replay.jsonl';a.click();URL.revokeObjectURL(url);};
  $('import').onchange=async()=>{try{const f=$('import').files[0];if(f){if(f.size>4000000)throw Error('文件过大');replay(await f.text());$('replayStatus').textContent='回放完成';}}catch(e){$('replayStatus').textContent=e.message;}};
  window.erConsole={command:c=>apply({type:'command',command:c}),event:apply,step,observation,reset,exportLog,replay};
  setInterval(()=>{if(state.running){try{if(!state.drop)sendTarget();step();}catch(e){state.running=false;$('replayStatus').textContent=e.message;}}},20);reset();
  window.Module={onRuntimeInitialized(){wasm=window.Module;reset();$('replayStatus').textContent='C 核心已加载，已开始新会话';},onAbort(){wasm=null;reset();$('replayStatus').textContent='WASM 加载失败，当前为 JavaScript 演示';}};
  const script=document.createElement('script');script.src='er_core.js';script.onerror=()=>{$('replayStatus').textContent='未找到 WASM 构建产物，当前使用 JavaScript 演示';};document.body.appendChild(script);
})();
