// ==========================================
// --- DOM ELEMENTS & STATE ---
// ==========================================
const btnStart = document.getElementById('btn-start'); 
const btnSelect = document.getElementById('btn-select');
const btnA = document.getElementById('btn-a'); 
const btnB = document.getElementById('btn-b'); 
const consoleScreenInput = document.querySelector('.console-screen');

const btnPlayPause = document.getElementById('btn-play-pause');
const btnFastForward = document.getElementById('btn-ff');
const btnSave = document.getElementById('btn-save');
const btnLoad = document.getElementById('btn-load');
const btnEject = document.getElementById('btn-eject');

// ==========================================
// --- THE INPUT TRANSLATION LAYER ---
// ==========================================
const SDL_KEYS = {
    Up: 'ArrowUp', Down: 'ArrowDown', Left: 'ArrowLeft', Right: 'ArrowRight',
    A: 'KeyZ', B: 'KeyX', Start: 'Enter', Select: 'ShiftRight', Pause: 'KeyP', FastForward: 'KeyF'
};

const UI_MAPPING = {
    Up: document.querySelector('.dpad-btn.up'),
    Down: document.querySelector('.dpad-btn.down'),
    Left: document.querySelector('.dpad-btn.left'),
    Right: document.querySelector('.dpad-btn.right'),
    A: btnA,
    B: btnB,
    Start: btnStart,
    Select: btnSelect
};

let userBinds = JSON.parse(localStorage.getItem('pocketBinds')) || {
    Up: 'ArrowUp', Down: 'ArrowDown', Left: 'ArrowLeft', Right: 'ArrowRight',
    A: 'KeyZ', B: 'KeyX', Start: 'Enter', Select: 'ShiftRight'
};

function animateButton(action, isDown) {
    const btn = UI_MAPPING[action];
    if (btn) {
        if (isDown) btn.classList.add('pressed');
        else btn.classList.remove('pressed');
    }
}

function sendKeyEvent(keyCode, isDown) {
    const eventType = isDown ? 'keydown' : 'keyup';
    const event = new KeyboardEvent(eventType, { code: keyCode, key: keyCode, bubbles: true, cancelable: true });
    const canvas = document.getElementById('canvas');
    if (canvas) canvas.dispatchEvent(event);
    else window.dispatchEvent(event);
}

function bindHtmlButton(element, action) {
    if (!element) return;
    const press = (down) => { sendKeyEvent(SDL_KEYS[action], down); animateButton(action, down); };
    element.addEventListener('mousedown', (e) => { e.preventDefault(); press(true); });
    element.addEventListener('mouseup', (e) => { e.preventDefault(); press(false); });
    element.addEventListener('mouseleave', (e) => { e.preventDefault(); press(false); });
    element.addEventListener('touchstart', (e) => { e.preventDefault(); press(true); });
    element.addEventListener('touchend', (e) => { e.preventDefault(); press(false); });
}

function bindDiagonal(element, action1, action2) {
    if (!element) return;
    const press = (isDown) => { 
        sendKeyEvent(SDL_KEYS[action1], isDown); animateButton(action1, isDown);
        sendKeyEvent(SDL_KEYS[action2], isDown); animateButton(action2, isDown); 
    };
    element.addEventListener('mousedown', (e) => { e.preventDefault(); press(true); });
    element.addEventListener('mouseup', (e) => { e.preventDefault(); press(false); });
    element.addEventListener('mouseleave', (e) => { e.preventDefault(); press(false); });
    element.addEventListener('touchstart', (e) => { e.preventDefault(); press(true); });
    element.addEventListener('touchend', (e) => { e.preventDefault(); press(false); });
}

bindHtmlButton(btnA, 'A');
bindHtmlButton(btnB, 'B');
bindHtmlButton(btnStart, 'Start');
bindHtmlButton(btnSelect, 'Select');
bindHtmlButton(UI_MAPPING.Up, 'Up');
bindHtmlButton(UI_MAPPING.Down, 'Down');
bindHtmlButton(UI_MAPPING.Left, 'Left');
bindHtmlButton(UI_MAPPING.Right, 'Right');

