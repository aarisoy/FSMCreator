// canvas.js — mouse, drag, zoom/pan, mode switching, panel collapse
//
// Handles all user interaction with the SVG canvas.
// Reads/writes: drag, selId, conFrom, mode, zoom, panX, panY
//
// Key functions:
//   nodeMD(e, id)    — mousedown on a state node
//   svgMD/MM/MU      — canvas mouse events (wired in HTML)
//   svgDbl(e)        — double-click opens state editor
//   fitView()        — fit all states in viewport
//   setMode(m)       — switch sel/con/del mode
//   toggleCanvasMax()— hide both panels for full canvas

/* ═══ MOUSE ═══ */
function nodeMD(e,id){
  const s=fsm.states.find(s=>s.id===id);if(!s)return;
  if(mode==='con'){if(!conFrom){conFrom=id;}else if(conFrom!==id){openConnectModal(conFrom,id);conFrom=null;setMode('sel');}return;}
  if(mode==='del'){snap();fsm.states=fsm.states.filter(x=>x.id!==id);fsm.transitions=fsm.transitions.filter(t=>t.from!==id&&t.to!==id);renderAll();return;}
  selId=id;const pt=svgPt(e);drag=id;dox=pt.x-s.x;doy=pt.y-s.y;renderAll();if(typeof showStateProperties==='function'){showStateProperties(id);}
}
function svgMD(e){if(e.target===S||!e.target.closest('g')){selId=null;conFrom=null;renderAll();}}
function svgDbl(e){
  // Walk up from clicked element until we hit a <g> with a dataset.id
  let el=e.target;
  while(el && el!==S){
    if(el.tagName==='g' && el.dataset && el.dataset.id){
      const s=fsm.states.find(x=>x.id===el.dataset.id);
      if(s){e.stopPropagation();e.preventDefault();drag=null;openSE(s.id);return;}
    }
    el=el.parentNode;
  }
}
function svgMM(e){if(!drag)return;const pt=svgPt(e);const s=fsm.states.find(x=>x.id===drag);if(s){s.x=pt.x-dox;s.y=pt.y-doy;renderAll();}}
function svgMU(){if(drag){snap();drag=null;}}

/* ═══ DRAG FROM PALETTE ═══ */
let palType=null;
function palDrag(e,t){palType=t;}
function canvasDrop(e){
  e.preventDefault();
  const r=document.getElementById('cw').getBoundingClientRect();
  const x=(e.clientX-r.left-panX)/zoom,y=(e.clientY-r.top-panY)/zoom;
  const t=palType||'state';if(t==='transition')return;
  snap();
  const id=nid();
  fsm.states.push({id,name:`S${fsm.states.length+1}`,type:t==='initial'?'initial':t==='final'?'final':'normal',x,y,onEntry:'',onExit:'',entryGuard:'',exitGuard:'',internalEvents:[],comment:'',history:'none'});
  if(t==='initial'&&!fsm.initial)fsm.initial=id;
  palType=null;renderAll();
}

/* ═══ ZOOM/PAN ═══ */
document.getElementById('cw').addEventListener('wheel',e=>{e.preventDefault();const f=e.deltaY<0?1.1:.9;zoom=Math.min(3,Math.max(.18,zoom*f));renderAll();},{passive:false});
function zoomIn(){zoom=Math.min(3,zoom*1.18);renderAll();}
function zoomOut(){zoom=Math.max(.18,zoom/1.18);renderAll();}
function fitView(){
  if(!fsm.states.length)return;
  const w=document.getElementById('cw').getBoundingClientRect();
  const xs=fsm.states.map(s=>s.x),ys=fsm.states.map(s=>s.y);
  const mn_x=Math.min(...xs)-90,mx_x=Math.max(...xs)+90;
  const mn_y=Math.min(...ys)-70,mx_y=Math.max(...ys)+70;
  zoom=Math.min(w.width/(mx_x-mn_x),w.height/(mx_y-mn_y),2);
  panX=(w.width-(mx_x+mn_x)*zoom)/2;panY=(w.height-(mx_y+mn_y)*zoom)/2;renderAll();
}
function centerView(){panX=0;panY=0;zoom=1;renderAll();}

/* ═══ PANEL COLLAPSE/EXPAND ═══ */
let cxMode=false;
function toggleCanvasMax(){
  cxMode=!cxMode;
  const ws=document.getElementById('workspace'),btn=document.getElementById('btn-cx');
  ws.classList.toggle('lc',cxMode);ws.classList.toggle('rc',cxMode);
  btn.style.background=cxMode?'var(--accent)':'';btn.style.color=cxMode?'#fff':'';
  setTimeout(renderAll,280);
}
function collapsePanel(s){document.getElementById('workspace').classList.add(s==='l'?'lc':'rc');setTimeout(renderAll,280);}
function expandPanel(s){
  document.getElementById('workspace').classList.remove(s==='l'?'lc':'rc');
  if(cxMode){cxMode=false;document.getElementById('workspace').classList.remove('lc','rc');document.getElementById('btn-cx').style.background='';document.getElementById('btn-cx').style.color='';}
  setTimeout(renderAll,280);
}

