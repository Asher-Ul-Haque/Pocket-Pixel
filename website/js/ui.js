let availableBoxarts = [];
const bootScreen = document.getElementById('boot-screen'), startPrompt = document.getElementById('start-prompt'), mascot = document.getElementById('mascot'), bootSound = document.getElementById('startup-sound'), mainLayout = document.getElementById('main-layout'), consoleScreen = document.querySelector('.console-screen'), romUploadInput = document.getElementById('rom-upload');
let isBooting = false;
window.GlobalRomBuffer = null;

async function initializeLibrary() {
    try { const res = await fetch('assets/boxarts.json'); availableBoxarts = await res.json(); }
    catch (e) { console.warn("Fallback to initials activated."); }
}
function normalize(n) { return n.toLowerCase().replace(/[^a-z0-9 ]/g, " ").replace(/\s+/g, " ").trim(); }
function stripExtension(n) { return n.replace(/\.[a-z0-9]{1,5}$/i, ""); }
function editDistance(a, b) {
    const m = a.length, n = b.length, dp = Array.from({ length: m + 1 }, () => Array(n + 1).fill(0));
    for (let i = 0; i <= m; i++) dp[i][0] = i;
    for (let j = 0; j <= n; j++) dp[0][j] = j;
    for (let i = 1; i <= m; i++) for (let j = 1; j <= n; j++) dp[i][j] = Math.min(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + (a[i - 1] === b[j - 1] ? 0 : 1));
    return dp[m][n];
}
function findClosestBoxart(target, candidates, threshold = 0.65) {
    const norm = normalize(target);
    let best = null, score = threshold;
    for (const url of candidates) {
        const raw = normalize(stripExtension(decodeURIComponent(url.split('/').pop())));
        const dist = editDistance(raw, norm), max = Math.max(raw.length, norm.length);
        const sim = max === 0 ? 1.0 : (1.0 - (dist / max));
        if (sim >= score) { best = url; score = sim; }
    }
    return best;
}
function renderCartridge(name, url) {
    if (url) consoleScreen.innerHTML = `<img src="${url}" alt="${name}" style="width:100%;height:100%;object-fit:contain;image-rendering:pixelated;">`;
    else {
        const i = name.substring(0, 2).toUpperCase();
        consoleScreen.innerHTML = `<div style="display:flex;flex-direction:column;align-items:center;justify-content:center;height:100%;"><span style="font-size:5rem;color:var(--pure-white);">${i}</span><span style="margin-top:10px;font-size:1.2rem;color:var(--pure-white);">${name}</span></div>`;
    }
}
consoleScreen.addEventListener('click', () => { if (consoleScreen.querySelector('.cartridge-prompt')) romUploadInput.click(); });
romUploadInput.addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    consoleScreen.innerHTML = `<div class="cartridge-prompt pulse-text">WRITING TO CHIP...</div>`;
    const name = stripExtension(file.name), box = findClosestBoxart(name, availableBoxarts);
    await window.PocketDB.insertCartridge(name, await file.arrayBuffer(), box);
    renderCartridge(name, box);
});
async function initiateBootSequence() {
    if (isBooting) return;
    isBooting = true;
    startPrompt.style.display = 'none';
    bootSound.play().catch(() => {});
    mascot.classList.add('slide-up');
    document.body.classList.add('booted');
    const saved = await window.PocketDB.getCartridge();
    setTimeout(() => {
        bootScreen.style.display = 'none';
        mainLayout.style.display = 'flex';
        if (saved) { window.GlobalRomBuffer = saved.romData; renderCartridge(saved.fileName, saved.boxartUrl); }
        setTimeout(() => { mainLayout.style.opacity = '1'; }, 50);
    }, 1500);
}
bootScreen.addEventListener('click', initiateBootSequence);
window.addEventListener('keydown', (e) => { if (e.code === 'Space' && !isBooting) initiateBootSequence(); });
initializeLibrary();

const sBtn = document.getElementById('settings-btn'), sModalSet = document.getElementById('settings-modal'), sClose = document.getElementById('close-settings');
if (sBtn && sModalSet && sClose) {
    sBtn.addEventListener('click', () => sModalSet.style.display = 'flex');
    sClose.addEventListener('click', () => sModalSet.style.display = 'none');
}
let volumes = JSON.parse(localStorage.getItem('pocketVols')) || { ch1: 1.0, ch2: 1.0, ch3: 1.0, ch4: 1.0 };
function updateAudioMixer() {
    localStorage.setItem('pocketVols', JSON.stringify(volumes));
    window.PocketEngine?.isEngineRunning && window.PocketEngine.setChannelVolumes(volumes.ch1, volumes.ch2, volumes.ch3, volumes.ch4);
}
['ch1', 'ch2', 'ch3', 'ch4'].forEach(ch => {
    const s = document.getElementById(`vol-${ch}`);
    if (s) { s.value = volumes[ch]; s.addEventListener('input', (e) => { volumes[ch] = parseFloat(e.target.value); updateAudioMixer(); }); }
});

