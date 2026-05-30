window.PocketNetwork = {
    ws: null,
    isConnected: false,
    backendUrl: "wss://pocket-pixel-link-cable-club.onrender.com/ws",

    modal: null, viewIdle: null, viewWaiting: null, btnDisconnect: null,
    displayCode: null, errorMsg: null,

    init: function() {
        this.modal = document.getElementById('link-modal');
        this.viewIdle = document.getElementById('link-view-idle');
        this.viewWaiting = document.getElementById('link-view-waiting');
        this.btnDisconnect = document.getElementById('link-btn-disconnect');
        this.displayCode = document.getElementById('link-display-code');
        this.errorMsg = document.getElementById('link-error-msg');

        const btnLinkClub = document.getElementById('btn-link-club');
        const closeLinkModal = document.getElementById('close-link-modal');
        const btnCreate = document.getElementById('link-btn-create');
        const btnJoin = document.getElementById('link-btn-join');
        const joinInput = document.getElementById('link-join-input');

        // Stop the emulator from stealing your keystrokes while typing the code
        if (joinInput) {
            joinInput.addEventListener('keydown', e => e.stopPropagation());
            joinInput.addEventListener('keyup', e => e.stopPropagation());
            joinInput.addEventListener('keypress', e => e.stopPropagation());
        }

        if (btnLinkClub) btnLinkClub.addEventListener('click', () => this.modal.style.display = 'flex');
        if (closeLinkModal) closeLinkModal.addEventListener('click', () => this.modal.style.display = 'none');

        if (btnCreate) {
            btnCreate.addEventListener('click', () => {
                this.errorMsg.innerText = "CONNECTING...";
                this.initSocket();
                this.ws.onopen = () => {
                    this.errorMsg.innerText = "";
                    this.ws.send(JSON.stringify({ action: "create" }));
                };
            });
        }

        if (btnJoin) {
            btnJoin.addEventListener('click', () => {
                const code = joinInput ? joinInput.value.trim() : "";
                if (!code) { 
                    this.errorMsg.innerText = "ENTER A CODE FIRST"; 
                    return; 
                }
                this.errorMsg.innerText = "CONNECTING...";
                this.initSocket();
                this.ws.onopen = () => {
                    this.errorMsg.innerText = "";
                    this.ws.send(JSON.stringify({ action: "join", code: code }));
                };
            });
        }

        if (this.btnDisconnect) {
            this.btnDisconnect.addEventListener('click', () => {
                if (this.ws) this.ws.send(JSON.stringify({ action: "disconnect" }));
                this.setUIState('IDLE');
            });
        }
    },

    setUIState: function(state) {
        this.errorMsg.innerText = "";
        if (state === 'IDLE') {
            this.viewIdle.style.display = 'block';
            this.viewWaiting.style.display = 'none';
            this.btnDisconnect.style.display = 'none';
            this.isConnected = false;
            if (this.ws) { 
                this.ws.close(); 
                this.ws = null; 
            }
        } 
        else if (state === 'WAITING') {
            this.viewIdle.style.display = 'none';
            this.viewWaiting.style.display = 'block';
            this.btnDisconnect.style.display = 'block';
        } 
        else if (state === 'CONNECTED') {
            this.viewIdle.style.display = 'none';
            this.viewWaiting.style.display = 'none';
            this.btnDisconnect.style.display = 'block';
            this.isConnected = true;
            setTimeout(() => { this.modal.style.display = 'none'; }, 1000); 
        }
    },

    initSocket: function() {
        if (this.ws) {
            if (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CONNECTING) {
                this.ws.close();
            }
            this.ws = null;
        }

        console.log("[POCKET NETWORK] Dialing:", this.backendUrl);
        this.ws = new WebSocket(this.backendUrl);
        this.ws.binaryType = "arraybuffer"; 

        this.ws.onmessage = (event) => {
            if (typeof event.data === "string") {
                const data = JSON.parse(event.data);
                if (data.type === "created") {
                    this.displayCode.innerText = data.code;
                    this.setUIState('WAITING');
                } else if (data.type === "connected") {
                    this.setUIState('CONNECTED');
                } else if (data.type === "error") {
                    this.setUIState('IDLE');
                    this.errorMsg.innerText = data.message;
                } else if (data.type === "peer_disconnected") {
                    this.setUIState('IDLE');
                    this.errorMsg.innerText = "The other player unplugged the cable.";
                    if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
                        Module.ccall('webCompleteSerialTransfer', 'null', ['number'], [0xFF]);
                    }
                }
            } 
            else if (event.data instanceof ArrayBuffer) {
                const view = new Uint8Array(event.data);
                if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
                    Module.ccall('webCompleteSerialTransfer', 'null', ['number'], [view[0]]);
                }
            }
        };

        this.ws.onerror = (err) => {
            console.error("[POCKET NETWORK] WebSocket Error:", err);
            this.errorMsg.innerText = "CONNECTION ERROR.";
        };
        
        this.ws.onclose = () => {
            if (this.isConnected || this.viewWaiting.style.display === 'block') {
                this.setUIState('IDLE');
            }
        };
    },

    sendByte: function(outgoingByte, isMaster) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN && this.isConnected) {
            const payload = new Uint8Array([outgoingByte]);
            this.ws.send(payload);
        } else {
            if (window.PocketEngine && window.PocketEngine.isEngineRunning) {
                Module.ccall('webCompleteSerialTransfer', 'null', ['number'], [0xFF]);
            }
        }
    }
};

document.addEventListener('DOMContentLoaded', () => window.PocketNetwork.init());
