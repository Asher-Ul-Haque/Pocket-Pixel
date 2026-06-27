const btnStart = document.getElementById('btn-start'), btnSelect = document.getElementById('btn-select'), btnA = document.getElementById('btn-a'), btnB = document.getElementById('btn-b'), consoleScreenInput = document.querySelector('.console-screen');
const btnPlayPause = document.getElementById('btn-play-pause'), btnFastForward = document.getElementById('btn-ff'), btnSave = document.getElementById('btn-save'), btnLoad = document.getElementById('btn-load'), btnEject = document.getElementById('btn-eject'), btnFullscreen = document.getElementById('btn-fullscreen');
if (btnFullscreen) {
    btnFullscreen.addEventListener('click', () => { attemptFullscreen(); });
}
const SDL_KEYS = { Up: 'ArrowUp', Down: 'ArrowDown', Left: 'ArrowLeft', Right: 'ArrowRight', A: 'KeyZ', B: 'KeyX', Start: 'Enter', Select: 'ShiftRight', Pause: 'KeyP', FastForward: 'KeyF' };
const UI_MAPPING = { Up: document.querySelector('.dpad-btn.up'), Down: document.querySelector('.dpad-btn.down'), Left: document.querySelector('.dpad-btn.left'), Right: document.querySelector('.dpad-btn.right'), A: btnA, B: btnB, Start: btnStart, Select: btnSelect };
let userBinds = JSON.parse(localStorage.getItem('pocketBinds')) || { Up: 'ArrowUp', Down: 'ArrowDown', Left: 'ArrowLeft', Right: 'ArrowRight', A: 'KeyZ', B: 'KeyX', Start: 'Enter', Select: 'ShiftRight' };

function animateButton(action, isDown) {
    const btn = UI_MAPPING[action];
    if (btn) isDown ? btn.classList.add('pressed') : btn.classList.remove('pressed');
}
function sendKeyEvent(keyCode, isDown) {
    const event = new KeyboardEvent(isDown ? 'keydown' : 'keyup', { code: keyCode, key: keyCode, bubbles: true, cancelable: true });
    const canvas = document.getElementById('canvas');
    (canvas || window).dispatchEvent(event);
}
function bindHtmlButton(element, action) {
    if (!element) return;
    const press = (down) => { sendKeyEvent(SDL_KEYS[action], down); animateButton(action, down); };
    ['mousedown', 'touchstart'].forEach(e => element.addEventListener(e, (ev) => { ev.preventDefault(); press(true); }));
    ['mouseup', 'mouseleave', 'touchend'].forEach(e => element.addEventListener(e, (ev) => { ev.preventDefault(); press(false); }));
}
function bindDiagonal(element, a1, a2) {
    if (!element) return;
    const press = (d) => { sendKeyEvent(SDL_KEYS[a1], d); animateButton(a1, d); sendKeyEvent(SDL_KEYS[a2], d); animateButton(a2, d); };
    ['mousedown', 'touchstart'].forEach(e => element.addEventListener(e, (ev) => { ev.preventDefault(); press(true); }));
    ['mouseup', 'mouseleave', 'touchend'].forEach(e => element.addEventListener(e, (ev) => { ev.preventDefault(); press(false); }));
}
[btnA, btnB, btnStart, btnSelect, UI_MAPPING.Up, UI_MAPPING.Down, UI_MAPPING.Left, UI_MAPPING.Right].forEach((b, i) => bindHtmlButton(b, ['A', 'B', 'Start', 'Select', 'Up', 'Down', 'Left', 'Right'][i]));
bindDiagonal(document.querySelector('.diag.top-left'), 'Up', 'Left');
bindDiagonal(document.querySelector('.diag.top-right'), 'Up', 'Right');
bindDiagonal(document.querySelector('.diag.bottom-left'), 'Down', 'Left');
bindDiagonal(document.querySelector('.diag.bottom-right'), 'Down', 'Right');

