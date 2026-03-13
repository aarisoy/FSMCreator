// renderer.js — SVG canvas drawing
//
// Turns the `fsm` model into SVG on screen.
// Never modifies `fsm` directly — only reads it.
// Call renderAll() after any model change.
//
// Globals: zoom, panX, panY, drag, selId, mode, conFrom
//          renderAll(), renderStates(), renderTrans()
//          svgPt(e), ns(), mkC(), mkR(), mkT(), mkP()

/* ═══ RENDER ═══ */
let zoom=1,panX=0,panY=0,drag=null,dox=0,doy=0,conFrom=null,mode='sel',selId=null;
const S=document.getElementById('fsm-svg'),SL=document.getElementById('svg-sl'),TL=document.getElementById('svg-tl');

function renderAll(){renderTrans();renderStates();refreshProps();updSB();document.getElementById('ch').style.opacity=fsm.states.length?'0':'1';}

function svgPt(e){const r=S.getBoundingClientRect();return{x:(e.clientX-r.left-panX)/zoom,y:(e.clientY-r.top-panY)/zoom};}

function renderStates(){
  SL.innerHTML='';
  fsm.states.forEach(s=>{
    const g=ns('g');g.setAttribute('transform',`translate(${s.x*zoom+panX},${s.y*zoom+panY})`);
    g.dataset.id=s.id;
    g.style.cursor=mode==='del'?'not-allowed':mode==='con'?'crosshair':'move';
    const isSel=s.id===selId;
    if(s.type==='initial'){
      const o=mkC(0,0,(48+9)*zoom,'none',isSel?'var(--accent)':'var(--green)',1.5*zoom);
      o.setAttribute('stroke-dasharray',`${4*zoom} ${3*zoom}`);o.setAttribute('opacity','0.45');g.appendChild(o);
      const c=mkC(0,0,48*zoom,'var(--state-fill)',isSel?'var(--accent)':'var(--green)',isSel?2.5*zoom:2*zoom);
      if(isSel)c.setAttribute('filter','drop-shadow(0 0 8px var(--accent))');g.appendChild(c);
    } else if(s.type==='final'){
      const c=mkC(0,0,48*zoom,'var(--state-fill)',isSel?'var(--accent)':'var(--red)',isSel?2.5*zoom:2*zoom);
      if(isSel)c.setAttribute('filter','drop-shadow(0 0 8px var(--accent))');g.appendChild(c);
      const i=mkC(0,0,38*zoom,'none',isSel?'var(--accent)':'var(--red)',1.5*zoom);i.setAttribute('opacity','0.5');g.appendChild(i);
    } else {
      const rx=56*zoom,ry=27*zoom;
      const r=mkR(-rx,-ry,rx*2,ry*2,10*zoom,'var(--state-fill)',isSel?'var(--accent)':'var(--state-stroke)',isSel?2.5*zoom:1.5*zoom);
      if(isSel)r.setAttribute('filter','drop-shadow(0 0 8px var(--accent))');g.appendChild(r);
    }
    g.appendChild(mkT(0,0,s.name,Math.max(10,13*zoom),'600','var(--state-text)'));
    if(s.type==='initial')g.appendChild(mkT(0,-38*zoom,'INIT',Math.max(7,9*zoom),'700','var(--green)'));
    if(s.type==='final'){const b=mkT(0,54*zoom,'FINAL',Math.max(7,9*zoom),'700','var(--red)');b.setAttribute('opacity','0.65');g.appendChild(b);}
    // Small indicator dots for onEntry / onExit / internalEvents
    const hasEntry=s.onEntry&&s.onEntry.trim();
    const hasExit=s.onExit&&s.onExit.trim();
    const hasIE=s.internalEvents&&s.internalEvents.length>0;
    const indicators=[];
    if(hasEntry)indicators.push('var(--green)');
    if(hasExit)indicators.push('var(--red)');
    if(hasIE)indicators.push('var(--yellow)');
    const dotR=4*zoom,dotSpacing=10*zoom;
    const totalW=(indicators.length-1)*dotSpacing;
    indicators.forEach((col2,di)=>{
      const dotX=-totalW/2+di*dotSpacing;
      const dotY=(s.type==='normal'?28:50)*zoom;
      const dot=mkC(dotX,dotY,dotR,col2,'none',0);
      dot.setAttribute('opacity','0.85');
      g.appendChild(dot);
    });
    g.addEventListener('mousedown',e=>{e.stopPropagation();nodeMD(e,s.id);});
    g.addEventListener('dblclick',e=>{e.stopPropagation();openSE(s.id);});
    SL.appendChild(g);
  });
}

