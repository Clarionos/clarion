import { Context } from "@clarionos/bios";

import { ClarionDb } from "./db";

const wasmFilePath = "/clarion.wasm";
let context: Context;

export const getContext = async (): Promise<Context> => {
    if (!context) {
        console.info("Initializing Clarion Context...");
        const response = await fetch(wasmFilePath);
        const wasmBytes = new Uint8Array(await response.arrayBuffer());

        const clarionDb = new ClarionDb();
        context = new Context(["clarion.wasm"], clarionDb as any);
        await context.instanciate(wasmBytes);
    }
    return context;
};