let isFastForwarding = false, saveSyncInterval = null;
function attemptFullscreen() {
    const elem = document.documentElement;
    (elem.requestFullscreen || elem.webkitRequestFullscreen)?.call(elem).catch(e => console.warn("Fullscreen blocked:", e));
}
function startSaveSyncDaemon() {
    if (saveSyncInterval) clearInterval(saveSyncInterval);
    saveSyncInterval = setInterval(async () => {
        if (window.PocketEngine?.isEngineRunning) {
            if (Module.ccall('webCheckAndClearSaveUpdated', 'number', [], []) === 1) {
                try {
                    const saveBytes = FS.readFile('/game.sav');
                    await window.PocketDB.saveCartridgeRAM(saveBytes.buffer);
                    if (typeof Notification !== 'undefined' && Notification.permission === 'granted') new Notification("Pocket Pixel", { body: "Game saved!", icon: "assets/icons/svg/save.svg" });
                } catch (e) { console.error(e); }
            }
        } else clearInterval(saveSyncInterval);
    }, 1000);
}
btnPlayPause.addEventListener('click', async () => {
    if (typeof Notification !== 'undefined' && Notification.permission === 'default') Notification.requestPermission();
    if (!window.PocketEngine?.isEngineRunning) {
        const saved = await window.PocketDB.getCartridge();
        if (saved?.romData) {
            attemptFullscreen();
            consoleScreenInput.innerHTML = `<canvas id="canvas" tabindex="-1" style="width:100%;height:100%;image-rendering:pixelated;outline:none;"></canvas>`;
            window.Module = { canvas: document.getElementById('canvas'), ...window.Module };
            window.PocketEngine.boot(saved.romData);
            const orig = window.Module.onRuntimeInitialized;
            window.Module.onRuntimeInitialized = () => {
                if (saved.saveData) FS.writeFile('/game.sav', new Uint8Array(saved.saveData));
                if (orig) orig();
                startSaveSyncDaemon();
            };
            document.body.classList.add('playing');
            btnPlayPause.querySelector('img').src = 'assets/icons/svg/pause.svg';
        }
    } else {
        const p = !window.PocketEngine.isPaused();
        window.PocketEngine.setPaused(p);
        btnPlayPause.querySelector('img').src = `assets/icons/svg/${p ? 'play' : 'pause'}.svg`;
    }
});
btnFastForward.addEventListener('click', () => {
    if (window.PocketEngine?.isEngineRunning) {
        isFastForwarding = !isFastForwarding;
        sendKeyEvent(SDL_KEYS.FastForward, true);
        setTimeout(() => sendKeyEvent(SDL_KEYS.FastForward, false), 50);
        btnFastForward.querySelector('img').src = `assets/icons/svg/${isFastForwarding ? 'slow' : 'fast'}.svg`;
    }
});
btnEject.addEventListener('click', async () => {
    if (confirm("Eject cartridge? All saves will be deleted!")) {
        if (saveSyncInterval) clearInterval(saveSyncInterval);
        try { FS.unlink('/game.sav'); FS.unlink('/game.rom'); } catch(e) {}
        await window.PocketDB.deleteCartridge();
        window.location.reload();
    }
});
document.addEventListener('click', () => document.getElementById('canvas')?.focus());
if (btnSave && btnLoad) {
    btnSave.addEventListener('click', () => window.PocketEngine?.isEngineRunning && window.openSaveStateModal('SAVE', window.PocketEngine.captureScreenshot()));
    btnLoad.addEventListener('click', () => window.PocketEngine?.isEngineRunning && window.openSaveStateModal('LOAD'));
}

