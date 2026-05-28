const DB_NAME = 'PocketPixelDB';
const DB_VERSION = 3; // Bumped version for new schema
const STORE_NAME = 'cartridges';

window.PocketDB = {
    init: function() {
        return new Promise((resolve, reject) => {
            const request = indexedDB.open(DB_NAME, DB_VERSION);

            request.onupgradeneeded = (event) => {
                const db = event.target.result;
                if (!db.objectStoreNames.contains(STORE_NAME)) {
                    db.createObjectStore(STORE_NAME, { keyPath: 'id' });
                }
            };

            request.onsuccess = (event) => resolve(event.target.result);
            request.onerror = (event) => reject(event.target.error);
        });
    },

    insertCartridge: async function(fileName, romBuffer, boxartUrl) {
        const db = await this.init();
        return new Promise((resolve, reject) => {
            const tx = db.transaction(STORE_NAME, 'readwrite');
            const store = tx.objectStore(STORE_NAME);
            const cartridgeData = {
                id: 1, 
                fileName: fileName,
                romData: romBuffer,
                boxartUrl: boxartUrl,
                // Initialize 5 empty save slots
                states: [null, null, null, null, null] 
            };
            
            const request = store.put(cartridgeData);
            request.onsuccess = () => resolve();
            request.onerror = (e) => reject(e.target.error);
        });
    },

    getCartridge: async function() {
        const db = await this.init();
        return new Promise((resolve, reject) => {
            const tx = db.transaction(STORE_NAME, 'readonly');
            const store = tx.objectStore(STORE_NAME);
            const request = store.get(1);
            
            request.onsuccess = () => resolve(request.result || null);
            request.onerror = (e) => reject(e.target.error);
        });
    },

    // Save a specific slot (0-4) with an image and timestamp
    saveSlot: async function(index, stateBuffer, screenshotURI) {
        const db = await this.init();
        return new Promise((resolve, reject) => {
            const tx = db.transaction(STORE_NAME, 'readwrite');
            const store = tx.objectStore(STORE_NAME);
            const request = store.get(1); 
            
            request.onsuccess = () => {
                const data = request.result;
                if (data) {
                    if (!data.states) data.states = [null, null, null, null, null];
                    
                    data.states[index] = {
                        buffer: stateBuffer,
                        screenshot: screenshotURI,
                        timestamp: Date.now()
                    };
                    
                    store.put(data); 
                    resolve(true);
                } else {
                    resolve(false);
                }
            };
            request.onerror = (e) => reject(e.target.error);
        });
    },

    // Completely nuke the current game and all saves from existence
    deleteCartridge: async function() {
        const db = await this.init();
        return new Promise((resolve, reject) => {
            const tx = db.transaction(STORE_NAME, 'readwrite');
            const store = tx.objectStore(STORE_NAME);
            const request = store.delete(1);
            request.onsuccess = () => resolve();
            request.onerror = (e) => reject(e.target.error);
        });
    }
};
