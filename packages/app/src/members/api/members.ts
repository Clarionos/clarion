import csvParse from "csv-parse/lib/sync";
import { MemberData, MemberSocialHandles } from "../interfaces";

const BASE_URL = "https://test.wax.api.atomicassets.io/atomicassets/v1";
const COLLECTION_NAME = "edenmembers1";
const SCHEMA_NAME = "edenmembers1";

export const getMember = async (
    edenAccount: string
): Promise<MemberData | undefined> => {
    // to lookup for a member template we need to read the whole edenAccount
    // collection and then filter itself (we don't have an easier way to lookup
    // from the `edenacc` field on the immutable data of the NFT)
    const members = await getCollection(edenAccount);
    return members.find((member) => member.edenAccount === edenAccount);
};

export const getMembers = async (
    page = 1,
    limit = 20,
    ids: string[] = [],
    sortField = "created",
    order = "asc"
): Promise<MemberData[]> => {
    let url = `${BASE_URL}/templates?collection_name=${COLLECTION_NAME}&schema_name=${SCHEMA_NAME}&page=${page}&limit=${limit}&order=${order}&sort=${sortField}`;

    url += "&lower_bound=66281"; // TODO: remove when resetting collection

    if (ids.length) {
        url += `&ids=${ids.join(",")}`;
    }

    const { data } = await executeAtomicAssetRequest(url);
    console.info("members data", data);
    return data.map(convertAtomicAssetToMember);
};

export const getCollection = async (
    edenAccount: string
): Promise<MemberData[]> => {
    const url = `${BASE_URL}/accounts/${edenAccount}/${COLLECTION_NAME}`;
    const {
        data: { templates },
    } = await executeAtomicAssetRequest(url);

    const templateIds: string[] = templates.map(
        (template: any) => template.template_id
    );
    if (templateIds.length === 0) {
        return [];
    }

    return getMembers(1, 9999, templateIds);
};

export const getCollectedBy = async (
    templateId: number,
    page = 1,
    limit = 20,
    sortField = "created",
    order = "asc"
): Promise<MemberData[]> => {
    const url = `${BASE_URL}/assets?collection_name=${COLLECTION_NAME}&schema_name=${SCHEMA_NAME}&template_id=${templateId}&page=${page}&limit=${limit}&order=${order}&sort=${sortField}`;
    const { data } = await executeAtomicAssetRequest(url);

    const edenAccs: string[] = data.map((item: any) => item.owner);

    // TODO: very expensive lookups here, we need to revisit
    const collectedMembers = edenAccs.map(getMember);
    const members = await Promise.all(collectedMembers);

    return members.filter((member) => member !== undefined) as MemberData[];
};

const convertAtomicAssetToMember = (data: any): MemberData => ({
    templateId: data.template_id,
    name: data.immutable_data.name,
    image: data.immutable_data.img,
    edenAccount: data.immutable_data.edenacc,
    bio: data.immutable_data.bio,
    inductionVideo: data.immutable_data.inductionvid || "",
    role: data.immutable_data.role,
    createdAt: new Date(parseInt(data.created_at_time)),
    socialHandles: parseMemberSocialHandles(data.immutable_data.social),
});

const executeAtomicAssetRequest = async (url: string): Promise<any> => {
    const response = await fetch(url);
    if (!response.ok) {
        console.error(response);
        throw new Error("response not ok");
    }

    const json = await response.json();
    if (!json.success || !json.data) {
        console.error("unsuccessfull response", json);
    }

    return json;
};

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
