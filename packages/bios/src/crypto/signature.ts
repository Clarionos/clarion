import { BNInput, ec } from "elliptic";
import { KeyPair, KeyType } from "./interfaces";

const DEFAULT_EC = new ec("secp256k1");

export const sign = async (
    keyPair: KeyPair,
    digest: Uint8Array
): Promise<Uint8Array> => {
    console.info("signing digest", digest);

    return keyPair.type === KeyType.k1
        ? constructK1Signature(keyPair, digest)
        : constructSignature(keyPair, digest, { canonical: true });
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
    let recoveryParam;
    if (keyPair.type === KeyType.k1 || keyPair.type === KeyType.r1) {
        recoveryParam = ellipticSignature.recoveryParam + 27;
        if (ellipticSignature.recoveryParam <= 3) {
            recoveryParam += 4;
        }
    }
    // else if (keyType === KeyType.wa) { // todo: are wa cases useful for hardware wallets too?
    //     recoveryParam = ellipticSignature.recoveryParam;
    // }

    return new Uint8Array([recoveryParam].concat(r, s));
};

const isCanonical = (sigData: Uint8Array) =>
    !(sigData[1] & 0x80) &&
    !(sigData[1] === 0 && !(sigData[2] & 0x80)) &&
    !(sigData[33] & 0x80) &&
    !(sigData[33] === 0 && !(sigData[34] & 0x80));
