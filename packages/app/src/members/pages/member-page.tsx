import React, { useEffect, useState } from "react";
import { useParams } from "react-router";
import { Heading } from "../../_global";

import { getMember } from "../api";
import { MemberCard } from "../components/member-card";
import { MemberCollections } from "../components/member-collections";
import { MemberData } from "../interfaces";

interface PageParams {
    edenAccount: string;
}

export const MemberPage = () => {
    const { edenAccount } = useParams<PageParams>();
    const [member, setMember] = useState<MemberData | undefined>(undefined);

    useEffect(() => {
        const loadMember = async () => {
            const member = await getMember(edenAccount);
            console.info("got member", member);
            setMember(member);
        };
        loadMember();
    }, [edenAccount]);

    return (
        <div className="bg-gray-50 min-h-screen">
            <div className="px-8 pt-8">
                <Heading>EdenOS Member Profile</Heading>
                <hr />
            </div>
            {member ? (
                <>
                    <MemberCard member={member} />
                    <MemberCollections
                        edenAccount={edenAccount}
                        templateId={member.templateId}
                    />
                </>
            ) : (
                "loading member..."
            )}
        </div>
    );
};
