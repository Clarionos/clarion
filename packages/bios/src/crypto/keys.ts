import { ec } from "elliptic";
import { KeyPair, KeyType } from "./interfaces";

const keyPairs = new Map<string, KeyPair>();

export const generateKey = async (
    type: KeyType = KeyType.k1,
    ecOptions?: ec.GenKeyPairOptions
): Promise<KeyPair> => {
    const keyPair = constructKeyPair(type, ecOptions);
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

const constructKeyPair = (
    type: KeyType = KeyType.k1,
    ecOptions?: ec.GenKeyPairOptions
): KeyPair => {
    console.info("generating keytype ", KeyType[type]);
    const elliptic = new ec(type === KeyType.k1 ? "secp256k1" : "p256");

    // todo: replace with a proper wallet management
    const ecKeyPair = elliptic.genKeyPair(ecOptions);
    const publicKey = constructPublicKeyData(ecKeyPair.getPublic());
    const privateKey = constructPrivateKeyData(ecKeyPair.getPrivate());

    return { type, publicKey, privateKey, ecKeyPair };
};

const constructPublicKeyData = (publicKey): Uint8Array => {
    const x = publicKey.getX().toArray("be", 32);
    const y = publicKey.getY().toArray("be", 32);
    return new Uint8Array([y[31] & 1 ? 3 : 2].concat(x));
};

const constructPrivateKeyData = (privateKey): Uint8Array => {
    return privateKey.toArrayLike(Buffer, "be", 32);
};
