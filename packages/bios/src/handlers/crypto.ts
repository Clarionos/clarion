import { throwError } from "../error";
import { MemoryHandler } from "./memory";
import {
    generateKey,
    getKeyPair,
    KeyType,
    sha256,
    sha256Sync,
    sign,
} from "../crypto";
import { recover } from "../crypto/signature";

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
            const keyPair = await generateKey(keyType);
            const createdKey = this.memoryHandler.addObj(keyPair.publicKey);
            this.memoryHandler.wasmCallback(wasmCbIndex, wasmCbPtr, createdKey);
        } catch (e) {
            throwError(e);
        }
    };

    sha256 = async (
        blobIndex: number,
        blobLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const blob = this.memoryHandler.uint8Array(blobIndex, blobLen);
            const hashBytes = await sha256(blob);
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(hashBytes)
            );
        } catch (e) {
            throwError(e);
        }
    };

    sha256Sync = (blobIndex: number, blobLen: number) => {
        try {
            const blob = this.memoryHandler.uint8Array(blobIndex, blobLen);
            const hashBytes = sha256Sync(blob);
            return this.memoryHandler.addObj(hashBytes);
        } catch (e) {
            throwError(e);
        }
    };

    sign = async (
        publicKeyIndex: number,
        publicKeyLen: number,
        digestIndex: number,
        digestLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const publicKeyBytes = this.memoryHandler.uint8Array(
                publicKeyIndex,
                publicKeyLen
            );

            const keyPair = getKeyPair(publicKeyBytes);

            const digestBytes = this.memoryHandler.uint8Array(
                digestIndex,
                digestLen
            );

            const signedBytes = await sign(keyPair, digestBytes);

            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(signedBytes)
            );
        } catch (e) {
            throwError(e);
        }
    };

    recover = async (
        keyType: KeyType,
        signatureIndex: number,
        signatureLen: number,
        digestIndex: number,
        digestLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const signatureBytes = this.memoryHandler.uint8Array(
                signatureIndex,
                signatureLen
            );

            const digestBytes = this.memoryHandler.uint8Array(
                digestIndex,
                digestLen
            );

            const recoveredKey = await recover(
                keyType,
                signatureBytes,
                digestBytes
            );

            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(recoveredKey)
            );
        } catch (e) {
            throwError(e);
        }
    };

    imports = {
        createKey: this.createKey.bind(this),
        sha256: this.sha256.bind(this),
        sha256Sync: this.sha256Sync.bind(this),
        sign: this.sign.bind(this),
        recover: this.recover.bind(this),
    };
}
