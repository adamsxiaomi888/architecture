(() => {
  const $ = id => document.getElementById(id), canvas = $('chart'), ctx = canvas.getContext('2d');
  const state = { running:false, fault:false, t:0, pos:[0,0], target:[1,-.5], history:[] };
  const limits = { min:-3.14, max:3.14, speed:1.4 };
  function render(){
    const w=canvas.width,h=canvas.height; ctx.clearRect(0,0,w,h); ctx.strokeStyle='#e8e8e8';ctx.lineWidth=1;
    for(let i=0;i<=6;i++){const y=25+i*(h-50)/6;ctx.beginPath();ctx.moveTo(42,y);ctx.lineTo(w-12,y);ctx.stroke();ctx.fillStyle='#888';ctx.font='11px ui-monospace';ctx.fillText((limits.max-i).toFixed(0),8,y+4)}
    const map=(v)=>42+(v-limits.min)/(limits.max-limits.min)*(w-54);
    const draw=(idx,color,actual)=>{ctx.strokeStyle=color;ctx.lineWidth=actual?2:1.5;ctx.beginPath();state.history.forEach((p,i)=>{const x=42+i/(Math.max(1,119-state.history.length+1))*(w-54);const y=25+(limits.max-(actual?p[idx]:state.target[idx]))/(limits.max-limits.min)*(h-50);i?ctx.lineTo(x,y):ctx.moveTo(x,y)});ctx.stroke()};
    draw(0,'#111',false);draw(1,'#777',false);draw(0,'#e4572e',true);draw(1,'#f0a07d',true);
  }
  function publish(){ $('pos0').textContent=state.pos[0].toFixed(2)+' rad';$('pos1').textContent=state.pos[1].toFixed(2)+' rad';$('enabled').textContent=state.running&&!state.fault?'启用':'禁用';$('enabled').style.color=state.running&&!state.fault?'#18794e':'#b42318';$('faultCode').textContent=state.fault?'0x00000004':'0x00000000';$('clock').textContent='t = '+state.t.toFixed(1)+' s';$('runState').textContent=state.fault?'故障锁存':state.running?'运行中':'已停止';$('json').textContent=JSON.stringify({time_s:+state.t.toFixed(3),target:state.target.map(x=>+x.toFixed(3)),position:state.pos.map(x=>+x.toFixed(3)),enabled:state.running&&!state.fault,faults:state.fault?['SENSOR_INVALID']:[]},null,2);render()}
  function tick(){if(state.running&&!state.fault){const dt=.02;state.t+=dt;state.pos=state.pos.map((p,i)=>p+Math.max(-limits.speed*dt,Math.min(limits.speed*dt,state.target[i]-p)));state.history.push([...state.pos]);if(state.history.length>120)state.history.shift();publish()}}
  $('start').onclick=()=>{if(!state.fault)state.running=!state.running;publish()};$('reset').onclick=()=>{state.running=false;state.fault=false;state.t=0;state.pos=[0,0];state.history=[];publish()};$('fault').onclick=()=>{state.fault=true;state.running=false;publish()};['0','1'].forEach(i=>{const el=$('target'+i);el.oninput=()=>{state.target[i]=+el.value;$('target'+i+'Out').value=state.target[i].toFixed(2);$('target'+i+'Out').textContent=state.target[i].toFixed(2);publish()}});setInterval(tick,20);publish();
})();
