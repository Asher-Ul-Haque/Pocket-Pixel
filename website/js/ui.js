let availableBoxarts = [];

const bootScreen = document.getElementById('boot-screen');
const startPrompt = document.getElementById('start-prompt');
const mascot = document.getElementById('mascot');
const bootSound = document.getElementById('startup-sound');
const mainLayout = document.getElementById('main-layout');
const consoleScreen = document.querySelector('.console-screen');
const romUploadInput = document.getElementById('rom-upload');

let isBooting = false;
window.GlobalRomBuffer = null; 

async function initializeLibrary() {
    try {
        const response = await fetch('assets/boxarts.json');
        availableBoxarts = await response.json();
    } catch (error) { console.warn("Fallback to initials activated."); }
}

function normalize(name) { return name.toLowerCase().replace(/[^a-z0-9 ]/g, " ").replace(/\s+/g, " ").trim(); }
function stripExtension(name) { return name.replace(/\.[a-z0-9]{1,5}$/i, ""); }
function editDistance(a, b) {
    const m = a.length, n = b.length;
    const dp = Array.from({ length: m + 1 }, () => Array(n + 1).fill(0));
    for (let i = 0; i <= m; i++) dp[i][0] = i;
    for (let j = 0; j <= n; j++) dp[0][j] = j;
    for (let i = 1; i <= m; i++) {
        for (let j = 1; j <= n; j++) {
            const cost = a[i - 1] === b[j - 1] ? 0 : 1;
            dp[i][j] = Math.min(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost);
        }
    }
    return dp[m][n];
}
function findClosestBoxart(targetName, candidates, threshold = 0.65) {
    const normalizedTarget = normalize(targetName);
    let bestMatchUrl = null; let bestScore = threshold;
    for (const url of candidates) {
        const rawName = decodeURIComponent(url.split('/').pop()); 
        const distance = editDistance(normalize(stripExtension(rawName)), normalizedTarget);
        const MathMax = Math.max(normalize(stripExtension(rawName)).length, normalizedTarget.length);
        const similarity = MathMax === 0 ? 1.0 : (1.0 - (distance / MathMax));
        if (similarity >= bestScore) { bestMatchUrl = url; bestScore = similarity; }
    }
    return bestMatchUrl;
}

function renderCartridge(fileName, boxartUrl) {
    if (boxartUrl) {
        consoleScreen.innerHTML = `<img src="${boxartUrl}" alt="${fileName}" style="width: 100%; height: 100%; object-fit: contain; image-rendering: pixelated;">`;
    } else {
        const initials = fileName.substring(0, 2).toUpperCase();
        consoleScreen.innerHTML = `<div style="display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100%;"><span style="font-size: 5rem; color: var(--pure-white);">${initials}</span><span style="margin-top: 10px; font-size: 1.2rem; color: var(--pure-white);">${fileName}</span></div>`;
    }
}

consoleScreen.addEventListener('click', () => { if (consoleScreen.querySelector('.cartridge-prompt')) romUploadInput.click(); });

romUploadInput.addEventListener('change', async (event) => {
    const file = event.target.files[0];
    if (!file) return;
    consoleScreen.innerHTML = `<div class="cartridge-prompt pulse-text">WRITING TO CHIP...</div>`;
    const cleanName = stripExtension(file.name);
    const boxart = findClosestBoxart(cleanName, availableBoxarts);
    const buffer = await file.arrayBuffer();
    await window.PocketDB.insertCartridge(cleanName, buffer, boxart);
    renderCartridge(cleanName, boxart);
});

async function initiateBootSequence() {
    if (isBooting) return;
    isBooting = true;
    startPrompt.style.display = 'none';
    bootSound.play().catch(e => console.warn("Audio blocked by browser"));
    mascot.classList.add('slide-up');
    document.body.classList.add('booted');

    const savedCartridge = await window.PocketDB.getCartridge();
    setTimeout(() => {
        bootScreen.style.display = 'none';
        mainLayout.style.display = 'flex';
        if (savedCartridge) {
            window.GlobalRomBuffer = savedCartridge.romData;
            renderCartridge(savedCartridge.fileName, savedCartridge.boxartUrl);
        }
        setTimeout(() => { mainLayout.style.opacity = '1'; }, 50);
    }, 1500); 
}
bootScreen.addEventListener('click', initiateBootSequence);
window.addEventListener('keydown', (event) => { if (event.code === 'Space' && !isBooting) initiateBootSequence(); });
initializeLibrary();

