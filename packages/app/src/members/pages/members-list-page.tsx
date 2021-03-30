import React, { useEffect, useState } from "react";
import { Heading } from "../../_global";

import { getMembers } from "../api";
import { MembersGrid } from "../components/members-grid";
import { MemberData } from "../interfaces";

export const MembersListPage = () => {
    const [members, setMembers] = useState<MemberData[]>([]);

    useEffect(() => {
        const loadMembers = async () => {
            const members = await getMembers();
            console.info(members);
            setMembers(members);
        };
        loadMembers();
    }, []);

    return (
        <div className="bg-gray-50 min-h-screen">
            <div className="px-8 pt-8">
                <Heading>EdenOS Members List</Heading>
                <hr />
            </div>
            <div className="px-5 py-5 mx-auto flex justify-around">
                <div className="bg-white rounded-lg p-8 w-full mt-0 md:mt-0 shadow-md">
                    <MembersGrid members={members} />
                </div>
            </div>
        </div>
    );
};
