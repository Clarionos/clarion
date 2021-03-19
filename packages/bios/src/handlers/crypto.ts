import crypto from "isomorphic-webcrypto";

import { throwError } from "../error";
import { MemoryHandler } from "./memory";

export class CryptoHandler {
    memoryHandler: MemoryHandler;

    constructor(memoryHandler: MemoryHandler) {
        this.memoryHandler = memoryHandler;
    }

    createKey = async (wasmCbPtr: number, wasmCbIndex: number) => {
        try {
            const createdKey = this.memoryHandler.addObj(
                "todo: implement createKey"
            );
            this.memoryHandler.wasmCallback(wasmCbIndex, wasmCbPtr, createdKey);
        } catch (e) {
            throwError(e);
        }
    };

    hash256 = async (
        blobIndex: number,
        blobLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const blob = new Uint8Array(
                this.memoryHandler.uint8Array(blobIndex, blobLen)
            );

            const hashBuffer = await crypto.subtle.digest("SHA-256", blob);
            const hashBytes = new Uint8Array(hashBuffer);

            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(hashBytes)
            );
        } catch (e) {
            throwError(e);
        }
    };

    imports = {
        createKey: this.createKey.bind(this),
        hash256: this.hash256.bind(this),
    };
}
