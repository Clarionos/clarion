import { Context } from "../../libraries/clwasmer/src/index.ts";
import level from "level";

const wasmFilePath = "/clarion.wasm";

const init = async () => {
    const response = await fetch(wasmFilePath);
    const wasmBytes = new Uint8Array(await response.arrayBuffer());

    const clarionDb = {
        open(name: string, callback: Function) {
            console.info("initializing level... ", name);
            level(
                name,
                { keyEncoding: "binary", valueEncoding: "binary" },
                callback
            );
        },
    };

    const context = new Context(["clarion.wasm"], clarionDb as any);
    await context.instanciate(wasmBytes);
    (context.instance!.exports._start as Function)();
};

init();
