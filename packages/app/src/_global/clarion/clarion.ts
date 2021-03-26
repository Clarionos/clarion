import { useState, useEffect } from "react";
import { Context } from "@clarionos/bios";

import { ClarionDb } from "./db";
import { ClarionConnectionCreator } from "./websocket";

const wasmFilePath = "/assets/clarion.wasm";
let context: Context | undefined = undefined;

export const useClarion = () => {
    const [isClarionReady, setClarionReady] = useState(Boolean(context));
    const [clarionInitError, setClarionInitError] = useState(undefined);

    useEffect(() => {
        const initContext = async () => {
            if (!context) {
                try {
                    console.info("Initializing Clarion Context...");
                    const response = await fetch(wasmFilePath);
                    const wasmBytes = new Uint8Array(
                        await response.arrayBuffer()
                    );

                    const clarionDbManager = new ClarionDb();
                    const clarionConnectionManager = new ClarionConnectionCreator();
                    context = new Context(
                        ["clarion.wasm"],
                        clarionDbManager,
                        clarionConnectionManager
                    );
                    await context.instanciate(wasmBytes);
                } catch (e) {
                    console.error("Clarion Init Context error", e);
                    setClarionInitError(e);
                }

                setClarionReady(Boolean(context));
            }
        };

        initContext();
    }, []);

    return { context, isClarionReady, clarionInitError };
};
