import { ec, curve } from "elliptic";
import BN from "bn.js";

import { KeyPair, KeyType } from "./interfaces";

const keyPairs = new Map<string, KeyPair>();

export const generateKey = async (
    type: KeyType = KeyType.k1,
    ecOptions?: ec.GenKeyPairOptions
): Promise<KeyPair> => {
    const keyPair = await constructKeyPair(type, ecOptions);
    console.info("!!!!!!!!!!!!! generated keypair", keyPair);

    // todo: save the keys to a proper storage... in fact we should not have to generate
    // the keys with this function, this is merely for tests and we should use hardware keys
    keyPairs.set(keyPair.publicKey.toString(), keyPair);

    return keyPair;
};

export const getKeyPair = (publicKey: Uint8Array): KeyPair => {
    const keyPair = keyPairs.get(publicKey.toString());
    if (!keyPair) {
        throw new Error("fail to locate required public key");
    }
    return keyPair;
};

export const publicKeyPairFromData = (type: KeyType, publicKey: Uint8Array) => {
    const curve = new ec(type === KeyType.k1 ? "secp256k1" : "p256");
    const ecKeyPair = curve.keyFromPublic(publicKey);
    return { type, publicKey, ecKeyPair };
};

export const deriveEcdhSharedSecret = (
    localKeyPair: KeyPair,
    remoteKeyPair: KeyPair
): Uint8Array => {
    const sharedSecret = localKeyPair.ecKeyPair.derive(
        remoteKeyPair.ecKeyPair.getPublic()
    );
    return new Uint8Array(sharedSecret.toArrayLike(Buffer, "be", 32));
};

const constructKeyPair = async (
    type: KeyType = KeyType.k1,
    ecOptions?: ec.GenKeyPairOptions
): Promise<KeyPair> => {
    console.info("generating keytype ", KeyType[type]);
    const curve = new ec(type === KeyType.k1 ? "secp256k1" : "p256");

    // todo: replace with a proper wallet management
    const ecKeyPair = curve.genKeyPair(ecOptions);
    const publicKey = constructPublicKeyData(ecKeyPair.getPublic());
    const privateKey = constructPrivateKeyData(ecKeyPair.getPrivate());

    return { type, publicKey, privateKey, ecKeyPair };
};

export const constructPublicKeyData = (
    publicKey: curve.base.BasePoint
): Uint8Array => {
    const x = publicKey.getX().toArray("be", 32);
    const y = publicKey.getY().toArray("be", 32);
    return new Uint8Array([y[31] & 1 ? 3 : 2].concat(x));
};

const constructPrivateKeyData = (privateKey: BN): Uint8Array => {
    return privateKey.toArrayLike(Buffer, "be", 32);
};
