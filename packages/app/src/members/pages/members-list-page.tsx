import React, { useEffect, useState } from "react";
import {
    FaAirbnb,
    FaBitcoin,
    FaTelegram,
    FaTwitter,
    FaVideo,
} from "react-icons/fa";

import { Button, Heading, SmallText, Text } from "../../_global";
import { getMembers } from "../api";
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
            {members.map((member, index) => {
                return (
                    <div
                        className="px-5 py-8 mx-auto flex justify-around"
                        key={index}
                    >
                        <div className="max-w-sm mr-4">
                            <img
                                src={`https://ipfs.pink.gg/ipfs/${member.image}`}
                                className="object-contain rounded-md"
                            />
                            <div className="text-center mt-2">
                                <Text>{member.name}</Text>
                                <SmallText>
                                    {member.createdAt.toLocaleString()}
                                </SmallText>
                            </div>
                        </div>
                        <div className="max-w-md bg-white rounded-lg p-8 flex flex-col w-full mt-10 md:mt-0 shadow-md">
                            <Heading size={2} className="mb-4">
                                {member.name}
                            </Heading>
                            <nav className="space-y-2 flex flex-col">
                                <a
                                    href={`https://bloks.io/account/${member.edenAccount}`}
                                    target="_blank"
                                    className="text-gray-700 hover:underline items-center flex space-x-2"
                                >
                                    <FaBitcoin className="text-gray-700 w-6 h-6 inline-flex" />
                                    <span>{member.edenAccount}</span>
                                </a>
                                {member.socialHandles.telegram && (
                                    <a
                                        href="#"
                                        className="text-blue-400 hover:underline items-center flex space-x-2"
                                    >
                                        <FaTelegram className="text-blue-400 w-6 h-6 inline-flex" />
                                        <span>
                                            {member.socialHandles.telegram}
                                        </span>
                                    </a>
                                )}
                                {member.socialHandles.twitter && (
                                    <a
                                        href={`https://twitter.com/${member.socialHandles.twitter}`}
                                        target="_blank"
                                        className="text-blue-800 hover:underline items-center flex space-x-2"
                                    >
                                        <FaTwitter className="text-blue-800 w-6 h-6 inline-flex" />
                                        <span>
                                            {member.socialHandles.twitter}
                                        </span>
                                    </a>
                                )}
                                {member.socialHandles.eosCommunity && (
                                    <a
                                        href={`https://eoscommunity.org/u/${member.socialHandles.eosCommunity}`}
                                        target="_blank"
                                        className="text-red-500 hover:underline items-center flex space-x-2"
                                    >
                                        <FaAirbnb className="text-red-500 w-6 h-6 inline-flex" />
                                        <span>
                                            {member.socialHandles.eosCommunity}
                                        </span>
                                    </a>
                                )}
                            </nav>
                            <Text className="mt-4">{member.bio}</Text>

                            {member.inductionVideo && (
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
                            )}
                        </div>
                    </div>
                );
            })}
        </div>
    );
};
