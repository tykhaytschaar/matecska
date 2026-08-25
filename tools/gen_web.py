# web/index.html generalasa: bongeszos jatszooldal egyetlen onallo fajlban.
#
# A web/wasmboy.wasm.umd.js (vendorolt wasmBoy emulator-mag, a wasm
# duplan base64-elve benne van) + a build/matecska.gb base64-kent kerul
# a sablonba. Az eredmeny fuggoseg nelkul fut file://-rol vagy barmilyen
# statikus hostrol (pl. GitHub Pages).
#
# Futtatas a repo gyokerebol (elotte: make):  python3 tools/gen_web.py [kimenet]
# Vagy egyszeruen:  make web   (a CI is ezt futtatja, lasd .github/workflows)
import base64
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIB = os.path.join(ROOT, "web", "wasmboy.wasm.umd.js")
ROM = os.path.join(ROOT, "build", "matecska.gb")
VERSION_SH = os.path.join(ROOT, "tools", "version.sh")
OUT = os.path.join(ROOT, "web", "index.html")

TEMPLATE = r"""<!doctype html>
<html lang="hu">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>MATECSKA</title>
<style>
  :root { --bg:#0f380f; --mid:#306230; --lite:#9bbc0f; --lite2:#8bac0f; }
  * { margin:0; padding:0; box-sizing:border-box; -webkit-tap-highlight-color:transparent; }
  html, body { height:100%; }
  body { background:var(--bg); color:var(--lite);
         font-family:"Courier New", ui-monospace, monospace;
         display:flex; flex-direction:column; align-items:center;
         justify-content:center; gap:12px; padding:10px;
         user-select:none; -webkit-user-select:none; touch-action:manipulation; }
  h1 { font-size:20px; letter-spacing:6px; font-weight:bold; }
  #shell { background:var(--mid); padding:12px; border-radius:10px;
           box-shadow:0 0 0 2px var(--bg), 0 0 0 4px var(--mid); }
  canvas { display:block; background:var(--lite);
           image-rendering:pixelated; image-rendering:crisp-edges; }
  #hint { font-size:12px; opacity:.85; text-align:center; line-height:1.7; }
  #hint a { color:var(--lite); }
  body:not(.classic) canvas { background:#fdf6e3; }   /* CGB "papir" szin */
  body.embed h1, body.embed #hint { display:none; }
  body.embed { gap:8px; padding:6px; }
  #overlay { position:fixed; inset:0; background:rgba(15,56,15,.94);
             display:flex; flex-direction:column; align-items:center;
             justify-content:center; gap:18px; cursor:pointer; z-index:9; }
  #overlay h1 { font-size:26px; }
  #overlay b { font-size:18px; letter-spacing:2px; border:2px solid var(--lite);
               padding:12px 30px; border-radius:6px; }
  /* erintogombok - csak erintokijelzon latszanak (teszt: ?pads) */
  #pads { display:none; align-items:center; justify-content:center;
          gap:28px; margin-top:4px; }
  @media (pointer:coarse) { #pads { display:flex; } .kbd { display:none; } }
  body.show-pads #pads { display:flex; }
  .btn  { background:var(--mid); color:var(--lite); border:2px solid var(--lite);
          border-radius:10px; font:bold 15px monospace;
          display:flex; align-items:center; justify-content:center; }
  .btn.press { background:var(--lite); color:var(--bg); }
  /* D-pad: 3x3 racs, 46px-es cellak -> 142px */
  .dpad { display:grid; grid-template-columns:repeat(3,46px);
          grid-template-rows:repeat(3,46px); gap:2px; }
  /* A fent jobbra, B lent balra - a Game Boy atlos elrendezese */
  .ab   { position:relative; width:118px; height:118px; }
  .ab .btn { position:absolute; width:60px; height:60px; border-radius:50%;
             font-size:20px; }
  .ab .a { top:0; right:0; }
  .ab .b { bottom:0; left:0; }
  /* START kulon sorban, kozepen */
  #startrow { display:none; justify-content:center; margin-top:2px; }
  @media (pointer:coarse) { #startrow { display:flex; } }
  body.show-pads #startrow { display:flex; }
  #startrow .btn { width:96px; height:32px; font-size:12px; border-radius:16px; }
</style>
</head>
<body>
<h1>MATECSKA</h1>
<div id="shell"><canvas id="screen" width="160" height="144"></canvas></div>
<div id="pads">
  <div class="dpad">
    <div></div><div class="btn" data-btn="UP">&#9650;</div><div></div>
    <div class="btn" data-btn="LEFT">&#9664;</div><div></div><div class="btn" data-btn="RIGHT">&#9654;</div>
    <div></div><div class="btn" data-btn="DOWN">&#9660;</div><div></div>
  </div>
  <div class="ab">
    <div class="btn a" data-btn="A">A</div>
    <div class="btn b" data-btn="B">B</div>
  </div>
</div>
<div id="startrow"><div class="btn" data-btn="START">START</div></div>
<div id="hint">
  <span class="kbd">nyilak = mozgás &nbsp; A = A &nbsp; B = B &nbsp; Enter = START<br></span>
  v@@VER@@ &nbsp;·&nbsp; a mentés a böngészőben tárolódik (FOLYTATÁS újraindítás után is megy)<br>
  <a id="dl" href="#" download="matecska.gb">matecska.gb letöltése (@@ROMKB@@ KB) – eredeti Game Boyra / emulátorba</a><br>
  megjelenítés: <a id="look" href="#">klasszikus zöld (Game Boy)</a>
</div>
<div id="overlay"><h1>MATECSKA</h1><b>KATT / ÉRINTS: INDÍTÁS</b></div>

<script>@@LIB@@</script>
<script>
"use strict";
var ROM_B64 = "@@ROM@@";
/* a UMD build a globalis WasmBoy NEVTERBE exportal: az emulator maga
 * WasmBoy.WasmBoy (regebbi buildeknel kozvetlenul WasmBoy) */
var WB = (window.WasmBoy && window.WasmBoy.WasmBoy) || window.WasmBoy;

function romBytes() {
  var bin = atob(ROM_B64), u = new Uint8Array(bin.length);
  for (var i = 0; i < bin.length; i++) u[i] = bin.charCodeAt(i);
  return u;
}

var canvas  = document.getElementById("screen");
var overlay = document.getElementById("overlay");
var started = false;

/* --- meretezes: a legnagyobb egesz szorzo, ami elfer ------------------- */
function rescale() {
  var pads = document.getElementById("pads");
  var padH = (getComputedStyle(pads).display !== "none") ? 250 : 90;
  var s = Math.min((window.innerWidth - 44) / 160,
                   (window.innerHeight - padH) / 144);
  s = Math.max(1, Math.floor(s));
  canvas.style.width  = (160 * s) + "px";
  canvas.style.height = (144 * s) + "px";
}
if (location.search.indexOf("pads") >= 0) document.body.classList.add("show-pads");
/* megjelenites: szines (Game Boy Color) alapbol; ?dmg vagy a mentett
 * valasztas -> klasszikus zold DMG-nezet. Ugyanaz a ROM fut mindkettoben,
 * a wasmBoy a fejlec CGB-flagje alapjan bootol szinesben (isGbcEnabled). */
var classic = false;
try {
  if (location.search.indexOf("dmg") >= 0) classic = true;
  else if (location.search.indexOf("color") >= 0) classic = false;
  else classic = (localStorage.getItem("matecska.look") === "dmg");
} catch (e) { /* localStorage tiltva: marad a szines */ }
if (classic) document.body.classList.add("classic");
(function () {
  var l = document.getElementById("look");
  l.textContent = classic ? "színes (Game Boy Color)" : "klasszikus zöld (Game Boy)";
  l.addEventListener("click", function (e) {
    e.preventDefault();
    try { localStorage.setItem("matecska.look", classic ? "color" : "dmg"); } catch (x) {}
    location.reload();
  });
})();
/* iframe-ben (pl. almos.me) a cim es a hint a befoglalo oldalon van */
if (window.top !== window.self || location.search.indexOf("embed") >= 0)
  document.body.classList.add("embed");
document.getElementById("dl").href = "data:application/octet-stream;base64," + ROM_B64;
window.addEventListener("resize", rescale);
rescale();

/* --- bemenet: sajat billentyu- es erintokezeles ------------------------ */
var state = {UP:false, DOWN:false, LEFT:false, RIGHT:false,
             A:false, B:false, START:false, SELECT:false};
function push() { if (started) WB.setJoypadState(state); }

/* egy gombnyomas legalabb MIN_HOLD ms-ig "lenyomva" marad, hogy az
 * emulator frame-ciklusa biztosan lassa a rovid koppintast is */
var MIN_HOLD = 100, pressedAt = {}, relTimer = {};
function setBtn(b, down) {
  clearTimeout(relTimer[b]);
  if (down) {
    if (!state[b]) { state[b] = true; pressedAt[b] = Date.now(); push(); }
    return;
  }
  var wait = MIN_HOLD - (Date.now() - (pressedAt[b] || 0));
  if (wait > 0) {
    relTimer[b] = setTimeout(function () { state[b] = false; push(); }, wait);
  } else if (state[b]) { state[b] = false; push(); }
}

var KEYS = {
  "arrowup":"UP", "arrowdown":"DOWN", "arrowleft":"LEFT", "arrowright":"RIGHT",
  "a":"A", "b":"B",            /* a billentyu = a Game Boy gomb */
  "z":"A", "x":"B",            /* csendes alternativa */
  "enter":"START", "shift":"SELECT"
};
function onKey(e, down) {
  var b = KEYS[e.key.toLowerCase()];
  if (!b) return;
  e.preventDefault();
  setBtn(b, down);
}
window.addEventListener("keydown", function (e) { onKey(e, true);  });
window.addEventListener("keyup",   function (e) { onKey(e, false); });

var btns = document.querySelectorAll(".btn[data-btn]");
for (var i = 0; i < btns.length; i++) (function (el) {
  var b = el.getAttribute("data-btn");
  function set(down) {
    return function (e) {
      e.preventDefault();
      el.classList.toggle("press", down);
      setBtn(b, down);
    };
  }
  el.addEventListener("pointerdown",   set(true));
  el.addEventListener("pointerup",     set(false));
  el.addEventListener("pointerleave",  set(false));
  el.addEventListener("pointercancel", set(false));
})(btns[i]);

/* --- inditas (felhasznaloi gesztus kell az audiohoz) -------------------- */
/* a szurke DMG-kimenet atfestese az eredeti Game Boy zold palettara.
 * a callback a kirajzolas elott kapja a frame RGBA buffert, a modositas
 * ervenyesul. A >>6 negy savba sorolja a szurkearnyalatot. */
var DMG = [[15,56,15], [48,98,48], [139,172,15], [155,188,15]];
function dmgTint(data) {
  for (var i = 0; i < data.length; i += 4) {
    var p = DMG[data[i] >> 6];
    data[i] = p[0]; data[i + 1] = p[1]; data[i + 2] = p[2];
  }
}

function saveNow() {
  if (started) WB.saveLoadedCartridge().catch(function () {});
}

function boot(withAudio) {
  overlay.style.display = "none";
  console.log("MATECSKA boot, audio=" + withAudio);
  var cfg = {
    isAudioEnabled: withAudio,
    isGbcEnabled: !classic,            /* CGB-flag + ez -> szines mod */
    isGbcColorizationEnabled: false,   /* DMG-modban nyers szurke...  */
    gameboyFrameRate: 60
  };
  if (classic) cfg.updateGraphicsCallback = dmgTint;  /* ...amit zoldre festunk */
  WB.config(cfg, canvas)
  .then(function () { console.log("config OK");
                      return WB.loadROM(romBytes()); })
  .then(function () { console.log("loadROM OK");
                      /* CSAK a loadROM utan: az a lib sajat joypadjat
                       * ujra bekapcsolja, es felulirna a mienket */
                      WB.disableDefaultJoypad();
                      return withAudio ? WB.resumeAudioContext()
                                       : Promise.resolve(); })
  .then(function () { return WB.play(); })
  .then(function () {
    console.log("play OK");
    started = true;
    /* SRAM (FOLYTATAS) mentese a bongeszo tarolojaba - a wasmBoy a
       kazettahoz koti, ezert a szines es a klasszikus mod kozos mentest lat */
    setInterval(saveNow, 5000);
    document.addEventListener("visibilitychange", function () {
      if (document.hidden) saveNow();
    });
    rescale();
  })
  .catch(function (err) {
    console.error("MATECSKA boot hiba:", err);
    overlay.style.display = "flex";
    overlay.innerHTML = "<h1>HIBA</h1><b>" + String(err) + "</b>";
  });
}

overlay.addEventListener("click", function () { boot(true); });
/* ?autostart: teszteleshez, kattintas (gesztus) nelkul, hang nelkul */
if (location.search.indexOf("autostart") >= 0) boot(false);
</script>
</body>
</html>
"""