bindDiagonal(document.querySelector('.diag.top-left'), 'Up', 'Left');
bindDiagonal(document.querySelector('.diag.top-right'), 'Up', 'Right');
bindDiagonal(document.querySelector('.diag.bottom-left'), 'Down', 'Left');
bindDiagonal(document.querySelector('.diag.bottom-right'), 'Down', 'Right');

// ==========================================
// --- ENGINE PAUSE & MEDIA CONTROLS ---
// ==========================================
let isFastForwarding = false;
let saveSyncInterval = null;

function attemptFullscreen() {
    const elem = document.documentElement;
    if (elem.requestFullscreen) elem.requestFullscreen().catch(e => console.warn("Fullscreen blocked:", e));
    else if (elem.webkitRequestFullscreen) elem.webkitRequestFullscreen();
}

// Background daemon monitoring the C core save state changes
function startSaveSyncDaemon() {
    if (saveSyncInterval) clearInterval(saveSyncInterval);
    console.log("[POCKET DAEMON] Save synchronization monitor armed.");
    
    saveSyncInterval = setInterval(async () => {
        if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
            const updated = Module.ccall('webCheckAndClearSaveUpdated', 'number', [], []);
            if (updated === 1) {
                console.log("[POCKET DAEMON] C core signaled 'saveFileUpdated == true'. Fetching virtual disk...");
                try {
                    const saveBytes = FS.readFile('/game.sav');
                    await window.PocketDB.saveCartridgeRAM(saveBytes.buffer);
                    console.log("[POCKET DAEMON] IndexedDB synchronization completed successfully.");
                    
                    // TRIGGER NATIVE BROWSER SYSTEM NOTIFICATION
                    if (typeof Notification !== 'undefined' && Notification.permission === 'granted') {
                        new Notification("Pocket Pixel", {
                            body: "Game progress saved successfully!",
                            icon: "assets/icons/svg/save.svg", // Uses your crisp vector icon
                            silent: false // Leverages the native OS notification sound
                        });
                    }
                } catch (e) {
                    console.error("[POCKET DAEMON] Error writing save block to IndexedDB:", e);
                }
            }
        } else {
            console.log("[POCKET DAEMON] Engine offline. Disarming monitor.");
            clearInterval(saveSyncInterval);
        }
    }, 1000);
}

btnPlayPause.addEventListener('click', async () => {
    // Request native system notification access on user interaction
    if (typeof Notification !== 'undefined' && Notification.permission === 'default') {
        Notification.requestPermission().then(permission => {
            console.log("[POCKET FRONTEND] Native notification permission state:", permission);
        });
    }

    console.log("[POCKET FRONTEND] Play/Pause clicked. Active engine state running:", window.PocketEngine.isEngineRunning)
    
    if (!window.PocketEngine || !window.PocketEngine.isEngineRunning) {
        console.log("[POCKET FRONTEND] Requesting primary cartridge entity from IndexedDB...");
        const savedCartridge = await window.PocketDB.getCartridge();
        let romToBoot = savedCartridge ? savedCartridge.romData : null;

        if (romToBoot) {
            console.log("[POCKET FRONTEND] ROM array loaded successfully (" + romToBoot.byteLength + " bytes). Directing to Fullscreen mode.");
            attemptFullscreen(); 

            consoleScreenInput.innerHTML = `<canvas id="canvas" tabindex="-1" style="width: 100%; height: 100%; image-rendering: pixelated; outline: none;"></canvas>`;
            const canvas = document.getElementById('canvas');
            window.Module = window.Module || {};
            window.Module.canvas = canvas;
            
            console.log("[POCKET FRONTEND] Injecting WASM build script context wrappers...");
            window.PocketEngine.boot(romToBoot);
            
            const originalRuntimeInit = window.Module.onRuntimeInitialized;
            window.Module.onRuntimeInitialized = () => {
                console.log("[POCKET LIFECYCLE] Intercepted Emscripten Runtime Initialization event.");
                
                // FIX: Write the staged battery file to VFS BEFORE invoking C main loop execution vectors
                if (savedCartridge && savedCartridge.saveData) {
                    console.log("[POCKET LIFECYCLE] Pre-existing save array detected (" + savedCartridge.saveData.byteLength + " bytes). Mounting to VFS /game.sav...");
                    FS.writeFile('/game.sav', new Uint8Array(savedCartridge.saveData));
                    console.log("[POCKET LIFECYCLE] Mount verified. /game.sav is active on virtual drive.");
                } else {
                    console.log("[POCKET LIFECYCLE] No previous save block detected in database. Skipping mount (C core will allocate fresh array).");
                }
                
                console.log("[POCKET LIFECYCLE] Handing control flow over to native C++ callMain loop...");
                if (originalRuntimeInit) originalRuntimeInit();
                
                console.log("[POCKET LIFECYCLE] Native callMain loop completed setup. Initializing daemon pollers...");
                startSaveSyncDaemon();
            };

            const engineCheck = setInterval(() => {
                if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
                    if (window.applySavedSettingsToEngine) window.applySavedSettingsToEngine();
                    clearInterval(engineCheck);
                }
            }, 100);

            document.body.classList.add('playing');
            btnPlayPause.querySelector('img').src = 'assets/icons/svg/pause.svg';
            if (window.checkNagModal) window.checkNagModal();
        } else {
            console.warn("[POCKET FRONTEND] Execution halted: No cartridge found in database store.");
        }
    } else {
        const isPaused = window.PocketEngine.isPaused();
        const targetState = !isPaused;
        console.log("[POCKET FRONTEND] Toggling pause execution vector to:", targetState);
        
        window.PocketEngine.setPaused(targetState);
        btnPlayPause.querySelector('img').src = targetState ? 'assets/icons/svg/play.svg' : 'assets/icons/svg/pause.svg';
    }
});

