/* Dependency-free 3D servo view. It is a visualizer only: all angles come
 * from the runtime observation passed to erServoView.update(). */
(() => {
  'use strict';
  const RAD = Math.PI / 180;
  let canvas, ctx, yaw = -28 * RAD, pitch = 23 * RAD, zoom = 1;
  let dragging = false, px = 0, py = 0;
  let model = { position: [0, 0], target: [0, 0], faults: 0 };
  let cadMesh = null, cadLoading = false;

  function resize() {
    if (!canvas) return;
    const dpr = Math.max(1, window.devicePixelRatio || 1);
    const r = canvas.getBoundingClientRect();
    canvas.width = Math.max(1, Math.round(r.width * dpr));
    canvas.height = Math.max(1, Math.round(r.height * dpr));
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    draw();
  }
  function rotate(p) {
    const cy = Math.cos(yaw), sy = Math.sin(yaw), cp = Math.cos(pitch), sp = Math.sin(pitch);
    const x = p[0] * cy - p[2] * sy;
    const z = p[0] * sy + p[2] * cy;
    return [x, p[1] * cp - z * sp, p[1] * sp + z * cp];
  }
  function project(p) {
    const q = rotate(p), r = canvas.getBoundingClientRect();
    const scale = Math.min(r.width, r.height) * 0.105 * zoom;
    return { x: r.width / 2 + q[0] * scale, y: r.height * 0.56 - q[1] * scale, z: q[2] };
  }
  function poly(points, fill, stroke = 'rgba(20,30,45,.45)') {
    const q = points.map(project); ctx.beginPath(); ctx.moveTo(q[0].x, q[0].y);
    q.slice(1).forEach(v => ctx.lineTo(v.x, v.y)); ctx.closePath();
    ctx.fillStyle = fill; ctx.fill(); ctx.strokeStyle = stroke; ctx.lineWidth = 1; ctx.stroke();
  }
  function box(cx, cy, cz, sx, sy, sz, color) {
    const x0 = cx - sx / 2, x1 = cx + sx / 2, y0 = cy - sy / 2, y1 = cy + sy / 2;
    const v = [[x0,y0,cz-sz/2],[x1,y0,cz-sz/2],[x1,y1,cz-sz/2],[x0,y1,cz-sz/2],
      [x0,y0,cz+sz/2],[x1,y0,cz+sz/2],[x1,y1,cz+sz/2],[x0,y1,cz+sz/2]];
    [[0,1,2,3],[4,7,6,5],[0,4,5,1],[3,2,6,7],[1,5,6,2],[0,3,7,4]]
      .map(f => ({ f, z: f.reduce((a,i) => a + rotate(v[i])[2], 0) / 4 }))
      .sort((a,b) => a.z - b.z).forEach((f,i) => poly(f.f.map(k => v[k]), color[i % color.length]));
  }
  function cylinder(cx, cy, cz, radius, depth, color) {
    const n = 18, side = [], back = [], front = [];
    for (let i = 0; i < n; i++) { const a = i * 2 * Math.PI / n;
      side.push([[cx + radius*Math.cos(a), cy + radius*Math.sin(a), cz-depth/2], [cx + radius*Math.cos(a), cy + radius*Math.sin(a), cz+depth/2]]);
      back.push([cx + radius*Math.cos(a), cy + radius*Math.sin(a), cz-depth/2]); front.push([cx + radius*Math.cos(a), cy + radius*Math.sin(a), cz+depth/2]); }
    for (let i = 0; i < n; i++) poly([side[i][0],side[(i+1)%n][0],side[(i+1)%n][1],side[i][1]], color);
    poly(back, 'rgba(45,55,70,.95)'); poly(front, 'rgba(190,205,220,.95)');
  }
  function arm(x, angle, color, ghost) {
    const len = 1.05, w = .28, c = Math.cos(angle), s = Math.sin(angle);
    const base = .88;
    const pts = [[x-w/2,base,0],[x+w/2,base,0],[x+w/2+len*c,base+w*s,0],[x-w/2+len*c,base+w*s,0]];
    poly(pts, ghost ? 'rgba(110,170,255,.16)' : color, ghost ? 'rgba(80,140,230,.8)' : 'rgba(20,35,55,.7)');
    const end = [x+len*c, base+len*s, 0];
    poly([[end[0]-.18,end[1]-.1,-.12],[end[0]+.18,end[1]-.1,-.12],[end[0]+.18,end[1]+.1,-.12],[end[0]-.18,end[1]+.1,-.12]], ghost ? 'rgba(110,170,255,.12)' : color);
  }
  function drawCad(cx, color) {
    /* CAD STL is retained as an engineering reference; vendor STEP often
     * contains hidden internals. The product view uses the calibrated shell
     * below so the housing remains legible at browser scale. */
    return false;
    if (!cadMesh) return false;
    const tris = cadMesh.map(t => ({ t: t.map(v => [cx + v[0], v[1] - .2, v[2]]), z: t.reduce((a,v) => a + rotate(v)[2], 0) / 3 }));
    tris.sort((a,b) => a.z - b.z).forEach(q => poly(q.t, color, 'rgba(12,32,48,.35)'));
    return true;
  }
  function loadCad() {
    if (cadLoading || cadMesh) return;
    cadLoading = true;
    fetch('assets/hd1910.step.stl').then(r => { if (!r.ok) throw Error('STL HTTP '+r.status); return r.text(); }).then(text => {
      const raw = [], re = /vertex\s+([-+\deE.]+)\s+([-+\deE.]+)\s+([-+\deE.]+)/g; let m;
      while ((m = re.exec(text))) raw.push([+m[1], +m[2], +m[3]]);
      if (raw.length < 3) throw Error('STL has no vertices');
      let min=[Infinity,Infinity,Infinity], max=[-Infinity,-Infinity,-Infinity]; raw.forEach(v => v.forEach((n,i)=>{min[i]=Math.min(min[i],n);max[i]=Math.max(max[i],n);}));
      const size=max.map((n,i)=>n-min[i]), scale=Math.min(2.1/size[0],1.25/size[1],1.15/size[2]);
      const verts=raw.map(v=>[(v[0]-(min[0]+max[0])/2)*scale,(v[1]-(min[1]+max[1])/2)*scale,(v[2]-(min[2]+max[2])/2)*scale]);
      const stride=Math.max(1,Math.ceil(verts.length/18000)); cadMesh=[]; for(let i=0;i+2<verts.length;i+=3*stride) cadMesh.push([verts[i],verts[i+1],verts[i+2]]);
      const s=document.getElementById('servoCadStatus'); if(s)s.textContent='HD-1910 外壳模型已加载（STEP 参考）'; draw();
    }).catch(e => { const s=document.getElementById('servoCadStatus'); if(s)s.textContent='CAD 加载失败，使用尺寸模型'; }).finally(()=>{cadLoading=false;});
  }
  function draw() {
    if (!canvas) return; const r = canvas.getBoundingClientRect();
    ctx.clearRect(0, 0, r.width, r.height);
    const grad = ctx.createLinearGradient(0,0,0,r.height); grad.addColorStop(0,'#101c2e'); grad.addColorStop(1,'#07101c'); ctx.fillStyle=grad; ctx.fillRect(0,0,r.width,r.height);
    // floor grid
    for (let x=-8;x<=8;x++) poly([[x,-1.05,-5],[x,-1.05,5],[x+.015,-1.05,5],[x+.015,-1.05,-5]], 'rgba(120,160,200,.06)','rgba(120,160,200,.08)');
    for (let z=-5;z<=5;z++) poly([[-8,-1.04,z],[8,-1.04,z],[8,-1.03,z+.015],[-8,-1.03,z+.015]], 'rgba(120,160,200,.05)','rgba(120,160,200,.07)');
    const colors = model.faults ? ['#8f222a','#671b22','#431118'] : ['#25282d','#17191d','#0d0f12'];
    [-2.5,2.5].forEach((x,i) => { const actual = model.position[i] || 0, target = model.target[i] || 0;
      if (!drawCad(x, colors[0])) {
        /* HD-1910-C001 envelope: 34 x 20 x 23 mm, calibrated to the
         * visible shell. Chamfer-like layered faces keep the housing solid. */
        /* recessed centre plus narrower end caps suggest the PA66 shell's
         * rounded/chamfered profile instead of a single rectangular block. */
        box(x, -.18, 0, 2.08, 1.22, 1.62, colors);
        box(x, -.18, -.80, 1.94, 1.04, .12, ['#1b1d21','#111317','#090a0c']);
        box(x, -.18, .80, 1.94, 1.04, .12, ['#1b1d21','#111317','#090a0c']);
        box(x, .45, 0, 1.86, .08, 1.36, ['#34383e','#202329','#121419']);
        box(x, -.82, 0, 1.86, .08, 1.36, ['#15171b','#0d0f12','#08090b']);
        /* compact mounting tabs integrated into the black case */
        box(x - 1.08, -.18, 0, .16, .58, 1.38, ['#202328','#121419','#090a0c']);
        box(x + 1.08, -.18, 0, .16, .58, 1.38, ['#202328','#121419','#090a0c']);
      }
      /* output bearing, shaft and the round 25T horn */
      cylinder(x, .58, 0, .29, .24, '#111317');
      cylinder(x, .80, 0, .33, .10, ['#1d2025','#0b0d10','#050608']);
      cylinder(x, .87, 0, .10, .12, ['#444a52','#24282e','#111317']);
      /* 25T horn: compact metal disc with six visible fastener recesses. */
      for (let h=0; h<6; h++) { const a=h*Math.PI/3; cylinder(x+.23*Math.cos(a), .87+.23*Math.sin(a), .08, .028, .025, '#737b84'); }
      arm(x, target, '#7eafff', true); arm(x, actual, model.faults ? '#f05b65' : '#d2a15c', false);
      const brand = project([x - .48, -.88, .25]); ctx.fillStyle='#e8edf2'; ctx.font='700 9px system-ui'; ctx.textAlign='center'; ctx.fillText('HD Servo', brand.x, brand.y); ctx.fillStyle='#f0d735'; ctx.font='700 8px ui-monospace'; ctx.fillText('HD1910M', brand.x, brand.y+11);
      const p = project([x, -1.35, 0]); ctx.fillStyle='#dbe8f5'; ctx.font='600 13px system-ui'; ctx.textAlign='center'; ctx.fillText(`舵机 ${i+1}`, p.x, p.y+24);
      ctx.font='12px ui-monospace'; ctx.fillStyle='#93abc5'; ctx.fillText(`实际 ${(actual / RAD).toFixed(1)}°  · 目标 ${(target / RAD).toFixed(1)}°`, p.x, p.y+42);
    });
    ctx.textAlign='left'; ctx.font='12px system-ui'; ctx.fillStyle='#9fb4c9'; ctx.fillText('拖动旋转视角 · 滚轮缩放 · 实线=实际  虚线=目标', 16, r.height-16);
  }
  function init() {
    canvas = document.getElementById('servoView'); if (!canvas) return;
    ctx = canvas.getContext('2d'); loadCad(); canvas.addEventListener('pointerdown', e => { dragging=true; px=e.clientX; py=e.clientY; canvas.setPointerCapture(e.pointerId); });
    canvas.addEventListener('pointermove', e => { if (!dragging) return; yaw += (e.clientX-px)*.012; pitch = Math.max(-.9, Math.min(.9, pitch+(e.clientY-py)*.012)); px=e.clientX; py=e.clientY; draw(); });
    canvas.addEventListener('pointerup', () => { dragging=false; }); canvas.addEventListener('pointercancel', () => { dragging=false; });
    canvas.addEventListener('wheel', e => { e.preventDefault(); zoom=Math.max(.55,Math.min(1.8,zoom*Math.exp(-e.deltaY*.001))); draw(); }, {passive:false});
    window.addEventListener('resize', resize); resize();
  }
  window.erServoView = { update(v) { model = {...model, ...v}; if (!canvas) init(); draw(); }, resetView() { yaw=-28*RAD; pitch=23*RAD; zoom=1; draw(); } };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init); else init();
})();
