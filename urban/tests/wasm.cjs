/* Native/WASM parity uses the same public C ABI, with real compiled WASM. */
const assert=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const PRA=require('../build/web/pra.js');
(async()=>{
  const m=await PRA();
  assert.equal(m._pra_demo_init(20600,18,1),0);
  for(const seed of [0,1,17,1114112,655490,372114642,4294967295])for(const houses of [1,4]) {
    assert.equal(m._pra_demo_init(seed&65535,seed>>>16,houses),1);
    const native=JSON.parse(execFileSync(require('node:path').resolve(__dirname,'../build/pra'),['--seed',String(seed),'--houses',String(houses)],{encoding:'utf8'}));
    const actual=JSON.parse(m.UTF8ToString(m._pra_demo_json()));for(let h=0;h<houses;h++)for(let j=0;j<actual.houses[h].doors.length;j++){
      const a=actual.houses[h].doors[j],b=native.houses[h].doors[j];
      for(let k=0;k<2;k++)assert.ok(Math.abs(a.center[k]-b.center[k])<1e-7);
      a.center=b.center;
    }
    assert.deepEqual(actual,native);
    for(const d of actual.houses.flatMap(h=>h.doors))assert.equal(m._pra_demo_blocked(...d.center),1);
    for(const d of actual.houses.flatMap(h=>h.doors)){
      assert.equal(m._pra_demo_set_door(d.id,1),1);
      assert.equal(m._pra_demo_blocked(...d.center),0);
      assert.equal(m._pra_demo_set_door(d.id,0),1);
    }
    m._pra_demo_open_all(1);
    for(const d of actual.houses.flatMap(h=>h.doors))assert.equal(m._pra_demo_blocked(...d.center),0);
    for(const house of actual.houses)for(const room of house.rooms){
      const x=(room[0][0]+room[2][0])/2*.07+house.center[0],z=(room[0][1]+room[2][1])/2*.07+house.center[1];
      assert.equal(m._pra_demo_probe(x,z,.3,1,1.8),1);
      assert.equal(m._pra_demo_probe(x,z,.3,-.05,1.8),1);
      assert.equal(m._pra_demo_probe(x,z,.3,2.88,1.8),0);
    }
    for(const flags of [0,1,2,3]) {
      const p=m._pra_demo_mesh(flags),n=m._pra_demo_vertex_count();assert.ok(p>0&&n>0&&n%3===0);
      const vertices=m.HEAPF32.subarray(p/4,p/4+n*9);assert.equal(vertices.length,n*9);assert.ok(vertices.every(Number.isFinite));
    }
    m._pra_demo_destroy();
  }
  console.log('PRA native/WASM C parity passed: 14 scenes, original room records, door state/collision, all mesh flags');
})().catch(e=>{console.error(e);process.exitCode=1;});