function renderTrans(){
  TL.innerHTML='';
  const pc={};
  fsm.transitions.forEach(t=>{const k=[t.from,t.to].sort().join('|');pc[k]=(pc[k]||0)+1;});
  const pi={};
  fsm.transitions.forEach(t=>{
    const fr=fsm.states.find(s=>s.id===t.from),to=fsm.states.find(s=>s.id===t.to);
    if(!fr||!to)return;
    const k=[t.from,t.to].sort().join('|');
    pi[k]=(pi[k]||0)+1;
    const idx=pi[k],tot=pc[k],off=(idx-(tot+1)/2)*30;
    const isSel=t.id===selId;
    const col=isSel?'var(--accent)':'var(--accent2)';
    const mkr=isSel?'url(#arrow-sel)':'url(#arrow)';
    const g=ns('g');g.dataset.id=t.id;g.style.cursor='pointer';
    const fx=fr.x*zoom+panX,fy=fr.y*zoom+panY,tx=to.x*zoom+panX,ty=to.y*zoom+panY;
    const self=t.from===t.to;
    if(self){
      const r=48*zoom;
      const p=mkP(`M ${fx-12} ${fy-r} C ${fx-72*zoom} ${fy-115*zoom}, ${fx+72*zoom} ${fy-115*zoom}, ${fx+12} ${fy-r}`,'none',col,1.7*zoom);
      p.setAttribute('marker-end',mkr);g.appendChild(p);
      if(t.event)g.appendChild(mkT(fx,fy-90*zoom,t.event,Math.max(9,11*zoom),'600',col));
    } else {
      const dx=tx-fx,dy=ty-fy,len=Math.sqrt(dx*dx+dy*dy)||1;
      const sr=(fr.type==='normal'?60:50)*zoom,er=(to.type==='normal'?60:50)*zoom;
      // perpendicular offset for parallel transitions
      const ox=-dy/len*off,oy=dx/len*off;
      const sx=fx+dx/len*sr+ox,sy=fy+dy/len*sr+oy;
      const ex=tx-dx/len*(er+10*zoom)+ox,ey=ty-dy/len*(er+10*zoom)+oy;
      const p=mkP(`M ${sx} ${sy} L ${ex} ${ey}`,'none',col,1.7*zoom);
      p.setAttribute('marker-end',mkr);g.appendChild(p);
      const lbl=(t.event||'')+(t.guard?` [${t.guard}]`:'');
      if(lbl){
        const lx=(sx+ex)/2,ly=(sy+ey)/2;
        const lox=-dy/len*12*zoom,loy=dx/len*12*zoom;
        const tw=lbl.length*6.5*zoom+14;
        const bg=mkR(lx+lox-tw/2,ly+loy-8*zoom,tw,15*zoom,4*zoom,'var(--surface)','none',0);bg.setAttribute('opacity','0.9');g.appendChild(bg);
        g.appendChild(mkT(lx+lox,ly+loy+1,lbl,Math.max(9,11*zoom),'600',col));
      }
    }
    g.addEventListener('mousedown',e=>{e.stopPropagation();selId=t.id;renderAll();});
    TL.appendChild(g);
  });
}

/* SVG helpers */
function ns(t){return document.createElementNS('http://www.w3.org/2000/svg',t);}
function mkC(cx,cy,r,fill,stroke,sw){const c=ns('circle');c.setAttribute('cx',cx);c.setAttribute('cy',cy);c.setAttribute('r',r);c.setAttribute('fill',fill);c.setAttribute('stroke',stroke);c.setAttribute('stroke-width',sw);return c;}
function mkR(x,y,w,h,rx,fill,stroke,sw){const r=ns('rect');r.setAttribute('x',x);r.setAttribute('y',y);r.setAttribute('width',w);r.setAttribute('height',h);r.setAttribute('rx',rx);r.setAttribute('fill',fill);r.setAttribute('stroke',stroke);r.setAttribute('stroke-width',sw);return r;}
function mkT(x,y,txt,sz,fw,fill){const t=ns('text');t.setAttribute('x',x);t.setAttribute('y',y);t.setAttribute('text-anchor','middle');t.setAttribute('dominant-baseline','central');t.setAttribute('fill',fill);t.setAttribute('font-size',sz);t.setAttribute('font-weight',fw);t.textContent=txt;return t;}
function mkP(d,fill,stroke,sw){const p=ns('path');p.setAttribute('d',d);p.setAttribute('fill',fill);p.setAttribute('stroke',stroke);p.setAttribute('stroke-width',sw);return p;}

