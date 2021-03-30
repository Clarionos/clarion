import React from "react";
import {
    FaAirbnb,
    FaBitcoin,
    FaTelegram,
    FaTwitter,
    FaVideo,
} from "react-icons/fa";
import { Link } from "react-router-dom";

import { Button, Heading, SmallText, SocialButton, Text } from "../../_global";
import { MemberData } from "../interfaces";

interface Props {
    member: MemberData;
}

export const MemberCard = ({ member }: Props) => {
    return (
        <div className="px-5 py-8 mx-auto flex justify-around">
            <div className="max-w-sm mr-4">
                <img
                    src={`https://ipfs.pink.gg/ipfs/${member.image}`}
                    className="object-contain rounded-md"
                />
                <div className="text-center mt-2">
                    <Text>
                        <Link to={`/members/${member.edenAccount}`}>
                            {member.name}
                        </Link>
                    </Text>
                    <SmallText>{member.createdAt.toLocaleString()}</SmallText>
                </div>
            </div>
            <div className="max-w-md bg-white rounded-lg p-8 flex flex-col w-full mt-10 md:mt-0 shadow-md">
                <Heading size={2} className="mb-4">
                    {member.name}
                </Heading>
                <MemberSocialLinks member={member} />
                <Text className="mt-4">{member.bio}</Text>
                <div className="mx-auto">
                    <Button
                        href={`https://ipfs.video/#/ipfs/${member.inductionVideo}`}
                        target="_blank"
                        className="mt-10 inline-flex"
                        icon={FaVideo}
                    >
                        Induction Ceremony
                    </Button>
                </div>
            </div>
        </div>
    );
};

const MemberSocialLinks = ({ member }: { member: MemberData }) => (
    <nav className="space-y-2 flex flex-col">
        <SocialButton
            handle={member.edenAccount}
            icon={FaBitcoin}
            color="black"
            href={`https://bloks.io/account/${member.edenAccount}`}
        />
        {member.socialHandles.telegram && (
            <SocialButton
                handle={member.socialHandles.telegram}
                icon={FaTelegram}
                color="indigo"
                href={`https://t.me/${member.socialHandles.telegram}`}
            />
        )}
        {member.socialHandles.twitter && (
            <SocialButton
                handle={member.socialHandles.twitter}
                icon={FaTwitter}
                color="blue"
                href={`https://twitter.com/${member.socialHandles.twitter}`}
            />
        )}
        {member.socialHandles.eosCommunity && (
            <SocialButton
                handle={member.socialHandles.eosCommunity}
                icon={FaAirbnb}
                color="red"
                href={`https://eoscommunity.org/u/${member.socialHandles.eosCommunity}`}
            />
        )}
    </nav>
);
