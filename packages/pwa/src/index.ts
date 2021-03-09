import { Context } from "@clarionos/bios";

import { ClarionDb } from "./db";

const wasmFilePath = "/clarion.wasm";

const init = async (): Promise<void> => {
    const response = await fetch(wasmFilePath);
    const wasmBytes = new Uint8Array(await response.arrayBuffer());

    const clarionDb = new ClarionDb();
    const context = new Context(["clarion.wasm"], clarionDb as any);

    await context.instanciate(wasmBytes);
    (context.instance!.exports._start as Function)();
};

init();
