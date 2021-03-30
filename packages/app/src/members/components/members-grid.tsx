import React from "react";
import { Link } from "react-router-dom";
import { SmallText } from "../../_global";
import { MemberData } from "../interfaces";

interface Props {
    members: MemberData[];
}

export const MemberSquare = ({ member }: { member: MemberData }) => (
    <div>
        <Link to={`/members/${member.edenAccount}`}>
            <img
                src={`https://ipfs.pink.gg/ipfs/${member.image}`}
                className="max-h-44 block rounded-md mx-auto"
            />
            <div className="text-center mt-4">
                <SmallText>{member.name}</SmallText>
            </div>
        </Link>
    </div>
);

export const MembersGrid = ({ members }: Props) => {
    return (
        <div className="grid grid-cols-3 gap-4 max-w-4xl mx-auto p-8">
            {members.map((member, index) => (
                <MemberSquare key={index} member={member} />
            ))}
        </div>
    );
};
