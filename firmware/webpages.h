const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>HUB75 Display</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      background: #0d0d14;
      color: #c8c8de;
      font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
      font-size: 14px;
      padding: 16px;
      min-height: 100vh;
    }

    .page-header { margin-bottom: 20px; }
    .page-header h1 { font-size: 1.25em; font-weight: 600; color: #fff; margin-bottom: 3px; }
    .page-header p  { font-size: 0.8em; color: #555; }
    .page-header a  { color: #7070d0; text-decoration: none; }
    .page-header a:hover { color: #9898f0; }

    .card {
      background: #13131d;
      border: 1px solid #22223a;
      border-radius: 10px;
      padding: 16px 18px;
      margin-bottom: 12px;
    }
    .card-title {
      font-size: 0.7em;
      text-transform: uppercase;
      letter-spacing: 0.09em;
      font-weight: 700;
      color: #4a4a90;
      margin-bottom: 13px;
    }

    .btn {
      display: inline-flex;
      align-items: center;
      gap: 5px;
      background: #1c1c2e;
      color: #b0b0d0;
      border: 1px solid #2a2a46;
      border-radius: 7px;
      padding: 7px 13px;
      font-size: 0.84em;
      cursor: pointer;
      transition: background 0.14s, color 0.14s, border-color 0.14s;
      margin: 3px 3px 3px 0;
      line-height: 1;
    }
    .btn:hover { background: #252540; color: #fff; border-color: #4a4a80; }
    .btn-primary { background: #232380; border-color: #4040c0; color: #d0d0ff; }
    .btn-primary:hover { background: #2e2eb0; }
    .btn-sm { padding: 5px 10px; font-size: 0.8em; }

    .toggles { display: flex; flex-wrap: wrap; gap: 6px 20px; }
    .sw-label {
      display: inline-flex; align-items: center; gap: 8px;
      cursor: pointer; font-size: 0.87em; color: #a0a0c0; user-select: none;
    }
    .sw-label input[type=checkbox] {
      appearance: none; -webkit-appearance: none;
      width: 36px; height: 20px;
      background: #22223a; border-radius: 10px; border: 1px solid #2e2e50;
      cursor: pointer; position: relative; transition: background 0.18s; flex-shrink: 0;
    }
    .sw-label input[type=checkbox]::after {
      content: ''; position: absolute;
      width: 14px; height: 14px; background: #fff; border-radius: 50%;
      top: 2px; left: 2px; transition: left 0.16s; box-shadow: 0 1px 3px #0006;
    }
    .sw-label input[type=checkbox]:checked { background: #3838c0; border-color: #5656e0; }
    .sw-label input[type=checkbox]:checked::after { left: 18px; }

    input[type=text], input[type=number], select {
      background: #0c0c18; border: 1px solid #22223a; color: #c8c8de;
      border-radius: 6px; padding: 6px 9px; font-size: 0.87em;
      outline: none; transition: border-color 0.15s;
    }
    input[type=text]:focus, input[type=number]:focus, select:focus { border-color: #4040b0; }
    select option { background: #13131d; }

    .brightness-row { display: flex; align-items: center; gap: 12px; }
    input[type=range] {
      -webkit-appearance: none; flex: 1; height: 5px;
      background: #22223a; border-radius: 3px; outline: none; cursor: pointer;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 17px; height: 17px; background: #5050d0;
      border-radius: 50%; border: 2px solid #8080ff; cursor: pointer;
    }
    input[type=range]::-moz-range-thumb {
      width: 15px; height: 15px; background: #5050d0;
      border-radius: 50%; border: 2px solid #8080ff; cursor: pointer;
    }
    .num-box { width: 64px; text-align: center; }

    .color-row { display: flex; flex-wrap: wrap; align-items: center; gap: 10px; }
    input[type=color] {
      width: 40px; height: 40px; border: 1px solid #2a2a46;
      border-radius: 7px; cursor: pointer; padding: 2px; background: #0c0c18;
    }
    .hex-group { display: inline-flex; align-items: center; gap: 3px; }
    .hex-prefix { color: #4a4a80; font-family: monospace; font-size: 1em; }
    .hex-input { width: 80px; font-family: monospace; letter-spacing: 0.05em; }
    .color-swatch {
      width: 40px; height: 40px; border-radius: 7px;
      border: 1px solid #2a2a46; flex-shrink: 0; transition: background 0.1s;
    }
    .color-rgb { font-family: monospace; font-size: 0.8em; color: #4a4a80; }

    .scroll-grid { display: flex; flex-direction: column; gap: 10px; }
    .field-row { display: flex; flex-wrap: wrap; align-items: center; gap: 8px; }
    .field-label { font-size: 0.82em; color: #5a5a90; white-space: nowrap; }

    #status-msg {
      min-height: 22px; font-size: 0.82em; color: #5a5a90;
      padding: 3px 0; transition: opacity 0.4s;
    }

    #fileArea { display: none; }
    .file-table { width: 100%; border-collapse: collapse; font-size: 0.84em; }
    .file-table th {
      text-align: left; padding: 7px 10px; color: #5a5a90;
      font-weight: 600; border-bottom: 1px solid #1e1e32;
    }
    .file-table td { padding: 7px 10px; border-bottom: 1px solid #191928; vertical-align: middle; }
    .file-table tr:last-child td { border-bottom: none; }
    .file-table tr:hover td { background: #16162a; }
    .file-table td img { border-radius: 4px; display: block; }

    progress { width: 100%; height: 6px; border: none; border-radius: 3px; overflow: hidden; }
    progress::-webkit-progress-bar { background: #1a1a2e; }
    progress::-webkit-progress-value { background: #4040c0; }
    progress::-moz-progress-bar { background: #4040c0; }
    #uploadStatusText { font-size: 0.8em; color: #5050a0; margin-top: 5px; }

    .info-bar { display: flex; flex-wrap: wrap; gap: 6px 20px; }
    .info-item { font-size: 0.82em; color: #444; }
    .info-item b { color: #6060a0; font-weight: 500; }
  </style>
</head>
<body>

<div class="page-header">
  <h1>&#9635; HUB75 Pixel Display</h1>
  <p>Firmware: %FIRMWARE% &nbsp;&bull;&nbsp; Modified By 
    <a href="https://github.com/Hybrizat/HUB75-Pixel-Art-Display" target="_blank">Hybrizat</a>
    Original By 
    <a href="https://github.com/mzashh/HUB75-Pixel-Art-Display/" target="_blank">mzashh</a>
  </p>
</div>

<div class="card">
  <div class="card-title">Storage</div>
  <div class="info-bar">
    <div class="info-item">Free: <b>%FREEFLASH%</b></div>
    <div class="info-item">Used: <b>%USEDFLASH%</b></div>
    <div class="info-item">Total: <b>%TOTALFLASH%</b></div>
  </div>
</div>

<div class="card">
  <div class="card-title">Display</div>
  <div class="toggles" style="margin-bottom:13px;">
    <label class="sw-label">
      <input type="checkbox" id="gifToggle" checked onchange="toggleGIF(this)"> Play GIFs
    </label>
    <label class="sw-label">
      <input type="checkbox" id="loopGifToggle" checked onchange="toggleLoopGif(this)"> Loop GIFs
    </label>
    <label class="sw-label">
      <input type="checkbox" id="clockToggle" checked onchange="toggleClock(this)"> Show Clock
    </label>
  </div>
  <div>
    <button class="btn" onclick="listFilesButton()">&#128193; Files</button>
    <button class="btn btn-primary" onclick="showUploadButtonFancy()">&#8679; Upload GIF</button>
    <button class="btn" onclick="rebootButton()">&#8635; Reboot</button>
    <button class="btn" onclick="logoutButton()">&#128274; Logout</button>
  </div>
</div>

<div class="card">
  <div class="card-title">Brightness</div>
  <div class="brightness-row">
    <input type="range" id="pwmSlider" min="0" max="255" value="%SLIDERVALUE%" step="1"
      oninput="onSliderInput(this)" onchange="sendBrightness(this.value)">
    <input type="number" class="num-box" id="brightnessNum" min="0" max="255"
      value="%SLIDERVALUE%" onchange="onNumChange(this)">
  </div>
</div>

<div class="card">
  <div class="card-title">Text Color</div>
  <div class="color-row">
    <input type="color" id="colorPicker" value="#ffffff" oninput="onPickerInput(this.value)">
    <div class="hex-group">
      <span class="hex-prefix">#</span>
      <input type="text" class="hex-input" id="hexInput" value="ffffff"
        maxlength="6" placeholder="ffffff" oninput="onHexInput(this)">
    </div>
    <div class="color-swatch" id="colorSwatch" style="background:#ffffff;"></div>
    <span class="color-rgb" id="colorRGB">R:255 G:255 B:255</span>
    <button class="btn btn-primary btn-sm" onclick="setColor()">Apply</button>
  </div>
  <input type="hidden" id="r" value="255">
  <input type="hidden" id="g" value="255">
  <input type="hidden" id="b" value="255">
</div>

<div class="card">
  <div class="card-title">Scrolling Text</div>
  <div class="scroll-grid">
    <div class="field-row">
      <label class="sw-label">
        <input type="checkbox" id="scrollTextToggle" onchange="toggleScrollText(this)"> Enable
      </label>
    </div>
    <div class="field-row">
      <span class="field-label">Text:</span>
      <input type="text" id="scrollText" placeholder="Supports UTF-8 characters"
        style="flex:1; min-width:140px;">
    </div>
    <div class="field-row">
      <span class="field-label">Font:</span>
      <select id="fontSizeToggle">
        <option value="1">Small</option>
        <option value="2" selected>Normal</option>
        <option value="3">Big</option>
        <option value="4">Huge</option>
      </select>
      <span class="field-label">Speed:</span>
      <input type="number" id="scrollSpeed" min="1" max="150" value="50"
        style="width:66px;" title="1=fastest, 150=slowest">
      <button class="btn btn-primary btn-sm" onclick="sendScrollTextData()">Send</button>
    </div>
  </div>
</div>

<div id="status-msg"></div>

<div class="card" id="fileArea">
  <div class="card-title" id="fileAreaTitle">Files</div>
  <div id="fileList"></div>
  <div id="uploadProgressArea" style="display:none; margin-top:10px;">
    <progress id="progressBar" value="0" max="100"></progress>
    <div id="uploadStatusText"></div>
  </div>
</div>

<script>
var _statusTimer;
function showStatus(msg, dur) {
  var el = document.getElementById('status-msg');
  el.textContent = msg; el.style.opacity = '1';
  clearTimeout(_statusTimer);
  _statusTimer = setTimeout(function(){ el.style.opacity='0'; }, dur||3000);
}

// Brightness
function onSliderInput(el) { document.getElementById('brightnessNum').value = el.value; }
function onNumChange(el) {
  var v = Math.min(255, Math.max(0, parseInt(el.value)||0));
  el.value = v;
  document.getElementById('pwmSlider').value = v;
  sendBrightness(v);
}
function sendBrightness(v) {
  fetch('/slider?value='+v).catch(function(e){console.error(e);});
}

// Color picker
function hexToRGB(hex) {
  var h = hex.replace(/^#/,'');
  if (h.length===3) h = h[0]+h[0]+h[1]+h[1]+h[2]+h[2];
  var n = parseInt(h,16);
  return {r:(n>>16)&255, g:(n>>8)&255, b:n&255};
}
function applyColor(hex) {
  var rgb = hexToRGB(hex);
  document.getElementById('colorPicker').value = '#'+hex;
  document.getElementById('hexInput').value = hex;
  document.getElementById('colorSwatch').style.background = '#'+hex;
  document.getElementById('colorRGB').textContent = 'R:'+rgb.r+' G:'+rgb.g+' B:'+rgb.b;
  document.getElementById('r').value = rgb.r;
  document.getElementById('g').value = rgb.g;
  document.getElementById('b').value = rgb.b;
}
function onPickerInput(val) { applyColor(val.replace('#','').toLowerCase()); }
function onHexInput(el) {
  var v = el.value.replace(/[^0-9a-fA-F]/g,'').toLowerCase();
  el.value = v;
  if (v.length===6) applyColor(v);
}
function setColor() {
  var r=document.getElementById('r').value;
  var g=document.getElementById('g').value;
  var b=document.getElementById('b').value;
  fetch('/setColor?r='+r+'&g='+g+'&b='+b)
    .then(function(res){return res.text();})
    .then(function(){ showStatus('Color applied \u2714'); })
    .catch(function(){ showStatus('Failed to set color'); });
}

// Toggles — _suppress prevents mutual-disable infinite recursion
var _suppress = false;
// GIF 开关（新增与 scrollText 的互斥）
function toggleGIF(cb) {
  if (cb.checked && !_suppress) {
    var st = document.getElementById('scrollTextToggle');
    if (st.checked) {
      st.checked = false;
      _suppress = true;
      fetch('/toggleScrollText?state=off').catch(function(){});
      _suppress = false;
      showStatus('滚动文字已自动关闭（与 GIF 互斥）');
    }
  }
  fetch('/toggleGIF?state=' + (cb.checked ? 'on' : 'off'))
    .then(r => r.text())
    .then(() => showStatus('GIF 播放: ' + (cb.checked ? 'ON' : 'OFF')))
    .catch(() => showStatus('Error'));
}
function toggleLoopGif(cb) {
  fetch('/toggleLoopGif?state='+(cb.checked?'on':'off'))
    .then(function(r){return r.text();})
    .then(function(){ showStatus('Loop GIF: '+(cb.checked?'ON':'OFF')); })
    .catch(function(){ showStatus('Error'); });
}
function toggleClock(cb) {
  if (cb.checked && !_suppress) {
    var st = document.getElementById('scrollTextToggle');
    if (st.checked) {
      st.checked = false;
      _suppress = true;
      fetch('/toggleScrollText?state=off').catch(function(){});
      _suppress = false;
    }
  }
  fetch('/toggleClock?state='+(cb.checked?'on':'off'))
    .then(function(r){return r.text();})
    .then(function(){ showStatus('Clock: '+(cb.checked?'ON':'OFF')); })
    .catch(function(){ showStatus('Error'); });
}
// 滚动文字开关（保持与 clock 的互斥 + 新增与 GIF 的互斥）
function toggleScrollText(cb) {
  if (cb.checked && !_suppress) {
    // 原有：与 clock 互斥
    var ck = document.getElementById('clockToggle');
    if (ck.checked) {
      ck.checked = false;
      _suppress = true;
      fetch('/toggleClock?state=off').catch(function(){});
      _suppress = false;
    }

    // 新增：与 GIF 互斥
    var gif = document.getElementById('gifToggle');  // 假设你的 GIF 开关 id 是 gifToggle
    if (gif && gif.checked) {
      gif.checked = false;
      _suppress = true;
      fetch('/toggleGIF?state=off').catch(function(){});
      _suppress = false;
      showStatus('GIF 播放已自动关闭（与滚动文字互斥）');
    }
  }

  fetch('/toggleScrollText?state=' + (cb.checked ? 'on' : 'off'))
    .then(r => r.text())
    .then(() => showStatus('滚动文字: ' + (cb.checked ? 'ON' : 'OFF')))
    .catch(() => showStatus('Error'));
}

// Scroll text — encodeURIComponent keeps UTF-8 byte sequences intact
function sendScrollTextData() {
  var text=document.getElementById('scrollText').value;
  var fontSize=document.getElementById('fontSizeToggle').value;
  var speed=document.getElementById('scrollSpeed').value;
  fetch('/updateScrollText?text='+encodeURIComponent(text)+'&fontSize='+fontSize+'&speed='+speed)
    .then(function(r){return r.text();})
    .then(function(){ showStatus('Text sent \u2714'); })
    .catch(function(){ showStatus('Failed to send text'); });
}

// Navigation
function logoutButton() {
  fetch('/logout').catch(function(){});
  setTimeout(function(){ window.open('/logged-out','_self'); }, 500);
}
function rebootButton() {
  showStatus('Rebooting\u2026', 15000);
  fetch('/reboot').catch(function(){});
  setTimeout(function(){ window.open('/reboot','_self'); }, 800);
}

// File list — async fetch replaces legacy sync XHR
function listFilesButton() {
  fetch('/listfiles')
    .then(function(r){return r.text();})
    .then(function(data){
      document.getElementById('fileAreaTitle').textContent = 'Files';
      document.getElementById('fileList').innerHTML = data;
      document.getElementById('uploadProgressArea').style.display = 'none';
      document.getElementById('fileArea').style.display = 'block';
    })
    .catch(function(){ showStatus('Error loading file list'); });
}
function downloadDeleteButton(filename, action) {
  if (action==='delete') {
    if (!confirm('Delete "'+filename+'"?')) return;
    fetch('/file?name='+encodeURIComponent(filename)+'&action=delete')
      .then(function(r){return r.text();})
      .then(function(){ showStatus('Deleted \u2714'); listFilesButton(); })
      .catch(function(){ showStatus('Delete failed'); });
  } else if (action==='download') {
    window.open('/file?name='+encodeURIComponent(filename)+'&action=download','_blank');
  } else if (action==='play') {
    fetch('/file?name='+encodeURIComponent(filename)+'&action=play')
      .then(function(r){return r.text();})
      .then(function(){ showStatus('Now playing: '+filename); })
      .catch(function(){ showStatus('Failed to play'); });
  }
}
window.navigatePage = function(page) {
  fetch('/list?page='+page)
    .then(function(r){return r.text();})
    .then(function(data){ document.getElementById('fileList').innerHTML=data; })
    .catch(function(){ showStatus('Error'); });
};

// Upload
function showUploadButtonFancy() {
  document.getElementById('fileAreaTitle').textContent = 'Upload GIF';
  document.getElementById('fileList').innerHTML =
    '<div style="margin-bottom:8px;">'+
    '<input type="file" id="file1" accept="image/gif" onchange="uploadFile()" '+
    'style="color:#a0a0c0;font-size:0.87em;">'+
    '</div>';
  document.getElementById('uploadProgressArea').style.display = 'none';
  document.getElementById('progressBar').value = 0;
  document.getElementById('fileArea').style.display = 'block';
}
function _(id){ return document.getElementById(id); }
function uploadFile() {
  var file = _('file1').files[0]; if (!file) return;
  var fd = new FormData(); fd.append('file1', file);
  var ajax = new XMLHttpRequest();
  _('uploadProgressArea').style.display = 'block';
  ajax.upload.addEventListener('progress', function(e){
    var pct = Math.round((e.loaded/e.total)*100);
    _('progressBar').value = pct;
    _('uploadStatusText').textContent = pct<100 ? pct+'% \u2014 uploading\u2026' : 'Writing to filesystem\u2026';
  });
  ajax.addEventListener('load', function(){
    _('uploadStatusText').textContent = 'Upload complete!';
    _('progressBar').value = 0;
    setTimeout(listFilesButton, 600);
  });
  ajax.addEventListener('error', function(){ _('uploadStatusText').textContent = 'Upload failed.'; });
  ajax.open('POST', '/');
  ajax.send(fd);
}
</script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>Logged Out</title>
  <style>
    body { background:#0d0d14; color:#888; font-family:'Segoe UI',sans-serif;
      display:flex; align-items:center; justify-content:center; height:100vh; margin:0; }
    a { color:#7070d0; text-decoration:none; font-size:1.05em; }
    a:hover { color:#9090f0; }
  </style>
</head>
<body>
  <div style="text-align:center;">
    <p style="margin-bottom:12px;color:#444;">You have been logged out.</p>
    <a href="/">&#8594; Log Back In</a>
  </div>
</body>
</html>
)rawliteral";

const char reboot_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Rebooting</title>
  <style>
    body { background:#0d0d14; color:#888; font-family:'Segoe UI',sans-serif;
      display:flex; align-items:center; justify-content:center; height:100vh; margin:0; }
    h3 { color:#fff; font-size:1.1em; margin-bottom:10px; }
    #countdown { color:#5050d0; font-size:1.6em; font-weight:700; }
    p { color:#444; font-size:0.82em; margin-top:8px; }
  </style>
</head>
<body>
  <div style="text-align:center;">
    <h3>Rebooting&hellip;</h3>
    <div>Returning in <span id="countdown">10</span>s</div>
    <p>The device is restarting.</p>
  </div>
  <script>
    var s=10;
    (function cd(){
      document.getElementById('countdown').textContent=s;
      if(s-->0) setTimeout(cd,1000); else window.location='/';
    })();
  </script>
</body>
</html>
)rawliteral";