def main():
    if not os.path.exists(ROM):
        sys.exit("nincs %s - elobb: make" % ROM)
    lib = open(LIB, encoding="utf-8").read()
    if "</script" in lib.lower():
        sys.exit("a lib '</script'-et tartalmaz, nem inline-olhato igy")
    rom_b64 = base64.b64encode(open(ROM, "rb").read()).decode("ascii")

    out = sys.argv[1] if len(sys.argv) > 1 else OUT
    rom = open(ROM, "rb").read()
    try:
        ver = subprocess.check_output(["sh", VERSION_SH], text=True).strip()
    except (OSError, subprocess.CalledProcessError):
        ver = "dev"

    html = (TEMPLATE.replace("@@LIB@@", lib).replace("@@ROM@@", rom_b64)
                    .replace("@@ROMKB@@", str(len(rom) // 1024))
                    .replace("@@VER@@", ver))
    open(out, "w", encoding="utf-8").write(html)
    # a ROM fajlkent is a kimenet melle (letoltes-link a befoglalo oldalon)
    rom_out = os.path.join(os.path.dirname(os.path.abspath(out)), "matecska.gb")
    open(rom_out, "wb").write(rom)
    print("kesz: %s (%d KB) + %s" % (out, len(html) // 1024, rom_out))


if __name__ == "__main__":
    main()
