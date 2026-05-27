let availableBoxarts = [];

const bootScreen = document.getElementById('boot-screen');
const startPrompt = document.getElementById('start-prompt');
const mascot = document.getElementById('mascot');
const bootSound = document.getElementById('startup-sound');
const mainLayout = document.getElementById('main-layout');
const consoleScreen = document.querySelector('.console-screen');
const romUploadInput = document.getElementById('rom-upload');

let isBooting = false;

// --- 1. Startup: Load the Pre-computed Boxarts ---
async function initializeLibrary() {
    try {
        const response = await fetch('assets/boxarts.json');
        availableBoxarts = await response.json();
    } catch (error) {
        console.error("Failed to load boxarts.json. Search will fallback to initials.", error);
    }
}

// --- 2. Fuzzy Finder Math ---
function normalize(name) {
    return name.toLowerCase().replace(/[^a-z0-9 ]/g, " ").replace(/\s+/g, " ").trim();
}

function stripExtension(name) {
    return name.replace(/\.[a-z0-9]{1,5}$/i, "");
}

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
    let bestMatchUrl = null;
    let bestScore = threshold;

    for (const url of candidates) {
        const fileNameWithExt = url.split('/').pop();
        const rawName = decodeURIComponent(fileNameWithExt); 
        const normalizedCandidate = normalize(stripExtension(rawName));
        
        const distance = editDistance(normalizedCandidate, normalizedTarget);
        const MathMax = Math.max(normalizedCandidate.length, normalizedTarget.length);
        const similarity = MathMax === 0 ? 1.0 : (1.0 - (distance / MathMax));

        if (similarity >= bestScore) {
            bestMatchUrl = url;
            bestScore = similarity;
        }
    }
    return bestMatchUrl;
}

// --- 3. UI Updater ---
function renderCartridge(fileName, boxartUrl) {
    if (boxartUrl) {
        consoleScreen.innerHTML = `<img src="${boxartUrl}" alt="${fileName}" style="width: 100%; height: 100%; object-fit: contain; image-rendering: pixelated;">`;
    } else {
        const initials = fileName.substring(0, 2).toUpperCase();
        consoleScreen.innerHTML = `
            <div style="display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100%;">
                <span style="font-size: 5rem; color: var(--pure-white);">${initials}</span>
                <span style="margin-top: 10px; font-size: 1.2rem; color: var(--pure-white);">${fileName}</span>
            </div>
        `;
    }
}

// --- 4. File Upload Handler ---
consoleScreen.addEventListener('click', () => {
    if (consoleScreen.querySelector('.cartridge-prompt')) {
        romUploadInput.click();
    }
});

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

// --- 5. Boot Sequence Logic ---
async function initiateBootSequence() {
    if (isBooting) return;
    isBooting = true;

    startPrompt.style.display = 'none';
    bootSound.play().catch(e => console.warn("Audio blocked by browser", e));
    mascot.classList.add('slide-up');
    document.body.classList.add('booted');

    const savedCartridge = await window.PocketDB.getCartridge();

    setTimeout(() => {
        bootScreen.style.display = 'none';
        mainLayout.style.display = 'flex';
        
        if (savedCartridge) {
            renderCartridge(savedCartridge.fileName, savedCartridge.boxartUrl);
        }

        setTimeout(() => {
            mainLayout.style.opacity = '1';
        }, 50);

    }, 1500); 
}

bootScreen.addEventListener('click', initiateBootSequence);
window.addEventListener('keydown', (event) => {
    if (event.code === 'Space' && !isBooting) initiateBootSequence();
});

initializeLibrary();
