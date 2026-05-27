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
    }
};
