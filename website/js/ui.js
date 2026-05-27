// --- DOM Elements ---
const bootScreen = document.getElementById('boot-screen');
const startPrompt = document.getElementById('start-prompt');
const mascot = document.getElementById('mascot');
const bootSound = document.getElementById('startup-sound');
const mainLayout = document.getElementById('main-layout');

let isBooting = false;

// --- Boot Sequence Logic ---
function initiateBootSequence() {
    if (isBooting) return;
    isBooting = true;

    // 1. Instantly hide the "Click" prompt
    startPrompt.style.display = 'none';

    // 2. Play the sound (Browser allows this because of the user click/keypress)
    bootSound.play().catch(e => console.warn("Audio blocked by browser", e));

    // 3. Trigger the mascot slide-up animation
    mascot.classList.add('slide-up');

    // 4. Fade the body background to Dark Green
    document.body.classList.add('booted');

    // 5. Wait for the mascot animation to finish, then swap the layouts
    // The CSS transition is 1.8s, so we wait roughly 1.5s to start fading in the UI
    setTimeout(() => {
        bootScreen.style.display = 'none';
        mainLayout.style.display = 'flex';
        
        // Slight delay before fading in to ensure display:flex is registered
        setTimeout(() => {
            mainLayout.style.opacity = '1';
        }, 50);

    }, 1500); 
}

// --- Event Listeners ---
// Listen for mouse click anywhere on the boot screen
bootScreen.addEventListener('click', initiateBootSequence);

// Listen for Spacebar
window.addEventListener('keydown', (event) => {
    if (event.code === 'Space' && !isBooting) {
        initiateBootSequence();
    }
});
