window.PocketEngine = {
    isEngineRunning: false,

    boot: async function(romBuffer) {
        if (this.isEngineRunning) return;

        console.log("Preparing WebAssembly Environment...");

        window.Module = {
            noInitialRun: true, 
            arguments: ['/game.rom'], 
            
            // CRITICAL FIX: Tell Emscripten to map to "canvas"
            canvas: document.getElementById('canvas'), 
            
            print: (text) => console.log("[C++] " + text),
            printErr: (text) => console.error("[C++ ERROR] " + text),
            
            onRuntimeInitialized: () => {
                console.log("WASM Runtime Initialized. Injecting ROM...");

                const romArray = new Uint8Array(romBuffer);
                FS.writeFile('/game.rom', romArray);

                console.log("Starting C++ Execution Loop...");
                Module.callMain(Module.arguments);
                this.isEngineRunning = true;
            }
        };

        const script = document.createElement('script');
        // Matches the file name from your terminal logs!
        script.src = 'wasm/pixel_pocket.js'; 
        document.body.appendChild(script);
    },

    setPalette: function(hex0, hex1, hex2, hex3) {
        if (!this.isEngineRunning) return;
        Module.ccall('webSetPalette', 'null', 
            ['number', 'number', 'number', 'number'], 
            [hex0, hex1, hex2, hex3]
        );
    },

    setChannelVolumes: function(ch1, ch2, ch3, ch4) {
        if (!this.isEngineRunning) return;
        Module.ccall('webSetChannelVolumes', 'null', 
            ['number', 'number', 'number', 'number'], 
            [ch1, ch2, ch3, ch4]
        );
    },

    // --- Save States ---
    saveState: function() {
        if (!this.isEngineRunning) return null;

        // 1. Allocate 4 bytes in WASM to hold the outgoing size integer
        const sizePtr = Module.ccall('webAllocate', 'number', ['number'], [4]);
        
        // 2. Call your C function (it returns the buffer pointer)
        const bufferPtr = Module.ccall('webSaveState', 'number', ['number'], [sizePtr]);
        
        if (bufferPtr === 0) {
            Module.ccall('webFree', 'null', ['number'], [sizePtr]);
            return null;
        }

        // 3. Read the 32-bit integer size out of the pointer
        const size = Module.HEAPU32[sizePtr >> 2];

        // 4. Create a JS view of that WASM memory chunk and clone it
        const wasmView = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, size);
        const jsClone = new Uint8Array(wasmView); // Clone so it survives C free()

        // 5. Clean up the C memory so we don't leak RAM!
        Module.ccall('webFree', 'null', ['number'], [bufferPtr]);
        Module.ccall('webFree', 'null', ['number'], [sizePtr]);

        return jsClone.buffer; // Return raw ArrayBuffer
    },

    loadState: function(arrayBuffer) {
        if (!this.isEngineRunning || !arrayBuffer) return false;

        const jsData = new Uint8Array(arrayBuffer);
        const size = jsData.length;

        // 1. Allocate a chunk of memory in C to hold the incoming JS array
        const bufferPtr = Module.ccall('webAllocate', 'number', ['number'], [size]);

        // 2. Copy the JS data into the C heap
        Module.HEAPU8.set(jsData, bufferPtr);

        // 3. Call your C function to unpack it
        const success = Module.ccall('webLoadState', 'boolean', ['number', 'number'], [bufferPtr, size]);

        // 4. Clean up the incoming buffer
        Module.ccall('webFree', 'null', ['number'], [bufferPtr]);

        return success;
    }
};
