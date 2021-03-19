import { ec as EC } from "elliptic";

export enum KeyType {
    k1 = 0,
    r1 = 1,
}

export const generateKey = (
    type: KeyType = KeyType.k1,
    ecOptions?: EC.GenKeyPairOptions
) => {
    console.info("generating keytype ", KeyType[type]);
    let ec;
    if (type === KeyType.k1) {
        ec = new EC("secp256k1") as any;
    } else {
        ec = new EC("p256") as any;
    }

    // todo: replace with a proper wallet management
    const keyPair = ec.genKeyPair(ecOptions);
    const publicKeyData = constructPublicKeyData(keyPair.getPublic());
    const privateKeyData = constructPrivateKeyData(keyPair.getPrivate());
    console.info(
        "!!!!!!!!!!!!! generated keypair",
        "priv",
        privateKeyData,
        "public",
        publicKeyData
    );

    return {
        privateKeyData,
        publicKeyData,
    };
};

const constructPublicKeyData = (publicKey): Uint8Array => {
    const x = publicKey.getX().toArray("be", 32);
    const y = publicKey.getY().toArray("be", 32);
    return new Uint8Array([y[31] & 1 ? 3 : 2].concat(x));
};

const constructPrivateKeyData = (privateKey): Uint8Array => {
    return privateKey.toArrayLike(Buffer, "be", 32);
};
