#include "StoreScreen.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdlib>

namespace fs = std::filesystem;

// ── Embedded HTML mini-apps ───────────────────────────────────────────────────
static const char* HTML_SNAKE = R"HTML(<!DOCTYPE html><html><head><title>Snake</title>
<style>*{margin:0;padding:0}body{background:#000d1a;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;font-family:sans-serif;color:#0af}
h1{font-size:28px;margin-bottom:12px;color:#3af}#score{margin-bottom:8px;font-size:18px}
canvas{border:2px solid #0af;box-shadow:0 0 20px #0af4}</style></head><body>
<h1>Snake</h1><div id="score">Score: 0</div>
<canvas id="c" width="400" height="400"></canvas>
<p style="margin-top:10px;font-size:13px;color:#4af">Arrow keys to move</p>
<script>
const c=document.getElementById('c'),ctx=c.getContext('2d'),S=document.getElementById('score');
const SZ=20,COLS=20;let snake=[{x:10,y:10}],dir={x:1,y:0},food=rndFood(),score=0,running=true;
function rndFood(){return{x:Math.floor(Math.random()*COLS),y:Math.floor(Math.random()*COLS)};}
document.addEventListener('keydown',e=>{const k=e.key;
  if(k==='ArrowUp'&&dir.y===0)dir={x:0,y:-1};
  if(k==='ArrowDown'&&dir.y===0)dir={x:0,y:1};
  if(k==='ArrowLeft'&&dir.x===0)dir={x:-1,y:0};
  if(k==='ArrowRight'&&dir.x===0)dir={x:1,y:0};});
function draw(){ctx.fillStyle='#000d1a';ctx.fillRect(0,0,400,400);
  snake.forEach((s,i)=>{ctx.fillStyle=i===0?'#0df':'#06a';ctx.fillRect(s.x*SZ+1,s.y*SZ+1,SZ-2,SZ-2);});
  ctx.fillStyle='#f44';ctx.beginPath();ctx.arc(food.x*SZ+SZ/2,food.y*SZ+SZ/2,SZ/2-2,0,Math.PI*2);ctx.fill();}
function tick(){if(!running)return;
  const h={x:snake[0].x+dir.x,y:snake[0].y+dir.y};
  if(h.x<0||h.x>=COLS||h.y<0||h.y>=COLS||snake.some(s=>s.x===h.x&&s.y===h.y)){
    running=false;ctx.fillStyle='rgba(0,0,0,0.7)';ctx.fillRect(0,0,400,400);
    ctx.fillStyle='#f44';ctx.font='32px sans-serif';ctx.textAlign='center';
    ctx.fillText('GAME OVER',200,190);ctx.font='18px sans-serif';ctx.fillStyle='#0af';
    ctx.fillText('Score: '+score,200,230);ctx.fillText('Refresh to restart',200,260);return;}
  snake.unshift(h);
  if(h.x===food.x&&h.y===food.y){score++;S.textContent='Score: '+score;food=rndFood();}
  else snake.pop();draw();}
setInterval(tick,120);draw();
</script></body></html>)HTML";

static const char* HTML_TTT = R"HTML(<!DOCTYPE html><html><head><title>Tic-Tac-Toe</title>
<style>*{box-sizing:border-box;margin:0;padding:0}body{background:#000d1a;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;font-family:sans-serif;color:#0af}
h1{font-size:28px;margin-bottom:16px;color:#3af}#status{font-size:20px;margin-bottom:14px}
#board{display:grid;grid-template-columns:repeat(3,110px);gap:6px}
.cell{width:110px;height:110px;background:#051428;border:2px solid #0af;font-size:52px;cursor:pointer;display:flex;align-items:center;justify-content:center;transition:background .2s}
.cell:hover{background:#0a2040}.X{color:#0af}.O{color:#f84}
button{margin-top:18px;padding:10px 30px;background:#0a4080;color:#fff;border:none;border-radius:8px;font-size:16px;cursor:pointer}</style></head><body>
<h1>Tic-Tac-Toe</h1><div id="status">Your turn (X)</div>
<div id="board"></div><button onclick="reset()">New Game</button>
<script>
let board=Array(9).fill(''),turn='X',over=false;
const status=document.getElementById('status');
const cells=Array.from({length:9},(_,i)=>{const d=document.createElement('div');
  d.className='cell';d.onclick=()=>move(i);document.getElementById('board').appendChild(d);return d;});
const wins=[[0,1,2],[3,4,5],[6,7,8],[0,3,6],[1,4,7],[2,5,8],[0,4,8],[2,4,6]];
function checkWin(p){return wins.some(([a,b,c])=>board[a]===p&&board[b]===p&&board[c]===p);}
function aiMove(){const empty=board.map((v,i)=>v===''?i:-1).filter(i=>i>=0);
  // Try to win, block, or random
  for(const p of['O','X']){for(const i of empty){board[i]=p;if(checkWin(p)){board[i]='';if(p==='O')return i;}board[i]='';}}
  return empty[Math.floor(Math.random()*empty.length)];}
function move(i){if(over||board[i])return;board[i]='X';render();
  if(checkWin('X')){status.textContent='You win!';over=true;return;}
  if(board.every(v=>v)){status.textContent='Draw!';over=true;return;}
  const ai=aiMove();board[ai]='O';render();
  if(checkWin('O')){status.textContent='AI wins!';over=true;}
  else if(board.every(v=>v)){status.textContent='Draw!';over=true;}
  else status.textContent='Your turn (X)';}
function render(){cells.forEach((c,i)=>{c.textContent=board[i];c.className='cell '+(board[i]||'');});}
function reset(){board=Array(9).fill('');over=false;status.textContent='Your turn (X)';render();}
</script></body></html>)HTML";

static const char* HTML_CALC = R"HTML(<!DOCTYPE html><html><head><title>Calculator</title>
<style>*{box-sizing:border-box;margin:0;padding:0}body{background:#000d1a;display:flex;align-items:center;justify-content:center;height:100vh;font-family:sans-serif}
#calc{background:#051428;border:2px solid #0af;border-radius:16px;padding:20px;width:300px;box-shadow:0 0 30px #0af4}
#display{background:#000;color:#0af;font-size:36px;text-align:right;padding:14px 16px;border-radius:8px;margin-bottom:14px;min-height:60px;word-break:break-all}
.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
button{padding:18px 0;font-size:20px;border:none;border-radius:8px;cursor:pointer;transition:all .15s}
.num{background:#0a2a4a;color:#fff}.num:hover{background:#0d3a6a}
.op{background:#0040a0;color:#fff}.op:hover{background:#0055cc}
.eq{background:#0070dd;color:#fff;grid-column:span 2}.eq:hover{background:#0090ff}
.cl{background:#400010;color:#fff}.cl:hover{background:#600020}</style></head><body>
<div id="calc"><div id="display">0</div>
<div class="grid">
<button class="cl" onclick="cls()">C</button>
<button class="op" onclick="op('+/-')">+/-</button>
<button class="op" onclick="op('%')">%</button>
<button class="op" onclick="op('/')">÷</button>
<button class="num" onclick="num('7')">7</button>
<button class="num" onclick="num('8')">8</button>
<button class="num" onclick="num('9')">9</button>
<button class="op" onclick="op('*')">×</button>
<button class="num" onclick="num('4')">4</button>
<button class="num" onclick="num('5')">5</button>
<button class="num" onclick="num('6')">6</button>
<button class="op" onclick="op('-')">−</button>
<button class="num" onclick="num('1')">1</button>
<button class="num" onclick="num('2')">2</button>
<button class="num" onclick="num('3')">3</button>
<button class="op" onclick="op('+')">+</button>
<button class="num" style="grid-column:span 2" onclick="num('0')">0</button>
<button class="num" onclick="num('.')">.</button>
<button class="eq" onclick="eq()">=</button>
</div></div>
<script>
let disp=document.getElementById('display'),cur='0',prev='',op2='',fresh=false;
function upd(){disp.textContent=cur;}
function num(n){if(fresh){cur='';fresh=false;}if(n==='.'&&cur.includes('.'))return;
  if(cur==='0'&&n!=='.')cur='';cur+=n;upd();}
function op(o){if(o==='+/-'){cur=String(-parseFloat(cur));}
  else if(o==='%'){cur=String(parseFloat(cur)/100);}
  else{prev=cur;op2=o;fresh=true;}upd();}
function eq(){if(!op2)return;let r=0,a=parseFloat(prev),b=parseFloat(cur);
  if(op2==='+')r=a+b;if(op2==='-')r=a-b;if(op2==='*')r=a*b;
  if(op2==='/')r=b?a/b:0;cur=String(r);op2='';fresh=true;upd();}
function cls(){cur='0';prev='';op2='';fresh=false;upd();}
</script></body></html>)HTML";

static const char* HTML_MEMORY = R"HTML(<!DOCTYPE html><html><head><title>Memory Cards</title>
<style>*{box-sizing:border-box;margin:0;padding:0}body{background:#000d1a;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;font-family:sans-serif;color:#0af}
h1{font-size:26px;margin-bottom:10px;color:#3af}#info{font-size:16px;margin-bottom:14px}
#grid{display:grid;grid-template-columns:repeat(4,90px);gap:10px}
.card{width:90px;height:90px;background:#051428;border:2px solid #0af;border-radius:10px;font-size:36px;cursor:pointer;display:flex;align-items:center;justify-content:center;transition:all .2s;user-select:none}
.card.flipped{background:#0a3060;transform:rotateY(0deg)}
.card.matched{background:#003820;border-color:#0f8;cursor:default}
button{margin-top:16px;padding:8px 24px;background:#0a4080;color:#fff;border:none;border-radius:8px;font-size:15px;cursor:pointer}</style></head><body>
<h1>Memory Cards</h1><div id="info">Moves: 0 | Pairs: 0/8</div>
<div id="grid"></div><button onclick="init()">New Game</button>
<script>
const emojis='🎮🎯🎲🎸🚀🌟💎🔥'.split('');
let cards=[],flipped=[],matched=0,moves=0,locked=false;
function init(){const pairs=[...emojis,...emojis].sort(()=>Math.random()-.5);
  matched=0;moves=0;flipped=[];locked=false;
  const grid=document.getElementById('grid');grid.innerHTML='';
  cards=pairs.map((e,i)=>{const d=document.createElement('div');
    d.className='card';d.dataset.val=e;d.dataset.i=i;
    d.onclick=()=>flip(d);grid.appendChild(d);return d;});upd();}
function flip(d){if(locked||d.classList.contains('matched')||flipped.includes(d))return;
  d.textContent=d.dataset.val;d.classList.add('flipped');flipped.push(d);
  if(flipped.length===2){moves++;locked=true;
    if(flipped[0].dataset.val===flipped[1].dataset.val){
      flipped.forEach(c=>c.classList.add('matched'));matched++;flipped=[];locked=false;
      if(matched===8)setTimeout(()=>alert('You won in '+moves+' moves!'),200);}
    else setTimeout(()=>{flipped.forEach(c=>{c.textContent='';c.classList.remove('flipped');});flipped=[];locked=false;},900);}
  upd();}
function upd(){document.getElementById('info').textContent='Moves: '+moves+' | Pairs: '+matched+'/8';}
init();
</script></body></html>)HTML";

static const char* HTML_PONG = R"HTML(<!DOCTYPE html><html><head><title>Pong</title>
<style>*{margin:0;padding:0}body{background:#000;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;font-family:sans-serif;color:#fff}
h1{font-size:24px;margin-bottom:8px;color:#0af}p{font-size:13px;color:#4af;margin-bottom:8px}</style></head><body>
<h1>Pong</h1><p>W/S = left paddle | ↑/↓ = right paddle</p>
<canvas id="c" width="600" height="400"></canvas>
<script>
const c=document.getElementById('c'),ctx=c.getContext('2d');
let p1={y:160,s:0},p2={y:160,s:0},ball={x:300,y:200,vx:4,vy:3};
const PH=80,PW=12,keys={};
document.addEventListener('keydown',e=>keys[e.key]=true);
document.addEventListener('keyup',e=>delete keys[e.key]);
function draw(){ctx.fillStyle='#000';ctx.fillRect(0,0,600,400);
  ctx.setLineDash([10,10]);ctx.strokeStyle='#222';ctx.beginPath();ctx.moveTo(300,0);ctx.lineTo(300,400);ctx.stroke();ctx.setLineDash([]);
  ctx.fillStyle='#0af';ctx.fillRect(10,p1.y,PW,PH);ctx.fillRect(578,p2.y,PW,PH);
  ctx.fillStyle='#fff';ctx.beginPath();ctx.arc(ball.x,ball.y,8,0,Math.PI*2);ctx.fill();
  ctx.font='28px monospace';ctx.textAlign='center';ctx.fillText(p1.s+'  '+p2.s,300,30);}
function update(){
  if(keys['w'])p1.y=Math.max(0,p1.y-5);
  if(keys['s'])p1.y=Math.min(320,p1.y+5);
  if(keys['ArrowUp'])p2.y=Math.max(0,p2.y-5);
  if(keys['ArrowDown'])p2.y=Math.min(320,p2.y+5);
  ball.x+=ball.vx;ball.y+=ball.vy;
  if(ball.y<8||ball.y>392)ball.vy*=-1;
  if(ball.x<22&&ball.y>p1.y&&ball.y<p1.y+PH){ball.vx=Math.abs(ball.vx);}
  if(ball.x>578&&ball.y>p2.y&&ball.y<p2.y+PH){ball.vx=-Math.abs(ball.vx);}
  if(ball.x<0){p2.s++;reset();}if(ball.x>600){p1.s++;reset();}draw();}
function reset(){ball={x:300,y:200,vx:(Math.random()>.5?4:-4),vy:(Math.random()>.5?3:-3)};}
setInterval(update,16);draw();
</script></body></html>)HTML";

static const char* HTML_SHOOTER = R"HTML(<!DOCTYPE html><html><head><title>Space Shooter</title>
<style>*{margin:0;padding:0}body{background:#000;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;font-family:sans-serif}
p{color:#0af;font-size:13px;margin-bottom:6px}</style></head><body>
<p>← → to move | SPACE to shoot</p><canvas id="c" width="500" height="500"></canvas>
<script>
const c=document.getElementById('c'),ctx=c.getContext('2d');
let ship={x:250,y:450},bullets=[],enemies=[],stars=[],score=0,lives=3,frame=0;
const keys={};
document.addEventListener('keydown',e=>{keys[e.key]=true;if(e.key===' ')shoot();});
document.addEventListener('keyup',e=>delete keys[e.key]);
for(let i=0;i<60;i++)stars.push({x:Math.random()*500,y:Math.random()*500,s:Math.random()*2+.5});
function spawnEnemy(){enemies.push({x:Math.random()*460+20,y:-20,vy:1.5+Math.random()*1.5});}
function shoot(){bullets.push({x:ship.x,y:ship.y-20});}
function update(){
  if(keys['ArrowLeft'])ship.x=Math.max(20,ship.x-5);
  if(keys['ArrowRight'])ship.x=Math.min(480,ship.x+5);
  bullets=bullets.filter(b=>{b.y-=8;return b.y>0;});
  if(frame%60===0)spawnEnemy();
  enemies=enemies.filter(e=>{e.y+=e.vy;
    bullets=bullets.filter(b=>{if(Math.abs(b.x-e.x)<20&&Math.abs(b.y-e.y)<20){score+=10;return false;}return true;});
    if(Math.abs(e.x-ship.x)<20&&Math.abs(e.y-ship.y)<20){lives--;return false;}
    return e.y<520;});
  stars.forEach(s=>s.y=(s.y+s.s)%500);frame++;
  draw();}
function draw(){ctx.fillStyle='#000010';ctx.fillRect(0,0,500,500);
  stars.forEach(s=>{ctx.fillStyle='rgba(150,200,255,'+s.s/2.5+')';ctx.fillRect(s.x,s.y,s.s,s.s);});
  // Ship
  ctx.fillStyle='#0af';ctx.beginPath();ctx.moveTo(ship.x,ship.y-20);ctx.lineTo(ship.x-14,ship.y+10);ctx.lineTo(ship.x+14,ship.y+10);ctx.closePath();ctx.fill();
  // Bullets
  ctx.fillStyle='#ff0';bullets.forEach(b=>{ctx.fillRect(b.x-2,b.y-8,4,12);});
  // Enemies
  ctx.fillStyle='#f44';enemies.forEach(e=>{ctx.beginPath();ctx.arc(e.x,e.y,14,0,Math.PI*2);ctx.fill();});
  ctx.fillStyle='#0af';ctx.font='16px monospace';ctx.fillText('Score: '+score+'  Lives: '+lives,10,20);
  if(lives<=0){ctx.fillStyle='rgba(0,0,0,.8)';ctx.fillRect(0,0,500,500);ctx.fillStyle='#f44';ctx.font='36px sans-serif';ctx.textAlign='center';ctx.fillText('GAME OVER',250,230);ctx.font='20px sans-serif';ctx.fillStyle='#0af';ctx.fillText('Score: '+score,250,270);}}
setInterval(update,16);
</script></body></html>)HTML";

// ── StoreScreen ───────────────────────────────────────────────────────────────
void StoreScreen::initApps() {
    allApps = {
        {"Snake",         "Classic snake — eat food, grow longer!",      "Games", "SNK", 0.1f,0.7f,0.2f, false, HTML_SNAKE},
        {"Tic-Tac-Toe",   "Play against the AI in this classic game.",    "Games", "TTT", 0.2f,0.5f,0.9f, false, HTML_TTT},
        {"Memory Cards",  "Flip cards and find matching pairs.",          "Games", "MEM", 0.7f,0.3f,0.9f, false, HTML_MEMORY},
        {"Pong",          "Classic 2-player paddle game.",               "Games", "PNG", 0.1f,0.6f,0.8f, false, HTML_PONG},
        {"Space Shooter", "Shoot enemies in this retro space game.",     "Games", "SPC", 0.9f,0.5f,0.1f, false, HTML_SHOOTER},
        {"Calculator",    "A clean, easy-to-use calculator.",            "Tools", "CAL", 0.3f,0.7f,0.5f, false, HTML_CALC},
        {"Notes",         "Simple text notes stored locally.",           "Tools", "NOT", 0.9f,0.8f,0.2f, false, R"HTML(<!DOCTYPE html><html><head><title>Notes</title><style>body{background:#000d1a;color:#0af;font-family:sans-serif;padding:20px;display:flex;flex-direction:column;height:100vh;box-sizing:border-box}h1{margin-bottom:12px}textarea{flex:1;background:#051428;color:#fff;border:2px solid #0af;border-radius:8px;padding:12px;font-size:15px;resize:none;outline:none}button{margin-top:10px;padding:10px;background:#0a4080;color:#fff;border:none;border-radius:6px;cursor:pointer;font-size:14px}</style></head><body><h1>Notes</h1><textarea id="t" placeholder="Type your notes here..."></textarea><button onclick="save()">Save to localStorage</button><script>const t=document.getElementById('t');t.value=localStorage.getItem('notes')||'';function save(){localStorage.setItem('notes',t.value);alert('Saved!');}</script></body></html>)HTML"},
        {"Clock",         "World clock showing multiple time zones.",    "Tools", "CLK", 0.2f,0.6f,0.9f, false, R"HTML(<!DOCTYPE html><html><head><title>Clock</title><style>body{background:#000d1a;color:#0af;font-family:monospace;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh}#t{font-size:72px;letter-spacing:4px;margin-bottom:12px}#d{font-size:22px;color:#4af}.zones{margin-top:30px;display:flex;gap:30px}.zone{text-align:center;background:#051428;padding:14px 20px;border:1px solid #0af4;border-radius:10px}.z-t{font-size:24px}.z-l{font-size:12px;color:#4af;margin-top:4px}</style></head><body><div id="t">00:00:00</div><div id="d"></div><div class="zones"><div class="zone"><div class="z-t" id="ny"></div><div class="z-l">New York</div></div><div class="zone"><div class="z-t" id="ld"></div><div class="z-l">London</div></div><div class="zone"><div class="z-t" id="tk"></div><div class="z-l">Tokyo</div></div></div><script>function fmt(d){return d.toLocaleTimeString('en',{hour:'2-digit',minute:'2-digit',hour12:false});}function tick(){const n=new Date();document.getElementById('t').textContent=n.toLocaleTimeString('en',{hour12:false});document.getElementById('d').textContent=n.toDateString();document.getElementById('ny').textContent=fmt(new Date(n.toLocaleString('en',{timeZone:'America/New_York'})));document.getElementById('ld').textContent=fmt(new Date(n.toLocaleString('en',{timeZone:'Europe/London'})));document.getElementById('tk').textContent=fmt(new Date(n.toLocaleString('en',{timeZone:'Asia/Tokyo'})));}setInterval(tick,1000);tick();</script></body></html>)HTML"},
    };

    // Mark already-installed apps
    for (auto& a : allApps) {
        std::string dir = "mtv_root/applications/" + a.name;
        if (fs::exists(dir + "/app.json")) {
            std::ifstream f(dir+"/app.json");
            std::string j((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
            if (j.find("\"store\":true") != std::string::npos) a.installed=true;
        }
    }
    filterApps();
}

void StoreScreen::filterApps() {
    filteredIdx.clear();
    std::string cat = categories[selectedCategory];
    for (int i=0;i<(int)allApps.size();++i) {
        auto& a = allApps[i];
        bool catOk  = (cat=="All") || (a.category==cat);
        bool srchOk = searchText.empty() ||
            a.name.find(searchText)!=std::string::npos ||
            a.description.find(searchText)!=std::string::npos;
        if (catOk && srchOk) filteredIdx.push_back(i);
    }
}

void StoreScreen::installApp(int idx) {
    auto& a = allApps[idx];
    std::string dir = "mtv_root/applications/" + a.name;
    fs::create_directories(dir);
    // Write app.html
    std::ofstream html(dir+"/app.html");
    html << a.htmlContent;
    html.close();
    // Write app.json
    std::ofstream json(dir+"/app.json");
    json << "{\"name\":\"" << a.name << "\",\"description\":\"" << a.description
         << "\",\"main\":\"app.html\",\"store\":true,\"imported\":true}";
    json.close();
    a.installed = true;
    // Refresh imported apps list in the main app
    app->loadImportedApps();
}

void StoreScreen::enter() {
    fadeIn=0.f; fadeOut=0.f; exiting=false; done=false; time=0.f;
    hovered=-1; scroll=0; searchText=""; selectedCategory=0;
    initApps();
    SDL_StartTextInput();
}

void StoreScreen::handleEvent(const SDL_Event& e) {
    if (exiting) return;
    float W=(float)app->screenW, H=(float)app->screenH;

    if (e.type==SDL_KEYDOWN) {
        if (e.key.keysym.sym==SDLK_ESCAPE) { exiting=true; nextState=AppState::Home; }
        if (e.key.keysym.sym==SDLK_BACKSPACE && !searchText.empty()) {
            searchText.pop_back(); filterApps();
        }
    }
    if (e.type==SDL_TEXTINPUT) {
        searchText += e.text.text;
        filterApps();
    }
    if (e.type==SDL_MOUSEWHEEL) {
        scroll -= e.wheel.y * 3;
        scroll = std::max(0, scroll);
    }
    if (e.type==SDL_MOUSEMOTION) {
        float mx=(float)e.motion.x, my=(float)e.motion.y;
        hovered=-1;
        float cols=3.f, cw=(W-120.f)/cols, ch=180.f, startY=220.f;
        for (int fi=0;fi<(int)filteredIdx.size();++fi) {
            int col=fi%3, row=fi/3;
            float cx=60.f+(float)col*cw, cy=startY+(float)(row-scroll)*ch;
            if (cx<=mx&&mx<=cx+cw-16.f&&cy<=my&&my<=cy+ch-16.f) hovered=fi;
        }
    }
    if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {
        float mx=(float)e.button.x, my=(float)e.button.y;
        // Categories
        float catX=60.f;
        for (int i=0;i<(int)categories.size();++i) {
            float cw2=110.f;
            if (mx>=catX&&mx<=catX+cw2&&my>=148.f&&my<=184.f) {
                selectedCategory=i; scroll=0; filterApps(); return;
            }
            catX+=120.f;
        }
        // Back
        if (mx>=W-180.f&&mx<=W-20.f&&my>=H-80.f&&my<=H-20.f) {
            exiting=true; nextState=AppState::Home; return;
        }
        // Install buttons
        float cols=3.f, cw=(W-120.f)/cols, ch=180.f, startY=220.f;
        for (int fi=0;fi<(int)filteredIdx.size();++fi) {
            int col=fi%3, row=fi/3;
            float cx=60.f+(float)col*cw, cy=startY+(float)(row-scroll)*ch;
            float bx=cx+10.f, by=cy+ch-52.f, bw=cw-36.f, bh=34.f;
            if (mx>=bx&&mx<=bx+bw&&my>=by&&my<=by+bh) {
                int idx=filteredIdx[fi];
                if (!allApps[idx].installed) installApp(idx);
                else {
                    // Launch it
                    std::string cmd="xdg-open '"+std::string("mtv_root/applications/")+allApps[idx].name+"/app.html' &";
                    system(cmd.c_str());
                }
                return;
            }
        }
    }
}

void StoreScreen::update(float dt) {
    time+=dt;
    if (!exiting) fadeIn=std::min(1.f,fadeIn+dt*2.f);
    else { fadeOut+=dt*2.f; if(fadeOut>=1.f) done=true; }
}

void StoreScreen::renderTopBar() {
    auto& tr=app->tr;
    float W=(float)app->screenW;
    float alpha=fadeIn;
    tr.drawRect(0,0,W,100.f,0.f,0.f,0.f,alpha*0.5f);
    tr.drawRect(0,99.f,W,1.f,0.2f,0.55f,1.f,alpha*0.3f);
    tr.drawText(app->L("Store"),"titleBold",30.f,14.f,0.3f,0.75f,1.f,alpha);

    // Search bar
    float sx=W*0.4f, sw=W*0.3f;
    tr.drawRectRounded(sx,20.f,sw,55.f,10.f,0.08f,0.22f,0.5f,alpha*0.7f);
    std::string disp = searchText.empty() ? "Search apps..." : searchText;
    float ta = searchText.empty() ? 0.4f : 1.f;
    tr.drawText(disp,"body",sx+14.f,34.f,0.6f,0.85f,1.f,alpha*ta);

    // Back
    tr.drawRectRounded(W-180.f,22.f,155.f,50.f,10.f,0.1f,0.35f,0.8f,alpha*0.8f);
    tr.drawTextCentered("< Back","headingBold",W-102.f,47.f,1.f,1.f,1.f,alpha);
}

void StoreScreen::renderCategories() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    float catX=60.f;
    for (int i=0;i<(int)categories.size();++i) {
        bool sel=(i==selectedCategory);
        float bw=110.f, bh=36.f;
        tr.drawRectRounded(catX,148.f,bw,bh,8.f,
                           sel?0.12f:0.05f, sel?0.5f:0.2f, sel?1.f:0.5f, alpha*(sel?0.9f:0.4f));
        tr.drawTextCentered(categories[i],"body",catX+bw*0.5f,166.f,1.f,1.f,1.f,alpha);
        catX+=120.f;
    }
}

void StoreScreen::renderGrid() {
    auto& tr=app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;
    float alpha=fadeIn;
    float cols=3.f, cw=(W-120.f)/cols, ch=180.f, startY=220.f;

    if (filteredIdx.empty()) {
        tr.drawTextCentered("No apps found","heading",W*0.5f,H*0.5f,0.5f,0.7f,1.f,alpha*0.5f);
        return;
    }

    // Clip to screen
    glEnable(GL_SCISSOR_TEST);
    glScissor(0,(int)(H-H+100),(int)W,(int)(H-160));

    for (int fi=0;fi<(int)filteredIdx.size();++fi) {
        int idx=filteredIdx[fi];
        auto& a=allApps[idx];
        int col=fi%3, row=fi/3;
        float cx=60.f+(float)col*cw, cy=startY+(float)(row-scroll)*ch;
        if (cy+ch<100.f||cy>H) continue;
        bool hov=(hovered==fi);

        // Card
        tr.drawRectRounded(cx,cy,cw-16.f,ch-16.f,14.f,a.r,a.g,a.b,alpha*(hov?0.30f:0.16f));
        if(hov) tr.drawRing(cx+(cw-16)*0.5f,cy+(ch-16)*0.5f,
                            std::max(cw-16.f,ch-16.f)*0.5f+2.f,2.f,48,
                            a.r*0.5f+0.5f,a.g*0.5f+0.5f,a.b*0.5f+0.5f,alpha*0.5f);
        // Icon
        tr.drawTextCentered(a.iconText,"headingBold",cx+(cw-16)*0.5f,cy+45.f,
                            a.r*0.3f+0.7f,a.g*0.3f+0.7f,a.b*0.3f+0.7f,alpha);
        // Name
        tr.drawTextCentered(a.name,"body",cx+(cw-16)*0.5f,cy+82.f,1.f,1.f,1.f,alpha);
        // Desc (clipped)
        int dw,dh; tr.getTextSize(a.description,"small",dw,dh);
        float maxW=cw-36.f;
        std::string desc=a.description;
        if((float)dw>maxW) desc=desc.substr(0,22)+"...";
        tr.drawTextCentered(desc,"small",cx+(cw-16)*0.5f,cy+104.f,0.6f,0.8f,1.f,alpha*0.7f);

        // Install / Open button
        float bx=cx+10.f, by=cy+ch-52.f, bw2=cw-36.f, bh=34.f;
        bool inst=a.installed;
        tr.drawRectRounded(bx,by,bw2,bh,8.f,
                           inst?0.05f:0.12f, inst?0.35f:0.5f, inst?0.6f:1.f, alpha*0.9f);
        tr.drawTextCentered(inst?"Open":app->L("Install"),"small",
                            bx+bw2*0.5f,by+bh*0.5f,1.f,1.f,1.f,alpha);
    }
    glDisable(GL_SCISSOR_TEST);
}

void StoreScreen::render() {
    float W=(float)app->screenW, H=(float)app->screenH;
    app->tr.drawGradientRect(0,0,W,H,0.01f,0.03f,0.14f,1.f,0.04f,0.10f,0.30f,1.f);
    renderTopBar();
    renderCategories();
    renderGrid();
    float alpha=clamp01(exiting?1.f-fadeOut:fadeIn);
    if(exiting) app->tr.drawRect(0,0,W,H,0.f,0.f,0.f,clamp01(fadeOut));
    else if(fadeIn<1.f) app->tr.drawRect(0,0,W,H,0.f,0.f,0.f,1.f-fadeIn);
    (void)alpha;
}
