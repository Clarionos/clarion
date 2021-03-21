import { throwError } from "../error";
import { MemoryHandler } from "./memory";
import {
    randomAesCbIv,
    deriveEcdhSharedSecret,
    generateKey,
    getKeyPair,
    KeyType,
    publicKeyPairFromData,
    sha256,
    sha256Sync,
    sign,
    aesCbcEncrypt,
    aesCbcDecrypt,
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

    diffieHellman = async (
        keyType: KeyType,
        localPublicKeyIndex: number,
        localPublicKeyLen: number,
        remotePublicKeyIndex: number,
        remotePublicKeyLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const localPublicKeyBytes = this.memoryHandler.uint8Array(
                localPublicKeyIndex,
                localPublicKeyLen
            );
            const localKeyPair = getKeyPair(localPublicKeyBytes);

            const remotePublicKeyBytes = this.memoryHandler.uint8Array(
                remotePublicKeyIndex,
                remotePublicKeyLen
            );
            const remoteKeyPair = publicKeyPairFromData(
                keyType,
                remotePublicKeyBytes
            );

            const ecdhSharedSecret = deriveEcdhSharedSecret(
                localKeyPair,
                remoteKeyPair
            );
            console.info("generated ecdh shared secret", ecdhSharedSecret);
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(ecdhSharedSecret)
            );
        } catch (e) {
            throwError(e);
        }
    };

    randomAesCbIv = () => {
        try {
            const iv = randomAesCbIv();
            return this.memoryHandler.addObj(iv);
        } catch (e) {
            throwError(e);
        }
    };

    aesCbcEncrypt = async (
        sharedSecretIndex: number,
        sharedSecretLen: number,
        ivIndex: number,
        ivLen: number,
        blobIndex: number,
        blobLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const sharedSecret = this.memoryHandler.uint8Array(
                sharedSecretIndex,
                sharedSecretLen
            );

            const iv = this.memoryHandler.uint8Array(ivIndex, ivLen);

            const blob = this.memoryHandler.uint8Array(blobIndex, blobLen);

            const encryptedMessage = await aesCbcEncrypt(
                sharedSecret,
                iv,
                blob
            );
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(encryptedMessage)
            );
        } catch (e) {
            throwError(e);
        }
    };

    aesCbcDecrypt = async (
        sharedSecretIndex: number,
        sharedSecretLen: number,
        ivIndex: number,
        ivLen: number,
        encryptedBlobIndex: number,
        encryptedBlobLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const sharedSecret = this.memoryHandler.uint8Array(
                sharedSecretIndex,
                sharedSecretLen
            );

            const iv = this.memoryHandler.uint8Array(ivIndex, ivLen);

            const encryptedBlob = this.memoryHandler.uint8Array(
                encryptedBlobIndex,
                encryptedBlobLen
            );

            const decryptedMessage = await aesCbcDecrypt(
                sharedSecret,
                iv,
                encryptedBlob
            );
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(decryptedMessage)
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
        diffieHellman: this.diffieHellman.bind(this),
        randomAesCbIv: this.randomAesCbIv.bind(this),
        aesCbcEncrypt: this.aesCbcEncrypt.bind(this),
        aesCbcDecrypt: this.aesCbcDecrypt.bind(this),
    };
}