// ==========================================
// --- SETTINGS MODAL LOGIC (PERSISTENCE) ---
// ==========================================
const settingsBtn = document.getElementById('settings-btn');
const settingsModal = document.getElementById('settings-modal');
const closeSettingsBtn = document.getElementById('close-settings');

if (settingsBtn && settingsModal && closeSettingsBtn) {
    settingsBtn.addEventListener('click', () => settingsModal.style.display = 'flex');
    closeSettingsBtn.addEventListener('click', () => settingsModal.style.display = 'none');
}

let volumes = JSON.parse(localStorage.getItem('pocketVols')) || { ch1: 1.0, ch2: 1.0, ch3: 1.0, ch4: 1.0 };
function updateAudioMixer() {
    localStorage.setItem('pocketVols', JSON.stringify(volumes));
    if (window.PocketEngine && window.PocketEngine.isEngineRunning) window.PocketEngine.setChannelVolumes(volumes.ch1, volumes.ch2, volumes.ch3, volumes.ch4);
}
['ch1', 'ch2', 'ch3', 'ch4'].forEach(ch => {
    const slider = document.getElementById(`vol-${ch}`);
    if (slider) {
        slider.value = volumes[ch];
        slider.addEventListener('input', (e) => { volumes[ch] = parseFloat(e.target.value); updateAudioMixer(); });
    }
});

let currentTheme = localStorage.getItem('pocketTheme') || 'Default (Standard)';
let customColors = JSON.parse(localStorage.getItem('pocketCustomColors')) || ["#ffffff", "#aaaaaa", "#555555", "#000000"];

const themes = {
    "Default (Standard)": ["#d1cb95", "#40985e", "#1a644e", "#04373b"],
    "Classic": ["#9aa13c", "#6c712a", "#4d511e", "#1f200c"],
    "Fizzle": ["#cee5ff", "#c589dc", "#564991", "#1e182a"],
    "Ice cream": ["#fff6d3", "#f9a875", "#eb6b6f", "#7c3f58"],
    "Hollow": ["#fafbf6", "#c6b7be", "#565a75", "#0f0f1b"],
    "Rustic": ["#edb4a1", "#a96868", "#764462", "#2c2137"],
    "Mint": ["#c4f0c2", "#5ab9a8", "#1e606e", "#2d1b00"],
    "SpaceHaze": ["#f8e3c4", "#cc3495", "#6b1fb1", "#0b0630"],
    "Fiery Plague": ["#713141", "#512839", "#312137", "#1a2129"],
    "Gold": ["#cfab51", "#9d654c", "#4d222c", "#210b1b"],
    "Honey": ["#e9f5da", "#f0b695", "#877286", "#3e3a42"],
    "Coral": ["#ffd0a4", "#f4949c", "#7c9aac", "#68518a"],
    "Rabbit": ["#f1e0cd", "#ffa49a", "#da3467", "#35333f"],
    "Caramel autumn": ["#fff4b8", "#ff8b40", "#a22fc9", "#290143"],
    "Snow flake": ["#e7edeb", "#8ecece", "#62a1c7", "#3f6ecc"],
    "Lemon and Lime": ["#fff37b", "#5fcc86", "#39809c", "#28375b"],
    "Kirokaze": ["#e2f3e4", "#94e344", "#46878f", "#332c50"],
    "Red is dead": ["#fffcfe", "#ff0015", "#860020", "#11070a"]
};

function applyColorsToEngine(colors, themeName) {
    if (themeName) localStorage.setItem('pocketTheme', themeName);
    if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
        const c0 = parseInt(colors[0].replace('#', '') + 'FF', 16);
        const c1 = parseInt(colors[1].replace('#', '') + 'FF', 16);
        const c2 = parseInt(colors[2].replace('#', '') + 'FF', 16);
        const c3 = parseInt(colors[3].replace('#', '') + 'FF', 16);
        window.PocketEngine.setPalette(c0, c1, c2, c3);
    }
}

const themeListContainer = document.getElementById('theme-list');
if (themeListContainer) {
    Object.keys(themes).forEach(name => {
        const colors = themes[name];
        const btn = document.createElement('button');
        btn.className = 'theme-btn';
        btn.innerHTML = `<div class="theme-title">${name}</div><div class="swatch-container"><div class="swatch" style="background-color: ${colors[0]}"></div><div class="swatch" style="background-color: ${colors[1]}"></div><div class="swatch" style="background-color: ${colors[2]}"></div><div class="swatch" style="background-color: ${colors[3]}"></div></div>`;
        btn.addEventListener('click', () => applyColorsToEngine(colors, name));
        themeListContainer.appendChild(btn);
    });
}

