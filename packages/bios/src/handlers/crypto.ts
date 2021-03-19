import crypto from "isomorphic-webcrypto";
import forge from "node-forge";

import { throwError } from "../error";
import { MemoryHandler } from "./memory";
import { generateKey, KeyType } from "../crypto";

export class CryptoHandler {
    memoryHandler: MemoryHandler;

    constructor(memoryHandler: MemoryHandler) {
        this.memoryHandler = memoryHandler;
    }

    createKey = async (
        keyType: KeyType,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const keyPair = generateKey(keyType);
            const createdKey = this.memoryHandler.addObj(keyPair.publicKeyData);
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
            const blob = this.memoryHandler.uint8Array(blobIndex, blobLen);

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

    hash256Sync = (blobIndex: number, blobLen: number) => {
        try {
            const buffer = this.memoryHandler.uint8Array(blobIndex, blobLen);
            const bytes = forge.util.binary.raw.encode(buffer);

            // todo: benchmark forge for sync hash
            const md = forge.md.sha256.create();
            md.update(bytes);
            const hashDigest = md.digest();
            const hashBytes = forge.util.binary.raw.decode(hashDigest.bytes());

            return this.memoryHandler.addObj(hashBytes);
        } catch (e) {
            throwError(e);
        }
    };

    imports = {
        createKey: this.createKey.bind(this),
        hash256: this.hash256.bind(this),
        hash256Sync: this.hash256Sync.bind(this),
    };
}
