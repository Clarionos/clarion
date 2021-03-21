import crypto from "isomorphic-webcrypto";

const ALGORITHM = "AES-CBC";

export const randomAesCbIv = () => crypto.getRandomValues(new Uint8Array(16));

export const aesCbcEncrypt = async (
    sharedSecret: Uint8Array,
    iv: Uint8Array,
    blob: Uint8Array
): Promise<Uint8Array> => {
    const encryptionKey = await crypto.subtle.importKey(
        "raw",
        sharedSecret,
        ALGORITHM,
        false,
        ["encrypt"]
    );

    const encryptedMessage = await crypto.subtle.encrypt(
        { name: ALGORITHM, iv },
        encryptionKey,
        blob
    );

    return new Uint8Array(encryptedMessage);
};

export const aesCbcDecrypt = async (
    sharedSecret: Uint8Array,
    iv: Uint8Array,
    encryptedBlob: Uint8Array
): Promise<Uint8Array> => {
    const decryptionKey = await crypto.subtle.importKey(
        "raw",
        sharedSecret,
        ALGORITHM,
        false,
        ["decrypt"]
    );

    const decryptedMessage = await crypto.subtle.decrypt(
        { name: ALGORITHM, iv },
        decryptionKey,
        encryptedBlob
    );

    return new Uint8Array(decryptedMessage);
};
