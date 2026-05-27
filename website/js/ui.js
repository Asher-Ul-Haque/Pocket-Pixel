// --- 1. Global State ---
let availableBoxarts = [];
let userGames = []; // Will store FileHandles

// --- 2. Startup: Load the Pre-computed Boxarts ---
async function initializeLibrary() {
    try {
        const response = await fetch('assets/boxarts.json');
        availableBoxarts = await response.json();
        console.log(`Loaded ${availableBoxarts.length} box art URLs from cache.`);
    } catch (error) {
        console.error("Failed to load boxarts.json. Make sure it is in the assets folder.", error);
    }
}

// --- 3. The Math: JS Translation of your Kotlin Logic ---
function normalize(name) {
    return name.toLowerCase().replace(/[^a-z0-9 ]/g, " ").replace(/\s+/g, " ").trim();
}

function stripExtension(name) {
    return name.replace(/\.[a-z0-9]{1,5}$/i, "");
}

function editDistance(a, b) {
    const m = a.length;
    const n = b.length;
    const dp = Array.from({ length: m + 1 }, () => Array(n + 1).fill(0));

    for (let i = 0; i <= m; i++) dp[i][0] = i;
    for (let j = 0; j <= n; j++) dp[0][j] = j;

    for (let i = 1; i <= m; i++) {
        for (let j = 1; j <= n; j++) {
            const cost = a[i - 1] === b[j - 1] ? 0 : 1;
            dp[i][j] = Math.min(
                dp[i - 1][j] + 1,      // Deletion
                dp[i][j - 1] + 1,      // Insertion
                dp[i - 1][j - 1] + cost // Substitution
            );
        }
    }
    return dp[m][n];
}

function findClosestBoxart(targetName, candidates, threshold = 0.65) {
    const normalizedTarget = normalize(targetName);
    let bestMatchUrl = null;
    let bestScore = threshold;

    for (const url of candidates) {
        // Extract just the filename from the long URL for comparison
        const fileNameWithExt = url.split('/').pop();
        const rawName = decodeURIComponent(fileNameWithExt); // Decode %20 to spaces
        const normalizedCandidate = normalize(stripExtension(rawName));
        
        // Calculate similarity (1.0 is exact match, 0.0 is nothing alike)
        const distance = editDistance(normalizedCandidate, normalizedTarget);
        const maxLength = Math.max(normalizedCandidate.length, normalizedTarget.length);
        const similarity = maxLength === 0 ? 1.0 : (1.0 - (distance / maxLength));

        if (similarity >= bestScore) {
            bestMatchUrl = url;
            bestScore = similarity;
        }
    }
    return bestMatchUrl;
}

// --- 4. The UI Builder ---
function createGameCard(fileHandle) {
    const cleanName = stripExtension(fileHandle.name);
    
    // Create the container
    const card = document.createElement('div');
    card.style.width = '120px';
    card.style.cursor = 'pointer';
    card.style.display = 'flex';
    card.style.flexDirection = 'column';
    card.style.alignItems = 'center';
    card.style.transition = 'transform 0.1s';
    
    // Hover effect
    card.addEventListener('mouseenter', () => card.style.transform = 'scale(1.05)');
    card.addEventListener('mouseleave', () => card.style.transform = 'scale(1)');

    // Find the boxart!
    const boxartUrl = findClosestBoxart(cleanName, availableBoxarts);

    const imgContainer = document.createElement('div');
    imgContainer.style.width = '120px';
    imgContainer.style.height = '120px';
    imgContainer.style.backgroundColor = 'var(--medium-green)';
    imgContainer.style.border = '2px solid var(--green)';
    imgContainer.style.display = 'flex';
    imgContainer.style.justifyContent = 'center';
    imgContainer.style.alignItems = 'center';
    imgContainer.style.marginBottom = '5px';

    if (boxartUrl) {
        // We found an image! Let the browser load it via the URL.
        const img = document.createElement('img');
        img.src = boxartUrl;
        img.style.width = '100%';
        img.style.height = '100%';
        img.style.objectFit = 'cover';
        img.style.imageRendering = 'pixelated'; // Keep it crispy
        imgContainer.appendChild(img);
    } else {
        // Fallback: The Initials Design
        // Grab up to the first 2 letters of the name, uppercase them
        const initials = cleanName.substring(0, 2).toUpperCase();
        imgContainer.innerHTML = `<span style="font-size: 3rem; color: var(--dark-green);">${initials}</span>`;
    }

    // Title label
    const title = document.createElement('div');
    title.textContent = cleanName;
    title.style.fontSize = '0.8rem';
    title.style.textAlign = 'center';
    title.style.whiteSpace = 'nowrap';
    title.style.overflow = 'hidden';
    title.style.textOverflow = 'ellipsis';
    title.style.width = '100%';

    card.appendChild(imgContainer);
    card.appendChild(title);

    // TODO Phase 3: Click to boot game
    card.addEventListener('click', () => {
        console.log("Preparing to boot:", fileHandle.name);
    });

    return card;
}

// --- 5. The File System Scanner ---
async function scanDirectory() {
    if (!window.showDirectoryPicker) {
        alert("Your browser doesn't support the File System Access API. Please use Chrome, Edge, or Opera.");
        return;
    }

    try {
        // Ask user for the folder
        const dirHandle = await window.showDirectoryPicker();
        userGames = [];

        // Iterate through all files in the folder
        for await (const entry of dirHandle.values()) {
            if (entry.kind === 'file') {
                const name = entry.name.toLowerCase();
                if (name.endsWith('.gb') || name.endsWith('.gbc')) {
                    userGames.push(entry);
                }
            }
        }

        // Render the UI
        const grid = document.getElementById('game-grid');
        grid.innerHTML = ''; // Clear previous

        if (userGames.length === 0) {
            grid.innerHTML = '<div class="placeholder-text">No .gb or .gbc files found in that folder.</div>';
            return;
        }

        // Create a card for each game
        userGames.forEach(handle => {
            const card = createGameCard(handle);
            grid.appendChild(card);
        });

    } catch (error) {
        console.log("User cancelled folder selection or an error occurred:", error);
    }
}

// --- 6. Event Listeners ---
document.getElementById('scan-btn').addEventListener('click', scanDirectory);

// Boot up the database when the script loads
initializeLibrary();