btnFastForward.addEventListener('click', () => {
    if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
        isFastForwarding = !isFastForwarding;
        console.log("[POCKET FRONTEND] Fast Forward toggled to:", isFastForwarding);
        sendKeyEvent(SDL_KEYS.FastForward, true);
        setTimeout(() => sendKeyEvent(SDL_KEYS.FastForward, false), 50);
        btnFastForward.querySelector('img').src = isFastForwarding ? 'assets/icons/svg/slow.svg' : 'assets/icons/svg/fast.svg';
    }
});

btnEject.addEventListener('click', async () => {
    console.log("[POCKET FRONTEND] Eject cartridge request caught.");
    const confirmation = confirm("Are you sure you want to eject the cartridge?\nAll Save States and persistent Save Files (.sav) will be permanently deleted!");
    if (confirmation) {
        console.log("[POCKET FRONTEND] Purging core synchronization timers...");
        if (saveSyncInterval) clearInterval(saveSyncInterval);
        
        try {
            if (typeof FS !== 'undefined') {
                console.log("[POCKET FRONTEND] Unlinking virtual filesystem disk files...");
                FS.unlink('/game.sav');
                FS.unlink('/game.rom');
            }
        } catch(e) {
            console.log("[POCKET FRONTEND] Virtual file context clean note: Files already absent.");
        }

        console.log("[POCKET FRONTEND] Deleting database entities...");
        await window.PocketDB.deleteCartridge();
        console.log("[POCKET FRONTEND] Purge completed. Resetting sandbox environment.");
        window.location.reload();
    }
});

document.addEventListener('click', () => {
    const canvas = document.getElementById('canvas');
    if (canvas && window.PocketEngine && window.PocketEngine.isEngineRunning) canvas.focus();
});

// ==========================================
// --- SAVE STATE MODAL HOOKS ---
// ==========================================
if (btnSave && btnLoad) {
    btnSave.addEventListener('click', () => { 
        if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
            console.log("[POCKET FRONTEND] Snapshot save state invoked.");
            const preCapturedImg = window.PocketEngine.captureScreenshot();
            window.openSaveStateModal('SAVE', preCapturedImg);
        }
    });
    btnLoad.addEventListener('click', () => { 
        if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
            console.log("[POCKET FRONTEND] Open load state modal view invoked.");
            window.openSaveStateModal('LOAD'); 
        }
    });
}

// ==========================================
// --- DYNAMIC KEYBINDING / GAMEPAD ENGINE ---
// ==========================================
let listeningKbAction = null; 
let listeningGpAction = null; 

let gpBinds = JSON.parse(localStorage.getItem('pocketGpBinds')) || {
    Up: 12, Down: 13, Left: 14, Right: 15, A: 1, B: 0, Start: 9, Select: 8
};

