import createApeSDK from './apesdk.js?rev=6';

const state=document.querySelector('#state'), population=document.querySelector('#population');
const selectedLocation=document.querySelector('#location'), toggle=document.querySelector('#toggle');
const restart=document.querySelector('#restart'), reportButton=document.querySelector('#report');
const reportOutput=document.querySelector('#failure-report');
const canvases=[['view',0],['terrain',1],['control',2]].map(([id,view])=>({canvas:document.querySelector(`#${id}`),view}));
const EMAIL='barbalet@gmail.com', STALL_MS=5000;
const diagnostics={phase:'loading',lastFrameAt:performance.now(),frame:0,lastCycleMs:0,lastDrawMs:0,failures:[]};
let running=true, failed=false, api;
const seed=()=> (Date.now()^Math.floor(Math.random()*0xffffffff))>>>0;
function note(kind,detail){ diagnostics.failures.push({at:new Date().toISOString(),kind,detail:String(detail).slice(0,600)}); if(diagnostics.failures.length>12) diagnostics.failures.shift(); }
function reportText(){ return JSON.stringify({reportVersion:1,generatedAt:new Date().toISOString(),phase:diagnostics.phase,url:location.href,visibility:document.visibilityState,userAgent:navigator.userAgent,frame:diagnostics.frame,lastCycleMs:diagnostics.lastCycleMs,lastDrawMs:diagnostics.lastDrawMs,population:population.textContent,selectedLocation:selectedLocation.textContent,failures:diagnostics.failures},null,2); }
function showReport(){ const text=reportText(); reportOutput.value=text; reportOutput.hidden=false; reportButton.disabled=false; return text; }
function emailReport(){ window.location.href=`mailto:${EMAIL}?subject=${encodeURIComponent('WASM crash')}&body=${encodeURIComponent(reportText().slice(0,1800))}`; }
function fail(kind,error){ if(failed) return; failed=true; running=false; diagnostics.phase='failed'; note(kind,error&&(error.stack||error.message)?(error.stack||error.message):error); state.textContent=`WebAssembly failure: ${kind}. A diagnostic report is ready.`; showReport(); emailReport(); }

function drawCanvas({canvas,view}){
  const started=performance.now(), pointer=api.draw(view,canvas.width,canvas.height);
  const source=new Uint8ClampedArray(api.heap().buffer,pointer,canvas.width*canvas.height*4);
  const image=canvas._image||(canvas._image=new ImageData(canvas.width,canvas.height));
  for(let i=0;i<source.length;i+=4){ image.data[i]=source[i+1]; image.data[i+1]=source[i+2]; image.data[i+2]=source[i+3]; image.data[i+3]=255; }
  canvas.getContext('2d').putImageData(image,0,0); diagnostics.lastDrawMs=Math.round((performance.now()-started)*10)/10;
}
function render(time){
  try { diagnostics.lastFrameAt=time; diagnostics.frame++; if(running){ const started=performance.now(); api.cycle(Math.floor(time)); diagnostics.lastCycleMs=Math.round((performance.now()-started)*10)/10; } canvases.forEach(drawCanvas); population.textContent=api.population().toLocaleString(); selectedLocation.textContent=`${api.selectedX()}, ${api.selectedY()}`; }
  catch(error){ fail('animation-frame exception',error); return; }
  requestAnimationFrame(render);
}
setInterval(()=>{ if(!failed&&running&&document.visibilityState==='visible'&&performance.now()-diagnostics.lastFrameAt>STALL_MS) fail(`visible-frame stall over ${STALL_MS}ms`,`last frame ${Math.round(performance.now()-diagnostics.lastFrameAt)}ms ago`); },1000);
window.addEventListener('error',event=>fail('window error',event.error||event.message));
window.addEventListener('unhandledrejection',event=>fail('unhandled promise rejection',event.reason));

try {
  const module=await createApeSDK();
  api={start:module.cwrap('apesdk_start','number',['number']),cycle:module.cwrap('apesdk_cycle',null,['number']),draw:module.cwrap('apesdk_draw','number',['number','number','number']),mouse:module.cwrap('apesdk_mouse',null,['number','number','number','number']),mouseUp:module.cwrap('apesdk_mouse_up',null,[]),key:module.cwrap('apesdk_key',null,['number','number']),keyUp:module.cwrap('apesdk_key_up',null,[]),menu:module.cwrap('apesdk_menu',null,['number']),population:module.cwrap('apesdk_population','number',[]),selectedX:module.cwrap('apesdk_selected_x','number',[]),selectedY:module.cwrap('apesdk_selected_y','number',[]),stop:module.cwrap('apesdk_stop',null,[]),heap:()=>module.HEAPU8};
  const started=api.start(seed()); if(started!==2) throw new Error(`The simulation could not initialize (status ${started}).`);
  diagnostics.phase='running'; state.textContent='Running the shared graphical ApeSDK simulation in your browser.';
  document.querySelectorAll('button').forEach(button=>{if(button!==reportButton) button.disabled=false;});
  toggle.addEventListener('click',()=>{running=!running;toggle.textContent=running?'Pause':'Resume';});
  restart.addEventListener('click',()=>{api.stop();api.start(seed());diagnostics.phase='running';});
  reportButton.addEventListener('click',()=>{note('manual report','user requested diagnostics');showReport();emailReport();});
  document.querySelectorAll('[data-menu]').forEach(button=>button.addEventListener('click',()=>api.menu(Number(button.dataset.menu))));
  canvases.forEach(({canvas,view})=>{ const point=event=>{const box=canvas.getBoundingClientRect();return[Math.floor((event.clientX-box.left)*canvas.width/box.width),Math.floor((box.bottom-event.clientY)*canvas.height/box.height)];}; canvas.addEventListener('pointerdown',event=>{const [x,y]=point(event);api.mouse(view,x,y,event.altKey||event.ctrlKey);canvas.setPointerCapture(event.pointerId);});canvas.addEventListener('pointermove',event=>{if(event.buttons){const [x,y]=point(event);api.mouse(view,x,y,event.altKey||event.ctrlKey);}});canvas.addEventListener('pointerup',()=>api.mouseUp());canvas.addEventListener('keydown',event=>{api.key(view,event.keyCode);event.preventDefault();});canvas.addEventListener('keyup',()=>api.keyUp()); });
  render();
} catch(error) { fail('WebAssembly load/initialization',error); }
