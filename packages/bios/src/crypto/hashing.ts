import crypto from "isomorphic-webcrypto";
import hash from "hash.js";

export const sha256 = async (blob: Uint8Array): Promise<Uint8Array> => {
    const hashBuffer = await crypto.subtle.digest("SHA-256", blob);
    return new Uint8Array(hashBuffer);
};

export const sha256Sync = (blob: Uint8Array): Uint8Array => {
    const digest = hash.sha256().update(blob).digest();
    return new Uint8Array(digest);
};