for (let i = 0; i < 4; i++) {
    const el = document.getElementById(`custom-c${i}`);
    if (el) el.value = customColors[i];
}
const btnApplyCustom = document.getElementById('btn-apply-custom');
if (btnApplyCustom) {
    btnApplyCustom.addEventListener('click', () => {
        customColors = [
            document.getElementById('custom-c0').value, document.getElementById('custom-c1').value,
            document.getElementById('custom-c2').value, document.getElementById('custom-c3').value
        ];
        localStorage.setItem('pocketCustomColors', JSON.stringify(customColors));
        applyColorsToEngine(customColors, 'CUSTOM');
    });
}

window.applySavedSettingsToEngine = function() {
    updateAudioMixer();
    if (currentTheme === 'CUSTOM') applyColorsToEngine(customColors, 'CUSTOM');
    else if (themes[currentTheme]) applyColorsToEngine(themes[currentTheme], currentTheme);
};

// ==========================================
// --- SOCIAL NAG LOGIC ---
// ==========================================
const githubUrl = "https://github.com/Asher-Ul-Haque/Pocket-Pixel";
const playstoreUrl = "https://play.google.com/store/apps/details?id=just.somebody.templates";

function markSocialInteracted() {
    localStorage.setItem('pocketNagInteracted', 'true');
    document.getElementById('nag-modal').style.display = 'none';
}

document.getElementById('github-btn').addEventListener('click', () => { window.open(githubUrl, '_blank'); markSocialInteracted(); });
document.getElementById('playstore-btn').addEventListener('click', () => { window.open(playstoreUrl, '_blank'); markSocialInteracted(); });
document.getElementById('nag-github-btn').addEventListener('click', () => { window.open(githubUrl, '_blank'); markSocialInteracted(); });
document.getElementById('nag-playstore-btn').addEventListener('click', () => { window.open(playstoreUrl, '_blank'); markSocialInteracted(); });
document.getElementById('nag-later-btn').addEventListener('click', () => { document.getElementById('nag-modal').style.display = 'none'; });

window.checkNagModal = function() {
    if (localStorage.getItem('pocketNagInteracted') === 'true') return;
    
    let playCount = parseInt(localStorage.getItem('pocketPlayCount')) || 0;
    playCount++;
    localStorage.setItem('pocketPlayCount', playCount);

    const thresholds = [6, 11, 51, 101, 501, 1001];
    if (thresholds.includes(playCount) || playCount % 1000 === 0) {
        setTimeout(() => { document.getElementById('nag-modal').style.display = 'flex'; }, 1500);
    }
};

// ==========================================
// --- SAVE STATE MODAL LOGIC ---
// ==========================================
const savestateModal = document.getElementById('savestate-modal');
const closeSavestateBtn = document.getElementById('close-savestate');
const stateGrid = document.getElementById('state-grid');
const savestateTitle = document.getElementById('savestate-title');

if (closeSavestateBtn) {
    closeSavestateBtn.addEventListener('click', () => savestateModal.style.display = 'none');
}

window.openSaveStateModal = async function(mode, preCapturedImg = null) {
    savestateTitle.innerText = mode === 'SAVE' ? "SAVE STATE" : "LOAD STATE";
    stateGrid.innerHTML = ''; 
    
    const cart = await window.PocketDB.getCartridge();
    const states = cart && cart.states ? cart.states : [null, null, null, null, null];

    for (let i = 0; i < 5; i++) {
        const slot = states[i];
        const btn = document.createElement('div');
        btn.className = 'state-slot';
        
        let imgHtml = slot && slot.screenshot ? `<img src="${slot.screenshot}">` : `<span>EMPTY</span>`;
        let infoHtml = slot ? `SLOT ${i + 1}<span class="state-timestamp">${new Date(slot.timestamp).toLocaleString()}</span>` : `SLOT ${i + 1}<span class="state-timestamp">No Data</span>`;

        btn.innerHTML = `<div class="state-image-box">${imgHtml}</div><div class="state-info">${infoHtml}</div>`;

        btn.addEventListener('click', async () => {
            if (mode === 'SAVE') {
                const buffer = window.PocketEngine.saveState();
                if (buffer) {
                    await window.PocketDB.saveSlot(i, buffer, preCapturedImg);
                    savestateModal.style.display = 'none';
                }
            } else if (mode === 'LOAD') {
                if (slot && slot.buffer) {
                    window.PocketEngine.loadState(slot.buffer);
                    savestateModal.style.display = 'none';
                }
            }
        });
        stateGrid.appendChild(btn);
    }
    savestateModal.style.display = 'flex';
};
