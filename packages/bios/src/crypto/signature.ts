import { BNInput, ec } from "elliptic";
import BN from "bn.js";

import { KeyPair, KeyType } from "./interfaces";
import { constructPublicKeyData } from "./keys";

export const sign = async (
    keyPair: KeyPair,
    digest: Uint8Array
): Promise<Uint8Array> => {
    console.info("signing digest", digest);

    return keyPair.type === KeyType.k1
        ? constructK1Signature(keyPair, digest)
        : constructSignature(keyPair, digest, { canonical: true });
};

export const recover = (
    keyType: KeyType,
    signature: Uint8Array,
    digest: Uint8Array,
    encoding: BufferEncoding = "utf8"
): Uint8Array => {
    console.info("recovering", signature, digest);

    const curve = new ec(keyType === KeyType.k1 ? "secp256k1" : "p256");

    const ellipticSignature = signatureBytesToElliptic(signature);
    const recoveredPublicKey = curve.recoverPubKey(
        digest,
        ellipticSignature,
        ellipticSignature.recoveryParam,
        encoding
    );

    const ellipticPubKey = curve.keyFromPublic(recoveredPublicKey);
    return constructPublicKeyData(ellipticPubKey.getPublic());
};

const constructK1Signature = (keyPair: KeyPair, data: BNInput): Uint8Array => {
    let signature;
    let tries = 0;
    do {
        signature = constructSignature(keyPair, data, {
            canonical: true,
            pers: [++tries],
        });
    } while (!isCanonical(signature));
    return signature;
};

const constructSignature = (
    keyPair: KeyPair,
    data: BNInput,
    options: ec.SignOptions
) => {
    const ellipticSignature = keyPair.ecKeyPair.sign(data, options);

    const r = ellipticSignature.r.toArray("be", 32);
    const s = ellipticSignature.s.toArray("be", 32);

    if (ellipticSignature.recoveryParam === null) {
        throw new Error("missing recovery param on elliptic signature");
    }

    let recoveryParam = ellipticSignature.recoveryParam + 27;
    if (ellipticSignature.recoveryParam <= 3) {
        recoveryParam += 4;
    }

    return new Uint8Array([recoveryParam].concat(r, s));
};

const isCanonical = (sigData: Uint8Array) =>
    !(sigData[1] & 0x80) &&
    !(sigData[1] === 0 && !(sigData[2] & 0x80)) &&
    !(sigData[33] & 0x80) &&
    !(sigData[33] === 0 && !(sigData[34] & 0x80));

const signatureBytesToElliptic = (sigData: Uint8Array) => {
    const lengthOfR = 32;
    const lengthOfS = 32;
    const r = new BN(sigData.slice(1, lengthOfR + 1));
    const s = new BN(sigData.slice(lengthOfR + 1, lengthOfR + lengthOfS + 1));

    let ellipticRecoveryBitField;
    ellipticRecoveryBitField = sigData[0] - 27;
    if (ellipticRecoveryBitField > 3) {
        ellipticRecoveryBitField -= 4;
    }

    const recoveryParam = ellipticRecoveryBitField & 3;
    return { r, s, recoveryParam };
};