let currentTheme = localStorage.getItem('pocketTheme') || 'Default (Standard)';
let customColors = JSON.parse(localStorage.getItem('pocketCustomColors')) || ["#ffffff", "#aaaaaa", "#555555", "#000000"];
const themes = { "Default (Standard)": ["#d1cb95", "#40985e", "#1a644e", "#04373b"], "Classic": ["#9aa13c", "#6c712a", "#4d511e", "#1f200c"], "Fizzle": ["#cee5ff", "#c589dc", "#564991", "#1e182a"], "Ice cream": ["#fff6d3", "#f9a875", "#eb6b6f", "#7c3f58"], "Hollow": ["#fafbf6", "#c6b7be", "#565a75", "#0f0f1b"], "Rustic": ["#edb4a1", "#a96868", "#764462", "#2c2137"], "Mint": ["#c4f0c2", "#5ab9a8", "#1e606e", "#2d1b00"], "SpaceHaze": ["#f8e3c4", "#cc3495", "#6b1fb1", "#0b0630"], "Fiery Plague": ["#713141", "#512839", "#312137", "#1a2129"], "Gold": ["#cfab51", "#9d654c", "#4d222c", "#210b1b"], "Honey": ["#e9f5da", "#f0b695", "#877286", "#3e3a42"], "Coral": ["#ffd0a4", "#f4949c", "#7c9aac", "#68518a"], "Rabbit": ["#f1e0cd", "#ffa49a", "#da3467", "#35333f"], "Caramel autumn": ["#fff4b8", "#ff8b40", "#a22fc9", "#290143"], "Snow flake": ["#e7edeb", "#8ecece", "#62a1c7", "#3f6ecc"], "Lemon and Lime": ["#fff37b", "#5fcc86", "#39809c", "#28375b"], "Kirokaze": ["#e2f3e4", "#94e344", "#46878f", "#332c50"], "Red is dead": ["#fffcfe", "#ff0015", "#860020", "#11070a"] };

function applyColorsToEngine(c, name) {
    if (name) localStorage.setItem('pocketTheme', name);
    if (window.PocketEngine?.isEngineRunning) {
        const h = c.map(col => parseInt(col.replace('#', '') + 'FF', 16));
        window.PocketEngine.setPalette(...h);
    }
}
const themeList = document.getElementById('theme-list');
if (themeList) {
    Object.keys(themes).forEach(name => {
        const c = themes[name], btn = document.createElement('button');
        btn.className = 'theme-btn';
        btn.innerHTML = `<div class="theme-title">${name}</div><div class="swatch-container">${c.map(col => `<div class="swatch" style="background-color:${col}"></div>`).join('')}</div>`;
        btn.addEventListener('click', () => applyColorsToEngine(c, name));
        themeList.appendChild(btn);
    });
}
for (let i = 0; i < 4; i++) {
    const customInput = document.getElementById(`custom-c${i}`);
    if (customInput) customInput.value = customColors[i];
}
document.getElementById('btn-apply-custom')?.addEventListener('click', () => {
    customColors = [0, 1, 2, 3].map(i => document.getElementById(`custom-c${i}`)?.value || "#000000");
    localStorage.setItem('pocketCustomColors', JSON.stringify(customColors));
    applyColorsToEngine(customColors, 'CUSTOM');
});
window.applySavedSettingsToEngine = function() {
    updateAudioMixer();
    if (currentTheme === 'CUSTOM') applyColorsToEngine(customColors, 'CUSTOM');
    else if (themes[currentTheme]) applyColorsToEngine(themes[currentTheme], currentTheme);
};

function markSocialInteracted() { localStorage.setItem('pocketNagInteracted', 'true'); document.getElementById('nag-modal') ? document.getElementById('nag-modal').style.display = 'none' : null; }
['github-btn', 'playstore-btn', 'nag-github-btn', 'nag-playstore-btn'].forEach(id => document.getElementById(id)?.addEventListener('click', () => { window.open(id.includes('github') ? "https://github.com/Asher-Ul-Haque/Pocket-Pixel" : "https://play.google.com/store/apps/details?id=just.somebody.templates", '_blank'); markSocialInteracted(); }));
document.getElementById('nag-later-btn')?.addEventListener('click', () => document.getElementById('nag-modal').style.display = 'none');
window.checkNagModal = function() {
    if (localStorage.getItem('pocketNagInteracted') === 'true') return;
    let count = (parseInt(localStorage.getItem('pocketPlayCount')) || 0) + 1;
    localStorage.setItem('pocketPlayCount', count);
    if ([6, 11, 51, 101, 501, 1001].includes(count) || count % 1000 === 0) setTimeout(() => { const nag = document.getElementById('nag-modal'); if(nag) nag.style.display = 'flex'; }, 1500);
};

const ssModal = document.getElementById('savestate-modal'), ssClose = document.getElementById('close-savestate'), ssGrid = document.getElementById('state-grid');
ssClose?.addEventListener('click', () => ssModal.style.display = 'none');
window.openSaveStateModal = async function(mode, preImg = null) {
    const title = document.getElementById('savestate-title');
    if(title) title.innerText = mode === 'SAVE' ? "SAVE STATE" : "LOAD STATE";
    if(ssGrid) {
        ssGrid.innerHTML = '';
        const cart = await window.PocketDB.getCartridge(), states = cart?.states || Array(5).fill(null);
        states.forEach((s, i) => {
            const btn = document.createElement('div');
            btn.className = 'state-slot';
            btn.innerHTML = `<div class="state-image-box">${s?.screenshot ? `<img src="${s.screenshot}">` : '<span>EMPTY</span>'}</div><div class="state-info">SLOT ${i+1}<span class="state-timestamp">${s ? new Date(s.timestamp).toLocaleString() : 'No Data'}</span></div>`;
            btn.addEventListener('click', async () => {
                if (mode === 'SAVE') {
                    const b = window.PocketEngine.saveState();
                    if (b) { await window.PocketDB.saveSlot(i, b, preImg); ssModal.style.display = 'none'; }
                } else if (mode === 'LOAD' && s?.buffer) { window.PocketEngine.loadState(s.buffer); ssModal.style.display = 'none'; }
            });
            ssGrid.appendChild(btn);
        });
        if(ssModal) ssModal.style.display = 'flex';
    }
};