import csvParse from "csv-parse/lib/sync";
import { MemberData, MemberSocialHandles } from "../interfaces";

const BASE_URL = "https://test.wax.api.atomicassets.io/atomicassets/v1";
const COLLECTION_NAME = "edenmembers1";
const SCHEMA_NAME = "edenmembers1";

export const getMembers = async (
    page = 1,
    limit = 20,
    sortField = "created",
    order = "asc"
) => {
    const url = `${BASE_URL}/templates?collection_name=${COLLECTION_NAME}&schema_name=${SCHEMA_NAME}&page=${page}&limit=${limit}&order=${order}&sort=${sortField}`;

    const response = await fetch(url);
    if (!response.ok) {
        console.error(response);
        throw new Error("response not ok");
    }

    const json = await response.json();
    if (!json.success) {
        console.error("unsuccessfull response", json);
    }

    const atomicAssets: any[] = json.data;
    return atomicAssets.map(convertAtomicAssetToMember);
};

const convertAtomicAssetToMember = (data: any): MemberData => ({
    name: data.immutable_data.name,
    image: data.immutable_data.img,
    edenAccount: data.immutable_data.edenacc,
    bio: data.immutable_data.bio,
    inductionVideo: data.immutable_data.inductionvid || "",
    role: data.immutable_data.role,
    createdAt: new Date(parseInt(data.created_at_time)),
    socialHandles: parseMemberSocialHandles(data.immutable_data.social),
});

const parseMemberSocialHandles = (csvSocial: string): MemberSocialHandles => {
    try {
        const social: MemberSocialHandles = {};

        const socialHandles: string[] = csvParse(csvSocial)[0];
        socialHandles.forEach((value) =>
            fillSocialHandlesFromAtomicAssetValue(social, value)
        );

        return social;
    } catch (e) {
        console.error("fail to parse social handles", e);
        return {};
    }
};

const HANDLE_SEPARATOR = ":";
const ATOMIC_ASSETS_SOCIAL_MAP: { [key: string]: keyof MemberSocialHandles } = {
    twitter: "twitter",
    telegram: "telegram",
    eoscommunity: "eosCommunity",
    blog: "blogUrl",
};

const fillSocialHandlesFromAtomicAssetValue = (
    social: MemberSocialHandles,
    value: string
) => {
    const [type, handle] = value.split(HANDLE_SEPARATOR);
    const socialHandleKey = ATOMIC_ASSETS_SOCIAL_MAP[type];
    if (socialHandleKey) {
        social[socialHandleKey] = handle;
    }
};