let listeningKbAction = null, listeningGpAction = null, gpBinds = JSON.parse(localStorage.getItem('pocketGpBinds')) || { Up: 'A7-', Down: 'A7+', Left: 'A6-', Right: 'A6+', A: 0, B: 1, Start: 11, Select: 10 };
window.addEventListener('keydown', (e) => {
    if (!e.isTrusted) return;
    if (listeningKbAction) {
        e.preventDefault();
        userBinds[listeningKbAction] = e.code;
        localStorage.setItem('pocketBinds', JSON.stringify(userBinds));
        const btn = document.querySelector(`.keybind-btn.kb[data-action="${listeningKbAction}"]`);
        if (btn) { btn.textContent = e.code; btn.classList.remove('listening'); }
        listeningKbAction = null; return;
    }
    const action = Object.keys(userBinds).find(key => userBinds[key] === e.code);
    if (action && window.PocketEngine?.isEngineRunning) { e.preventDefault(); sendKeyEvent(SDL_KEYS[action], true); animateButton(action, true); }
});
window.addEventListener('keyup', (e) => {
    if (!e.isTrusted || !window.PocketEngine?.isEngineRunning) return;
    const action = Object.keys(userBinds).find(key => userBinds[key] === e.code);
    if (action) { sendKeyEvent(SDL_KEYS[action], false); animateButton(action, false); }
});
document.querySelectorAll('.keybind-btn.kb').forEach(btn => {
    const action = btn.getAttribute('data-action');
    if (userBinds[action]) btn.textContent = userBinds[action];
    btn.addEventListener('click', () => {
        document.querySelectorAll('.keybind-btn').forEach(b => b.classList.remove('listening'));
        listeningKbAction = action; listeningGpAction = null;
        btn.classList.add('listening'); btn.textContent = "PRESS KEY...";
    });
});
let baselineGamepadState = null;
document.querySelectorAll('.keybind-btn.gp').forEach(btn => {
    const action = btn.getAttribute('data-action');
    if (gpBinds[action] !== undefined) btn.textContent = (typeof gpBinds[action] === 'string') ? gpBinds[action] : `Btn ${gpBinds[action]}`;
    btn.addEventListener('click', () => {
        document.querySelectorAll('.keybind-btn').forEach(b => b.classList.remove('listening'));
        listeningGpAction = action; listeningKbAction = null;
        btn.classList.add('listening'); btn.textContent = "WAIT...";
        const pad = navigator.getGamepads()[0];
        if (pad) baselineGamepadState = { buttons: pad.buttons.map(b => b.pressed), axes: pad.axes.slice() };
    });
});
const prevGpActions = {};
function updateGamepadAction(action, isPressed) {
    if (prevGpActions[action] !== isPressed) { sendKeyEvent(SDL_KEYS[action], isPressed); animateButton(action, isPressed); prevGpActions[action] = isPressed; }
}
function evaluateGpMap(pad, val) {
    if (val === null || val === undefined) return false;
    if (typeof val === 'number') return pad.buttons?.[val]?.pressed || false;
    if (typeof val === 'string' && val.startsWith('A')) {
        const idx = parseInt(val.charAt(1)), s = val.charAt(2);
        return s === '-' ? pad.axes?.[idx] < -0.5 : pad.axes?.[idx] > 0.5;
    }
    return false;
}
function pollGamepads() {
    const pad = navigator.getGamepads()[0];
    if (pad) {
        if (listeningGpAction && baselineGamepadState) {
            let mapped = null;
            for(let i = 0; i < pad.buttons.length; i++) if (pad.buttons[i].pressed && !baselineGamepadState.buttons[i]) { mapped = i; break; }
            if (mapped === null && pad.axes) for(let i = 0; i < pad.axes.length; i++) { if (pad.axes[i] < -0.6 && baselineGamepadState.axes[i] >= -0.6) mapped = `A${i}-`; else if (pad.axes[i] > 0.6 && baselineGamepadState.axes[i] <= 0.6) mapped = `A${i}+`; }
            if (mapped !== null) {
                gpBinds[listeningGpAction] = mapped; localStorage.setItem('pocketGpBinds', JSON.stringify(gpBinds));
                const btn = document.querySelector(`.keybind-btn.gp[data-action="${listeningGpAction}"]`);
                if (btn) { btn.textContent = (typeof mapped === 'string') ? mapped : `Btn ${mapped}`; btn.classList.remove('listening'); }
                listeningGpAction = baselineGamepadState = null;
            }
        } else if (window.PocketEngine?.isEngineRunning) {
            Object.keys(gpBinds).forEach(a => updateGamepadAction(a, evaluateGpMap(pad, gpBinds[a])));
        }
    }
    requestAnimationFrame(pollGamepads);
}
pollGamepads();