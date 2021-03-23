import { Context } from "@clarionos/bios";

import { ClarionDb } from "./db";
import { ClarionConnectionCreator } from "./websocket";

const wasmFilePath = "/assets/clarion.wasm";
let context: Context;

export const getContext = async (): Promise<Context> => {
    if (!context) {
        console.info("Initializing Clarion Context...");
        const response = await fetch(wasmFilePath);
        const wasmBytes = new Uint8Array(await response.arrayBuffer());

        const clarionDbManager = new ClarionDb();
        const clarionConnectionManager = new ClarionConnectionCreator();
        context = new Context(
            ["clarion.wasm"],
            clarionDbManager,
            clarionConnectionManager
        );
        await context.instanciate(wasmBytes);
    }
    return context;
};
