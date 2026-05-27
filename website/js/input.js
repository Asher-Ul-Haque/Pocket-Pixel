// --- DOM Elements ---
const btnStart = document.getElementById('btn-start');
const btnPause = document.getElementById('btn-pause');
const consoleScreenInput = document.querySelector('.console-screen');

// Game Boy Action Buttons
const btnA = document.querySelectorAll('.action-btn')[1]; // Bottom button
const btnB = document.querySelectorAll('.action-btn')[0]; // Top button
const btnSelect = document.querySelector('.left-pane .pill-btn');

// D-Pad Nodes
const dpadUp = document.querySelector('.dpad-btn.up');
const dpadDown = document.querySelector('.dpad-btn.down');
const dpadLeft = document.querySelector('.dpad-btn.left');
const dpadRight = document.querySelector('.dpad-btn.right');
const diagTopLeft = document.querySelector('.diag.top-left');
const diagTopRight = document.querySelector('.diag.top-right');
const diagBotLeft = document.querySelector('.diag.bottom-left');
const diagBotRight = document.querySelector('.diag.bottom-right');

// --- Virtual Keyboard Engine ---
// We simulate actual hardware keyboard events so the C++ SDL3 layer catches them
function sendKeyEvent(keyCode, isDown) {
    console.log(`[JS Debug] Firing KeyboardEvent: ${keyCode} | isDown: ${isDown}`);
    const eventType = isDown ? 'keydown' : 'keyup';
    window.dispatchEvent(new KeyboardEvent(eventType, { code: keyCode }));
}

// Helper to bind HTML buttons to virtual keys
function bindButton(element, keyCode) {
    if (!element) return;
    
    // Mouse support
    element.addEventListener('mousedown', (e) => { e.preventDefault(); sendKeyEvent(keyCode, true); });
    element.addEventListener('mouseup', (e) => { e.preventDefault(); sendKeyEvent(keyCode, false); });
    element.addEventListener('mouseleave', (e) => { e.preventDefault(); sendKeyEvent(keyCode, false); });
    
    // Touch screen support
    element.addEventListener('touchstart', (e) => { e.preventDefault(); sendKeyEvent(keyCode, true); });
    element.addEventListener('touchend', (e) => { e.preventDefault(); sendKeyEvent(keyCode, false); });
}

// --- The Boot & Pause Logic ---
async function handleStartPress(e) {
    if (e) e.preventDefault();

    if (!window.PocketEngine.isEngineRunning) {
        const savedCartridge = await window.PocketDB.getCartridge();
        
        if (savedCartridge && savedCartridge.romData) {
            console.log("Booting Cartridge...");
            
            // Clear boxart and inject HTML5 Canvas
            consoleScreenInput.innerHTML = `
                <canvas id="canvas" style="width: 100%; height: 100%; image-rendering: pixelated; outline: none;"></canvas>
            `;
            
            window.Module = window.Module || {};
            window.Module.canvas = document.getElementById('canvas');
            
            window.PocketEngine.boot(savedCartridge.romData);
        } else {
            const prompt = document.querySelector('.cartridge-prompt');
            if (prompt) {
                prompt.style.color = 'var(--error-red)';
                setTimeout(() => prompt.style.color = 'var(--pure-white)', 500);
            }
        }
    } else {
        // If engine is running, send the actual START button command (Enter key)
        sendKeyEvent('Enter', true);
        setTimeout(() => sendKeyEvent('Enter', false), 50);
    }
}

// --- Wire Up Standard Buttons ---
// Default C++ Binds: A=Z, B=X, Select=ShiftRight
bindButton(btnA, 'KeyZ');
bindButton(btnB, 'KeyX');
bindButton(btnSelect, 'ShiftRight');

// The START button is special because it acts as the power button first
btnStart.addEventListener('mousedown', handleStartPress);
btnStart.addEventListener('touchstart', handleStartPress);

// Hotbar Pause Button
btnPause.addEventListener('click', () => {
    if (window.PocketEngine.isEngineRunning) {
        sendKeyEvent('KeyP', true);
        setTimeout(() => sendKeyEvent('KeyP', false), 50);
        console.log("Emulation Paused/Resumed via Hotbar.");
    }
});

