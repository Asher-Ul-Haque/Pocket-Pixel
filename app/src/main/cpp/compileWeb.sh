source ~/debug/emsdk/emsdk_env.sh 
make PLATFORM=web clean all
mv bin/pixel_pocket.js ../../../../website/wasm/pixel_pocket.js
mv bin/pixel_pocket.wasm ../../../../website/wasm/pixel_pocket.wasm
