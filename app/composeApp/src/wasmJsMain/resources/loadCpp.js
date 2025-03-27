Module.onRuntimeInitialized = () => {
    // Export the C++ function to work with an ArrayBuffer
    window.getNativeFrame = (buffer) => {
        console.log("I dont know js");
/*        const byteOffset = buffer.byteOffset || 0; // Get the start offset of the ArrayBuffer
        const size = buffer.byteLength;           // Get the length of the ArrayBuffer
        const ptr = Module.HEAPU8.subarray(byteOffset, byteOffset + size);*/

        // Call the C++ function
        //Module.ccall('getFrame', null, ['number', 'number'], [ptr.byteOffset, size]);
    };

    // Example C++ function that returns a string
    window.helloFromCpp = () => Module.ccall('helloFromCpp', 'string');
};