// --- Wire Up D-Pad ---
bindButton(dpadUp, 'ArrowUp');
bindButton(dpadDown, 'ArrowDown');
bindButton(dpadLeft, 'ArrowLeft');
bindButton(dpadRight, 'ArrowRight');

// --- The Diagonal Multi-Touch Trick ---
// If the user clicks the invisible corner buttons, we press TWO keys at once
function bindDiagonal(element, key1, key2) {
    if (!element) return;
    const press = (isDown) => {
        sendKeyEvent(key1, isDown);
        sendKeyEvent(key2, isDown);
    };
    
    element.addEventListener('mousedown', (e) => { e.preventDefault(); press(true); });
    element.addEventListener('mouseup', (e) => { e.preventDefault(); press(false); });
    element.addEventListener('mouseleave', (e) => { e.preventDefault(); press(false); });
    
    element.addEventListener('touchstart', (e) => { e.preventDefault(); press(true); });
    element.addEventListener('touchend', (e) => { e.preventDefault(); press(false); });
}

bindDiagonal(diagTopLeft, 'ArrowUp', 'ArrowLeft');
bindDiagonal(diagTopRight, 'ArrowUp', 'ArrowRight');
bindDiagonal(diagBotLeft, 'ArrowDown', 'ArrowLeft');
bindDiagonal(diagBotRight, 'ArrowDown', 'ArrowRight');

// Keep canvas focused so physical keyboard presses don't scroll the web page
window.addEventListener('keydown', (e) => {
    if (window.PocketEngine.isEngineRunning && ['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Space'].includes(e.code)) {
        e.preventDefault();
    }
});

// ==========================================
// --- GAMEPAD API INTEGRATION ---
// ==========================================

const prevGamepadState = {
    buttons: {},
    axes: { up: false, down: false, left: false, right: false }
};

// Helper: Safely check if a button exists on this specific hardware before reading
function getBtn(pad, index) {
    if (pad.buttons && pad.buttons.length > index) {
        return pad.buttons[index].pressed;
    }
    return false;
}

function updateGamepadButton(buttonIndex, keyCode, isPressed) {
    if (prevGamepadState.buttons[buttonIndex] !== isPressed) {
        sendKeyEvent(keyCode, isPressed);
        prevGamepadState.buttons[buttonIndex] = isPressed;
    }
}

function updateGamepadAxis(axisName, keyCode, isPressed) {
    if (prevGamepadState.axes[axisName] !== isPressed) {
        sendKeyEvent(keyCode, isPressed);
        prevGamepadState.axes[axisName] = isPressed;
    }
}

function pollGamepads() {
    if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
        const gamepads = navigator.getGamepads();
        let pad = null;

        for (let i = 0; i < gamepads.length; i++) {
            if (gamepads[i]) {
                pad = gamepads[i];
                break;
            }
        }

        if (pad) {
            // Face Buttons
            updateGamepadButton(0, 'KeyX', getBtn(pad, 0));
            updateGamepadButton(1, 'KeyZ', getBtn(pad, 1));
            
            // Start & Select
            updateGamepadButton(8, 'ShiftRight', getBtn(pad, 8));
            updateGamepadButton(9, 'Enter', getBtn(pad, 9));
            
            // D-Pad (Safely checked for controllers that have them as buttons)
            updateGamepadButton(12, 'ArrowUp', getBtn(pad, 12));
            updateGamepadButton(13, 'ArrowDown', getBtn(pad, 13));
            updateGamepadButton(14, 'ArrowLeft', getBtn(pad, 14));
            updateGamepadButton(15, 'ArrowRight', getBtn(pad, 15));

            // Analog Stick Support (Also catches D-Pads mapped to axes)
            if (pad.axes && pad.axes.length >= 2) {
                const leftStickX = pad.axes[0];
                const leftStickY = pad.axes[1];

                updateGamepadAxis('left', 'ArrowLeft', leftStickX < -0.5);
                updateGamepadAxis('right', 'ArrowRight', leftStickX > 0.5);
                updateGamepadAxis('up', 'ArrowUp', leftStickY < -0.5);
                updateGamepadAxis('down', 'ArrowDown', leftStickY > 0.5);
            }
        }
    }
    requestAnimationFrame(pollGamepads);
}

pollGamepads();

window.addEventListener("gamepadconnected", (e) => {
    console.log(`[Gamepad] Connected: ${e.gamepad.id}`);
});
