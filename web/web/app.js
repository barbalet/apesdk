/* The revision query prevents a browser retaining an older generated bridge
 * after a local rebuild. The Wasm loader still resolves apesdk.wasm normally. */
import createApeSDK from './apesdk.js?rev=5';
const state = document.querySelector('#state'), population = document.querySelector('#population');
const location = document.querySelector('#location'), toggle = document.querySelector('#toggle'), restart = document.querySelector('#restart');
const canvases = [['view', 0], ['terrain', 1], ['control', 2]].map(([id, view]) => ({ canvas: document.querySelector(`#${id}`), view }));
let running = true, api;
const seed = () => (Date.now() ^ Math.floor(Math.random() * 0xffffffff)) >>> 0;
function drawCanvas({ canvas, view }) { const pointer = api.draw(view, canvas.width, canvas.height); const source = new Uint8ClampedArray(api.heap().buffer, pointer, canvas.width * canvas.height * 4); const image = canvas._image || (canvas._image = new ImageData(canvas.width, canvas.height)); for (let i = 0; i < source.length; i += 4) { image.data[i] = source[i + 1]; image.data[i + 1] = source[i + 2]; image.data[i + 2] = source[i + 3]; image.data[i + 3] = 255; } canvas.getContext('2d').putImageData(image, 0, 0); }
function render(time) { if (running) api.cycle(Math.floor(time)); canvases.forEach(drawCanvas); population.textContent = api.population().toLocaleString(); location.textContent = `${api.selectedX()}, ${api.selectedY()}`; requestAnimationFrame(render); }
try {
  const module = await createApeSDK();
  api = { start: module.cwrap('apesdk_start', 'number', ['number']), cycle: module.cwrap('apesdk_cycle', null, ['number']), draw: module.cwrap('apesdk_draw', 'number', ['number', 'number', 'number']), mouse: module.cwrap('apesdk_mouse', null, ['number', 'number', 'number', 'number']), mouseUp: module.cwrap('apesdk_mouse_up', null, []), key: module.cwrap('apesdk_key', null, ['number', 'number']), keyUp: module.cwrap('apesdk_key_up', null, []), menu: module.cwrap('apesdk_menu', null, ['number']), population: module.cwrap('apesdk_population', 'number', []), selectedX: module.cwrap('apesdk_selected_x', 'number', []), selectedY: module.cwrap('apesdk_selected_y', 'number', []), stop: module.cwrap('apesdk_stop', null, []), heap: () => module.HEAPU8 };
  const started = api.start(seed());
  if (started !== 2) throw new Error(`The simulation could not initialize (status ${started}).`);
  state.textContent = 'Running the shared graphical ApeSDK simulation in your browser.'; document.querySelectorAll('button').forEach(button => button.disabled = false);
  toggle.addEventListener('click', () => { running = !running; toggle.textContent = running ? 'Pause' : 'Resume'; });
  restart.addEventListener('click', () => { api.stop(); api.start(seed()); });
  document.querySelectorAll('[data-menu]').forEach(button => button.addEventListener('click', () => api.menu(Number(button.dataset.menu))));
  canvases.forEach(({ canvas, view }) => { const point = event => { const box = canvas.getBoundingClientRect(); return [Math.floor((event.clientX - box.left) * canvas.width / box.width), Math.floor((box.bottom - event.clientY) * canvas.height / box.height)]; }; canvas.addEventListener('pointerdown', event => { const [x, y] = point(event); api.mouse(view, x, y, event.altKey || event.ctrlKey); canvas.setPointerCapture(event.pointerId); }); canvas.addEventListener('pointermove', event => { if (event.buttons) { const [x, y] = point(event); api.mouse(view, x, y, event.altKey || event.ctrlKey); } }); canvas.addEventListener('pointerup', () => api.mouseUp()); canvas.addEventListener('keydown', event => { api.key(view, event.keyCode); event.preventDefault(); }); canvas.addEventListener('keyup', () => api.keyUp()); }); render();
} catch (error) { state.textContent = `Unable to load WebAssembly: ${error.message}`; }
