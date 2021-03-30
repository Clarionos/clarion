import React, { useEffect, useState } from "react";

import { Button } from "../../_global";
import { getCollection, getCollectedBy } from "../api";
import { MemberData } from "../interfaces";
import { MembersGrid } from "./members-grid";

interface Props {
    edenAccount: string;
    templateId: number;
}

export const MemberCollections = ({ edenAccount, templateId }: Props) => {
    const [tab, setTab] = useState<"collection" | "collectedBy">("collection");
    const [isLoading, setLoading] = useState(false);
    const [members, setMembers] = useState<MemberData[] | undefined>(undefined);

    useEffect(() => {
        const loadMember = async () => {
            const collectionRequest =
                tab === "collection"
                    ? getCollection(edenAccount)
                    : getCollectedBy(templateId);
            const members = await collectionRequest;
            setMembers(members);
            setLoading(false);
        };
        setLoading(true);
        loadMember();
    }, [edenAccount, templateId, tab]);

    return (
        <div className="px-5 py-5 mx-auto flex justify-around">
            <div className="bg-white rounded-lg p-8 w-full mt-0 md:mt-0 shadow-md">
                <Button
                    color="gray"
                    outline={tab !== "collection"}
                    disabled={tab === "collection"}
                    onClick={() => setTab("collection")}
                >
                    Collection
                </Button>
                <Button
                    color="gray"
                    outline={tab !== "collectedBy"}
                    disabled={tab === "collectedBy"}
                    onClick={() => setTab("collectedBy")}
                    className="ml-4"
                >
                    Collected By
                </Button>
                <hr className="m-2" />
                {isLoading ? (
                    "loading..."
                ) : (
                    <MembersGrid members={members || []} />
                )}
            </div>
        </div>
    );
};
