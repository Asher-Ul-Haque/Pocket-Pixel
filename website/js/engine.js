window.PocketEngine = {
    isEngineRunning: false,

    boot: async function(romBuffer) {
        if (this.isEngineRunning) return;
        console.log("Preparing WebAssembly Environment...");

        window.Module = window.Module || {};
        window.Module.noInitialRun = true; 
        window.Module.arguments = ['/game.rom']; 
        window.Module.canvas = document.getElementById('canvas'); 
        
        window.Module.print = (text) => console.log("[C++] " + text);
        window.Module.printErr = (text) => console.error("[C++ ERROR] " + text);
        
        window.Module.onRuntimeInitialized = () => {
            console.log("WASM Runtime Initialized. Injecting ROM...");
            const romArray = new Uint8Array(romBuffer);
            FS.writeFile('/game.rom', romArray);
            console.log("Starting C++ Execution Loop...");
            Module.callMain(Module.arguments);
            this.isEngineRunning = true;
        };

        const script = document.createElement('script');
        script.src = 'wasm/pixel_pocket.js'; 
        document.body.appendChild(script);
    },

    setPalette: function(hex0, hex1, hex2, hex3) {
        if (!this.isEngineRunning) return;
        Module.ccall('webSetPalette', 'null', ['number', 'number', 'number', 'number'], [hex0, hex1, hex2, hex3]);
    },

    setChannelVolumes: function(ch1, ch2, ch3, ch4) {
        if (!this.isEngineRunning) return;
        Module.ccall('webSetChannelVolumes', 'null', ['number', 'number', 'number', 'number'], [ch1, ch2, ch3, ch4]);
    },

    injectKey: function(scancode, isDown) {
        if (!this.isEngineRunning) return;
        Module.ccall('webInjectKey', 'null', ['number', 'boolean'], [scancode, isDown]);
    },

    setPaused: function(paused) {
        if (!this.isEngineRunning) return;
        // Pass the boolean state as a 1 or 0 numeric flag
        Module.ccall('webSetPaused', 'null', ['number'], [paused ? 1 : 0]);
    },

    isPaused: function() {
        if (!this.isEngineRunning) return false;
        // Read the returned numeric integer and cast it back to a true JS boolean
        return !!Module.ccall('webIsPaused', 'number', [], []);
    },

    saveState: function() {
        if (!this.isEngineRunning) return null;
        const sizePtr = Module.ccall('webAllocate', 'number', ['number'], [4]);
        const bufferPtr = Module.ccall('webSaveState', 'number', ['number'], [sizePtr]);
        
        if (bufferPtr === 0) {
            Module.ccall('webFree', 'null', ['number'], [sizePtr]);
            return null;
        }

        const size = Module.HEAPU32[sizePtr >> 2];
        const wasmView = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, size);
        const jsClone = new Uint8Array(wasmView); 

        Module.ccall('webFree', 'null', ['number'], [bufferPtr]);
        Module.ccall('webFree', 'null', ['number'], [sizePtr]);
        return jsClone.buffer; 
    },

    loadState: function(arrayBuffer) {
        if (!this.isEngineRunning || !arrayBuffer) return false;
        const jsData = new Uint8Array(arrayBuffer);
        const size = jsData.length;
        const bufferPtr = Module.ccall('webAllocate', 'number', ['number'], [size]);
        Module.HEAPU8.set(jsData, bufferPtr);
        const success = Module.ccall('webLoadState', 'boolean', ['number', 'number'], [bufferPtr, size]);
        Module.ccall('webFree', 'null', ['number'], [bufferPtr]);
        return success;
    },

    captureScreenshot: function() {
        if (!this.isEngineRunning) return null;

        const bufferPtr = Module.ccall('webCaptureFrameBuffer', 'number', [], []);
        if (bufferPtr === 0) return null;

        const size = 160 * 144 * 4; 
        const view = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, size);

        const offscreenCanvas = document.createElement('canvas');
        offscreenCanvas.width = 160;
        offscreenCanvas.height = 144;
        const ctx = offscreenCanvas.getContext('2d');
        const imgData = ctx.createImageData(160, 144);

        // SWIZZLE LAYER: Rearrange the incoming ABGR raw heap bytes into Canvas RGBA format
        for (let i = 0; i < size; i += 4) {
            let r = view[i + 0];
            let g = view[i + 1];
            let b = view[i + 2];
            let a = view[i + 3];

            // Re-order the flipped color channels explicitly to match Canvas primitives
            imgData.data[i + 0] = b; // True Red channel maps out of index 2
            imgData.data[i + 1] = g; // Green channel stays consistent at index 1
            imgData.data[i + 2] = r; // True Blue channel maps out of index 0
            imgData.data[i + 3] = a; // Alpha channel stays solid at index 3
        }

        ctx.putImageData(imgData, 0, 0);

        const dataUrl = offscreenCanvas.toDataURL('image/jpeg', 0.85);
        Module.ccall('webFree', 'null', ['number'], [bufferPtr]);

        return dataUrl;
    }
};