window.addEventListener('keydown', (e) => {
    if (!e.isTrusted) return;

    if (listeningKbAction) {
        e.preventDefault();
        userBinds[listeningKbAction] = e.code;
        localStorage.setItem('pocketBinds', JSON.stringify(userBinds));
        const btn = document.querySelector(`.keybind-btn.kb[data-action="${listeningKbAction}"]`);
        if (btn) { btn.textContent = e.code; btn.classList.remove('listening'); }
        listeningKbAction = null;
        return;
    }
    if (!window.PocketEngine || !window.PocketEngine.isEngineRunning) return;
    const action = Object.keys(userBinds).find(key => userBinds[key] === e.code);
    if (action) { e.preventDefault(); sendKeyEvent(SDL_KEYS[action], true); animateButton(action, true); }
});

window.addEventListener('keyup', (e) => {
    if (!e.isTrusted) return;
    if (!window.PocketEngine || !window.PocketEngine.isEngineRunning) return;
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
        const gamepads = navigator.getGamepads();
        for (let i = 0; i < gamepads.length; i++) {
            if (gamepads[i]) { baselineGamepadState = { buttons: gamepads[i].buttons.map(b => b.pressed), axes: gamepads[i].axes.slice() }; break; }
        }
    });
});

const prevGpActions = {}; 
function updateGamepadAction(action, isPressed) {
    if (prevGpActions[action] !== isPressed) {
        sendKeyEvent(SDL_KEYS[action], isPressed);
        animateButton(action, isPressed); 
        prevGpActions[action] = isPressed;
    }
}

function evaluateGpMap(pad, mappedVal) {
    if (mappedVal === undefined || mappedVal === null) return false;
    if (typeof mappedVal === 'number' || !isNaN(mappedVal)) return pad.buttons && pad.buttons.length > mappedVal ? pad.buttons[mappedVal].pressed : false;
    if (typeof mappedVal === 'string' && mappedVal.startsWith('A')) {
        const axisIdx = parseInt(mappedVal.charAt(1)); const sign = mappedVal.charAt(2);
        if (pad.axes && pad.axes.length > axisIdx) { if (sign === '-') return pad.axes[axisIdx] < -0.5; if (sign === '+') return pad.axes[axisIdx] > 0.5; }
    }
    return false;
}

function pollGamepads() {
    const gamepads = navigator.getGamepads();
    let pad = null;
    for (let i = 0; i < gamepads.length; i++) { if (gamepads[i]) { pad = gamepads[i]; break; } }

    if (pad) {
        if (listeningGpAction && baselineGamepadState) {
            let mappedValue = null;
            for(let i = 0; i < pad.buttons.length; i++) {
                if (pad.buttons[i].pressed && !baselineGamepadState.buttons[i]) { mappedValue = i; break; }
            }
            if (mappedValue === null && pad.axes) {
                for(let i = 0; i < pad.axes.length; i++) {
                    if (pad.axes[i] < -0.6 && baselineGamepadState.axes[i] >= -0.6) { mappedValue = `A${i}-`; break; }
                    if (pad.axes[i] > 0.6 && baselineGamepadState.axes[i] <= 0.6) { mappedValue = `A${i}+`; break; }
                }
            }
            if (mappedValue !== null) {
                gpBinds[listeningGpAction] = mappedValue; localStorage.setItem('pocketGpBinds', JSON.stringify(gpBinds));
                const btn = document.querySelector(`.keybind-btn.gp[data-action="${listeningGpAction}"]`);
                if (btn) { btn.textContent = (typeof mappedValue === 'string') ? mappedValue : `Btn ${mappedValue}`; btn.classList.remove('listening'); }
                listeningGpAction = null; baselineGamepadState = null;
            }
        } 
        else if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
            const actionState = {
                Left: evaluateGpMap(pad, gpBinds.Left), Right: evaluateGpMap(pad, gpBinds.Right),
                Up: evaluateGpMap(pad, gpBinds.Up), Down: evaluateGpMap(pad, gpBinds.Down),
                A: evaluateGpMap(pad, gpBinds.A), B: evaluateGpMap(pad, gpBinds.B),
                Start: evaluateGpMap(pad, gpBinds.Start), Select: evaluateGpMap(pad, gpBinds.Select)
            };
            Object.keys(actionState).forEach(action => updateGamepadAction(action, actionState[action]));
        }
    }
    requestAnimationFrame(pollGamepads);
}
pollGamepads();
